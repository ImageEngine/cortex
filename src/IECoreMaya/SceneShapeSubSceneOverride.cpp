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

#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/SampledSceneInterface.h"
#include "IECoreScene/TypeIds.h"

#include "IECore/LRUCache.h"
#include "IECore/MessageHandler.h"
#include "IECore/NullObject.h"
#include "IECore/Object.h"
#include "IECore/SimpleTypedData.h"

#include "maya/MDrawRegistry.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnSingleIndexedComponent.h"
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

#include <memory>
#include <numeric>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreMaya;
using namespace MHWRender;

namespace
{

// Limit is given in bytes.
size_t memoryLimit()
{
	// Environment variable sets limit in megabytes - defaulting to 500mb
	const char *m = getenv( "IECORE_MAYA_VP2_MEMORY" );
	size_t mi = m ? boost::lexical_cast<size_t>( m ) : 500;

	return mi * 1024 * 1024;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Expandulator
// We define expandulation as the process of taking a primitive variable and
// converting it to a facevarying representation on a triangulated mesh. This
// abstraction is here so that, for a given topology, we compute the triangulated
// version only once. Doing so improves performance if the topology of a mesh
// doesn't change across frames and even more so if multiple meshes in the scene
// have the same topology.
////////////////////////////////////////////////////////////////////////////////////////////

class Expandulator
{

public:

	explicit Expandulator( const Primitive *primitive )
	{
		if( !primitive )
		{
			// null primitives represent a bounding box, which have constant topology
			fillBoundData();
		}
		else if( const auto *mesh = runTimeCast<const MeshPrimitive>( primitive ) )
		{
			fillMeshData( mesh );
		}
		else if( const auto *curves = runTimeCast<const CurvesPrimitive>( primitive ) )
		{
			fillCurvesData( curves );
		}
		else if( const auto *points = runTimeCast<const PointsPrimitive>( primitive ) )
		{
			fillPointsData( points );
		}
	}

	~Expandulator() = default;

	/// \todo: should this return a MVertexBuffer directly?
	ConstV3fVectorDataPtr expandulateVertex( const PrimitiveVariable::IndexedView<Imath::V3f> &in ) const
	{
		const std::vector<int> &triangulatedIndices = m_triangulatedIndices->readable();

		V3fVectorDataPtr result( new V3fVectorData() );
		std::vector<Imath::V3f> &resultWritable = result->writable();
		resultWritable.resize( triangulatedIndices.size() );

		tbb::parallel_for(
			tbb::blocked_range<int>( 0, triangulatedIndices.size() ),
			[&in, &resultWritable, &triangulatedIndices]( const tbb::blocked_range<int> &range )
			{
				for( int i = range.begin(); i != range.end(); ++i )
				{
					resultWritable[i] = in[triangulatedIndices[i]];
				}
			}
		);

		return result;
	}

	/// \todo: should this return a MVertexBuffer directly?
	ConstV2fVectorDataPtr expandulateFacevarying( const PrimitiveVariable::IndexedView<Imath::V2f> &in ) const
	{
		const std::vector<int> &triangulatedIndices = m_triangulatedIndices->readable();

		V2fVectorDataPtr result( new V2fVectorData() );
		std::vector<Imath::V2f> &resultWritable = result->writable();
		resultWritable.resize( triangulatedIndices.size() );

		const std::vector<int> &mapToOldFacevaryingReadable = m_mapToOldFacevarying->readable();

		tbb::parallel_for(
			tbb::blocked_range<int>( 0, triangulatedIndices.size() ),
			[&in, &resultWritable, &mapToOldFacevaryingReadable]( const tbb::blocked_range<int> &range )
			{
				for( int i = range.begin(); i != range.end(); ++i )
				{
					resultWritable[i] = in[mapToOldFacevaryingReadable[i]];
				}
			}
		);

		return result;
	}

	IndexBufferPtr &indices()
	{
		return m_indexBuffer;
	}

	IndexBufferPtr &wireframeIndices( bool force = true )
	{
		if( !m_wireframeIndexBuffer && force )
		{
			computeWireframeIndices();
		}

		return m_wireframeIndexBuffer;
	}

	size_t memoryUsage() const
	{
		size_t total = m_indexBuffer->size() * sizeof( unsigned int );

		// For meshes the indices and wireframe indices are distinct and we compute wireframe indices
		// lazily. We store several additional members to do so and their combined memory is more than
		// the wireframe indices would be. In practice `memoryUsage()` is only called when inserting an
		// Expandulator into the global Cache, at which point we haven't computed wireframe indices,
		// so we accumulate these members instead. If we do eventually compute wireframe indices,
		// these members will be cleared and m_wireframeIndexBuffer will be filled. Its unlikely
		// that `memoryUsage()` will be re-called, but we'll have a safe over-estimate in any case.
		if( m_numVerticesPerFace )
		{
			total += m_numVerticesPerFace->Object::memoryUsage();
			total += m_originalIndices->Object::memoryUsage();
			total += m_oldToTriangulatedIndexMapping->Object::memoryUsage();
		}
		else
		{
			// For curves, points, and bounding boxes the wireframe indices and normal indices
			// are shared, so we don't need to re-count the memory usage, since we've already
			// accumulated m_indexBuffer above.
		}

		// For meshes, we store 2 extra members in order to perform expandulation of PrimitiveVariables
		// later on (eg. when an MVertexBuffer is requested from the global Cache). These members will
		// not be cleared, but they are null for curves, points, and bounding boxes.
		if( m_triangulatedIndices )
		{
			total += m_triangulatedIndices->Object::memoryUsage();
		}
		if( m_mapToOldFacevarying )
		{
			total += m_mapToOldFacevarying->Object::memoryUsage();
		}

		return total;
	}

private :

	void fillPointsData( const PointsPrimitive *points )
	{
		size_t numPoints = points->getNumPoints();

		std::vector<unsigned int> indices;
		indices.reserve( numPoints );
		for( size_t i = 0; i < numPoints; ++i )
		{
			indices.emplace_back( i );
		}

		m_indexBuffer = createIndexBuffer( indices );
		m_wireframeIndexBuffer = m_indexBuffer;
	}

	void fillCurvesData( const CurvesPrimitive *curves )
	{
		const auto &verticesPerCurve = curves->verticesPerCurve()->readable();

		size_t numElements = 0;
		for( size_t i = 0; i < curves->numCurves(); ++i )
		{
			numElements += curves->numSegments( i ) * 2;
		}

		std::vector<unsigned int> indices;
		indices.reserve( numElements );

		int positionBufferOffset = 0;
		int indexBufferOffset = 0;
		for( size_t i = 0; i < curves->numCurves(); ++i )
		{
			size_t numSegments = curves->numSegments( i );
			int endPointDuplication = ( verticesPerCurve[i] - ( (int)numSegments + 1 ) ) / 2;
			int segmentCurrent = positionBufferOffset + endPointDuplication;
			for( size_t j = 0; j < numSegments; ++j )
			{
				indices.emplace_back( segmentCurrent );
				indices.emplace_back( segmentCurrent + 1 );

				indexBufferOffset++;
				segmentCurrent++;
			}
			positionBufferOffset += verticesPerCurve[i];
		}

		m_indexBuffer = createIndexBuffer( indices );
		m_wireframeIndexBuffer = m_indexBuffer;
	}

	void fillMeshData( const MeshPrimitive *mesh )
	{
		m_numVerticesPerFace = mesh->verticesPerFace();

		// Store original indices
		m_originalIndices = mesh->vertexIds();

		// Store triangulated indices
		IECoreScene::MeshPrimitivePtr triangulated = IECore::runTimeCast<IECoreScene::MeshPrimitive>( mesh->copy() );
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

		m_oldToTriangulatedIndexMapping = new IntVectorData;
		std::vector<int> &oldToTriangulatedIndexMappingWritable = m_oldToTriangulatedIndexMapping->writable();
		oldToTriangulatedIndexMappingWritable.resize( mesh->variableSize( PrimitiveVariable::Vertex ) );

		std::vector<unsigned int> indices;
		indices.resize( triangulatedIndicesReadable.size() );

		tbb::parallel_for(
			tbb::blocked_range<size_t>( 0, indices.size() ), [&indices, &triangulatedIndicesReadable, &oldToTriangulatedIndexMappingWritable]( const tbb::blocked_range<size_t> &r )
			{
				for( size_t i = r.begin(); i != r.end(); ++i )
				{
					indices[i] = i;
					oldToTriangulatedIndexMappingWritable[ triangulatedIndicesReadable[i] ] = i;
				}
			}
		);

		m_mapToOldFacevarying = triangulated->expandedVariableData<IntVectorData>( "_indexMap" );

		// Note m_wireframeBuffer remains null until `wireframeIndices()` is called explicitly
		m_indexBuffer = createIndexBuffer( indices );
	}

	void fillBoundData()
	{
		// Indices:
		//
		//	  7------6
		//	 /|     /|
		//	3------2 |
		//	| 4----|-5
		//	|/     |/
		//	0------1

		m_indexBuffer = createIndexBuffer( { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 7, 3, 6, 2, 4, 0, 5, 1 } );
		m_wireframeIndexBuffer = indices();
	}

	void computeWireframeIndices()
	{
		// this is only called for meshes, so we can use the mesh specific data members.

		const std::vector<int> &numVerticesPerFaceReadable = m_numVerticesPerFace->readable();

		// Compute partial sum to make algorithm parallelisable.
		// [4, 4, 4, 4] -> [4, 8, 12, 16]
		std::vector<int> partialSum( numVerticesPerFaceReadable.size() );
		std::partial_sum(numVerticesPerFaceReadable.begin(), numVerticesPerFaceReadable.end(), partialSum.begin(), std::plus<int>());

		int totalNumEdgeIndices = partialSum[partialSum.size()-1] * 2;

		std::vector<unsigned int> indices;
		indices.resize( totalNumEdgeIndices );

		const std::vector<int> &originalIndicesReadable = m_originalIndices->readable();
		const std::vector<int> &oldToTriangulatedIndexMapping = m_oldToTriangulatedIndexMapping->readable();

		// Construct list of indices that can be used to render wireframes
		tbb::parallel_for(
			tbb::blocked_range<size_t>( 0, numVerticesPerFaceReadable.size() ), [&partialSum, &numVerticesPerFaceReadable, &originalIndicesReadable, &indices, oldToTriangulatedIndexMapping]( const tbb::blocked_range<size_t> &r )
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

						indices[insertion] = oldToTriangulatedIndexMapping[ oldIndexA ];
						indices[insertion + 1] = oldToTriangulatedIndexMapping[ oldIndexB ];
					}
				}
			}
		);

