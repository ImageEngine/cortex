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
	m_shaderSetup = 0;
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
			shader->setUniformParameter( it->first, it->second );
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

	if ( shader )
	{
		// disable all vertex shader parameters
		shader->unsetVertexParameters();
	}
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
	if( !m_shaderSetup )
	{
		return;
	}

	// \todo: consider getting requiredSize as a parameter or filtering them while creating m_vertexMap.
	size_t requiredSize = vertexAttributeSize();

	VertexDataMap::const_iterator it;
	for( it=m_vertexMap.begin(); it!=m_vertexMap.end(); it++ )
	{
		if ( boost::get<1>(it->second) == requiredSize )
		{
			m_shaderSetup->setVertexParameter( it->first, boost::get<0>(it->second) );
		}
	}
}

void Primitive::setVertexAttributesAsUniforms( unsigned int vertexIndex ) const
{
	if( !m_shaderSetup )
	{
		return;
	}
	UniformDataMap::const_iterator it;
	for( it=m_uniformMap.begin(); it!=m_uniformMap.end(); it++ )
	{
		m_shaderSetup->setUniformParameterFromVector( it->first, it->second, vertexIndex );
	}
}

void Primitive::setupVertexAttributes( ShaderPtr s ) const
{
	if( !s )
	{
		m_shaderSetup = 0;
		return;
	}

	if( m_shaderSetup && (*s)==(*m_shaderSetup) )
	{
		return;
	}

	m_uniformMap.clear();
	m_vertexMap.clear();
	IECore::TypedDataSize size;

	for( AttributeMap::const_iterator it=m_vertexAttributes.begin(); it!=m_vertexAttributes.end(); it++ )
	{
		if ( s->hasVertexParameter( it->first ) )
		{
			// vertex shader variable
			GLint parameterIndex = s->vertexParameterIndex( it->first );
			if ( s->vertexValueValid( parameterIndex, it->second ) )
			{
				m_vertexMap[ parameterIndex ] = 
					boost::tuple< IECore::ConstDataPtr, size_t >(
						it->second,
						IECore::despatchTypedData< IECore::TypedDataSize, IECore::TypeTraits::IsVectorTypedData >( IECore::constPointerCast<IECore::Data>( it->second ), size )
					);
			}
		}
		else if ( s->hasUniformParameter( it->first ) )
		{
			// uniform shader variable
			GLint parameterIndex = s->uniformParameterIndex( it->first );
			if ( s->uniformVectorValueValid( parameterIndex, it->second ) )
			{
				m_uniformMap[ parameterIndex ] = it->second;
			}
		}
	}
	m_shaderSetup = s;
}

bool Primitive::depthSortRequested( ConstStatePtr state ) const
{
	return state->get<Primitive::TransparencySort>()->value() &&
		state->get<TransparentShadingStateComponent>()->value();
}
