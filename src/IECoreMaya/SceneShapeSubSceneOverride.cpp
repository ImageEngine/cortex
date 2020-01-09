//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "IECoreMaya/SceneShapeSubSceneOverride.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/MeshNormalsOp.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/SampledSceneInterface.h"
#include "IECoreScene/TypeIds.h"

#include "IECore/LRUCache.h"
#include "IECore/MessageHandler.h"
#include "IECore/NullObject.h"
#include "IECore/Object.h"

#include "maya/MDrawRegistry.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnInstancer.h"
#include "maya/MFnMatrixData.h"
#include "maya/MFnSingleIndexedComponent.h"
#include "maya/MFnTransform.h"
#include "maya/MHWGeometry.h"
#include "maya/MGlobal.h"
#include "maya/MHWGeometryUtilities.h"
#include "maya/MItDependencyGraph.h"
#include "maya/MItDependencyNodes.h"
#include "maya/MItSelectionList.h"
#include "maya/MObjectArray.h"
#include "maya/MPlugArray.h"
#include "maya/MPxComponentConverter.h"
#include "maya/MSelectionContext.h"
#include "maya/MSelectionList.h"
#include "maya/MShaderManager.h"
#include "maya/MUserData.h"
#include "maya/M3dView.h"

#include "boost/lexical_cast.hpp"

#include "tbb/parallel_reduce.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/task_group.h"

#include <numeric>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreMaya;
using namespace MHWRender;

namespace
{

enum class RenderStyle { BoundingBox, Wireframe, Solid, Textured, Last };
std::vector<RenderStyle> g_supportedStyles = { RenderStyle::BoundingBox, RenderStyle::Wireframe, RenderStyle::Solid, RenderStyle::Textured };

template<typename T>
T parallelMaxElement( const std::vector<T> &data )
{
	tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
	tbb::auto_partitioner partitioner;

	T maxValue = tbb::parallel_reduce(
		tbb::blocked_range<size_t>( 0, data.size() ), 0, [&data]( const tbb::blocked_range<size_t> &r, T init )->T
		{
			for( size_t i = r.begin(); i != r.end(); ++i )
			{
				init = std::max( init, data[i] );
			}
			return init;
		}, []( T x, T y )->T
		{
			return std::max( x, y );
		}, partitioner, taskGroupContext
	);

	return maxValue;
}

// We define expandulation as the process of taking a primitive variable and
// converting it to a facevarying representation on a triangulated mesh. This
// abstraction is here so that, for a given topology, we compute the triangulated
// version only once. Doing so improves performance if the topology of a mesh
// doesn't change across frames and even more so if multiple meshes in the scene
// have the same topology.
class Expandulator
{

public:

	Expandulator( const MeshPrimitive *meshPrimitive )
		: m_numVerticesPerFace( meshPrimitive->verticesPerFace() ), m_indices( new IntVectorData() ), m_oldToTriangulatedIndexMapping( new IntVectorData() ), m_wireframeIndices( new IntVectorData() )
	{
		// Store original indices
		m_originalIndices = meshPrimitive->vertexIds();
		const std::vector<int> &originalIndicesReadable = m_originalIndices->readable();

		// Store triangulated indices
		IECoreScene::MeshPrimitivePtr triangulated = IECore::runTimeCast<IECoreScene::MeshPrimitive>( meshPrimitive->copy() );
		IntVectorDataPtr sequentialIndices( new IntVectorData() );
		std::vector<int> &sequentialIndicesWritable =  sequentialIndices->writable();

		sequentialIndicesWritable.resize( triangulated->variableSize(PrimitiveVariable::Interpolation::FaceVarying) );
		tbb::parallel_for(
			tbb::blocked_range<size_t>( 0, sequentialIndicesWritable.size() ), [&sequentialIndicesWritable]( const tbb::blocked_range<size_t> &r )
			{
				for( size_t i = r.begin(); i != r.end(); ++i )
				{
					sequentialIndicesWritable[i] = i;
				}
			}
		);
		// We store these indices on the mesh that we'll triangulate in order to get
		// a mapping from new to old indices for facevarying data.
		triangulated->variables["_indexMap"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, sequentialIndices );

		triangulated = MeshAlgo::triangulate( triangulated.get() );

		m_triangulatedIndices = triangulated->vertexIds();
		const std::vector<int> &triangulatedIndicesReadable = m_triangulatedIndices->readable();

		std::vector<int> &oldToTriangulatedIndexMappingWritable = m_oldToTriangulatedIndexMapping->writable();
		oldToTriangulatedIndexMappingWritable.resize( parallelMaxElement( originalIndicesReadable ) + 1 );

		std::vector<int> &indicesWritable = m_indices->writable();
		indicesWritable.resize( triangulatedIndicesReadable.size() );

		tbb::parallel_for(
			tbb::blocked_range<size_t>( 0, indicesWritable.size() ), [&indicesWritable, &triangulatedIndicesReadable, &oldToTriangulatedIndexMappingWritable]( const tbb::blocked_range<size_t> &r )
			{
				for( size_t i = r.begin(); i != r.end(); ++i )
				{
					indicesWritable[i] = i;
					oldToTriangulatedIndexMappingWritable[ triangulatedIndicesReadable[i] ] = i;
				}
			}
		);

		m_mapToOldFacevarying = triangulated->expandedVariableData<IntVectorData>( "_indexMap" );

	}

	~Expandulator()
	{
	}

	ConstV3fVectorDataPtr expandulateVertex( PrimitiveVariable::IndexedView<Imath::V3f> in )
	{
		const std::vector<int> &triangulatedIndices = m_triangulatedIndices->readable();

		V3fVectorDataPtr result( new V3fVectorData() );
		std::vector<Imath::V3f> &resultWritable = result->writable();
		resultWritable.resize( triangulatedIndices.size() );

		tbb::parallel_for(
			tbb::blocked_range<int>( 0, triangulatedIndices.size() ),
			[&in, &resultWritable, &triangulatedIndices](
				const tbb::blocked_range<int> &range
			)
			{
				for( int i = range.begin(); i != range.end(); ++i )
				{
						resultWritable[i] = in[triangulatedIndices[i]];
				}
			}
		);

		return result;
	}

	ConstV2fVectorDataPtr expandulateFacevarying( PrimitiveVariable::IndexedView<Imath::V2f> in )
	{
		const std::vector<int> &triangulatedIndices = m_triangulatedIndices->readable();

		V2fVectorDataPtr result( new V2fVectorData() );
		std::vector<Imath::V2f> &resultWritable = result->writable();
		resultWritable.resize( triangulatedIndices.size() );

		const std::vector<int> &mapToOldFacevaryingReadable = m_mapToOldFacevarying->readable();

		tbb::parallel_for(
			tbb::blocked_range<int>( 0, triangulatedIndices.size() ),
			[&in, &resultWritable, &mapToOldFacevaryingReadable](
				const tbb::blocked_range<int> &range
			)
			{
				for( int i = range.begin(); i != range.end(); ++i )
				{
					resultWritable[i] = in[mapToOldFacevaryingReadable[i]];
				}
			}
		);

		return result;
	}

	IECore::ConstIntVectorDataPtr indices()
	{
		return m_indices;
	}

	IECore::ConstIntVectorDataPtr triangulatedIndices()
	{
		return m_triangulatedIndices;
	}

