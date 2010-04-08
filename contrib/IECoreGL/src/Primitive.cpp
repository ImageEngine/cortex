//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/Primitive.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/TextureUnits.h"
#include "IECoreGL/NumericTraits.h"
#include "IECoreGL/UniformFunctions.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/VectorTypedData.h"

#include "boost/format.hpp"

using namespace IECoreGL;
using namespace std;
using namespace boost;
using namespace Imath;

namespace IECoreGL
{

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawBound, PrimitiveBoundTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawWireframe, PrimitiveWireframeTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::WireframeWidth, PrimitiveWireframeWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawSolid, PrimitiveSolidTypeId, bool, true );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawOutline, PrimitiveOutlineTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::OutlineWidth, PrimitiveOutlineWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawPoints, PrimitivePointsTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::PointWidth, PrimitivePointWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::TransparencySort, PrimitiveTransparencySortStateComponentTypeId, bool, true );

}

IE_CORE_DEFINERUNTIMETYPED( Primitive );

Primitive::Primitive() : m_points( 0 ), m_normals( 0 ), m_colors( 0 ), m_texCoords( 0 )
{
	m_vertexToUniform.shader = 0;
	m_vertexToVertex.shader = 0;
}

Primitive::~Primitive()
{
}

void Primitive::render( ConstStatePtr state ) const
{
	if( !state->isComplete() )
	{
		throw Exception( "Primitive::render called with incomplete state object." );
	}

	Shader *shader = state->get<ShaderStateComponent>()->shader();

	// get ready in case the derived class calls setVertexAttributesAsUniforms or setVertexAttributes.
	setupVertexAttributes( shader );

	// set constant primVars on the uniform shader parameters
	for ( AttributeMap::const_iterator it = m_uniformAttributes.begin(); it != m_uniformAttributes.end(); it++ )
	{
		try
		{
			shader->setParameter( it->first, it->second );
		}
		catch( ... )
		{
		}
	}

	// \todo: consider binding at the end the whole original state. Check if that is enough to eliminate these push/pop calls.
	glPushAttrib( GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | GL_LINE_BIT | GL_LIGHTING_BIT );
	glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );

		if( depthSortRequested( state ) )
		{
			glDepthMask( false );
		}

		if( state->get<Primitive::DrawSolid>()->value() )
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glEnable( GL_LIGHTING );
			glDisable( GL_POLYGON_OFFSET_FILL );
			render( state, Primitive::DrawSolid::staticTypeId() );
		}

		glDisable( GL_LIGHTING );
		glActiveTexture( textureUnits()[0] );
		glDisable( GL_TEXTURE_2D );

		// turn off shader for other render modes.
		if( GLEW_VERSION_2_0 )
		{
			glUseProgram( 0 );
		}

			if( state->get<Primitive::DrawOutline>()->value() )
			{
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
				glEnable( GL_POLYGON_OFFSET_LINE );
				float width = 2 * state->get<Primitive::OutlineWidth>()->value();
				glPolygonOffset( 2 * width, 1 );
				glLineWidth( width );
				Color4f c = state->get<OutlineColorStateComponent>()->value();
				glColor4f( c[0], c[1], c[2], c[3] );
				render( state, Primitive::DrawOutline::staticTypeId() );
			}

			if( state->get<Primitive::DrawWireframe>()->value() )
			{
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
				float width = state->get<Primitive::WireframeWidth>()->value();
				glEnable( GL_POLYGON_OFFSET_LINE );
				glPolygonOffset( -1 * width, -1 );
				Color4f c = state->get<WireframeColorStateComponent>()->value();
				glColor4f( c[0], c[1], c[2], c[3] );
				glLineWidth( width );
				render( state, Primitive::DrawWireframe::staticTypeId() );
			}

			if( state->get<Primitive::DrawPoints>()->value() )
			{
				glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
				float width = state->get<Primitive::PointWidth>()->value();
				glEnable( GL_POLYGON_OFFSET_POINT );
				glPolygonOffset( -2 * width, -1 );
				glPointSize( width );
				Color4f c = state->get<PointColorStateComponent>()->value();
				glColor4f( c[0], c[1], c[2], c[3] );
				render( state, Primitive::DrawPoints::staticTypeId() );
			}

			if( state->get<Primitive::DrawBound>()->value() )
			{
				Box3f b = bound();
				Color4f c = state->get<BoundColorStateComponent>()->value();
				glColor4f( c[0], c[1], c[2], c[3] );
				glLineWidth( 1 );
				glBegin( GL_LINE_LOOP );
					glVertex3f( b.min.x, b.min.y, b.min.z );
					glVertex3f( b.max.x, b.min.y, b.min.z );
					glVertex3f( b.max.x, b.max.y, b.min.z );
					glVertex3f( b.min.x, b.max.y, b.min.z );
				glEnd();
				glBegin( GL_LINE_LOOP );
					glVertex3f( b.min.x, b.min.y, b.max.z );
					glVertex3f( b.max.x, b.min.y, b.max.z );
					glVertex3f( b.max.x, b.max.y, b.max.z );
					glVertex3f( b.min.x, b.max.y, b.max.z );
				glEnd();
				glBegin( GL_LINES );
					glVertex3f( b.min.x, b.min.y, b.min.z );
					glVertex3f( b.min.x, b.min.y, b.max.z );
					glVertex3f( b.max.x, b.min.y, b.min.z );
					glVertex3f( b.max.x, b.min.y, b.max.z );
					glVertex3f( b.max.x, b.max.y, b.min.z );
					glVertex3f( b.max.x, b.max.y, b.max.z );
					glVertex3f( b.min.x, b.max.y, b.min.z );
					glVertex3f( b.min.x, b.max.y, b.max.z );
				glEnd();
			}

	glPopClientAttrib();
	glPopAttrib();

	// revert to the original shader state.
	state->get<ShaderStateComponent>()->bind();
}