		// we don't need to store these anymore so we can free up some memory
		// this won't help us stay in the cache longer, but it should help the
		// user to stay within an overall memory budget.
		m_numVerticesPerFace = nullptr;
		m_originalIndices = nullptr;
		m_oldToTriangulatedIndexMapping = nullptr;

		m_wireframeIndexBuffer = createIndexBuffer( indices );
	}

	static IndexBufferPtr createIndexBuffer( const std::vector<unsigned int> &data )
	{
		IndexBufferPtr buffer( new MIndexBuffer( MGeometry::kUnsignedInt32 ) );
		size_t numEntries = data.size();
		void *bufferData = buffer->acquire( numEntries, true );
		if( bufferData && buffer )
		{
			memcpy( bufferData, data.data(), sizeof( unsigned int ) * numEntries );
			buffer->commit( bufferData );
		}
		else
		{
			msg( Msg::Error, "SceneShapeSubSceneOverride::bufferGetter", "Memory acquisition for index buffer failed." );
		}

		return buffer;
	}

	// for all primitives
	IndexBufferPtr m_indexBuffer;
	IndexBufferPtr m_wireframeIndexBuffer;

	// for meshes only
	IECore::ConstIntVectorDataPtr m_numVerticesPerFace;
	IECore::ConstIntVectorDataPtr m_originalIndices;
	IECore::ConstIntVectorDataPtr m_triangulatedIndices;
	IECore::IntVectorDataPtr m_oldToTriangulatedIndexMapping;
	IECore::IntVectorDataPtr m_mapToOldFacevarying;
};

