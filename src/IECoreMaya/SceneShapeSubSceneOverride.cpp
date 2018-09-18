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

#include "IECoreScene/MeshNormalsOp.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/SampledSceneInterface.h"
#include "IECoreScene/TriangulateOp.h"
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

#include "boost/lexical_cast.hpp"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreMaya;
using namespace MHWRender;

namespace
{

struct GeometryData
{
	public :
		GeometryData()
		{
		}

		~GeometryData()
		{
		}

		IECore::ConstV3fVectorDataPtr positionData;
		IECore::ConstV3fVectorDataPtr normalData;
		IECore::ConstV2fVectorDataPtr uvData;

		IECore::ConstIntVectorDataPtr indexData;
		IECore::ConstIntVectorDataPtr wireframeIndexData;
};

using GeometryDataPtr = std::shared_ptr<GeometryData>;

enum class RenderStyle { BoundingBox, Wireframe, Solid, Textured, Last };
std::vector<RenderStyle> g_supportedStyles = { RenderStyle::BoundingBox, RenderStyle::Wireframe, RenderStyle::Solid, RenderStyle::Textured };

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

void ensureFaceVaryingData( IECore::ConstIntVectorDataPtr &i, IECore::ConstV3fVectorDataPtr &p, IECore::ConstV3fVectorDataPtr &n, std::vector<int> &additionalIndices, bool interpolationIsLinear )
{
	const std::vector<int> &indicesReadable = i->readable();
	const std::vector<Imath::V3f> &positionsReadable = p->readable();
	const std::vector<Imath::V3f> &normalsReadable = n->readable();

	IECore::IntVectorDataPtr newIndices = new IECore::IntVectorData();
	IECore::V3fVectorDataPtr newPositions = new IECore::V3fVectorData();
	IECore::V3fVectorDataPtr newNormals = new IECore::V3fVectorData();

	std::vector<int> &newIndicesWritable = newIndices->writable();
	std::vector<Imath::V3f> &newPositionsWritable = newPositions->writable();
	std::vector<Imath::V3f> &newNormalsWritable = newNormals->writable();

	int numFaceVertices = indicesReadable.size();
	newIndicesWritable.reserve( numFaceVertices );
	newPositionsWritable.reserve( numFaceVertices );
	newNormalsWritable.reserve( numFaceVertices );

	std::vector<int> indexMapping;
	indexMapping.resize( *std::max_element( indicesReadable.begin(), indicesReadable.end() ) + 1 );
	for( int i = 0; i < numFaceVertices; ++i )
	{
		int oldIndex = indicesReadable[i];

		int newIndex;
		newPositionsWritable.push_back( positionsReadable[oldIndex] );
		if( interpolationIsLinear )
		{
			// we know that we operate on triangulated meshes.
			newNormalsWritable.push_back( normalsReadable[i/3] );
		}
		else
		{
			newNormalsWritable.push_back( normalsReadable[oldIndex] );
		}

		newIndex = newPositionsWritable.size() - 1;
		newIndicesWritable.push_back( newIndex );
		indexMapping[oldIndex] = newIndex;
	}

	// still need to remap additional indices that the above will have messed up
	for( int &index : additionalIndices )
	{
		index = indexMapping[index];
	}

	i = newIndices;
	p = newPositions;
	n = newNormals;
}

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

void fillMeshData( ConstObjectPtr object, const MVertexBufferDescriptorList &descriptorList, GeometryDataPtr geometryData )
{
	IECoreScene::ConstMeshPrimitivePtr meshPrimitive = IECore::runTimeCast<const IECoreScene::MeshPrimitive>( object );
	if( !meshPrimitive )
	{
		return;
	}

	// \todo: Ideally we wouldn't copy a whole lot of data here, but just get an index buffer for tris.
	IECoreScene::MeshPrimitivePtr meshPrimitiveTriangulated = IECore::runTimeCast<IECoreScene::MeshPrimitive>( meshPrimitive->copy() );
	IECoreScene::TriangulateOpPtr op( new IECoreScene::TriangulateOp() );
	op->inputParameter()->setValue( meshPrimitiveTriangulated );
	op->throwExceptionsParameter()->setTypedValue( false ); // it's better to see something than nothing
	op->copyParameter()->setTypedValue( false );
	op->operate();

	// Get handles to relevant primitive variables.
	IECore::ConstV3fVectorDataPtr p = meshPrimitiveTriangulated->variableData<IECore::V3fVectorData>( "P" );
	IECore::ConstIntVectorDataPtr indicesTriangulated = meshPrimitiveTriangulated->vertexIds();
	IECore::ConstIntVectorDataPtr wireframeIndicesOriginal = meshPrimitive->vertexIds();

	// \todo: Room for optimization - we potentially already have normals in the data.
	IECoreScene::MeshNormalsOpPtr normalOp = new IECoreScene::MeshNormalsOp();
	normalOp->inputParameter()->setValue( meshPrimitiveTriangulated );
	normalOp->copyParameter()->setTypedValue( false );
	bool linearInterpolation = meshPrimitive->interpolation() == "linear";
	normalOp->interpolationParameter()->setNumericValue( linearInterpolation ? IECoreScene::PrimitiveVariable::Uniform : IECoreScene::PrimitiveVariable::Vertex );
	normalOp->operate();

	IECore::ConstV3fVectorDataPtr n = meshPrimitiveTriangulated->variableData<IECore::V3fVectorData>( "N" );
	IECore::ConstV2fVectorDataPtr uv = meshPrimitiveTriangulated->expandedVariableData<IECore::V2fVectorData>( "uv", IECoreScene::PrimitiveVariable::FaceVarying );

	// We need topology information to render wireframes. When changing the
	// data in `ensureFaceVaryingData`, we invalidate these indices, but the
	// function allows passing a list of indices to be updated along with the
	// data.
	std::vector<int> wireframeIndicesReadable;
	if( wireframeIndicesOriginal )
	{
		wireframeIndicesReadable = std::vector<int>( wireframeIndicesOriginal->readable() );
	}

 	// Expand out positions and normals so that they line up with UVs
	ensureFaceVaryingData( indicesTriangulated, p, n, wireframeIndicesReadable, linearInterpolation );

	geometryData->positionData = p;
	geometryData->normalData = n;
	geometryData->uvData = uv;

	// Prepare the index buffer for rendering a solid mesh
	geometryData->indexData = indicesTriangulated;

	IECore::IntVectorDataPtr wireframeIndexData( new IntVectorData() );
	std::vector<int> &wireframeIndexReadable = wireframeIndexData->writable();

	const IECore::IntVectorData *numVerticesPerFace = meshPrimitive->verticesPerFace();
	if( numVerticesPerFace )
	{
		const std::vector<int> &numVerticesPerFaceReadable = numVerticesPerFace->readable();

		int totalNumEdgeIndices = std::accumulate( numVerticesPerFaceReadable.begin(), numVerticesPerFaceReadable.end(), 0 ) * 2;
		wireframeIndexReadable.reserve( totalNumEdgeIndices );

		int offset = 0;
		for( int numVerts : numVerticesPerFaceReadable )
		{
			for( int k = 0; k < numVerts; ++k )
			{
				wireframeIndexReadable.push_back( wireframeIndicesReadable[offset + k] );
				wireframeIndexReadable.push_back( wireframeIndicesReadable[offset + (k+1)%numVerts] );
			}
			offset += numVerts;
		}
	}
	geometryData->wireframeIndexData = wireframeIndexData;
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

// Limit is given in bytes.
size_t memoryLimit()
{
	// Environment variable sets limit in megabytes - defaulting to 500mb
	const char *m = getenv( "IECORE_MAYA_VP2_MEMORY" );
	size_t mi = m ? boost::lexical_cast<size_t>( m ) : 500;

	return mi * 1024 * 1024;
}

// Cache for geometric data
using GeometryDataCache = IECore::LRUCache<IECore::MurmurHash, GeometryDataPtr, IECore::LRUCachePolicy::Parallel, GeometryDataCacheGetterKey>;
GeometryDataCache g_geometryDataCache(geometryGetter, memoryLimit() );

MRenderItem *acquireRenderItem( MSubSceneContainer &container, ConstObjectPtr object, const MString &name, RenderStyle style, bool &isNew )
{
	MRenderItem *renderItem = container.find( name );
	if( renderItem )
	{
		renderItem->enable( true );
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

		MPlug shaderPlug = fnSet.findPlug( "surfaceShader" );
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

MShaderInstance* getShader( const MObject &object, RenderStyle style, bool componentMode, bool isSelected )
{
	switch( style )
		{
		case RenderStyle::BoundingBox :
		case RenderStyle::Wireframe :
		{
			if( componentMode )
			{
				SelectionStateShaders shaders = getComponentWireShaders();
				return isSelected ? shaders.selected : shaders.unselected;
			}
			else
			{
				SelectionStateShaders shaders = getWireShaders();
				return isSelected ? shaders.selected : shaders.unselected;
			}
		}
		case RenderStyle::Solid :
		case RenderStyle::Textured :
			{
				TextureStateShaders shaders = getAssignedSurfaceShaders( object );
				return style == RenderStyle::Solid ? shaders.untextured : shaders.textured;
			}
		default :
			return nullptr;
		}
}

bool componentsSelectable( const MDagPath &path )
{
	MHWRender::DisplayStatus displayStatus = MHWRender::MGeometryUtilities::displayStatus( path );
	bool selectable = displayStatus == MHWRender::kHilite;
	return selectable;
}

// Convenience function that should eventually help with also rendering all instances generated by instancers.
// Currently not in use, but will come in handy when we get to look into MASH support.
MObjectArray getConnectedInstancers( MObject object )
{
	MObjectArray result;

	MStatus status;
	MObject current;

	MDagPathArray paths;
	MMatrixArray matrices;
	MIntArray pathStartIndices;
	MIntArray pathIndices;

	MItDependencyNodes it( MFn::kInstancer );
	for( ; !it.isDone(); it.next() )
	{
		MObject currentInstancer = it.item();
		MFnInstancer instFn( currentInstancer );

		MStatus status;
		MPlug inputsPlug = instFn.findPlug( "inputHierarchy", &status );
		if( !status )
		{
			continue;
		}

		// We will need to figure out if our SceneShape feeds into this instancer.
		for( unsigned int i = 0; i < inputsPlug.numElements(); ++i )
		{
			MPlug element = inputsPlug.elementByLogicalIndex( i );
			MPlugArray srcPlugs;
			element.connectedTo( srcPlugs, true, false );

			bool found = false;
			for(unsigned int i=0; i < srcPlugs.length(); ++i)
			{
				MFnDagNode connectedNode(srcPlugs[i].node());
				if( connectedNode.object() == object || connectedNode.hasChild( object ) )
				{
					result.append( currentInstancer );
					found = true;
					break;
				}
			}

			if( found )
			{
				break;
			}
		}
	}

	return result;
}

bool sceneIsAnimated( const SceneInterface *sceneInterface )
{
	const SampledSceneInterface *scene = IECore::runTimeCast< const SampledSceneInterface >( sceneInterface );
	return ( !scene || scene->numBoundSamples() > 1 );
}

void createMayaBuffersForRenderItem( GeometryDataPtr geometryData, const MVertexBufferDescriptorList &descriptorList, RenderStyle style, MVertexBufferArray &vertexBufferArray, MIndexBuffer &indexBuffer )
{
	MVertexBufferDescriptor positionDescriptor( "", MGeometry::kPosition, MGeometry::kFloat, 3 );
	MVertexBufferDescriptor normalDescriptor( "", MGeometry::kNormal, MGeometry::kFloat, 3 );
	MVertexBufferDescriptor uvDescriptor( "", MGeometry::kTexture, MGeometry::kFloat, 2 );

	// For the uv descriptor, Maya sometimes requires the name to be 'uvCoord'
	// and the semantic name to be 'mayauvcoordsemantic'. By respecting the
	// given descriptors below, we should always supply data that maya can work
	// with. The UV descriptor is the only reason why we need to scan the
	// descriptor list, really. We could probably omit doing that for positions
	// and normals.
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
				positionDescriptor = descriptor; break;
			case MGeometry::kNormal :
				normalDescriptor = descriptor; break;
			case MGeometry::kTexture :
				uvDescriptor = descriptor; break;
			default :
				continue;
		}
	}

	if( geometryData->positionData )
	{
		MVertexBuffer* positionBuffer = new MVertexBuffer( positionDescriptor );
		vertexBufferArray.addBuffer( "positions", positionBuffer );

		const std::vector<Imath::V3f> &positionsReadable = geometryData->positionData->readable();
		size_t numPositions = positionsReadable.size();
		void* positionData = positionBuffer->acquire( numPositions, true );
		if( positionData && positionBuffer )
		{
			memcpy( positionData, positionsReadable.data(), sizeof( float ) * 3 * numPositions );
			positionBuffer->commit( positionData );
		}
	}

	if( geometryData->normalData )
	{
		MVertexBuffer *normalBuffer = new MVertexBuffer( normalDescriptor );
		vertexBufferArray.addBuffer( "normals", normalBuffer );

		const std::vector<Imath::V3f> &normalsReadable = geometryData->normalData->readable();
		size_t numNormals = normalsReadable.size();
		void *normalData = normalBuffer->acquire( numNormals, true );
		if( normalData && normalBuffer )
		{
			memcpy( normalData, normalsReadable.data(), sizeof( float ) * 3 * numNormals );
			normalBuffer->commit( normalData );
		}
	}

	if( geometryData->uvData )
	{
		MVertexBuffer *uvBuffer = new MVertexBuffer( uvDescriptor );
		vertexBufferArray.addBuffer( "uvs", uvBuffer );

		const std::vector<Imath::V2f> &uvReadable = geometryData->uvData->readable();
		size_t numUVs = uvReadable.size();
		void *uvData = uvBuffer->acquire( numUVs, true );
		if( uvData && uvBuffer )
		{
			memcpy( uvData, uvReadable.data(), sizeof( float ) * 2 * numUVs );
			uvBuffer->commit( uvData );
		}
	}

	ConstIntVectorDataPtr indexDataPtr;
	if( style == RenderStyle::Solid || style == RenderStyle::Textured )
	{
		indexDataPtr = geometryData->indexData;
	}
	else
	{
		indexDataPtr = geometryData->wireframeIndexData;
	}

	if( indexDataPtr )
	{
		const std::vector<int> &indexReadable = indexDataPtr->readable();
		void *indexData = indexBuffer.acquire( indexReadable.size(), true );
		memcpy( indexData, indexReadable.data(), sizeof( int ) * indexReadable.size() );
		indexBuffer.commit( indexData );
	}
}

} // namespace

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
		m_materialIsDirty = true;
	}

	// Disable all MRenderItems before traversing the scene \todo: performance
	// improvement -> we don't have to do some of this if all that changed is a
	// shader for example.
	MSubSceneContainer::Iterator *it = container.getIterator();
	MRenderItem *renderItem = nullptr;
	while( (renderItem = it->next()) != nullptr )
	{
		renderItem->enable( false );
		removeAllInstances( *renderItem );
	}
	it->destroy();

	// Create and enable MRenderItems while traversing the scene hierarchy
	RenderItemMap renderItems;
	visitSceneLocations( m_sceneInterface.get(), renderItems, container, Imath::M44d(), /* isRoot = */ true );

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
	: MPxSubSceneOverride( obj ), m_drawTagsFilter( "" ), m_time( -1 ), m_drawRootBounds( false ), m_shaderOutPlug(), m_materialIsDirty( true ), m_instancedRendering( false /* instancedRendering switch */ ), m_geometryVisible( false )
{
	MStatus status;
	MFnDependencyNode node( obj, &status );

	if( status )
	{
		m_sceneShape = dynamic_cast<SceneShape*>( node.userNode() );
	}
}