	IECore::ConstIntVectorDataPtr wireframeIndices()
	{
		std::vector<int> &wireframeIndicesWritable = m_wireframeIndices->writable();

		// We need to compute the wireframe only once per topology hash
		if( wireframeIndicesWritable.size() )
		{
			return m_wireframeIndices;
		}

		const std::vector<int> &numVerticesPerFaceReadable = m_numVerticesPerFace->readable();

		// Compute partial sum to make algorithm parallelisable.
		// [4, 4, 4, 4] -> [4, 8, 12, 16]
		std::vector<int> partialSum( numVerticesPerFaceReadable.size() );
		std::partial_sum(numVerticesPerFaceReadable.begin(), numVerticesPerFaceReadable.end(), partialSum.begin(), std::plus<int>());

		int totalNumEdgeIndices = partialSum[partialSum.size()-1] * 2;
		wireframeIndicesWritable.resize( totalNumEdgeIndices );

		const std::vector<int> &originalIndicesReadable = m_originalIndices->readable();
		const std::vector<int> &oldToTriangulatedIndexMapping = m_oldToTriangulatedIndexMapping->readable();

		// Construct list of indices that can be used to render wireframes
		tbb::parallel_for(
			tbb::blocked_range<size_t>( 0, numVerticesPerFaceReadable.size() ), [&partialSum, &numVerticesPerFaceReadable, &originalIndicesReadable, &wireframeIndicesWritable, oldToTriangulatedIndexMapping]( const tbb::blocked_range<size_t> &r )
			{
				for( size_t i = r.begin(); i != r.end(); ++i )
				{
					int offset = i == 0 ? 0 : partialSum[i-1];
					int numVerts = numVerticesPerFaceReadable[i];

					for( int k = 0; k < numVerts; ++k )
					{
						int oldIndexA = originalIndicesReadable[offset + k];
						int oldIndexB = originalIndicesReadable[offset + (k+1)%numVerts];

						int insertion = (offset + k) * 2;

						wireframeIndicesWritable[insertion] = oldToTriangulatedIndexMapping[ oldIndexA ];
						wireframeIndicesWritable[insertion + 1] = oldToTriangulatedIndexMapping[ oldIndexB ];
					}
				}
			}
		);

		return m_wireframeIndices;
	}

private :

	size_t m_dataSize;

	IECore::ConstIntVectorDataPtr m_numVerticesPerFace;

	IECore::IntVectorDataPtr m_indices;
	IECore::ConstIntVectorDataPtr m_originalIndices;
	IECore::ConstIntVectorDataPtr m_triangulatedIndices;

	IECore::IntVectorDataPtr m_oldToTriangulatedIndexMapping;
	IECore::IntVectorDataPtr m_mapToOldFacevarying;

	IECore::IntVectorDataPtr m_wireframeIndices;
};

using ExpandulatorPtr = std::shared_ptr<Expandulator>;


bool objectCanBeRendered( IECore::ConstObjectPtr object )
{
	switch( static_cast<IECoreScene::TypeId>( object->typeId() ) )
	{
		case IECoreScene::TypeId::MeshPrimitiveTypeId :
		case IECoreScene::TypeId::PointsPrimitiveTypeId :
		case IECoreScene::TypeId::CurvesPrimitiveTypeId :
		{
			return true;
		}
		default :
			return false;
	}
}

struct BufferCacheGetterKey
{
	BufferCacheGetterKey( const MVertexBufferDescriptor descriptor, IECore::ConstDataPtr data, bool index )
		: descriptor( descriptor ), data( data )
	{
		data->hash( hash );
		hash.append( descriptor.semantic() );
	}

	operator const IECore::MurmurHash & () const
	{
		return hash;
	}

	// the semantic of the descriptor also encodes whether this represents indices
	// or vertices. An invalid semantic implies indices.
	const MVertexBufferDescriptor descriptor;
	IECore::ConstDataPtr data;
	IECore::MurmurHash hash;
};

BufferPtr bufferGetter( const BufferCacheGetterKey &key, size_t &cost )
{

	if( key.descriptor.semantic() )
	{
		VertexBufferPtr buffer( new MVertexBuffer( key.descriptor ) );

		switch( key.descriptor.semantic() )
		{
			case MGeometry::kPosition :
			case MGeometry::kNormal :
			{
				IECore::ConstV3fVectorDataPtr v3fDataPtr = IECore::runTimeCast<const V3fVectorData>( key.data );
				cost = v3fDataPtr->Object::memoryUsage();
				const std::vector<Imath::V3f> &dataReadable = v3fDataPtr->readable();
				size_t numEntries = dataReadable.size();
				void* positionData = buffer->acquire( numEntries, true );
				if( positionData && buffer )
				{
					memcpy( positionData, dataReadable.data(), sizeof( float ) * 3 * numEntries );
					buffer->commit( positionData );
				}
				else
				{
					msg( Msg::Error, "SceneShapeSubSceneOverride::bufferGetter", "Memory acquisition for position/normal buffer failed." );
				}
				break;
			}

			case MGeometry::kTexture :
			{
				IECore::ConstV2fVectorDataPtr v2fDataPtr = IECore::runTimeCast<const V2fVectorData>( key.data );
				cost = v2fDataPtr->Object::memoryUsage();
				const std::vector<Imath::V2f> &dataReadable = v2fDataPtr->readable();
				size_t numEntries = dataReadable.size();
				void *uvData = buffer->acquire( numEntries, true );
				if( uvData && buffer )
				{
					memcpy( uvData, dataReadable.data(), sizeof( float ) * 2 * numEntries );
					buffer->commit( uvData );
				}
				else
				{
					msg( Msg::Error, "SceneShapeSubSceneOverride::bufferGetter", "Memory acquisition for uv buffer failed." );
				}
				break;
			}
			default :
				break;
		}

		return BufferPtr( new Buffer( buffer ) );
	}
	else // build indices
	{
		IndexBufferPtr buffer( new MIndexBuffer( MGeometry::kUnsignedInt32 ) );

		IECore::ConstIntVectorDataPtr indexDataPtr = IECore::runTimeCast<const IntVectorData>( key.data );
		cost = indexDataPtr->Object::memoryUsage();
		const std::vector<int> &indexReadable = indexDataPtr->readable();

		void *indexData = buffer->acquire( indexReadable.size(), true );
		if( indexData && buffer )
		{
			memcpy( indexData, indexReadable.data(), sizeof( int ) * indexReadable.size() );
			buffer->commit( indexData );
		}
		else
		{
			msg( Msg::Error, "SceneShapeSubSceneOverride::bufferGetter", "Memory acquisition for index buffer failed." );
		}

		return BufferPtr( new Buffer( buffer ) );
	}
}

struct GeometryDataCacheGetterKey
{
	GeometryDataCacheGetterKey( const IECore::ConstObjectPtr object, const MVertexBufferDescriptorList &descriptorList, const Imath::Box3d &bounds )
		:	object( object ), descriptorList( descriptorList ), bounds( bounds )
	{
		if( object )
		{
			object->hash( m_hash );
		}
		else
		{
			m_hash.append( bounds );
		}

	}

	operator const IECore::MurmurHash & () const
	{
		return m_hash;
	}

	const IECore::ConstObjectPtr object;
	const MVertexBufferDescriptorList &descriptorList;
	const Imath::Box3d &bounds;