using ExpandulatorPtr = std::shared_ptr<Expandulator>;

////////////////////////////////////////////////////////////////////////////////////////////
// Maya Buffer Cache
////////////////////////////////////////////////////////////////////////////////////////////

VertexBufferPtr createVertexBuffer( const MVertexBufferDescriptor &descriptor, const V3fVectorData *data )
{
	VertexBufferPtr buffer( new MVertexBuffer( descriptor ) );

	size_t numEntries = data->readable().size();
	void *bufferData = buffer->acquire( numEntries, true );
	if( bufferData && buffer )
	{
		memcpy( bufferData, data->baseReadable(), sizeof( float ) * 3 * numEntries );
		buffer->commit( bufferData );
	}
	else
	{
		msg( Msg::Error, "SceneShapeSubSceneOverride::bufferGetter", ( boost::format( "Memory acquisition for %s buffer failed." ) % descriptor.semanticName() ).str() );
	}

	return buffer;
}

VertexBufferPtr createVertexBuffer( const MVertexBufferDescriptor &descriptor, const V2fVectorData *data )
{
	VertexBufferPtr buffer( new MVertexBuffer( descriptor ) );

	size_t numEntries = data->readable().size();
	void *bufferData = buffer->acquire( numEntries, true );
	if( bufferData && buffer )
	{
		memcpy( bufferData, data->baseReadable(), sizeof( float ) * 2 * numEntries );
		buffer->commit( bufferData );
	}
	else
	{
		msg( Msg::Error, "SceneShapeSubSceneOverride::bufferGetter", "Memory acquisition for uv buffer failed." );
	}

	return buffer;
}

using CacheEntry = boost::variant<VertexBufferPtr, ExpandulatorPtr>;
using CacheEntryPtr = std::shared_ptr<CacheEntry>;

boost::signals2::signal<void (const BufferPtr & )> &bufferEvictedSignal()
{
	static boost::signals2::signal<void (const BufferPtr & )> g_bufferEvictedSignal;
	return g_bufferEvictedSignal;
}