size_t Primitive::vertexAttributeSize() const
{
	return 0;
}

void Primitive::addUniformAttribute( const std::string &name, IECore::ConstDataPtr data )
{
	m_uniformAttributes[name] = data->copy();
}

void Primitive::addVertexAttribute( const std::string &name, IECore::ConstDataPtr data )
{
	if( !vertexAttributeSize() )
	{
		throw Exception( std::string( typeName() ) + " does not support vertex attributes." );
	}

	size_t s = IECore::despatchTypedData< IECore::TypedDataSize, IECore::TypeTraits::IsTypedData >( IECore::constPointerCast<IECore::Data>( data ) );

	size_t rightSize = vertexAttributeSize();
	if( s!=rightSize )
	{
		throw Exception( boost::str( format( "Vertex attribute \"%s\" has wrong number of elements (%d but expected %d)." ) % name % s % rightSize ) );
	}

	if ( name == "P" )
	{
		m_points = IECore::runTimeCast< const IECore::V3fVectorData >( data );
	}
	else if ( name == "Cs" )
	{
		m_colors = IECore::runTimeCast< const IECore::Color3fVectorData >( data );
	}
	else if ( name == "N" )
	{
		m_normals = IECore::runTimeCast< const IECore::V3fVectorData >( data );
	}
	else if ( name == "st" )
	{
		m_texCoords = IECore::runTimeCast< const IECore::V2fVectorData >( data );
	}

	m_vertexAttributes[name] = data->copy();
}

void Primitive::setVertexAttributes( ) const
{
	if( !m_vertexToVertex.shader )
	{
		return;
	}
	std::map<GLint, IntData>::const_iterator it;
	for( it=m_vertexToVertex.intDataMap.begin(); it!=m_vertexToVertex.intDataMap.end(); it++ )
	{
		glEnableVertexAttribArray( it->first );
		glVertexAttribPointer( it->first, it->second.dimensions, NumericTraits<GLint>::glType(), false, 0, it->second.data );
	}
	std::map<GLint, FloatData>::const_iterator fit;
	for( fit=m_vertexToVertex.floatDataMap.begin(); fit!=m_vertexToVertex.floatDataMap.end(); fit++ )
	{
		glEnableVertexAttribArray( fit->first );
		glVertexAttribPointer( fit->first, fit->second.dimensions, NumericTraits<GLfloat>::glType(), false, 0, fit->second.data );
	}
}