	IECore::MurmurHash m_hash;
};

void fillBoundData( const Imath::Box3d &bounds, GeometryDataPtr geometryData )
{
	IECore::V3fVectorDataPtr p( new V3fVectorData() );
	std::vector<Imath::V3f> &pWritable = p->writable();
	pWritable.reserve( 8 );

	pWritable.emplace_back( bounds.min.x, bounds.min.y, bounds.min.z ); // Indices:
	pWritable.emplace_back( bounds.max.x, bounds.min.y, bounds.min.z );
	pWritable.emplace_back( bounds.max.x, bounds.max.y, bounds.min.z ); //								  7------6
	pWritable.emplace_back( bounds.min.x, bounds.max.y, bounds.min.z ); //								 /|     /|
	pWritable.emplace_back( bounds.min.x, bounds.min.y, bounds.max.z ); //								3------2 |
	pWritable.emplace_back( bounds.max.x, bounds.min.y, bounds.max.z ); //								| 4----|-5
	pWritable.emplace_back( bounds.max.x, bounds.max.y, bounds.max.z ); //								|/     |/
	pWritable.emplace_back( bounds.min.x, bounds.max.y, bounds.max.z ); //								0------1

	geometryData->positionData = p;

	geometryData->wireframeIndexData.reset( new IECore::IntVectorData( { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 7, 3, 6, 2, 4, 0, 5, 1 } ) );
}

void fillPointsData( ConstObjectPtr object, GeometryDataPtr geometryData )
{
	IECoreScene::ConstPointsPrimitivePtr pointsPrimitive = IECore::runTimeCast<const IECoreScene::PointsPrimitive>( object );
	assert( pointsPrimitive );

	IECore::ConstV3fVectorDataPtr p = pointsPrimitive->variableData<IECore::V3fVectorData>( "P" );
	geometryData->positionData = p;
}

void fillCurvesData( ConstObjectPtr object, GeometryDataPtr geometryData )
{
	// \todo: currently a curve is built from linear segments. Needs proper interpolation at some point.
	IECoreScene::ConstCurvesPrimitivePtr curvesPrimitive = IECore::runTimeCast<const IECoreScene::CurvesPrimitive>( object );
	if( !curvesPrimitive )
	{
		return;
	}

	geometryData->positionData = curvesPrimitive->variableData<IECore::V3fVectorData>( "P" );

	// provide indices
	const IntVectorData *verticesPerCurve = curvesPrimitive->verticesPerCurve();
	const std::vector<int> &verticesPerCurveReadable = verticesPerCurve->readable();

	int numElements = 0;
	for( unsigned int i = 0; i < curvesPrimitive->numCurves(); ++i )
	{
		numElements += curvesPrimitive->numSegments( i ) * 2;
	}

	IECore::IntVectorDataPtr indexData( new IECore::IntVectorData() );
	std::vector<int> &indexDataWritable = indexData->writable();
	indexDataWritable.reserve( numElements );

	int positionBufferOffset = 0;
	int indexBufferOffset = 0;
	for( unsigned int i = 0; i < curvesPrimitive->numCurves(); ++i )
	{
		int numSegments = curvesPrimitive->numSegments( i );
		int endPointDuplication = ( verticesPerCurveReadable[i] - ( numSegments + 1 ) ) / 2;
		int segmentCurrent = positionBufferOffset + endPointDuplication;
		for( int j = 0; j < numSegments; ++j )
		{
			indexDataWritable.push_back( segmentCurrent );
			indexDataWritable.push_back( segmentCurrent + 1 );

			indexBufferOffset++;
			segmentCurrent++;
		}
		positionBufferOffset += verticesPerCurveReadable[i];
	}

	geometryData->indexData = indexData;
	geometryData->wireframeIndexData = indexData;
}

struct ExpandulatorCacheGetterKey
{
	ExpandulatorCacheGetterKey( ConstMeshPrimitivePtr meshPrimitive )
		: meshPrimitive( meshPrimitive )
	{
		meshPrimitive->topologyHash( hash );
	}

	operator const IECore::MurmurHash & () const
	{
		return hash;
	}

	ConstMeshPrimitivePtr meshPrimitive;
	IECore::MurmurHash hash;
};

ExpandulatorPtr expandulatorGetter( const ExpandulatorCacheGetterKey &key, size_t cost )
{
	cost = 1;
	return std::make_shared<Expandulator>( key.meshPrimitive.get() );
}

// Limit is given in bytes.
size_t memoryLimit()
{
	// Environment variable sets limit in megabytes - defaulting to 500mb
	const char *m = getenv( "IECORE_MAYA_VP2_MEMORY" );
	size_t mi = m ? boost::lexical_cast<size_t>( m ) : 500;

	return mi * 1024 * 1024;
}

using ExpandulatorCache = IECore::LRUCache<IECore::MurmurHash, ExpandulatorPtr, IECore::LRUCachePolicy::Parallel, ExpandulatorCacheGetterKey>;
ExpandulatorCache g_expandulatorCache( expandulatorGetter, 100 ); // \todo: how big? Currently 100 unique topologies

void fillMeshData( ConstObjectPtr object, const MVertexBufferDescriptorList &descriptorList, GeometryDataPtr geometryData )
{
	IECoreScene::ConstMeshPrimitivePtr meshPrimitive = IECore::runTimeCast<const IECoreScene::MeshPrimitive>( object );
	if( !meshPrimitive )
	{
		return;
	}

	ExpandulatorPtr expandulator = g_expandulatorCache.get( ExpandulatorCacheGetterKey( meshPrimitive ) );

	MeshPrimitivePtr copy = meshPrimitive->copy();
	IECoreScene::MeshNormalsOpPtr normalOp = new IECoreScene::MeshNormalsOp();
	normalOp->inputParameter()->setValue( copy );
	normalOp->copyParameter()->setTypedValue( false );
	normalOp->operate();

	tbb::task_group g;
	g.run( [&geometryData, &expandulator, &meshPrimitive]
				 {
					 geometryData->positionData = expandulator->expandulateVertex( *(meshPrimitive->variableIndexedView<IECore::V3fVectorData>( "P" ) ) );
				 }
	);

	g.run( [&geometryData, &expandulator, &copy]
				 {
					 geometryData->normalData = expandulator->expandulateVertex( *(copy->variableIndexedView<IECore::V3fVectorData>( "N") ) );
				 }
	);

	auto uvView = meshPrimitive->variableIndexedView<IECore::V2fVectorData>( "uv" );
	if( uvView )
	{
		g.run( [&geometryData, &expandulator, &uvView]
					 {
						 geometryData->uvData = expandulator->expandulateFacevarying( *uvView);
					 }
		);
	}

	g.wait();

	geometryData->indexData = expandulator->indices();
	geometryData->wireframeIndexData = expandulator->wireframeIndices();
}

// Compute memory cost for given GeometryData in bytes
size_t computeGeometryDataCost( const GeometryData *geometryData )
{
	size_t result = 0;

	if( geometryData->positionData )
	{
		result += geometryData->positionData->Object::memoryUsage();
	}

	if( geometryData->normalData )
	{
		result += geometryData->normalData->Object::memoryUsage();
	}

	if( geometryData->uvData )
	{
		result += geometryData->uvData->Object::memoryUsage();
	}

	if( geometryData->indexData )
	{
		result += geometryData->indexData->Object::memoryUsage();
	}

	if( geometryData->wireframeIndexData )
	{
		result += geometryData->wireframeIndexData->Object::memoryUsage();
	}

	return result;
}

GeometryDataPtr geometryGetter( const GeometryDataCacheGetterKey &key, size_t &cost )
{
	GeometryDataPtr geometryData( new GeometryData() );

	if( !key.object )
	{
		fillBoundData( key.bounds, geometryData );
		cost = computeGeometryDataCost( geometryData.get() );
		return geometryData;
	}

	switch( static_cast<IECoreScene::TypeId>( key.object->typeId() ) )
	{
	case IECoreScene::TypeId::MeshPrimitiveTypeId :
		fillMeshData( key.object, key.descriptorList, geometryData ); break;
	case IECoreScene::TypeId::PointsPrimitiveTypeId :
		fillPointsData( key.object, geometryData ); break;
	case IECoreScene::TypeId::CurvesPrimitiveTypeId :
		fillCurvesData( key.object, geometryData ); break;
	default :
		// we should never get here as only the above types are supported, see `objectCanBeRendered`.
		break;
	}

	cost = computeGeometryDataCost( geometryData.get() );

	return geometryData;
}

boost::signals2::signal<void (const BufferPtr )> bufferEvictedSignal;

struct BufferCleanup
{
	void operator() ( const IECore::MurmurHash &key, const BufferPtr &buffer )
	{
		bufferEvictedSignal( buffer );
	}
};

// Cache for geometric data
// \todo remove this eventually. We're currently splitting the available memory
// between these two caches. Giving it all to the BufferCache would be better.
using GeometryDataCache = IECore::LRUCache<IECore::MurmurHash, GeometryDataPtr, IECore::LRUCachePolicy::Parallel, GeometryDataCacheGetterKey>;
GeometryDataCache g_geometryDataCache( geometryGetter, memoryLimit() * 0.5 );

using BufferCache = IECore::LRUCache<IECore::MurmurHash, BufferPtr, IECore::LRUCachePolicy::Parallel, BufferCacheGetterKey>;
BufferCache g_bufferCache( bufferGetter, BufferCleanup(), memoryLimit() * 0.5 );

MRenderItem *acquireRenderItem( MSubSceneContainer &container, ConstObjectPtr object, const MString &name, RenderStyle style, bool &isNew )
{
	MRenderItem *renderItem = container.find( name );
	if( renderItem )
	{
		isNew = false; // this comes from the container and was assigned some geo before.
		return renderItem;
	}

	// If the container does not have an appropriate MRenderItem yet, we'll construct an empty one.
	isNew = true;

	switch( style )
	{
	case RenderStyle::BoundingBox :
	{
		renderItem = MRenderItem::Create( name, MRenderItem::DecorationItem, MGeometry::kLines);
		renderItem->setDrawMode( MGeometry::kAll );
		renderItem->castsShadows( false );
		renderItem->receivesShadows( false );
		renderItem->setExcludedFromPostEffects( true );
		renderItem->depthPriority( MRenderItem::sActiveWireDepthPriority );
		break;
	}
	case RenderStyle::Wireframe :
	{
		switch( static_cast<IECoreScene::TypeId>( object->typeId() ) )
		{
		case IECoreScene::TypeId::PointsPrimitiveTypeId :
			renderItem = MRenderItem::Create( name, MRenderItem::DecorationItem, MGeometry::kPoints );
			break;
		case IECoreScene::TypeId::CurvesPrimitiveTypeId :
		case IECoreScene::TypeId::MeshPrimitiveTypeId :
			renderItem = MRenderItem::Create( name, MRenderItem::DecorationItem, MGeometry::kLines );
			break;
		default :
			return nullptr;
		}
		renderItem->setDrawMode( MGeometry::kAll );
		renderItem->castsShadows( false );
		renderItem->receivesShadows( false );
		renderItem->setExcludedFromPostEffects( true );
		renderItem->depthPriority( MRenderItem::sActiveWireDepthPriority );
		break;
	}
	case RenderStyle::Solid :
	case RenderStyle::Textured :
	{
		switch( static_cast<IECoreScene::TypeId>( object->typeId() ) )
		{
			case IECoreScene::TypeId::PointsPrimitiveTypeId :
			{
				renderItem = MRenderItem::Create( name, MRenderItem::MaterialSceneItem, MGeometry::kPoints );
				break;
			}
			case IECoreScene::TypeId::MeshPrimitiveTypeId :
			{
				renderItem = MRenderItem::Create( name, MRenderItem::MaterialSceneItem, MGeometry::kTriangles );
				break;
			}
			case IECoreScene::TypeId::CurvesPrimitiveTypeId :
			{
				renderItem = MRenderItem::Create( name, MRenderItem::MaterialSceneItem, MGeometry::kLines );
				break;
			}
		default :
			break;

		}

		renderItem->setDrawMode( ( style == RenderStyle::Solid ) ? MGeometry::kShaded : MGeometry::kTextured );
		renderItem->castsShadows( true );
		renderItem->receivesShadows( true );
		renderItem->setExcludedFromPostEffects( false );
	}
	default :
		break;
	}

	container.add( renderItem );

	return renderItem;
}

// For the given node return the out plug on the surface shader that is assigned
MPlug getShaderOutPlug( const MObject &sceneShapeNode )
{
	MPlug result = MPlug();

	MObjectArray sets, components;
	MFnDagNode node( sceneShapeNode );
	if( !node.getConnectedSetsAndMembers( 0, sets, components, true ) )
	{
		return result;
	}

	for (unsigned int i = 0; i < sets.length(); i++)
	{
		MStatus status;
		MFnDependencyNode fnSet( sets[i], &status );

		if( !status )
		{
			return result;
		}

		MPlug shaderPlug = fnSet.findPlug( "surfaceShader", false );
		if( shaderPlug.isNull() )
		{
			return result;
		}

		MPlugArray connectedPlugs;
		shaderPlug.connectedTo( connectedPlugs, true, false );

		if( connectedPlugs.length() >= 1 )
		{
			return connectedPlugs[0];
		}
	}

	return result;
}

bool isPathSelected( const MSelectionList &selectionList, const MDagPath &path )
{
	MStatus status = MStatus::kSuccess;
	MDagPath pathCopy = path;

	while( status )
	{
		if( selectionList.hasItem( pathCopy ) )
		{
			return true;
		}

		status = pathCopy.pop();
	}

	return false;
}

struct SelectionStateShaders
{
	SelectionStateShaders( MHWRender::MShaderInstance *unselected, MHWRender::MShaderInstance *selected )
		: unselected( unselected ), selected( selected )
	{
	}