struct CacheCleanup
{
	void operator() ( const IECore::MurmurHash &key, const CacheEntryPtr &entry )
	{
		if( auto buffer = boost::get<VertexBufferPtr>( entry.get() ) )
		{
			bufferEvictedSignal()( std::make_shared<Buffer>( *buffer ) );
		}
		else if( auto expandulator = boost::get<ExpandulatorPtr>( entry.get() ) )
		{
			bufferEvictedSignal()( std::make_shared<Buffer>( expandulator->get()->indices() ) );

			auto wireframeIndices = expandulator->get()->wireframeIndices( /* force */ false );
			if( wireframeIndices && wireframeIndices != expandulator->get()->indices() )
			{
				bufferEvictedSignal()( std::make_shared<Buffer>( wireframeIndices ) );
			}
		}
	}
};

struct CacheKey
{
	CacheKey( const MVertexBufferDescriptor &descriptor, const IECoreScene::Primitive *primitive, const Expandulator *expandulator, const MBoundingBox &bound )
		: descriptor( descriptor ), primitive( primitive ), expandulator( expandulator ), bound( bound )
	{
		hash.append( descriptor.semantic() );

		if( !primitive )
		{
			// null primitives represent a bounding box wireframe
			hash.append( bound.min().x );
			hash.append( bound.min().y );
			hash.append( bound.min().z );
			hash.append( bound.max().x );
			hash.append( bound.max().y );
			hash.append( bound.max().z );
			return;
		}
		else if( !expandulator )
		{
			// an expandulator is being requested, so we hash the primitive topology and ignore the PrimitiveVariables.
			primitive->topologyHash( hash );
		}
		else
		{
			// a VertexBuffer is being requested, so we hash the associated PrimitiveVariable.

			// mesh variables will be expandulated, so we include the topology hash as well.
			if( const auto *mesh = runTimeCast<const MeshPrimitive>( primitive ) )
			{
				mesh->topologyHash( hash );
			}

			/// \todo: consider more efficent ways to hash PrimitiveVariable data. Can we assume taking
			/// the address of the data is enough given we know its coming straight off disk?
			switch( descriptor.semantic() )
			{
				case MGeometry::kPosition :
				{
					primitive->variableData<V3fVectorData>( "P" )->hash( hash );
					break;
				}
				case MGeometry::kNormal :
				{
					// for meshes implicit normals will be generated from the topology, which we've already hashed.
					// for other primitives, we don't yet support normals.
					break;
				}
				case MGeometry::kTexture :
				{
					const auto it = primitive->variables.find( "uv" );
					if( it != primitive->variables.end() )
					{
						it->second.data->hash( hash );
						if( it->second.indices )
						{
							it->second.indices->hash( hash );
						}
					}
					break;
				}
				default :
				{
					break;
				}
			}
		}
	}

	operator const IECore::MurmurHash & () const
	{
		return hash;
	}

	// the semantic of the descriptor also encodes whether this represents indices
	// or vertices. An invalid semantic implies indices.
	const MVertexBufferDescriptor descriptor;
	const Primitive *primitive;
	const Expandulator *expandulator;
	const MBoundingBox bound;
	IECore::MurmurHash hash;
};