void Primitive::setVertexAttributesAsUniforms( unsigned int vertexIndex ) const
{
	if( !m_vertexToUniform.shader )
	{
		return;
	}
	std::map<GLint, IntData>::const_iterator it;
	for( it=m_vertexToUniform.intDataMap.begin(); it!=m_vertexToUniform.intDataMap.end(); it++ )
	{
		assert( sizeof( int )==sizeof( GLint ) );
		uniformIntFunctions()[it->second.dimensions]( it->first, 1, ((const GLint *)it->second.data) + vertexIndex * it->second.dimensions );
	}
	std::map<GLint, FloatData>::const_iterator fit;
	for( fit=m_vertexToUniform.floatDataMap.begin(); fit!=m_vertexToUniform.floatDataMap.end(); fit++ )
	{
		uniformFloatFunctions()[fit->second.dimensions]( fit->first, 1, fit->second.data + vertexIndex * fit->second.dimensions );
	}
}

void Primitive::setupVertexAttributes( const Shader *s ) const
{
	if( !s )
	{
		m_vertexToUniform.shader = 0;
		m_vertexToVertex.shader = 0;
		return;
	}

	if( s==m_vertexToUniform.shader && (*s)==(*m_vertexToUniform.shader) )
	{
		return;
	}

	m_vertexToUniform.intDataMap.clear();
	m_vertexToUniform.floatDataMap.clear();
	m_vertexToVertex.intDataMap.clear();
	m_vertexToVertex.floatDataMap.clear();

	for( AttributeMap::const_iterator it=m_vertexAttributes.begin(); it!=m_vertexAttributes.end(); it++ )
	{
		IntData intData;
		FloatData floatData;
		switch( it->second->typeId() )
		{
			case IECore::IntVectorDataTypeId :
				intData = IntData( IECore::staticPointerCast<const IECore::IntVectorData>( it->second )->baseReadable(), 1 );
				break;
			case IECore::V2iVectorDataTypeId :
				intData = IntData( IECore::staticPointerCast<const IECore::V2iVectorData>( it->second )->baseReadable(), 2 );
				break;
			case IECore::V3iVectorDataTypeId :
				intData = IntData( IECore::staticPointerCast<const IECore::V3iVectorData>( it->second )->baseReadable(), 3 );
				break;
			case IECore::FloatVectorDataTypeId :
				floatData = FloatData( IECore::staticPointerCast<const IECore::FloatVectorData>( it->second )->baseReadable(), 1 );
				break;
			case IECore::V2fVectorDataTypeId :
				floatData = FloatData( IECore::staticPointerCast<const IECore::V2fVectorData>( it->second )->baseReadable(), 2 );
				break;
			case IECore::V3fVectorDataTypeId :
				floatData = FloatData( IECore::staticPointerCast<const IECore::V3fVectorData>( it->second )->baseReadable(), 3 );
				break;
			case IECore::Color3fVectorDataTypeId :
				floatData = FloatData( IECore::staticPointerCast<const IECore::Color3fVectorData>( it->second )->baseReadable(), 3 );
				break;
			default :
				// ignore other data types...
				continue;
		}
		
		try
		{
			GLint parameterIndex = glGetAttribLocation( s->m_program, it->first.c_str() );
			if ( parameterIndex != -1 )
			{
				// vertex shader variable
				if ( floatData.data )
				{
					m_vertexToVertex.floatDataMap[ parameterIndex ] = floatData;
				}
				else
				{
					m_vertexToVertex.intDataMap[ parameterIndex ] = intData;
				}
			}
			else
			{
				// uniform shader variable
				parameterIndex = s->parameterIndex( it->first );
				if ( floatData.data )
				{
					m_vertexToUniform.floatDataMap[ parameterIndex ] = floatData;
				}
				else
				{
					m_vertexToUniform.intDataMap[ parameterIndex ] = intData;
				}
			}
		}
		catch( ... )
		{
		}
	}

	m_vertexToUniform.shader = s;
	m_vertexToVertex.shader = s;
}

bool Primitive::depthSortRequested( ConstStatePtr state ) const
{
	return state->get<Primitive::TransparencySort>()->value() &&
		state->get<TransparentShadingStateComponent>()->value();
}
