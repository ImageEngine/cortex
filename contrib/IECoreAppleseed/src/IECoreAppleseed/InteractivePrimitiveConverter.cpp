//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/private/InteractivePrimitiveConverter.h"

#include "foundation/math/scalar.h"
#include "renderer/api/version.h"

#include "IECore/MessageHandler.h"
#include "IECore/MeshPrimitive.h"

#include "IECoreAppleseed/ToAppleseedConverter.h"

using namespace std;
using namespace IECore;
using namespace Imath;

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::InteractivePrimitiveConverter::InteractivePrimitiveConverter( const asf::SearchPaths &searchPaths ) : PrimitiveConverter( searchPaths )
{
}

asf::auto_release_ptr<asr::Object> IECoreAppleseed::InteractivePrimitiveConverter::doConvertPrimitive( PrimitivePtr primitive, const string &name )
{
	asf::auto_release_ptr<asr::Object> obj;

	if( ToAppleseedConverterPtr converter = ToAppleseedConverter::create( primitive.get() ) )
	{
		obj.reset( static_cast<asr::Object*>( converter->convert() ) );
	}

	if( obj.get() )
	{
		obj->set_name( name.c_str() );
	}
	else
	{
		msg( Msg::Warning, "IECoreAppleseed::PrimitiveConverter", "Couldn't convert object" );
	}

	return obj;
}

asf::auto_release_ptr<asr::Object> IECoreAppleseed::InteractivePrimitiveConverter::doConvertPrimitive( const vector<PrimitivePtr> &primitives, const string &name )
{
	assert( asf::is_pow2( primitives.size() ) );

	// convert the first primitive
	asf::auto_release_ptr<asr::Object> obj = doConvertPrimitive( primitives[0], name );

	if( !obj.get() )
	{
		return obj;
	}

	if( primitives[0]->typeId() == MeshPrimitiveTypeId )
	{
		// set the point positions for all other time samples.
		asr::MeshObject *mesh = static_cast<asr::MeshObject*>( obj.get() );
		mesh->set_motion_segment_count( primitives.size() - 1 );

		for( size_t i = 1, e = primitives.size(); i < e; ++i )
		{
			const MeshPrimitive *m = static_cast<const MeshPrimitive*>( primitives[i].get() );
			const V3fVectorData *p = m->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );

			if( !p )
			{
				throw Exception( "MeshPrimitive does not have \"P\" primitive variable of interpolation type Vertex." );
			}

			const std::vector<V3f> &points = p->readable();
			for( size_t j = 0, numVertices = p->readable().size() ; j < numVertices; ++j )
			{
				mesh->set_vertex_pose( j, i - 1, asr::GVector3( points[j].x, points[j].y, points[j].z ) );
			}

			// motion blur for normals and tangents is only supported since appleseed 1.2.0
			#if APPLESEED_VERSION >= 10200
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
						mesh->set_vertex_normal_pose( j, i - 1, asf::normalize( asr::GVector3( normals[j].x, normals[j].y, normals[j].z ) ) );
					}
				}

				// tangents
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
						mesh->set_vertex_tangent_pose( j, i - 1, asf::normalize( asr::GVector3( tangents[j].x, tangents[j].y, tangents[j].z ) ) );
					}
				}
			#endif
		}
	}

	return obj;
}

string IECoreAppleseed::InteractivePrimitiveConverter::objectEntityName( const string& objectName ) const
{
	return objectName;
}