	MHWRender::MShaderInstance *unselected;
	MHWRender::MShaderInstance *selected;
};

struct TextureStateShaders
{
	TextureStateShaders( MHWRender::MShaderInstance *untextured, MHWRender::MShaderInstance *textured )
		: untextured( untextured ), textured( textured )
	{
	}

	MHWRender::MShaderInstance *untextured;
	MHWRender::MShaderInstance *textured;
};

SelectionStateShaders getWireShaders()
{
	static SelectionStateShaders shaders( nullptr, nullptr );

	if( !shaders.unselected || !shaders.selected )
	{
		MRenderer* renderer = MRenderer::theRenderer();
		if( !renderer )
		{
			return shaders;
		}

		const MShaderManager* shaderManager = renderer->getShaderManager();
		if( !shaderManager )
		{
			return shaders;
		}

		shaders.unselected = shaderManager->getStockShader( MShaderManager::k3dSolidShader );
		shaders.selected = shaders.unselected->clone();

		MDoubleArray a, b;
		MGlobal::executeCommand( "colorIndex -q 19", a );
		MGlobal::executeCommand( "colorIndex -q 5", b );

		const float highlightedWireColor[] = {(float)a[0], (float)a[1], (float)a[2], 1.0f};
		const float unhighlightedWireColor[] = {(float)b[0], (float)b[1], (float)b[2], 1.0f};

		shaders.unselected->setParameter("solidColor", unhighlightedWireColor );
		shaders.selected->setParameter("solidColor", highlightedWireColor );
	}

	return shaders;
}

SelectionStateShaders getComponentWireShaders()
{
	static SelectionStateShaders shaders( nullptr, nullptr );

	if( !shaders.unselected || !shaders.selected )
	{
		MRenderer* renderer = MRenderer::theRenderer();
		if( !renderer )
		{
			return shaders;
		}

		const MShaderManager* shaderManager = renderer->getShaderManager();
		if( !shaderManager )
		{
			return shaders;
		}

		MDoubleArray a, b;
		MGlobal::executeCommand( "colorIndex -q 21", a );
		MGlobal::executeCommand( "colorIndex -q 18", b );

		const float highlightedWireColor[] = {(float)a[0], (float)a[1], (float)a[2], 1.0f};
		const float unhighlightedWireColor[] = {(float)b[0], (float)b[1], (float)b[2], 1.0f};

		shaders.unselected = shaderManager->getStockShader( MShaderManager::k3dSolidShader );
		shaders.selected = shaders.unselected->clone();

		shaders.unselected->setParameter("solidColor", unhighlightedWireColor );
		shaders.selected->setParameter("solidColor", highlightedWireColor );
	}

	return shaders;
}

TextureStateShaders getAssignedSurfaceShaders( const MObject &object )
{
	TextureStateShaders shaders( nullptr, nullptr );

	if( shaders.untextured && shaders.textured )
	{
		return shaders;
	}

	MRenderer* renderer = MRenderer::theRenderer();
	if( !renderer )
	{
		return shaders;
	}

	const MShaderManager* shaderManager = renderer->getShaderManager();
	if( !shaderManager )
	{
		return shaders;
	}

	MStatus s;
	MFnDagNode node( object, &s );
	if( !s )
	{
		return shaders;
	}

	MDagPathArray instances;
	node.getAllPaths( instances );

	MPlug shaderOutPlug = getShaderOutPlug( object );

	if( !shaderOutPlug.isNull() )
	{
		#if MAYA_API_VERSION >= 201650
			shaders.untextured = shaderManager->getShaderFromNode( shaderOutPlug.node(), instances[0], 0, 0, 0, 0, /* nonTextured = */ true );
			shaders.textured = shaderManager->getShaderFromNode( shaderOutPlug.node(), instances[0], 0, 0, 0, 0, /* nonTextured = */ false );
		#else
			shaders.untextured = shaderManager->getShaderFromNode( shaderOutPlug.node(), instances[0], 0, 0, 0, 0 );
			shaders.textured = shaderManager->getShaderFromNode( shaderOutPlug.node(), instances[0], 0, 0, 0, 0 );
		#endif
	}
	else
	{
		shaders.untextured = shaderManager->getStockShader( MShaderManager::MStockShader::k3dDefaultMaterialShader );
		shaders.textured = shaders.untextured;
	}

	return shaders;
}

bool componentsSelectable( const MDagPath &path )
{
	MHWRender::DisplayStatus displayStatus = MHWRender::MGeometryUtilities::displayStatus( path );
	bool selectable = displayStatus == MHWRender::kHilite;
	return selectable;
}

bool sceneIsAnimated( const SceneInterface *sceneInterface )
{
	const SampledSceneInterface *scene = IECore::runTimeCast< const SampledSceneInterface >( sceneInterface );
	return ( !scene || scene->numBoundSamples() > 1 );
}

} // namespace

