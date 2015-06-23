//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#include "renderer/api/object.h"

#include "IECore/MeshPrimitive.h"
#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"

#include "IECore/TriangulateOp.h"
#include "IECore/FaceVaryingPromotionOp.h"

#include "IECoreAppleseed/ToAppleseedMeshConverter.h"

using namespace IECoreAppleseed;
using namespace IECore;
using namespace Imath;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

IE_CORE_DEFINERUNTIMETYPED( ToAppleseedMeshConverter );

ToAppleseedMeshConverter::ConverterDescription<ToAppleseedMeshConverter> ToAppleseedMeshConverter::g_description;

ToAppleseedMeshConverter::ToAppleseedMeshConverter( MeshPrimitivePtr toConvert )
	:	ToAppleseedShapeConverter( "Converts IECore::MeshPrimitives to appleseed mesh object entities", MeshPrimitive::staticTypeId() )
{
	srcParameter()->setValue( toConvert );
}

ToAppleseedMeshConverter::~ToAppleseedMeshConverter()
{
}

asr::Entity *ToAppleseedMeshConverter::doConversion( ConstObjectPtr from, ConstCompoundObjectPtr operands ) const
{
	MeshPrimitivePtr mesh = static_cast<const MeshPrimitive *>( from.get() )->copy();
	const V3fVectorData *p = mesh->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );
	if( !p )
	{
		throw Exception( "MeshPrimitive does not have \"P\" primitive variable of interpolation type Vertex." );
	}

	asf::auto_release_ptr<asr::MeshObject> meshObj = asr::MeshObjectFactory::create( "mesh", asr::ParamArray() );
	const size_t materialSlot = meshObj->push_material_slot( "default" );

	// vertices
	{
		size_t numVertices = p->readable().size();
		meshObj->reserve_vertices( numVertices );
		const std::vector<V3f> &points = p->readable();
		for( size_t i = 0; i < numVertices; ++i )
		{
			meshObj->push_vertex( asr::GVector3( points[i].x, points[i].y, points[i].z ) );
		}
	}

	// triangulate primitive (this should be in appleseed at some point)
	{
		TriangulateOpPtr op = new TriangulateOp();
		op->inputParameter()->setValue( mesh );
		op->throwExceptionsParameter()->setTypedValue( false ); // it's better to see something than nothing
		op->copyParameter()->setTypedValue( false );
		op->operate();
	}

	// triangles
	size_t numTriangles = mesh->numFaces();
	std::vector<asr::Triangle> triangles;
	triangles.reserve( numTriangles );
	const std::vector<int> &vidx = mesh->vertexIds()->readable();
	for( size_t i = 0; i < vidx.size(); i += 3 )
	{
		triangles.push_back( asr::Triangle( vidx[i], vidx[i+1], vidx[i+2], materialSlot ) );
	}

	// texture coords
	{
		const FloatVectorData *s = mesh->variableData<FloatVectorData>( "s" );
		const FloatVectorData *t = mesh->variableData<FloatVectorData>( "t" );
		if( s && t )
		{
			PrimitiveVariable::Interpolation sInterpolation = mesh->variables.find( "s" )->second.interpolation;
			PrimitiveVariable::Interpolation tInterpolation = mesh->variables.find( "t" )->second.interpolation;
			if( sInterpolation == tInterpolation )
			{
				if( sInterpolation == PrimitiveVariable::Varying || sInterpolation == PrimitiveVariable::Vertex || sInterpolation == PrimitiveVariable::FaceVarying )
				{
					size_t numSTs = s->readable().size();
					meshObj->reserve_tex_coords( numSTs );
					const std::vector<float> &svec = s->readable();
					const std::vector<float> &tvec = t->readable();

					for( size_t i = 0; i < numSTs; ++i)
					{
						meshObj->push_tex_coords( asr::GVector2( svec[i], tvec[i] ) );
					}

					if( sInterpolation == PrimitiveVariable::FaceVarying )
					{
						for( size_t i = 0, j = 0; i < numTriangles; ++i)
						{
							asr::Triangle& tri = triangles[i];
							tri.m_a0 = j++;
							tri.m_a1 = j++;
							tri.m_a2 = j++;
						}
					}
					else
					{
						for( size_t i = 0; i < vidx.size(); i += 3)
						{
							asr::Triangle& tri = triangles[i / 3];
							tri.m_a0 = vidx[i];
							tri.m_a1 = vidx[i+1];
							tri.m_a2 = vidx[i+2];
						}
					}
				}
				else
				{
					msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", "Variables s and t have unsupported interpolation type - not generating uvs." );
				}
			}
			else
			{
				msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", "Variables s and t have different interpolation - not generating uvs." );
			}
		}
		else if( s || t )
		{
			msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", "Only one of s and t available - not generating uvs." );
		}
	}

	// normals
	{
		PrimitiveVariableMap::const_iterator nIt = mesh->variables.find( "N" );
		if( nIt != mesh->variables.end() )
		{
			const V3fVectorData *n = runTimeCast<const V3fVectorData>( nIt->second.data.get() );
			if( n )
			{
				PrimitiveVariable::Interpolation nInterpolation = nIt->second.interpolation;
				if( nInterpolation == PrimitiveVariable::Varying || nInterpolation == PrimitiveVariable::Vertex || nInterpolation == PrimitiveVariable::FaceVarying )
				{
					size_t numNormals = n->readable().size();
					meshObj->reserve_vertex_normals( numNormals );
					const std::vector<V3f> &normals = n->readable();
					for( size_t i = 0; i < numNormals; ++i)
					{
						asr::GVector3 n( normals[i].x, normals[i].y, normals[i].z );
						n = asf::normalize( n );
						meshObj->push_vertex_normal( n );
					}

					if( nInterpolation == PrimitiveVariable::FaceVarying )
					{
						for( size_t i = 0, j = 0; i < numTriangles; ++i)
						{
							asr::Triangle& tri = triangles[i];
							tri.m_n0 = j++;
							tri.m_n1 = j++;
							tri.m_n2 = j++;
						}
					}
					else
					{
						for( size_t i = 0; i < vidx.size(); i += 3)
						{
							asr::Triangle& tri = triangles[i / 3];
							tri.m_n0 = vidx[i];
							tri.m_n1 = vidx[i+1];
							tri.m_n2 = vidx[i+2];
						}
					}
				}
				else
				{
					msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", "Variable \"N\" has unsupported interpolation type - not generating normals." );
				}
			}
			else
			{
				msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", boost::format( "Variable \"N\" has unsupported type \"%s\" (expected V3fVectorData)." ) % nIt->second.data->typeName() );
			}
		}
	}

	// tangents, only if we have texture coords.
	if( meshObj->get_tex_coords_count() != 0 )
	{
		PrimitiveVariableMap::const_iterator tIt = mesh->variables.find( "uTangent" );
		if( tIt != mesh->variables.end() )
		{
			const V3fVectorData *t = runTimeCast<const V3fVectorData>( tIt->second.data.get() );
			if( t )
			{
				size_t numTangents = t->readable().size();
				if( numTangents == meshObj->get_tex_coords_count() )
				{
					meshObj->reserve_vertex_tangents( numTangents );
					const std::vector<V3f> &tangents = t->readable();
					for( size_t i = 0; i < numTangents; ++i)
					{
						asr::GVector3 t( tangents[i].x, tangents[i].y, tangents[i].z );
						t = asf::normalize( t );
						meshObj->push_vertex_tangent( t );
					}
				}
				else
				{
					msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", "Variable \"uTangent\" has different interpolation type than texture coordinates - not generating tangents." );
				}
			}
			else
			{
				msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", boost::format( "Variable \"uTangent\" has unsupported type \"%s\" (expected V3fVectorData)." ) % tIt->second.data->typeName() );
			}
		}
	}

	// copy triangles to mesh entity
	{
		meshObj->reserve_triangles( numTriangles );

		for( size_t i = 0; i < triangles.size(); ++i)
		{
			meshObj->push_triangle( triangles[i] );
		}
	}

	return meshObj.release();
}