CacheEntryPtr cacheGetter( const CacheKey &key, size_t &cost )
{
	if( key.descriptor.semantic() )
	{
		// a key with a valid semantic means we're creating an MVertexBuffer representing a specific primvar
		VertexBufferPtr buffer = nullptr;

		switch( key.descriptor.semantic() )
		{
			case MGeometry::kPosition :
			{
				ConstV3fVectorDataPtr data = nullptr;
				if( !key.primitive )
				{
					// bounding box
					std::vector<Imath::V3f> p;
					p.reserve( 8 );
					p.emplace_back( key.bound.min().x, key.bound.min().y, key.bound.min().z ); // Indices:
					p.emplace_back( key.bound.max().x, key.bound.min().y, key.bound.min().z );
					p.emplace_back( key.bound.max().x, key.bound.max().y, key.bound.min().z ); //	  7------6
					p.emplace_back( key.bound.min().x, key.bound.max().y, key.bound.min().z ); //	 /|     /|
					p.emplace_back( key.bound.min().x, key.bound.min().y, key.bound.max().z ); //	3------2 |
					p.emplace_back( key.bound.max().x, key.bound.min().y, key.bound.max().z ); //	| 4----|-5
					p.emplace_back( key.bound.max().x, key.bound.max().y, key.bound.max().z ); //	|/     |/
					p.emplace_back( key.bound.min().x, key.bound.max().y, key.bound.max().z ); //	0------1
					data = new V3fVectorData( p );
				}
				else if( runTimeCast<const MeshPrimitive>( key.primitive ) )
				{
					const auto positionsView = key.primitive->variableIndexedView<V3fVectorData>( "P" );
					data = IECore::runTimeCast<const V3fVectorData>( key.expandulator->expandulateVertex( *positionsView ) );
				}
				else
				{
					data = key.primitive->variableData<V3fVectorData>( "P" );
				}

				cost = data->Object::memoryUsage();
				buffer = createVertexBuffer( key.descriptor, data.get() );
				break;
			}
			case MGeometry::kNormal :
			{
				ConstV3fVectorDataPtr data = nullptr;
				if( const auto *mesh = runTimeCast<const MeshPrimitive>( key.primitive ) )
				{
					auto normals = MeshAlgo::calculateNormals( mesh );
					PrimitiveVariable::IndexedView<Imath::V3f> normalsView( normals );
					data = IECore::runTimeCast<const V3fVectorData>( key.expandulator->expandulateVertex( normalsView ) );
				}
				else
				{
					return nullptr;
				}

				cost = data->Object::memoryUsage();
				buffer = createVertexBuffer( key.descriptor, data.get() );
				break;
			}
			case MGeometry::kTexture :
			{
				auto uvView = key.primitive->variableIndexedView<V2fVectorData>( "uv" );
				if( !uvView )
				{
					return nullptr;
				}

				ConstV2fVectorDataPtr data = nullptr;
				if( runTimeCast<const MeshPrimitive>( key.primitive ) )
				{
					data = key.expandulator->expandulateFacevarying( *uvView );
				}
				else
				{
					data = key.primitive->expandedVariableData<V2fVectorData>( "uv" );
				}

				cost = data->Object::memoryUsage();
				buffer = createVertexBuffer( key.descriptor, data.get() );
				break;
			}
			default :
			{
				break;
			}
		}

		return std::make_shared<CacheEntry>( buffer );
	}
	else
	{
		// a key without a valid semantic means we're creating an Expandulator representing the topology of a primitive
		auto expandulator = std::make_shared<Expandulator>( key.primitive );
		cost = expandulator->memoryUsage();
		return std::make_shared<CacheEntry>( expandulator );
	}
}

using Cache = IECore::LRUCache<IECore::MurmurHash, CacheEntryPtr, IECore::LRUCachePolicy::Parallel, CacheKey>;