struct IECoreMaya::SceneShapeSubSceneOverride::AllShaders
{
	AllShaders(const MObject &object)
	: componentWireShaders(nullptr, nullptr ),
	  wireShaders(nullptr, nullptr ),
	  textureStateShaders(nullptr, nullptr)
	{
		componentWireShaders = getComponentWireShaders();
		wireShaders = getWireShaders();
		textureStateShaders = getAssignedSurfaceShaders( object );
	}

	MShaderInstance* getShader(RenderStyle style, bool componentMode, bool isSelected ) const
	{
		switch( style )
		{
			case RenderStyle::BoundingBox :
			case RenderStyle::Wireframe :
			{
				if( componentMode )
				{
					return isSelected ? componentWireShaders.selected : componentWireShaders.unselected;
				}
				else
				{
					return isSelected ? wireShaders.selected : wireShaders.unselected;
				}
			}
			case RenderStyle::Solid :
			case RenderStyle::Textured :
			{
				return style == RenderStyle::Solid ? textureStateShaders.untextured : textureStateShaders.textured;
			}
			default :
				return nullptr;
		}
	}
	SelectionStateShaders componentWireShaders;
	SelectionStateShaders wireShaders;
	TextureStateShaders textureStateShaders;
};

class IECoreMaya::SceneShapeSubSceneOverride::RenderItemUserData : public MUserData
{
public :

	RenderItemUserData( int componentIndex )
		: MUserData( false ),  // do not delete when render item is deleted, we manage life time
		  componentIndex( componentIndex )
	{
	}

	virtual ~RenderItemUserData()
	{
	}

	int componentIndex;

};

// As we render a render item per component, all we need to know here is the
// component index per render item.
class SceneShapeSubSceneOverride::ComponentConverter : public MPxComponentConverter
{
	public :

		static MPxComponentConverter *creator()
		{
			return new ComponentConverter();
		}

		ComponentConverter()
			: MHWRender::MPxComponentConverter()
		{
		}

		~ComponentConverter() override
		{
		}

		void initialize( const MRenderItem &renderItem ) override
		{
			RenderItemUserData *userData = dynamic_cast<RenderItemUserData*>(renderItem.customData());
			m_idx = userData->componentIndex;

			m_object = m_component.create( MFn::kMeshPolygonComponent );
		}

		void addIntersection( MIntersection &intersection ) override
		{
			m_component.addElement( m_idx );
		}

		MObject component()
		{
			return m_object;
		}

		MSelectionMask 	selectionMask () const override
		{
			return MSelectionMask::kSelectMeshFaces;
		}

	private :

		MFnSingleIndexedComponent m_component;
		MObject m_object;
		int m_idx;
};


MString& SceneShapeSubSceneOverride::drawDbClassification()
{
	static MString classification{ "drawdb/subscene/ieSceneShape" };
	return classification;
}

MString& SceneShapeSubSceneOverride::drawDbId()
{
	static MString id{ "SceneShapeSubSceneOverride" };
	return id;
}

MPxSubSceneOverride* SceneShapeSubSceneOverride::Creator( const MObject& obj )
{
	return new SceneShapeSubSceneOverride( obj );
}

SceneShapeSubSceneOverride::~SceneShapeSubSceneOverride()
{
}

bool SceneShapeSubSceneOverride::requiresUpdate(const MSubSceneContainer& container, const MFrameContext& frameContext) const
{
	bool allInvisible = true;
	MFnDagNode dagNode( m_sceneShape->thisMObject() );
	MDagPathArray dagPaths;
	dagNode.getAllPaths( dagPaths );
	for( auto &path : dagPaths )
	{
		if( path.isVisible() )
		{
			allInvisible = false;
			break;
		}
	}

	// check if there are viewport filters that hide the shape
	M3dView view;
	MString panelName;
	frameContext.renderingDestination( panelName );
	M3dView::getM3dViewFromModelPanel( panelName, view );

	bool allInvisibleByFilter = false;
	if( view.viewIsFiltered() )
	{
		allInvisibleByFilter = true;

		MObject component;
		MSelectionList viewSelectedSet;
		view.filteredObjectList( viewSelectedSet );

		for( auto &path : dagPaths )
		{
			if( viewSelectedSet.hasItemPartly( path, component ) )
			{
				allInvisibleByFilter = false;
				break;
			}
		}
	}

	bool anyRenderItemEnabled = false;
	MSubSceneContainer::ConstIterator *it = container.getConstIterator();
	const MRenderItem *renderItem = nullptr;
	while( (renderItem = it->next()) != nullptr )
	{
		if( renderItem->isEnabled() )
		{
			anyRenderItemEnabled = true;
			break;
		}
	}
	it->destroy();

	if( anyRenderItemEnabled )
	{
		if( allInvisible || allInvisibleByFilter )
		{
			// if some RenderItems are enabled and all instances are invisible
			// then we do require an update.
			return true;
		}
	}
	else
	{
		// if all RenderItems are disabled and all instances are invisible
		// then we can opt out of updating for a significant performance gain.
		// if all RenderItems are disabled but some instances are visible
		// then we do require an update.
		return !( allInvisible || allInvisibleByFilter );
	}

	// TIME UPDATED?
	if( m_sceneShape->time() != m_time && sceneIsAnimated( m_sceneInterface.get() ) )
	{
		return true;
	}

	// SCENE UPDATED?
	if( m_sceneInterface != m_sceneShape->getSceneInterface() )
	{
		return true;
	}

	// DRAW GEOMETRY SETTINGS UPDATED?
	StyleMask tmpMask;
	checkDisplayOverrides( frameContext.getDisplayStyle(), tmpMask );

	if( tmpMask != m_styleMask )
	{
		return true;
	}

	// ROOT BOUNDING BOX SETTING UPDATED?
	bool tmpDrawRootBound;
	MPlug drawRootBoundsPlug( m_sceneShape->thisMObject(), SceneShape::aDrawRootBound );
	drawRootBoundsPlug.getValue( tmpDrawRootBound );
	if( tmpDrawRootBound != m_drawRootBounds )
	{
		return true;
	}

	// TAGS FILTER UPDATED?
	MString tmpTagsFilter;
	MPlug drawTagsFilterPlug( m_sceneShape->thisMObject(), SceneShape::aDrawTagsFilter );
	drawTagsFilterPlug.getValue( tmpTagsFilter );
	if( tmpTagsFilter.asChar() != m_drawTagsFilter )
	{
		return true;
	}

	// \todo: This should scale better. It's depending on both the selection and the instances at the moment.
	// COMPONENT SELECTION UPDATED?
	IndexMap selectedComponents;
	selectedComponentIndices( selectedComponents );
	if( selectedComponents != m_selectedComponents )
	{
		return true;
	}

	// SHADER UPDATED?
	if( getShaderOutPlug( m_sceneShape->thisMObject() ) != m_shaderOutPlug )
	{
		return true;
	}

	Instances currentInstances;
	collectInstances( currentInstances );

	if( currentInstances.size() != m_instances.size() )
	{
		return true; // instance was added or removed
	}

	for( const auto &instance : currentInstances )
	{
		// \todo: doesn't properly account for duplicates in the containers, but is probably good enough for now.
		if( std::find( m_instances.begin(), m_instances.end(), instance ) == m_instances.end() )
		{
			return true;
		}
	}

	return false;
}

