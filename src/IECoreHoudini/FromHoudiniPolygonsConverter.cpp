//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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

#include "GA/GA_Names.h"

#include "IECore/CompoundObject.h"

#include "IECoreHoudini/FromHoudiniPolygonsConverter.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

namespace
{

static InternedString g_attributeFilter( "attributeFilter" );
static InternedString g_interpolationAttrib( "ieMeshInterpolation" );
static InternedString g_interpolationAttribNegated( " ^ieMeshInterpolation" );
static InternedString g_linear( "linear" );
static InternedString g_catmullClark( "catmullClark" );
static InternedString g_poly( "poly" );
static InternedString g_subdiv( "subdiv" );
static InternedString g_cornerWeightAttrib( "cornerweight" );
static InternedString g_creaseWeightAttrib( "creaseweight" );

} // namespace

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniPolygonsConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniPolygonsConverter> FromHoudiniPolygonsConverter::m_description( MeshPrimitive::staticTypeId() );

FromHoudiniPolygonsConverter::FromHoudiniPolygonsConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECoreScene::MeshPrimitive." )
{
}

FromHoudiniPolygonsConverter::FromHoudiniPolygonsConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECoreScene::MeshPrimitive." )
{
}

FromHoudiniPolygonsConverter::~FromHoudiniPolygonsConverter()
{
}

FromHoudiniGeometryConverter::Convertability FromHoudiniPolygonsConverter::canConvert( const GU_Detail *geo )
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();

	GA_Offset start, end;
	for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; offset < end; ++offset )
		{
			const GA_Primitive *prim = primitives.get( offset );
			if( prim->getTypeId() != GEO_PRIMPOLY )
			{
				return Inapplicable;
			}
		}
	}

	if ( hasOnlyOpenPolygons( geo ) )
	{
		return Suitable;
	}

	// is there a single named shape?
	GA_ROHandleS nameAttrib( geo, GA_ATTRIB_PRIMITIVE, GA_Names::name );
	if( nameAttrib.isValid() )
	{
		GA_StringTableStatistics stats;
		const GA_Attribute *nameAttr = nameAttrib.getAttribute();
		const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();
		tuple->getStatistics( nameAttr, stats );
		if ( stats.getEntries() < 2 )
		{
			return Ideal;
		}
	}

	return Suitable;
}

ObjectPtr FromHoudiniPolygonsConverter::doDetailConversion( const GU_Detail *geo, const CompoundObject *operands ) const
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();

	MeshPrimitivePtr result = new MeshPrimitive();

	size_t numEdges = 0;
	std::vector<int> vertIds;
	std::vector<int> vertsPerFace;

	GA_Offset start, end;
	for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; offset < end; ++offset )
		{
			const GA_Primitive *prim = primitives.get( offset );
			if( prim->getTypeId() != GEO_PRIMPOLY )
			{
				throw std::runtime_error( "FromHoudiniPolygonsConverter: Geometry contains non-polygon primitives" );
			}

			size_t numPrimVerts = prim->getVertexCount();
			vertsPerFace.push_back( numPrimVerts );
			numEdges += numPrimVerts;
			std::vector<int> ids( numPrimVerts );
			for( size_t j = 0; j < numPrimVerts; j++ )
			{
				vertIds.push_back( geo->pointIndex( prim->getPointOffset( numPrimVerts - 1 - j ) ) );
			}
		}
	}

	result->setTopology( new IntVectorData( vertsPerFace ), new IntVectorData( vertIds ) );

	CompoundObjectPtr modifiedOperands = transferMeshInterpolation( geo, operands, result.get() );

	if( geo->getNumVertices() )
	{
		transferAttribs( geo, result.get(), modifiedOperands ? modifiedOperands.get() : operands );
	}

	// check for corners and creases, which would have been extracted via transferAttribs()
	// as they are no different to standard attribs in Houdini.
	convertCorners( result.get() );
	convertCreases( result.get(), vertIds, numEdges );

	return result;
}

CompoundObjectPtr FromHoudiniPolygonsConverter::transferMeshInterpolation( const GU_Detail *geo, const IECore::CompoundObject *operands, IECoreScene::MeshPrimitive *mesh ) const
{
	// We store mesh interpolation in Houdini as an indexed string Prim Attrib (eg Uniform PrimitiveVariable)
	// but we don't want to extract it as such because it can be expensive to deal with indexed variables when
	// many meshes are stored in a single SOP. Since we know there is a fixed number of valid values, and we
	// only support a single value per mesh (rather than per polygon as its stored in Houdini), we can get
	// better performance with a specific extraction process.

	// try to get the interpolation type from the geo
	InternedString interpolation = g_linear;
	GA_ROHandleS attribHandle( geo, GA_ATTRIB_PRIMITIVE, g_interpolationAttrib.c_str() );
	if( !attribHandle.isValid() )
	{
		return nullptr;
	}

	CompoundObjectPtr modifiedOperands = operands->copy();
	std::string &attributeFilter = modifiedOperands->member<StringData>( g_attributeFilter )->writable();
	attributeFilter += g_interpolationAttribNegated.c_str();

	bool found = false;
	GA_Offset start, end;
	for( GA_Iterator it( geo->getPrimitiveRange() ); !found && it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; !found && offset < end; ++offset )
		{
			if( const char *value = attribHandle.get( offset ) )
			{
				if( !strcmp( value, g_subdiv.c_str() ) )
				{
					interpolation = g_catmullClark;
					// subdivision meshes should not have normals. we assume this occurred because the geo contained
					// both subdiv and linear meshes, inadvertantly extending the normals attribute to both.
					attributeFilter += " ^N";
					found = true;
					break;
				}
				else if( !strcmp( value, g_poly.c_str() ) )
				{
					interpolation = g_linear;
					found = true;
					break;
				}
			}
		}
	}

	mesh->setInterpolation( interpolation );

	return modifiedOperands;
}