Cache &cache()
{
	static Cache g_cache( cacheGetter, CacheCleanup(), memoryLimit() );
	return g_cache;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Misc Utilities
////////////////////////////////////////////////////////////////////////////////////////////

enum class RenderStyle { BoundingBox, Wireframe, Solid, Textured };
const std::vector<RenderStyle> &supportedRenderStyles()
{
	static std::vector<RenderStyle> g_supportedStyles = { RenderStyle::BoundingBox, RenderStyle::Wireframe, RenderStyle::Solid, RenderStyle::Textured };
	return g_supportedStyles;
}

MRenderItem *acquireRenderItem( MSubSceneContainer &container, const IECore::Object *object, const MString &name, RenderStyle style, bool &isNew )
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

	for( size_t i = 0; i < sets.length(); ++i )
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
			shaders.untextured = shaderManager->getShaderFromNode( shaderOutPlug.node(), instances[0], nullptr, nullptr, nullptr, nullptr, /* nonTextured = */ true );
			shaders.textured = shaderManager->getShaderFromNode( shaderOutPlug.node(), instances[0], nullptr, nullptr, nullptr, nullptr, /* nonTextured = */ false );
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

bool objectCanBeRendered( const IECore::ConstObjectPtr &object )
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

bool sceneIsAnimated( const SceneInterface *sceneInterface )
{
	const auto *scene = IECore::runTimeCast< const SampledSceneInterface >( sceneInterface );
	return ( !scene || scene->numBoundSamples() > 1 );
}

/// \todo: this is copied from a private member of SceneShapeInterface.
/// Either remove the need for it here or make that method public.
std::string relativePathName( const SceneInterface::Path &root, const SceneInterface::Path &path )
{
	if( root == path )
	{
		return "/";
	}

	std::string pathName;

	SceneInterface::Path::const_iterator it = path.begin();
	it += root.size();

	for ( ; it != path.end(); it++ )
	{
		pathName += '/';
		pathName += it->value();
	}

	return pathName;
}

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////
// Private subclasses
////////////////////////////////////////////////////////////////////////////////////////////

struct IECoreMaya::SceneShapeSubSceneOverride::AllShaders
{
	explicit AllShaders( const MObject &object )
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

	explicit RenderItemUserData( int componentIndex )
		: MUserData( false ),  // do not delete when render item is deleted, we manage life time
		  componentIndex( componentIndex )
	{
	}

	~RenderItemUserData() override = default;

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
			: MPxComponentConverter(), m_idx( -1 )
		{
		}

		~ComponentConverter() override = default;

		void initialize( const MRenderItem &renderItem ) override
		{
			m_idx = dynamic_cast<RenderItemUserData*>(renderItem.customData())->componentIndex;
			m_object = m_component.create( MFn::kMeshPolygonComponent );
		}

		void addIntersection( MIntersection &intersection ) override
		{
			m_component.addElement( m_idx );
		}

		MObject component() override
		{
			return m_object;
		}

		MSelectionMask selectionMask () const override
		{
			return MSelectionMask::kSelectMeshFaces;
		}

	private :

		MFnSingleIndexedComponent m_component;
		MObject m_object;
		int m_idx;
};

////////////////////////////////////////////////////////////////////////////////////////////
// Construction and book keeping
////////////////////////////////////////////////////////////////////////////////////////////

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

DrawAPI SceneShapeSubSceneOverride::supportedDrawAPIs() const
{
	return kAllDevices;
}

MPxSubSceneOverride* SceneShapeSubSceneOverride::Creator( const MObject& obj )
{
	return new SceneShapeSubSceneOverride( obj );
}

SceneShapeSubSceneOverride::SceneShapeSubSceneOverride( const MObject& obj )
	: MPxSubSceneOverride( obj ), m_drawTagsFilter( "" ), m_time( -1 ), m_drawRootBounds( false ), m_drawChildBounds( false ), m_shaderOutPlug(), m_instancedRendering( false /* instancedRendering switch */ ), m_geometryVisible( false ), m_objectOnly( false )
{
	MStatus status;
	MFnDependencyNode node( obj, &status );

	if( status )
	{
		m_sceneShape = dynamic_cast<SceneShape*>( node.userNode() );
	}

	m_allShaders = std::make_shared<AllShaders>( m_sceneShape->thisMObject() );

	m_evictionConnection = bufferEvictedSignal().connect( boost::bind( &SceneShapeSubSceneOverride::bufferEvictedCallback, this, ::_1 ) );
}

SceneShapeSubSceneOverride::~SceneShapeSubSceneOverride() = default;

////////////////////////////////////////////////////////////////////////////////////////////
// Drawing
////////////////////////////////////////////////////////////////////////////////////////////

bool SceneShapeSubSceneOverride::requiresUpdate(const MSubSceneContainer& container, const MFrameContext& frameContext) const
{
	bool allInvisible = true;
	MFnDagNode dagNode( m_sceneShape->thisMObject() );
	MDagPathArray dagPaths;
	dagNode.getAllPaths( dagPaths );
	for( size_t i = 0; i < dagPaths.length(); ++i )
	{
		if( dagPaths[i].isVisible() )
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

		for( size_t i = 0; i < dagPaths.length(); ++i )
		{
			if( viewSelectedSet.hasItemPartly( dagPaths[i], component ) )
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
	checkDisplayOverrides( (MFrameContext::DisplayStyle)frameContext.getDisplayStyle(), tmpMask );

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

	// The objectOnly toggle determines if we need to recurse our internal scene locations
	bool tmpObjectOnly;
	MPlug( m_sceneShape->thisMObject(), SceneShape::aObjectOnly ).getValue( tmpObjectOnly );
	if( tmpObjectOnly != m_objectOnly )
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

		const RenderItemNames &names = it->second;
		for( const auto &name : names )
		{
			container.remove( name );
	 	}
	 	m_bufferToRenderItems.erase( b );
	}

	m_markedForDeletion.clear(); // releases memory

	// We'll set internal state based on settings in maya and then perform updates
	// by disabling all MRenderItems and reenabling those needed by walking the
	// tree. MRenderItems can be found in the container via their name. We make unique
	// MRenderItems per location, style, time, & maya instance.

	m_time = m_sceneShape->time();

	IECoreScene::ConstSceneInterfacePtr tmpSceneInterface = m_sceneShape->getSceneInterface();
	if( m_sceneInterface != tmpSceneInterface )
	{
		// All data in the container is invalid now and we can safely clear it
		container.clear();
		m_sceneInterface = tmpSceneInterface;
	}

	// STYLE
	// Used to skip rendering independently of style mask
	MPlug drawGeometryPlug( m_sceneShape->thisMObject(), SceneShape::aDrawGeometry );
	drawGeometryPlug.getValue( m_geometryVisible );

	checkDisplayOverrides( (MFrameContext::DisplayStyle)frameContext.getDisplayStyle(), m_styleMask );

	// DRAWING ROOTS
	MPlug drawRootBoundsPlug( m_sceneShape->thisMObject(), SceneShape::aDrawRootBound );
	drawRootBoundsPlug.getValue( m_drawRootBounds );

	// DRAWING CHILD BOUNDS
	MPlug drawAllBoundsPlug( m_sceneShape->thisMObject(), SceneShape::aDrawChildBounds );
	drawAllBoundsPlug.getValue( m_drawChildBounds );

	// The objectOnly toggle determines if we need to recurse our internal scene locations
	bool tmpObjectOnly = MPlug( m_sceneShape->thisMObject(), SceneShape::aObjectOnly ).asBool();
	if( tmpObjectOnly != m_objectOnly )
	{
		m_objectOnly = tmpObjectOnly;
	}

	// TAGS
	MString tmpTagsFilter;
	MPlug drawTagsFilterPlug( m_sceneShape->thisMObject(), SceneShape::aDrawTagsFilter );
	drawTagsFilterPlug.getValue( tmpTagsFilter );
	if( tmpTagsFilter.asChar() != m_drawTagsFilter )
	{
		m_drawTagsFilter = tmpTagsFilter.asChar();
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
		renderItem->enable( false );
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
	M3dView::getM3dViewFromModelPanel( panelName, view );
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
	visitSceneLocations( m_sceneInterface.get(), renderItems, container, MMatrix(), /* isRoot = */ true );

	for( auto *item : m_renderItemsToEnable )
	{
		item->enable( true );
	}
	m_renderItemsToEnable.clear();
}

void SceneShapeSubSceneOverride::visitSceneLocations( const SceneInterface *sceneInterface, RenderItemMap &renderItems, MSubSceneContainer &container, const MMatrix &matrix, bool isRoot )
{
	if( !sceneInterface )
	{
		return;
	}

	MMatrix accumulatedMatrix;
	if( !isRoot )
	{
		accumulatedMatrix = IECore::convert<MMatrix, Imath::M44d>( sceneInterface->readTransformAsMatrix( m_time ) ) * matrix;
	}

	// Dispatch to children only if we need to draw them
	/// \todo: we should be accounting for the tag filter when recursing to children
	if( ( m_geometryVisible || m_drawChildBounds ) && !m_objectOnly )
	{
		SceneInterface::NameList childNames;
		sceneInterface->childNames( childNames );

		for( const auto &childName : childNames )
		{
			visitSceneLocations( sceneInterface->child( childName ).get(), renderItems, container, accumulatedMatrix, false );
		}
	}

	// Now handle current location.

	std::string location;
	IECoreScene::SceneInterface::Path path;
	sceneInterface->path( path );
	IECoreScene::SceneInterface::pathToString( path, location );

	if( isRoot && m_drawRootBounds )
	{
		std::string rootItemName = location + "_root_style_" + std::to_string( (int)RenderStyle::BoundingBox );
		if( sceneIsAnimated( sceneInterface ) )
		{
			rootItemName += "_" + std::to_string( m_time );
		}

		const MBoundingBox bound = IECore::convert<MBoundingBox>( sceneInterface->readBound( m_time ) );

		int count = 0;
		for( const auto &instance : m_instances )
		{
			std::string instanceName = rootItemName + "_" + std::to_string( count++ );

			bool isNew;
			MString itemName( instanceName.c_str() );
			MRenderItem *renderItem = acquireRenderItem( container, IECore::NullObject::defaultNullObject(), itemName, RenderStyle::BoundingBox, isNew );

			MShaderInstance *shader = m_allShaders->getShader( RenderStyle::BoundingBox, instance.componentMode, instance.selected );
			if( renderItem->getShader() != shader )
			{
				renderItem->setShader( shader );
			}

			if( isNew )
			{
				setBuffersForRenderItem( nullptr, renderItem, true, bound );
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

			addInstanceTransform( *renderItem, instance.transformation );
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
	const MBoundingBox bound = IECore::convert<MBoundingBox>( sceneInterface->readBound( m_time ) );

	SceneInterface::Path rootPath;
	m_sceneShape->getSceneInterface()->path( rootPath );
	/// \todo: stop using the SceneShapeInterface selectionIndex. It relies on a secondary IECoreGL render.
	int componentIndex = m_sceneShape->selectionIndex( ::relativePathName( rootPath, path ) );

	// Adding RenderItems as needed
	// ----------------------------
	for( RenderStyle style : supportedRenderStyles() )
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

		std::string baseItemName = location + "_style_" + std::to_string( (int)style );
		if( sceneIsAnimated( sceneInterface ) )
		{
			baseItemName += "_time_" + std::to_string( m_time );
		}

		int count = 0;
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

			std::string instanceName = baseItemName + "_instance_" + std::to_string( count++ );
			MString itemName( instanceName.c_str() );

			bool isNew;
			MRenderItem *renderItem = acquireRenderItem( container, object.get(), itemName, style, isNew );

			// Before setting geometry, a shader has to be assigned so that the data requirements are clear.
			std::string pathKey = instance.path.fullPathName().asChar();
			/// \todo: we're inserting pathKey into the map regardless of whether it existed before
			bool componentSelected = m_selectedComponents[pathKey].count( componentIndex ) > 0;

			MShaderInstance *shader = m_allShaders->getShader( style, instance.componentMode, instance.componentMode ? componentSelected : instance.selected );
			if( renderItem->getShader() != shader )
			{
				renderItem->setShader( shader );
			}

			// Update the selection mask to enable marquee selection of components we explicitly disable
			// wireframe selection in Solid mode, because otherwise we'd get double selections (the marquee
			// overlaps both the mesh and the wireframe). Note we're still allowing bounding box selection
			// so double selection is still possible, and this breaks toggle selection mode.
			if( style == RenderStyle::Wireframe && m_styleMask.test( (int)RenderStyle::Solid ) )
			{
				renderItem->setSelectionMask( MSelectionMask::kSelectMeshes );
			}
			else
			{
				renderItem->setSelectionMask( instance.componentMode ? MSelectionMask::kSelectMeshFaces : MSelectionMask::kSelectMeshes );
			}

			// set the geometry on the render item if it's a new one.
			if( isNew )
			{
				setBuffersForRenderItem(
					( style == RenderStyle::BoundingBox ) ? nullptr : primitive.get(),
					renderItem,
					style == RenderStyle::Wireframe || style == RenderStyle::BoundingBox,
					bound
				);

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

			addInstanceTransform( *renderItem, accumulatedMatrix * instance.transformation );
		}
	}
}

void SceneShapeSubSceneOverride::setBuffersForRenderItem( const Primitive *primitive, MRenderItem *renderItem, bool wireframe, const MBoundingBox &bound )
{
	CacheEntryPtr entry = cache().get( { MVertexBufferDescriptor(), primitive, nullptr, bound } );

	// get the MIndexBuffer for the topology
	ExpandulatorPtr expandulator = boost::get<ExpandulatorPtr>( *entry );
	IndexBufferPtr &indexBuffer = wireframe ? expandulator->wireframeIndices() : expandulator->indices();
	BufferPtr buffer = std::make_shared<Buffer>( indexBuffer );
	auto it = m_bufferToRenderItems.find( buffer.get() );
	if( it == m_bufferToRenderItems.end() )
	{
		m_bufferToRenderItems.emplace( buffer.get(), RenderItemNames{ renderItem->name() } );
	}
	else
	{
		it->second.emplace_back( renderItem->name() );
	}

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

		switch( descriptor.semantic() )
		{
			case MGeometry::kPosition :
			case MGeometry::kNormal :
			case MGeometry::kTexture :
			{
				entry = cache().get( { descriptor, primitive, expandulator.get(), bound } );
				break;
			}
			default :
			{
				continue;
			}
		}

		if( !entry )
		{
			continue;
		}

		VertexBufferPtr vertexBuffer = boost::get<VertexBufferPtr>( *entry );
		vertexBufferArray.addBuffer( descriptor.semanticName(), vertexBuffer.get() );
		buffer = std::make_shared<Buffer>( vertexBuffer );
		it = m_bufferToRenderItems.find( buffer.get() );
		if( it == m_bufferToRenderItems.end() )
		{
			m_bufferToRenderItems.emplace( buffer.get(), RenderItemNames{ renderItem->name() } );
		}
		else
		{
			it->second.emplace_back( renderItem->name() );
		}
	}

	setGeometryForRenderItem( *renderItem, vertexBufferArray, *indexBuffer, &bound );
}

void SceneShapeSubSceneOverride::checkDisplayOverrides( MFrameContext::DisplayStyle displayStyle, StyleMask &mask ) const
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
	if( geometryOverride && ( displayStyle & ( MHWRender::MFrameContext::kGouraudShaded | MHWRender::MFrameContext::kTextured | MHWRender::MFrameContext::kFlatShaded ) ) )
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

void SceneShapeSubSceneOverride::bufferEvictedCallback( const BufferPtr &buffer )
{
	m_markedForDeletion.emplace_back( buffer ); // hold on to the resource just a little longer
}

////////////////////////////////////////////////////////////////////////////////////////////
// Selection and Instancing Utilities
////////////////////////////////////////////////////////////////////////////////////////////

#if MAYA_API_VERSION > 201650
bool SceneShapeSubSceneOverride::getInstancedSelectionPath( const MRenderItem &renderItem, const MIntersection &intersection, MDagPath &dagPath ) const
{
	auto it = m_renderItemNameToDagPath.find( renderItem.name().asChar() );
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
	for( size_t i = 0; i < dagPaths.length(); ++i )
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
		for( size_t i = 0; i < componentIndices.length(); ++i )
		{
			/// \todo: this is inserting selected paths into the map regardless
			/// of whether they match the dag paths of our node.
			indexMap[key].insert( componentIndices[i] );
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
	size_t numInstances = dagPaths.length();

	instances.reserve( numInstances );
	for( size_t pathIndex = 0; pathIndex < numInstances; ++pathIndex )
	{
		MDagPath& path = dagPaths[pathIndex];
		MMatrix matrix = path.inclusiveMatrix();
		bool pathSelected = isPathSelected( selectionList, path );
		bool componentMode = componentsSelectable( path );
		MFnDagNode nodeFn( path );
		bool visible = path.isVisible();

		instances.emplace_back( matrix, pathSelected, componentMode, path, visible );
	}
}