void SceneShapeSubSceneOverride::visitSceneLocations( const SceneInterface *sceneInterface, RenderItemMap &renderItems, MSubSceneContainer &container, const Imath::M44d &matrix, bool isRoot )
{
	if( !sceneInterface )
	{
		return;
	}

	Imath::M44d accumulatedMatrix = sceneInterface->readTransformAsMatrix( m_time ) * matrix;
	MMatrix mayaMatrix = IECore::convert<MMatrix, Imath::M44d>( accumulatedMatrix );

	// Dispatch to all children.
	SceneInterface::NameList childNames;
	sceneInterface->childNames( childNames );

	for( const auto &childName : childNames )
	{
		visitSceneLocations( sceneInterface->child( childName ).get(), renderItems, container, accumulatedMatrix );
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

			Imath::M44d instanceMatrix = accumulatedMatrix * instance.transformation;
			MMatrix instanceMayaMatrix = IECore::convert<MMatrix, Imath::M44d>( instanceMatrix );

			bool isNew;
			MString itemName( instanceName.c_str() );
			MRenderItem *renderItem = acquireRenderItem( container, IECore::NullObject::defaultNullObject(), itemName, RenderStyle::BoundingBox, isNew );

			MShaderInstance *shader = getShader( m_sceneShape->thisMObject(), RenderStyle::BoundingBox, instance.componentMode, /* isComponentSelected = */ false );
			renderItem->setShader( shader );

			if( isNew )
			{
				GeometryDataPtr geometryData = g_geometryDataCache.get( GeometryDataCacheGetterKey( nullptr, renderItem->requiredVertexBuffers(), boundingBox ) );

				MVertexBufferArray vertexBufferArray;
				MIndexBuffer indexBuffer( MGeometry::kUnsignedInt32 );
				createMayaBuffersForRenderItem( geometryData, renderItem->requiredVertexBuffers(), RenderStyle::BoundingBox, vertexBufferArray, indexBuffer );
				setGeometryForRenderItem( *renderItem, vertexBufferArray, indexBuffer, &mayaBoundingBox );

				m_renderItemNameToDagPath[renderItem->name().asChar()] = instance.path;
			}
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
			MShaderInstance *shader = getShader( m_sceneShape->thisMObject(), style, instance.componentMode, instance.componentMode ? componentSelected : instance.selected );
			renderItem->setShader( shader );

			MMatrix instanceMatrix = IECore::convert<MMatrix, Imath::M44d>( accumulatedMatrix * instance.transformation );
			addInstanceTransform( *renderItem, instanceMatrix );

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

				MVertexBufferArray vertexBufferArray;
				MIndexBuffer indexBuffer( MGeometry::kUnsignedInt32 );
				createMayaBuffersForRenderItem( geometryData, renderItem->requiredVertexBuffers(), style, vertexBufferArray, indexBuffer );
				setGeometryForRenderItem( *renderItem, vertexBufferArray, indexBuffer, &mayaBoundingBox );

				RenderItemUserDataPtr userData = acquireUserData( componentIndex );
				renderItem->setCustomData( userData.get() );
				MDrawRegistry::registerComponentConverter( renderItem->name(), ComponentConverter::creator );

				m_renderItemNameToDagPath[renderItem->name().asChar()] = instance.path;
			}
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

		instances.emplace_back( matrix, pathSelected, componentMode, path );
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
	if( geometryOverride && ( displayStyle & ( MHWRender::MFrameContext::kGouraudShaded | MHWRender::MFrameContext::kTextured ) ) > 0 )
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