void SceneShapeSubSceneOverride::update( MSubSceneContainer& container, const MFrameContext& frameContext )
{
	// To make sure we keep within our memory bounds, we need to clean up buffers
	// that have been marked for deletion.
	for( BufferPtr &buffer : m_markedForDeletion )
	{
		Buffer *b = buffer.get();
		auto it = m_bufferToRenderItems.find( b );
		if( it == m_bufferToRenderItems.end() )
		{
			continue;
		}

		const RenderItemNameSet &names = it->second;
		for( const InternedString &itemName : names )
		{
			MString name( itemName.c_str() );
			if( container.find( name ) )
			{
				container.remove( name );
			}
	 	}
	 	m_bufferToRenderItems.erase( b );
	}

	m_markedForDeletion.clear(); // releases memory

	// We'll set internal state based on settings in maya and then perform updates
	// by disabling all MRenderItems and reenabling those needed by walking the
	// tree. MRenderItems can be found in the container via their name. Our naming
	// convention is to use the primitive's hash and append the style of the MRenderItem.

	m_time = m_sceneShape->time();

	IECoreScene::ConstSceneInterfacePtr tmpSceneInterface = m_sceneShape->getSceneInterface();
	if( m_sceneInterface != tmpSceneInterface )
	{
		// All data in the container is invalid now and we can safely clear it
		container.clear();
		m_sceneInterface = tmpSceneInterface;
		m_sceneShape->buildComponentIndexMap();
	}

	// STYLE
	// Used to skip rendering independently of style mask
	MPlug drawGeometryPlug( m_sceneShape->thisMObject(), SceneShape::aDrawGeometry );
	drawGeometryPlug.getValue( m_geometryVisible );

	checkDisplayOverrides( frameContext.getDisplayStyle(), m_styleMask );

	// DRAWING ROOTS
	MPlug drawRootBoundsPlug( m_sceneShape->thisMObject(), SceneShape::aDrawRootBound );
	drawRootBoundsPlug.getValue( m_drawRootBounds );

	// DRAWING CHILD BOUNDS
	MPlug drawAllBoundsPlug( m_sceneShape->thisMObject(), SceneShape::aDrawChildBounds );
	drawAllBoundsPlug.getValue( m_drawChildBounds );

	// TAGS
	MString tmpTagsFilter;
	MPlug drawTagsFilterPlug( m_sceneShape->thisMObject(), SceneShape::aDrawTagsFilter );
	drawTagsFilterPlug.getValue( tmpTagsFilter );
	if( tmpTagsFilter.asChar() != m_drawTagsFilter )
	{
		m_drawTagsFilter = tmpTagsFilter.asChar();
		m_sceneShape->buildComponentIndexMap();
	}

	// COMPONENT SELECTION
	m_selectedComponents.clear();
	selectedComponentIndices( m_selectedComponents );

	// INSTANCES - sort out transformation and selection
	m_instances.clear();
	collectInstances( m_instances );

	// SHADING
	MPlug currentShaderOutPlug = getShaderOutPlug( m_sceneShape->thisMObject() );
	if( currentShaderOutPlug.name() != m_shaderOutPlug.name() )
	{
		m_shaderOutPlug = currentShaderOutPlug;
		m_allShaders = std::make_shared<AllShaders>( m_sceneShape->thisMObject() );
	}

	// Disable all MRenderItems before traversing the scene \todo: performance
	// improvement -> we don't have to do some of this if all that changed is a
	// shader for example.
	MSubSceneContainer::Iterator *it = container.getIterator();
	MRenderItem *renderItem = nullptr;
	while( (renderItem = it->next()) != nullptr )
	{
		if( renderItem->isEnabled() )
		{
			renderItem->enable( false );
		}
	}
	it->destroy();

	bool allInvisible = true;
	for( auto &instance : m_instances )
	{
		if( instance.visible )
		{
			allInvisible = false;
			break;
		}
	}

	// check if there are viewport filters that hide the shape
	M3dView view;
	MString panelName;
	MSelectionList viewSelectedSet;
	MObject component;
	frameContext.renderingDestination( panelName );
	view.getM3dViewFromModelPanel( panelName, view );
	view.filteredObjectList( viewSelectedSet );

	bool allInvisibleByFilter = false;
	if ( view.viewIsFiltered() )
	{
		allInvisibleByFilter = true;
		for( auto &instance : m_instances )
		{
			if ( viewSelectedSet.hasItemPartly( instance.path, component ) )
			{
				allInvisibleByFilter = false;
				break;
			}
		}
	}

	if( allInvisible || allInvisibleByFilter )
	{
		return; // no need to do any scene traversing
	}

	// Create and enable MRenderItems while traversing the scene hierarchy
	RenderItemMap renderItems;
	visitSceneLocations( m_sceneInterface.get(), renderItems, container, Imath::M44d(), /* isRoot = */ true );

	for( MRenderItem *renderItem : m_renderItemsToEnable )
	{
		renderItem->enable( true );
	}
	m_renderItemsToEnable.clear();
}

#if MAYA_API_VERSION > 201650
bool SceneShapeSubSceneOverride::getInstancedSelectionPath( const MRenderItem &renderItem, const MIntersection &intersection, MDagPath &dagPath ) const
{
	auto it = m_renderItemNameToDagPath.find( std::string( renderItem.name().asChar() ) );
	if( it != m_renderItemNameToDagPath.end() )
	{
		dagPath.set( it->second );
		return true;
	}

	return false;
}
#endif

void SceneShapeSubSceneOverride::updateSelectionGranularity( const MDagPath &path, MSelectionContext &selectionContext )
{
	MDagPath parent( path );
	parent.pop();

	if( componentsSelectable( parent ) )
	{
		selectionContext.setSelectionLevel( MHWRender::MSelectionContext::kComponent );
	}
	else
	{
		selectionContext.setSelectionLevel( MHWRender::MSelectionContext::kObject );
	}
}

DrawAPI SceneShapeSubSceneOverride::supportedDrawAPIs() const
{
	return kAllDevices;
}

SceneShapeSubSceneOverride::SceneShapeSubSceneOverride( const MObject& obj )
	: MPxSubSceneOverride( obj ), m_drawTagsFilter( "" ), m_time( -1 ), m_drawRootBounds( false ), m_drawChildBounds( false ), m_shaderOutPlug(), m_instancedRendering( false /* instancedRendering switch */ ), m_geometryVisible( false )
{
	MStatus status;
	MFnDependencyNode node( obj, &status );

	if( status )
	{
		m_sceneShape = dynamic_cast<SceneShape*>( node.userNode() );
	}

	m_allShaders = std::make_shared<AllShaders>( m_sceneShape->thisMObject() );

	m_evictionConnection = bufferEvictedSignal.connect( boost::bind( &SceneShapeSubSceneOverride::bufferEvictedCallback, this, ::_1 ) );
}