void FromHoudiniPolygonsConverter::convertCorners( MeshPrimitive *mesh ) const
{
	// Houdini stores corners via a Point Attrib (which has been converted to a Vertex PrimitiveVariable)
	const auto *cornerWeightData = mesh->variableData<FloatVectorData>( g_cornerWeightAttrib, PrimitiveVariable::Vertex );
	if( !cornerWeightData )
	{
		return;
	}

	IntVectorDataPtr cornerIdsData = new IntVectorData();
	auto &cornerIds = cornerIdsData->writable();
	// likely larger than necessary, but we don't know the correct size yet
	cornerIds.reserve( mesh->variableSize( PrimitiveVariable::Vertex ) );

	FloatVectorDataPtr cornerSharpnessesData = new FloatVectorData();
	auto &cornerSharpnesses = cornerSharpnessesData->writable();
	cornerSharpnesses.reserve( cornerIds.size());

	const auto &cornerWeights = cornerWeightData->readable();
	for( size_t i = 0; i < cornerWeights.size(); ++i )
	{
		if( cornerWeights[i] > 0.0f )
		{
			cornerIds.push_back( i );
			cornerSharpnesses.push_back( cornerWeights[i] );
		}
	}

	if( !cornerIds.empty() )
	{
		mesh->setCorners( cornerIdsData.get(), cornerSharpnessesData.get() );
		mesh->variables.erase( g_cornerWeightAttrib );
	}
}

void FromHoudiniPolygonsConverter::convertCreases( MeshPrimitive *mesh, const std::vector<int> &vertIds, size_t numEdges ) const
{
	// Houdini stores creases via a Vertex Attrib (which has been converted to a FaceVarying PrimitiveVariable),
	// with the first face-vert of each creased face-edge containing the sharpness, and all other face-verts set to 0.
	const auto *creaseWeightData = mesh->variableData<FloatVectorData>( g_creaseWeightAttrib, PrimitiveVariable::FaceVarying );
	if( !creaseWeightData )
	{
		return;
	}

	IntVectorDataPtr creaseLengthsData = new IntVectorData();
	auto &creaseLengths = creaseLengthsData->writable();
	// likely larger than necessary, but we don't know the correct size yet
	creaseLengths.reserve( numEdges );

	IntVectorDataPtr creaseIdsData = new IntVectorData();
	auto &creaseIds = creaseIdsData->writable();
	creaseIds.reserve( creaseLengths.size() * 2 );

	FloatVectorDataPtr creaseSharpnessesData = new FloatVectorData();
	auto &creaseSharpnesses = creaseSharpnessesData->writable();
	creaseSharpnesses.reserve( creaseLengths.size() );

	// Calculate face-edge offsets based on winding order in Houdini,
	// which is opposite to that of Cortex. We need these to map from
	// single face-vert crease weights and find both verts of the edge.
	size_t faceOffset = 0;
	std::vector<int> windingOffsets;
	// most face-vert offsets will be to use the previous face-vert
	windingOffsets.resize( mesh->variableSize( PrimitiveVariable::FaceVarying ), -1 );
	for( auto numFaceVerts : mesh->verticesPerFace()->readable() )
	{
		// but we need to mark a wraparound vert for each face
		windingOffsets[faceOffset] = (numFaceVerts - 1);
		faceOffset += numFaceVerts;
	}

	const auto &creaseWeights = creaseWeightData->readable();
	for( int i = 0; i < creaseWeights.size(); ++i )
	{
		// there is a crease at this face-edge
		if( creaseWeights[i] > 0.0f )
		{
			// locate the 2nd vert of this face-edge
			int nextFaceVert = i + windingOffsets[i];

			// Since Houdini will have stored the crease in both directions
			// (once for each face-edge), we need to make sure we only record
			// it once, so we enforce that the vertIds are increasing.
			if( vertIds[i] < vertIds[nextFaceVert] )
			{
				creaseLengths.push_back( 2 );
				creaseIds.push_back( vertIds[i] );
				creaseIds.push_back( vertIds[nextFaceVert] );
				creaseSharpnesses.push_back( creaseWeights[i] );
			}
		}
	}

	if( !creaseLengths.empty() )
	{
		mesh->setCreases( creaseLengthsData.get(), creaseIdsData.get(), creaseSharpnessesData.get() );
		mesh->variables.erase( g_creaseWeightAttrib );
	}
}
