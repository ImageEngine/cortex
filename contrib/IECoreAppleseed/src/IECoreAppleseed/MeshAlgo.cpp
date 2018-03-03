//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/MeshAlgo.h"

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/TriangulateOp.h"

#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

void setMeshKey( renderer::MeshObject *mesh, size_t keyIndex, const Object *object )
{
	const MeshPrimitive *m = static_cast<const MeshPrimitive*>( object );
	const V3fVectorData *p = m->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );

	if( !p )
	{
		throw Exception( "MeshPrimitive does not have \"P\" primitive variable of interpolation type Vertex." );
	}

	const std::vector<V3f> &points = p->readable();
	for( size_t j = 0, numVertices = p->readable().size() ; j < numVertices; ++j )
	{
		mesh->set_vertex_pose( j, keyIndex, asr::GVector3( points[j].x, points[j].y, points[j].z ) );
	}

	if( mesh->get_vertex_normal_count() != 0 )
	{
		PrimitiveVariableMap::const_iterator nIt = m->variables.find( "N" );
		if( nIt == m->variables.end() )
		{
			throw Exception( "MeshPrimitive missing normals in motion sample." );
		}

		const V3fVectorData *n = runTimeCast<const V3fVectorData>( nIt->second.data.get() );
		if( !n )
		{
			throw Exception( ( boost::format( "MeshPrimitive \"N\" primitive variable has unsupported type \"%s\" (expected V3fVectorData)." ) % nIt->second.data->typeName() ).str() );
		}

		const std::vector<V3f> &normals = n->readable();
		size_t numNormals = normals.size();
		if( numNormals != mesh->get_vertex_normal_count() )
		{
			throw Exception( "MeshPrimitive \"N\" primitive variable has different interpolation than first deformation sample." );
		}

		for( size_t j = 0 ; j < numNormals; ++j )
		{
			const asr::GVector3 n( normals[j].x, normals[j].y, normals[j].z );
			mesh->set_vertex_normal_pose( j, keyIndex, asf::safe_normalize( n ) );
		}
	}

	if( mesh->get_vertex_tangent_count() != 0 )
	{
		PrimitiveVariableMap::const_iterator tIt = m->variables.find( "uTangent" );
		if( tIt == m->variables.end() )
		{
			throw Exception( "MeshPrimitive missing tangents in motion sample." );
		}

		const V3fVectorData *t = runTimeCast<const V3fVectorData>( tIt->second.data.get() );
		if( !t )
		{
			throw Exception( ( boost::format( "MeshPrimitive \"uTangent\" primitive variable has unsupported type \"%s\" (expected V3fVectorData)." ) % tIt->second.data->typeName() ).str() );
		}

		const std::vector<V3f> &tangents = t->readable();
		size_t numTangents = t->readable().size();
		if( numTangents != mesh->get_vertex_tangent_count() )
		{
			throw Exception( "MeshPrimitive \"uTangent\" primitive variable has different interpolation than first deformation sample." );
		}

		for( size_t j = 0 ; j < numTangents; ++j )
		{
			const asr::GVector3 t( tangents[j].x, tangents[j].y, tangents[j].z );
			mesh->set_vertex_tangent_pose( j, keyIndex, asf::safe_normalize( t ) );
		}
	}
}

}

//////////////////////////////////////////////////////////////////////////
// Implementation of public API.
//////////////////////////////////////////////////////////////////////////

namespace IECoreAppleseed
{

namespace MeshAlgo
{

renderer::MeshObject *convert( const IECore::Object *primitive )
{
	assert( primitive->typeId() == IECoreScene::MeshPrimitive::staticTypeId() );
	const IECoreScene::MeshPrimitive *mesh = static_cast<const IECoreScene::MeshPrimitive *>( primitive );

	const V3fVectorData *p = mesh->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );
	if( !p )
	{
		throw Exception( "MeshPrimitive does not have \"P\" primitive variable of interpolation type Vertex." );
	}

	asf::auto_release_ptr<asr::MeshObject> meshEntity( asr::MeshObjectFactory().create( "mesh", asr::ParamArray() ) );
	const size_t materialSlot = meshEntity->push_material_slot( "default" );

	// vertices
	{
		size_t numVertices = p->readable().size();
		meshEntity->reserve_vertices( numVertices );
		const std::vector<V3f> &points = p->readable();
		for( size_t i = 0; i < numVertices; ++i )
		{
			meshEntity->push_vertex( asr::GVector3( points[i].x, points[i].y, points[i].z ) );
		}
	}

	// triangulate primitive (this should be in appleseed at some point)
	MeshPrimitivePtr triangulatedMeshPrimPtr = mesh->copy();
	{
		TriangulateOpPtr op = new TriangulateOp();
		op->inputParameter()->setValue( triangulatedMeshPrimPtr );
		op->throwExceptionsParameter()->setTypedValue( false ); // it's better to see something than nothing
		op->copyParameter()->setTypedValue( false );
		op->operate();
	}

	// triangles
	size_t numTriangles = triangulatedMeshPrimPtr->numFaces();
	std::vector<asr::Triangle> triangles;
	triangles.reserve( numTriangles );
	const std::vector<int> &vidx = triangulatedMeshPrimPtr->vertexIds()->readable();
	for( size_t i = 0; i < vidx.size(); i += 3 )
	{
		triangles.push_back( asr::Triangle( vidx[i], vidx[i+1], vidx[i+2], materialSlot ) );
	}