void SceneShapeSubSceneOverride::visitSceneLocations( const SceneInterface *sceneInterface, RenderItemMap &renderItems, MSubSceneContainer &container, const Imath::M44d &matrix, bool isRoot )
{
	if( !sceneInterface )
	{
		return;
	}

	Imath::M44d accumulatedMatrix = sceneInterface->readTransformAsMatrix( m_time ) * matrix;
	MMatrix mayaMatrix = IECore::convert<MMatrix, Imath::M44d>( accumulatedMatrix );

	bool needsTraversal = true;
	if( !m_geometryVisible && !m_drawChildBounds )
	{
		needsTraversal = false;
	}

	// Dispatch to all children.
	if( needsTraversal )
	{
		SceneInterface::NameList childNames;
		sceneInterface->childNames( childNames );

		for( const auto &childName : childNames )
		{
			visitSceneLocations( sceneInterface->child( childName ).get(), renderItems, container, accumulatedMatrix, false );
		}
	}

	// Now handle current location.

	std::string name;
	IECoreScene::SceneInterface::Path path;
	sceneInterface->path( path );
	IECoreScene::SceneInterface::pathToString( path, name );

	if( isRoot && m_drawRootBounds )
	{
		std::string rootItemName = name + "_root_" + std::to_string( (int)RenderStyle::BoundingBox );
		if( sceneIsAnimated( sceneInterface ) )
		{
			rootItemName += "_" + std::to_string( m_time );
		}

		Imath::Box3d boundingBox = sceneInterface->readBound( m_time );
		const MBoundingBox mayaBoundingBox = IECore::convert<MBoundingBox>( boundingBox );

		int count = 0;
		for( const auto &instance : m_instances )
		{
			std::string instanceName = rootItemName + "_" + std::to_string( count++ );

			bool isNew;
			MString itemName( instanceName.c_str() );
			MRenderItem *renderItem = acquireRenderItem( container, IECore::NullObject::defaultNullObject(), itemName, RenderStyle::BoundingBox, isNew );

			MShaderInstance *shader = m_allShaders->getShader( RenderStyle::BoundingBox, instance.componentMode, instance.selected );
			renderItem->setShader( shader );

			if( isNew )
			{
				GeometryDataPtr geometryData = g_geometryDataCache.get( GeometryDataCacheGetterKey( nullptr, renderItem->requiredVertexBuffers(), boundingBox ) );
				setBuffersForRenderItem( geometryData, renderItem, true, mayaBoundingBox );
				m_renderItemNameToDagPath[renderItem->name().asChar()] = instance.path;
			}
			else
			{
				auto result = m_renderItemsToEnable.insert( renderItem );
				if( result.second ) // we hadn't removed the instance transforms on this renderItem yet
				{
					removeAllInstances( *renderItem );
				}
			}

			MMatrix instanceMatrix = IECore::convert<MMatrix, Imath::M44d>( accumulatedMatrix * instance.transformation );
			addInstanceTransform( *renderItem, instanceMatrix );
		}
	}

	if( !sceneInterface->hasObject() )
	{
		return;
	}

	// respect tags
	if( !m_drawTagsFilter.empty() && !sceneInterface->hasTag( m_drawTagsFilter ) )
	{
		return;
	}

	// respect visibility attribute
	if( sceneInterface->hasAttribute( "scene:visible" ) )
	{
		ConstBoolDataPtr vis = runTimeCast<const BoolData>( sceneInterface->readAttribute( "scene:visible", m_time ) );
		if( vis && !vis->readable() )
		{
			return;
		}
	}

	IECore::ConstObjectPtr object = sceneInterface->readObject( m_time );
	if( !objectCanBeRendered( object ) )
	{
		return;
	}

	ConstPrimitivePtr primitive = IECore::runTimeCast<const Primitive>( object );
	if( !primitive )
	{
		return;
	}

	// We're going to render this object - compute its bounds only once and reuse them.
	Imath::Box3d boundingBox = sceneInterface->readBound( m_time );
	const MBoundingBox mayaBoundingBox = IECore::convert<MBoundingBox>( boundingBox );

	int componentIndex = m_sceneShape->selectionIndex( name );

	// Hash primitive only once
	IECore::MurmurHash primitiveHash;
	primitive->hash( primitiveHash );

	// Adding RenderItems as needed
	// ----------------------------
	for( RenderStyle style : g_supportedStyles )
	{
		// Global switch to hide all geometry - continuing early to avoid drawing selected wireframes.
		// \todo: This is a little messy as it's kind of included in the mask, but we can't rely on the mask for selected instances.
		if( !m_geometryVisible && style != RenderStyle::BoundingBox )
		{
			continue;
		}

		// Skipping unneeded MRenderItems based on display modes etc.
		// NOTE: wireframe visibility is determined mostly by per-instance data.
		if( style !=RenderStyle::Wireframe )
		{
			int maskIndex = (int)style;

			if( style == RenderStyle::Textured )
			{
				maskIndex = (int)RenderStyle::Solid;
			}

			if( !m_styleMask.test( maskIndex ) )
			{
				continue;
			}
		}

		MString itemName;
		IECore::MurmurHash styleHash = primitiveHash; // copy
		styleHash.append( (int)style );
		itemName = MString( styleHash.toString().c_str() );

		for( const auto &instance : m_instances )
		{
			if( !instance.visible )
			{
				continue;
			}

			if( style == RenderStyle::Wireframe ) // wireframe visibility is mostly driven by instance data and needs to be handled here.
			{
				if( !m_styleMask.test( (int)RenderStyle::Wireframe ) && !instance.selected && !instance.componentMode )
				{
					continue;
				}
			}

			bool isNew;
			MRenderItem *renderItem = acquireRenderItem( container, object, itemName, style, isNew );

			// Before setting geometry, a shader has to be assigned so that the data requirements are clear.
			std::string pathKey = instance.path.fullPathName().asChar();
			bool componentSelected = m_selectedComponents[pathKey].count( componentIndex ) > 0;

			MShaderInstance *shader = m_allShaders->getShader( style, instance.componentMode, instance.componentMode ? componentSelected : instance.selected );
			renderItem->setShader( shader );

			// set the geometry on the render item if it's a new one.
			if( isNew )
			{
				GeometryDataPtr geometryData;
				if( style == RenderStyle::BoundingBox )
				{
					// passing a nullptr is how we currently signal that only a bounding box is required. \todo
					geometryData = g_geometryDataCache.get( GeometryDataCacheGetterKey( nullptr, renderItem->requiredVertexBuffers(), boundingBox ) );
				}
				else
				{
					ConstPrimitivePtr primitive = IECore::runTimeCast<const Primitive>( object );
					geometryData = g_geometryDataCache.get( GeometryDataCacheGetterKey( primitive, renderItem->requiredVertexBuffers(), boundingBox ) );
				}

				setBuffersForRenderItem( geometryData, renderItem, style == RenderStyle::Wireframe || style == RenderStyle::BoundingBox, mayaBoundingBox );

				RenderItemUserDataPtr userData = acquireUserData( componentIndex );
				renderItem->setCustomData( userData.get() );
				MDrawRegistry::registerComponentConverter( renderItem->name(), ComponentConverter::creator );

				m_renderItemNameToDagPath[renderItem->name().asChar()] = instance.path;
			}
			else
			{
				auto result = m_renderItemsToEnable.insert( renderItem );
				if( result.second ) // we hadn't removed the instance transforms on this renderItem yet
				{
					removeAllInstances( *renderItem );
				}
			}

			MMatrix instanceMatrix = IECore::convert<MMatrix, Imath::M44d>( accumulatedMatrix * instance.transformation );
			addInstanceTransform( *renderItem, instanceMatrix );

		}
	}
}