	// texture coords
	{
		PrimitiveVariableMap::const_iterator uvIt = triangulatedMeshPrimPtr->variables.find( "uv" );
		if( uvIt != triangulatedMeshPrimPtr->variables.end() && uvIt->second.data->typeId() == V2fVectorDataTypeId )
		{
			if( uvIt->second.interpolation == PrimitiveVariable::Varying || uvIt->second.interpolation == PrimitiveVariable::Vertex || uvIt->second.interpolation == PrimitiveVariable::FaceVarying )
			{
				const std::vector<Imath::V2f> &uvs = runTimeCast<const V2fVectorData>( uvIt->second.data )->readable();
				size_t numUVs = uvs.size();

				meshEntity->reserve_tex_coords( numUVs );

				for( size_t i = 0; i < numUVs; ++i)
				{
					meshEntity->push_tex_coords( asr::GVector2( uvs[i] ) );
				}

				const vector<int> *indices = nullptr;
				if( uvIt->second.indices )
				{
					indices = &uvIt->second.indices->readable();
				}

				for( size_t i = 0; i < numTriangles; ++i )
				{
					asr::Triangle& tri = triangles[i];
					if( uvIt->second.interpolation == PrimitiveVariable::FaceVarying )
					{
						tri.m_a0 = i * 3;
						tri.m_a1 = i * 3 + 1;
						tri.m_a2 = i * 3 + 2;
					}
					else
					{
						tri.m_a0 = vidx[i * 3];
						tri.m_a1 = vidx[i * 3 + 1];
						tri.m_a2 = vidx[i * 3 + 2];
					}

					if( indices )
					{
						tri.m_a0 = (*indices)[tri.m_a0];
						tri.m_a1 = (*indices)[tri.m_a1];
						tri.m_a2 = (*indices)[tri.m_a2];
					}
				}
			}
			else
			{
				msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", "Variable \"uv\" has unsupported interpolation type - not generating uvs." );
			}
		}
	}

	// normals
	{
		PrimitiveVariableMap::const_iterator nIt = triangulatedMeshPrimPtr->variables.find( "N" );
		if( nIt != triangulatedMeshPrimPtr->variables.end() )
		{
			const V3fVectorData *n = runTimeCast<const V3fVectorData>( nIt->second.data.get() );
			if( n )
			{
				PrimitiveVariable::Interpolation nInterpolation = nIt->second.interpolation;
				if( nInterpolation == PrimitiveVariable::Varying || nInterpolation == PrimitiveVariable::Vertex || nInterpolation == PrimitiveVariable::FaceVarying )
				{
					size_t numNormals = n->readable().size();
					meshEntity->reserve_vertex_normals( numNormals );
					const std::vector<V3f> &normals = n->readable();
					for( size_t i = 0; i < numNormals; ++i)
					{
						asr::GVector3 n( normals[i].x, normals[i].y, normals[i].z );
						meshEntity->push_vertex_normal( asf::safe_normalize( n ) );
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

	// tangents
	{
		PrimitiveVariableMap::const_iterator tIt = triangulatedMeshPrimPtr->variables.find( "uTangent" );
		if( tIt != triangulatedMeshPrimPtr->variables.end() )
		{
			const V3fVectorData *t = runTimeCast<const V3fVectorData>( tIt->second.data.get() );
			if( t )
			{
				PrimitiveVariable::Interpolation tInterpolation = tIt->second.interpolation;
				if( tInterpolation == PrimitiveVariable::Varying || tInterpolation == PrimitiveVariable::Vertex )
				{
					size_t numTangents = t->readable().size();
					meshEntity->reserve_vertex_tangents( numTangents );
					const std::vector<V3f> &tangents = t->readable();
					for( size_t i = 0; i < numTangents; ++i)
					{
						asr::GVector3 t( tangents[i].x, tangents[i].y, tangents[i].z );
						meshEntity->push_vertex_tangent( asf::safe_normalize( t ) );
					}
				}
				else
				{
					msg( Msg::Warning, "ToAppleseedMeshConverter::doConversion", "Variable \"uTangent\" has unsupported interpolation type - not generating tangents." );
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
		meshEntity->reserve_triangles( numTriangles );

		for( size_t i = 0; i < triangles.size(); ++i)
		{
			meshEntity->push_triangle( triangles[i] );
		}
	}

	return meshEntity.release();
}

renderer::MeshObject *convert( const std::vector<const IECore::Object *> &samples )
{
	if( !asf::is_pow2( samples.size() ) )
	{
		throw Exception( "Number of motion samples must be a power of 2." );
	}

	// convert the first sample.
	renderer::MeshObject *mesh = convert( samples[0] );

	// set the point, normal and tangent positions for all other time samples.
	mesh->set_motion_segment_count( samples.size() - 1 );

	for( size_t i = 1, e = samples.size(); i < e; ++i )
	{
		setMeshKey( mesh, i - 1, samples[i] );
	}

	return mesh;
}

renderer::MeshObject *convert( const std::vector<IECore::ObjectPtr> &samples )
{
	if( !asf::is_pow2( samples.size() ) )
	{
		throw Exception( "Number of motion samples must be a power of 2." );
	}

	// convert the first sample.
	renderer::MeshObject *mesh = convert( samples[0].get() );

	// set the point, normal and tangent positions for all other time samples.
	mesh->set_motion_segment_count( samples.size() - 1 );
	for( size_t i = 1, e = samples.size(); i < e; ++i )
	{
		setMeshKey( mesh, i - 1, samples[i].get() );
	}

	return mesh;
}

} // namespace MeshAlgo

} // namespace IECoreAppleseed