void SceneShapeSubSceneOverride::collectInstances( Instances &instances ) const
{
	MSelectionList selectionList;
	MGlobal::getActiveSelectionList( selectionList );

	MFnDagNode dagNode( m_sceneShape->thisMObject() );
	MDagPathArray dagPaths;
	dagNode.getAllPaths(dagPaths);
	int numInstances = dagPaths.length();

	instances.reserve( numInstances );
	for( int pathIndex = 0; pathIndex < numInstances; ++pathIndex )
	{
		MDagPath& path = dagPaths[pathIndex];
		Imath::M44d matrix = IECore::convert<Imath::M44d, MMatrix>( path.inclusiveMatrix() );
		bool pathSelected = isPathSelected( selectionList, path );
		bool componentMode = componentsSelectable( path );
		MFnDagNode nodeFn( path );
		bool visible = path.isVisible();

		instances.emplace_back( matrix, pathSelected, componentMode, path, visible );
	}
}

void SceneShapeSubSceneOverride::checkDisplayOverrides( unsigned int displayStyle, StyleMask &mask ) const
{
	bool boundsOverride = false;
	MPlug drawAllBoundsPlug( m_sceneShape->thisMObject(), SceneShape::aDrawChildBounds );
	drawAllBoundsPlug.getValue( boundsOverride );

	bool geometryOverride = false;
	MPlug drawGeometryPlug( m_sceneShape->thisMObject(), SceneShape::aDrawGeometry );
	drawGeometryPlug.getValue( geometryOverride );

	// Determine if we need to render bounding boxes
	mask.reset( (int)RenderStyle::BoundingBox );
	if( boundsOverride || ( displayStyle & MHWRender::MFrameContext::kBoundingBox ) > 0 )
	{
		mask.set( (int)RenderStyle::BoundingBox );
	}

	// Determine if we need to render wireframes
	mask.reset( (int)RenderStyle::Wireframe );
	if( geometryOverride && ( displayStyle & MHWRender::MFrameContext::kWireFrame ) > 0 )
	{
		mask.set( (int)RenderStyle::Wireframe );
	}

	// Determine if we need to render shaded geometry
	mask.reset( (int)RenderStyle::Solid );
	if( geometryOverride && ( displayStyle & ( MHWRender::MFrameContext::kGouraudShaded | MHWRender::MFrameContext::kTextured | MHWRender::MFrameContext::kFlatShaded ) ) > 0 )
	{
		mask.set( (int)RenderStyle::Solid );
	}
}

SceneShapeSubSceneOverride::RenderItemUserDataPtr SceneShapeSubSceneOverride::acquireUserData( int componentIndex )
{
	const auto &entry = m_userDataMap.find( componentIndex );
	if( entry != m_userDataMap.end() )
	{
		return entry->second;
	}

	RenderItemUserDataPtr data( new RenderItemUserData( componentIndex ) );
	m_userDataMap.emplace( componentIndex, data );
	return data;
}

void SceneShapeSubSceneOverride::selectedComponentIndices( SceneShapeSubSceneOverride::IndexMap &indexMap ) const
{
	MStatus s;

	MSelectionList selectionList;
	MGlobal::getActiveSelectionList( selectionList );
	MItSelectionList selectionIter( selectionList );

	MFnDagNode dagNode( m_sceneShape->thisMObject() );
	MDagPathArray dagPaths;
	dagNode.getAllPaths(dagPaths);

	// Initialize map with empty sets
	for( int i = 0; i < (int)dagPaths.length(); ++i )
	{
		std::string keyPath = dagPaths[i].fullPathName().asChar();
		indexMap[keyPath] = std::set<int>();
	}

	for( ; !selectionIter.isDone(); selectionIter.next() )
	{
		MDagPath selectedPath; // path to shape
		MObject comp;
		selectionIter.getDagPath( selectedPath, comp );

		if( comp.isNull() )
		{
			continue;
		}

		MFnSingleIndexedComponent compFn( comp, &s );
		if( !s )
		{
			continue;
		}

		MIntArray componentIndices;
		compFn.getElements( componentIndices );

		std::string key = selectedPath.fullPathName().asChar();
		for( unsigned int i = 0; i < componentIndices.length(); ++i )
		{
			indexMap[key].insert( componentIndices[i] );
		}
	}
}

void SceneShapeSubSceneOverride::setBuffersForRenderItem( GeometryDataPtr geometryData, MRenderItem *renderItem, bool useWireframeIndex, const MBoundingBox &mayaBoundingBox )
{
	// For the uv descriptor, Maya sometimes requires the name to be 'uvCoord'
	// and the semantic name to be 'mayauvcoordsemantic'. By respecting the
	// given descriptors below, we should always supply data that maya can work
	// with. The UV descriptor is the only reason why we need to scan the
	// descriptor list, really. We could probably omit doing that for positions
	// and normals.
	MVertexBufferArray vertexBufferArray;
	const MVertexBufferDescriptorList &descriptorList = renderItem->requiredVertexBuffers();
	for( int i = 0; i < descriptorList.length(); ++i )
	{
		MVertexBufferDescriptor descriptor;
		descriptorList.getDescriptor( i, descriptor );

		// Autodesk suggests this workaround to fix an issue with instanced
		// rendering of MRenderItems and selection issues. Will need to be removed
		// once we run this in a Maya version that has their internal patch applied.
		if( descriptor.offset() < 0 )
		{
			descriptor.setOffset( 0 );
		}
		// Workaround ends

		std::string bufferName;

		BufferPtr buffer;
		switch( descriptor.semantic() )
		{
			case MGeometry::kPosition :
				if( !geometryData->positionData )
				{
					continue;
				}
				bufferName = "positions";
				buffer = g_bufferCache.get( BufferCacheGetterKey( descriptor, geometryData->positionData, false ) );
				break;

			case MGeometry::kNormal :
				if( !geometryData->normalData )
				{
					continue;
				}

				bufferName = "normals";
				buffer = g_bufferCache.get( BufferCacheGetterKey( descriptor, geometryData->normalData, false ) );
				break;

			case MGeometry::kTexture :
				if( !geometryData->uvData ) 
				{
					continue;
				}

				bufferName = "uvs";
				buffer = g_bufferCache.get( BufferCacheGetterKey( descriptor, geometryData->uvData, false ) );
				break;

			default :
					continue;
		}

		vertexBufferArray.addBuffer( bufferName.c_str(), boost::get<VertexBufferPtr>( *buffer ).get() );
		auto it = m_bufferToRenderItems.find( buffer.get() );
		if( it == m_bufferToRenderItems.end() )
		{
			m_bufferToRenderItems.emplace( buffer.get(), RenderItemNameSet{ renderItem->name().asChar() } );
		}
		else
		{
			it->second.emplace( renderItem->name().asChar() );
		}
	}

	ConstIntVectorDataPtr indexDataPtr;
	if( useWireframeIndex )
	{
		indexDataPtr = geometryData->wireframeIndexData;
	}
	else
	{
		indexDataPtr = geometryData->indexData;
	}

	if( indexDataPtr )
	{
		BufferPtr buffer = g_bufferCache.get( BufferCacheGetterKey( MVertexBufferDescriptor(), indexDataPtr, true ) );

		auto it = m_bufferToRenderItems.find( buffer.get() );
		if( it == m_bufferToRenderItems.end() )
		{
			m_bufferToRenderItems.emplace( buffer.get(), RenderItemNameSet{ renderItem->name().asChar() } );
		}
		else
		{
			it->second.emplace( renderItem->name().asChar() );
		}
		setGeometryForRenderItem( *renderItem, vertexBufferArray, *( boost::get<IndexBufferPtr>( *buffer ) ), &mayaBoundingBox );
	}
}

void SceneShapeSubSceneOverride::bufferEvictedCallback( const BufferPtr buffer )
{
	m_markedForDeletion.push_back( buffer ); // hold on to the resource just a little longer
}
