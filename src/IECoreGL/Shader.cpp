//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#include <vector>
#include <iostream>

#include "boost/format.hpp"

#include "IECore/SimpleTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

#include "IECoreGL/Shader.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/Texture.h"
#include "IECoreGL/UniformFunctions.h"
#include "IECoreGL/Buffer.h"
#include "IECoreGL/NumericTraits.h"
#include "IECoreGL/CachedConverter.h"

using namespace std;
using namespace boost;
using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( Shader );

Shader::Shader( const std::string &vertexSource, const std::string &fragmentSource )
	:	m_vertexShader( 0 ), m_fragmentShader( 0 ), m_program( 0 )
{
	if( !GLEW_VERSION_2_0 )
	{
		throw Exception( "OpenGL version < 2" );
	}

	compile( vertexSource, GL_VERTEX_SHADER, m_vertexShader );
	compile( fragmentSource, GL_FRAGMENT_SHADER, m_fragmentShader );

	m_program = glCreateProgram();
	if( m_vertexShader )
	{
		glAttachShader( m_program, m_vertexShader );
	}
	if( m_fragmentShader )
	{
		glAttachShader( m_program, m_fragmentShader );
	}

	glLinkProgram( m_program );
	GLint linkStatus = 0;
	glGetProgramiv( m_program, GL_LINK_STATUS, &linkStatus );
	GLint logLength = 0;
	glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &logLength );
	if( !linkStatus )
	{
		std::string message = "Unknown linking error.";
		if( logLength )
		{
			vector<char> log( logLength );
			glGetProgramInfoLog( m_program, logLength, 0, &log[0] );
			message = &log[0];
		}
		release();
		throw Exception( message );
	}
	else if ( logLength > 1 )
	{
		std::string message;
		vector<char> log( logLength, ' ' );
		glGetProgramInfoLog( m_program, logLength, 0, &log[0] );
		message = &log[0];
		IECore::msg( IECore::Msg::Warning, "IECoreGL::Shader", message );
	}
	{
		// build the uniform parameter description map
		GLint numUniforms = 0;
		glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &numUniforms );
		GLint maxUniformNameLength = 0;
		glGetProgramiv( m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength );
		vector<char> name( maxUniformNameLength );
		for( int i=0; i<numUniforms; i++ )
		{
			ParameterDescription d;
			glGetActiveUniform( m_program, i, maxUniformNameLength, 0, &d.size, &d.type, &name[0] );
			d.name = &name[0];
			GLint location = glGetUniformLocation( m_program, &name[0] );

			// ignore native parameters
			if( 0 == d.name.compare( 0, 3, "gl_" ) )
			{
				continue;
			}
			
			if( d.size > 1 )
			{
				// remove the "[0]" from the end of the string
				size_t bracketPos = d.name.rfind( "[" );
				if( bracketPos != std::string::npos )
				{
					d.name = d.name.substr( 0, bracketPos );
				}
			}
			
			m_uniformParameters[location] = d;
		}
	}

	{
		// build the vertex parameter description map
		GLint numVertexs = 0;
		glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTES, &numVertexs );
		GLint maxVertexNameLength = 0;
		glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxVertexNameLength );
		
		// some versions of the nvidia drivers are returning maxVertexNameLength==0 when
		// no attributes other than built in gl_* ones are defined by the shader. we don't
		// bother retrieving anything in this case, as we skip built in parameters anyway.
		if( numVertexs && maxVertexNameLength )
		{
			vector<char> name( maxVertexNameLength );
			for( int i=0; i<numVertexs; i++ )
			{
				ParameterDescription d;
				glGetActiveAttrib( m_program, i, maxVertexNameLength, 0, &d.size, &d.type, &name[0] );
				d.name = &name[0];
				GLint location = glGetAttribLocation( m_program, &name[0] );

				// ignore native parameters
				if( 0 == d.name.compare( 0, 3, "gl_" ) )
				{
					continue;
				}

				// \todo: implement arrays
				if ( d.size != 1 )
					continue;

				m_vertexParameters[location] = d;
			}
		}
	}
}

Shader::~Shader()
{
	release();
}

bool Shader::operator==( const Shader &other ) const
{
	return m_program == other.m_program;
}

void Shader::bind() const
{
	glUseProgram( m_program );
}

void Shader::compile( const std::string &source, GLenum type, GLuint &shader )
{

	if( source!="" )
	{
		const char *s = source.c_str();
		shader = glCreateShader( type );
		glShaderSource( shader, 1, &s, 0 );
		glCompileShader( shader );
		GLint compileStatus = 0;
		glGetShaderiv( shader, GL_COMPILE_STATUS, &compileStatus );
		GLint logLength = 0;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );
		if( !compileStatus )
		{			
			std::string message = "Unknown compilation error.";
			if( logLength )
			{
				vector<char> log( logLength, ' ' );
				GLsizei l;
				glGetShaderInfoLog( shader, logLength, &l, &log[0] );
				message = &log[0];
			}
			release();
			throw Exception( message );
		}
		else if ( logLength > 1 )
		{
			std::string message;
			vector<char> log( logLength, ' ' );
			GLsizei l;
			glGetShaderInfoLog( shader, logLength, &l, &log[0] );
			message = &log[0];
			IECore::msg( IECore::Msg::Warning, "IECoreGL::Shader", message );
		}				
	}

}

void Shader::release()
{
	if ( m_program )
	{
		glDeleteShader( m_vertexShader );
		glDeleteShader( m_fragmentShader );
		glDeleteProgram( m_program );
	}
}

///////////////////////////////////////////////////////////////////////////////
// functions for uniform parameters
///////////////////////////////////////////////////////////////////////////////


void Shader::uniformParameterNames( std::vector<std::string> &names ) const
{
	for( ParameterMap::const_iterator it = m_uniformParameters.begin(); it != m_uniformParameters.end(); it++ )
	{
		names.push_back( it->second.name );
	}
}

GLint Shader::uniformParameterIndex( const std::string &parameterName ) const
{
	for( ParameterMap::const_iterator it = m_uniformParameters.begin(); it != m_uniformParameters.end(); it++ )
	{
		if( !strcmp( parameterName.c_str(), it->second.name.c_str() ) )
		{
			return it->first;
		}
	}

	throw( Exception( boost::str( boost::format( "No uniform parameter named \"%s\"." ) % parameterName ) ) );
}

bool Shader::hasUniformParameter( const std::string &parameterName ) const
{
	for( ParameterMap::const_iterator it = m_uniformParameters.begin(); it != m_uniformParameters.end(); it++ )
	{
		if( !strcmp( parameterName.c_str(), it->second.name.c_str() ) )
		{
			return true;
		}
	}

	return false;
}

IECore::TypeId Shader::uniformParameterType( GLint parameterIndex ) const
{
	const ParameterDescription &p = uniformParameterDescription( parameterIndex );
	if( p.size==1 )
	{
		switch( p.type )
		{
			case GL_BOOL :
				return IECore::BoolDataTypeId;

			case GL_INT :
				return IECore::IntDataTypeId;

			case GL_FLOAT :
				return IECore::FloatDataTypeId;

			case GL_BOOL_VEC2 :
				return IECore::V2iDataTypeId;

			case GL_INT_VEC2 :
				return IECore::V2iDataTypeId;

			case GL_FLOAT_VEC2 :
				return IECore::V2fDataTypeId;

			case GL_BOOL_VEC3 :
				return IECore::V3iDataTypeId;

			case GL_INT_VEC3 :
				return IECore::V3iDataTypeId;

			case GL_FLOAT_VEC3 :
				return IECore::V3fDataTypeId;

			case GL_FLOAT_VEC4 :
				return IECore::Color4fDataTypeId;

			case GL_SAMPLER_2D :
				return Texture::staticTypeId();

			case GL_FLOAT_MAT3 :
				return IECore::M33fDataTypeId;

			case GL_FLOAT_MAT4 :
				return IECore::M44fDataTypeId;

			default :
				throw Exception( "Unsupported uniform parameter type." );
		}
	}
	else
	{
		switch( p.type )
		{
			// TODO: handle GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3
			case GL_INT :
				return IECore::IntVectorDataTypeId;

			case GL_FLOAT :
				return IECore::FloatVectorDataTypeId;

			case GL_INT_VEC2 :
				return IECore::V2iVectorDataTypeId;

			case GL_FLOAT_VEC2 :
				return IECore::V2fVectorDataTypeId;

			case GL_INT_VEC3 :
				return IECore::V3iVectorDataTypeId;

			case GL_FLOAT_VEC3 :
				return IECore::V3fVectorDataTypeId;

			case GL_FLOAT_VEC4 :
				return IECore::Color4fVectorDataTypeId;

			case GL_FLOAT_MAT3 :
				return IECore::M33fVectorDataTypeId;

			case GL_FLOAT_MAT4 :
				return IECore::M44fVectorDataTypeId;

			default :
				throw Exception( "Unsupported uniform parameter type." );
		}
	}
}

IECore::TypeId Shader::uniformParameterType( const std::string &parameterName ) const
{
	return uniformParameterType( uniformParameterIndex( parameterName ) );
}

IECore::DataPtr Shader::getUniformParameterDefault( const std::string &parameterName ) const
{
	return getUniformParameterDefault( uniformParameterIndex( parameterName ) );
}

IECore::DataPtr Shader::getUniformParameterDefault( GLint parameterIndex ) const
{
	const ParameterDescription &p = uniformParameterDescription( parameterIndex );
	if( p.size==1 )
	{
		switch( p.type )
		{
			case GL_BOOL :
				{
					return new IECore::BoolData( 0 );
				}
			case GL_INT :
				{
					return new IECore::IntData( 0 );
				}
			case GL_FLOAT :
				{
					return new IECore::FloatData( 0 );
				}
			case GL_BOOL_VEC2 :
				{
					return new IECore::V2iData( Imath::V2i( 0 ) );
				}
			case GL_INT_VEC2 :
				{
					return new IECore::V2iData( Imath::V2i( 0 ) );
				}
			case GL_FLOAT_VEC2 :
				{
					return new IECore::V2fData( Imath::V2f( 0.0f ) );
				}
			case GL_BOOL_VEC3 :
				{
					return new IECore::V3iData( Imath::V3i( 0 ) );
				}
			case GL_INT_VEC3 :
				{
					return new IECore::V3iData( Imath::V3i( 0 ) );
				}
			case GL_FLOAT_VEC3 :
				{
					return new IECore::V3fData( Imath::V3f( 0.0f ) );
				}
			case GL_FLOAT_VEC4 :
				{
					return new IECore::Color4fData( Imath::Color4f( 0.0f ) );
				}
			case GL_FLOAT_MAT3 :
				{
					return new IECore::M33fData( Imath::M33f( 0.0f ) );
				}
			case GL_FLOAT_MAT4 :
				{
					return new IECore::M44fData( Imath::M44f( 0.0f ) );
				}
			default :
				throw Exception( "Unsupported uniform parameter type." );
		}
	}
	else
	{
		switch( p.type )
		{
			// TODO: GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3
			case GL_INT :
				{
					IECore::IntVectorDataPtr data = new IECore::IntVectorData();
					data->writable().resize( p.size, 0 );
					return data;
				}
			case GL_FLOAT :
				{
					IECore::FloatVectorDataPtr data = new IECore::FloatVectorData();
					data->writable().resize( p.size, 0.0f );
					return data;
				}
			case GL_INT_VEC2 :
				{
					IECore::V2iVectorDataPtr data = new IECore::V2iVectorData();
					data->writable().resize( p.size, Imath::V2i( 0 ) );
					return data;
				}
			case GL_FLOAT_VEC2 :
				{
					IECore::V2fVectorDataPtr data = new IECore::V2fVectorData();
					data->writable().resize( p.size, Imath::V2f( 0.0f ) );
					return data;
				}
			case GL_INT_VEC3 :
				{
					IECore::V3iVectorDataPtr data = new IECore::V3iVectorData();
					data->writable().resize( p.size, Imath::V3i( 0 ) );
					return data;
				}
			case GL_FLOAT_VEC3 :
				{
					IECore::V3fVectorDataPtr data = new IECore::V3fVectorData();
					data->writable().resize( p.size, Imath::V3f( 0 ) );
					return data;
				}
			case GL_FLOAT_VEC4 :
				{
					IECore::Color4fVectorDataPtr data = new IECore::Color4fVectorData();
					data->writable().resize( p.size, Imath::Color4f( 0.0 ) );
					return data;
				}
			case GL_FLOAT_MAT3 :
				{
					IECore::M33fVectorDataPtr data = new IECore::M33fVectorData();
					data->writable().resize( p.size, Imath::M33f( 0.0 ) );
					return data;
				}
			case GL_FLOAT_MAT4 :
				{
					IECore::M44fVectorDataPtr data = new IECore::M44fVectorData();
					data->writable().resize( p.size, Imath::M44f( 0.0 ) );
					return data;
				}
			default :
				throw Exception( "Unsupported uniform parameter type." );
		}
	}
}

IECore::DataPtr Shader::getUniformParameter( GLint parameterIndex ) const
{
	const ParameterDescription &p = uniformParameterDescription( parameterIndex );
	
	if( p.size==1 )
	{
		switch( p.type )
		{
			case GL_BOOL :
				{
					GLint v = 0;
					glGetUniformiv( m_program, parameterIndex, &v );
					return new IECore::BoolData( v );
				}
			case GL_INT :
				{
					GLint v = 0;
					glGetUniformiv( m_program, parameterIndex, &v );
					return new IECore::IntData( v );
				}
			case GL_FLOAT :
				{
					IECore::FloatDataPtr result = new IECore::FloatData;
					glGetUniformfv( m_program, parameterIndex, &result->writable() );
					return result;
				}
			case GL_BOOL_VEC2 :
				{
					GLint v[2];
					glGetUniformiv( m_program, parameterIndex, v );
					return new IECore::V2iData( Imath::V2i( v[0], v[1] ) );
				}
			case GL_INT_VEC2 :
				{
					GLint v[2];
					glGetUniformiv( m_program, parameterIndex, v );
					return new IECore::V2iData( Imath::V2i( v[0], v[1] ) );
				}
			case GL_FLOAT_VEC2 :
				{
					IECore::V2fDataPtr result = new IECore::V2fData;
					glGetUniformfv( m_program, parameterIndex, result->writable().getValue() );
					return result;
				}
			case GL_BOOL_VEC3 :
				{
					GLint v[3];
					glGetUniformiv( m_program, parameterIndex, v );
					return new IECore::V3iData( Imath::V3i( v[0], v[1], v[2] ) );
				}
			case GL_INT_VEC3 :
				{
					GLint v[3];
					glGetUniformiv( m_program, parameterIndex, v );
					return new IECore::V3iData( Imath::V3i( v[0], v[1], v[2] ) );
				}
			case GL_FLOAT_VEC3 :
				{
					IECore::V3fDataPtr result = new IECore::V3fData;
					glGetUniformfv( m_program, parameterIndex, result->writable().getValue() );
					return result;
				}
			case GL_FLOAT_VEC4 :
				{
					IECore::Color4fDataPtr result = new IECore::Color4fData;
					glGetUniformfv( m_program, parameterIndex, result->writable().getValue() );
					return result;
				}
			case GL_FLOAT_MAT3 :
				{
					IECore::M33fDataPtr result = new IECore::M33fData;
					glGetUniformfv( m_program, parameterIndex, result->writable().getValue() );
					return result;
				}
			case GL_FLOAT_MAT4 :
				{
					IECore::M44fDataPtr result = new IECore::M44fData;
					glGetUniformfv( m_program, parameterIndex, result->writable().getValue() );
					return result;
				}
			default :
				throw Exception( "Unsupported uniform parameter type." );
		}
	}
	else
	{
		switch( p.type )
		{
			// TODO: Handle the case for GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3
			case GL_INT :
				{
					IECore::IntVectorDataPtr result = new IECore::IntVectorData;
					std::vector<int>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLint v = 0;
						glGetUniformiv( m_program, parameterIndex + i, &v );
						values[i] = v;
					}
					
					return result;
				}
			case GL_FLOAT :
				{
					IECore::FloatVectorDataPtr result = new IECore::FloatVectorData;
					std::vector<float>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLfloat v = 0;
						glGetUniformfv( m_program, parameterIndex + i, &v );
						values[i] = v;
					}
					
					return result;
				}
			case GL_INT_VEC2 :
				{
					IECore::V2iVectorDataPtr result = new IECore::V2iVectorData;
					std::vector<Imath::V2i>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLint v[2];
						glGetUniformiv( m_program, parameterIndex + i, v );
						values[i].x = v[0];
						values[i].y = v[1];
					}
					
					return result;
				}
			case GL_FLOAT_VEC2 :
				{
					IECore::V2fVectorDataPtr result = new IECore::V2fVectorData;
					std::vector<Imath::V2f>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLfloat v[2];
						glGetUniformfv( m_program, parameterIndex + i, v );
						values[i].x = v[0];
						values[i].y = v[1];
					}
					
					return result;
				}
			case GL_INT_VEC3 :
				{
					IECore::V3iVectorDataPtr result = new IECore::V3iVectorData;
					std::vector<Imath::V3i>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLint v[3];
						glGetUniformiv( m_program, parameterIndex + i, v );
						values[i].x = v[0];
						values[i].y = v[1];
						values[i].z = v[2];
					}
					
					return result;
				}
			case GL_FLOAT_VEC3 :
				{
					IECore::V3fVectorDataPtr result = new IECore::V3fVectorData;
					std::vector<Imath::V3f>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLfloat v[3];
						glGetUniformfv( m_program, parameterIndex + i, v );
						values[i].x = v[0];
						values[i].y = v[1];
						values[i].z = v[2];
					}
					
					return result;
				}
			case GL_FLOAT_VEC4 :
				{
					IECore::Color4fVectorDataPtr result = new IECore::Color4fVectorData;
					std::vector<Imath::Color4f>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLfloat v[4];
						glGetUniformfv( m_program, parameterIndex + i, v );
						values[i].r = v[0];
						values[i].g = v[1];
						values[i].b = v[2];
						values[i].a = v[3];
					}
					
					return result;
				}
			case GL_FLOAT_MAT3 :
				{
					IECore::M33fVectorDataPtr result = new IECore::M33fVectorData;
					std::vector<Imath::M33f>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLfloat v[9];
						glGetUniformfv( m_program, parameterIndex + i, v );
						values[i][0][0] = v[0];
						values[i][0][1] = v[1];
						values[i][0][2] = v[2];
						values[i][1][0] = v[3];
						values[i][1][1] = v[4];
						values[i][1][2] = v[5];
						values[i][2][0] = v[6];
						values[i][2][1] = v[7];
						values[i][2][2] = v[8];
					}
					
					return result;
				}
			case GL_FLOAT_MAT4 :
				{
					IECore::M44fVectorDataPtr result = new IECore::M44fVectorData;
					std::vector<Imath::M44f>& values = result->writable();
					values.resize( p.size );
					for( int i=0; i < p.size; ++i )
					{
						GLfloat v[16];
						glGetUniformfv( m_program, parameterIndex + i, v );
						values[i][0][0] = v[0];
						values[i][0][1] = v[1];
						values[i][0][2] = v[2];
						values[i][0][3] = v[3];
						
						values[i][1][0] = v[4];
						values[i][1][1] = v[5];
						values[i][1][2] = v[6];
						values[i][1][3] = v[7];
						
						values[i][2][0] = v[8];
						values[i][2][1] = v[9];
						values[i][2][2] = v[10];
						values[i][2][3] = v[11];
						
						values[i][3][0] = v[12];
						values[i][3][1] = v[13];
						values[i][3][2] = v[14];
						values[i][3][3] = v[15];
					}
					
					return result;
				}
			default :
				throw Exception( "Unsupported uniform parameter type." );
		}
	}
}

IECore::DataPtr Shader::getUniformParameter( const std::string &parameterName ) const
{
	return getUniformParameter( uniformParameterIndex( parameterName ) );
}

bool Shader::uniformValueValid( GLint parameterIndex, IECore::TypeId type ) const
{
	IECore::TypeId pt = uniformParameterType( parameterIndex );

	if( pt==Texture::staticTypeId() )
	{
		return false;
	}
	if( type==IECore::IntDataTypeId && pt == IECore::BoolDataTypeId )
	{
		type = IECore::BoolDataTypeId;
	}
	if( type==IECore::BoolDataTypeId && pt!=IECore::BoolDataTypeId )
	{
		type = IECore::IntDataTypeId;
	}
	else if( type==IECore::Color3fDataTypeId )
	{
		type = IECore::V3fDataTypeId;
	}
	else if( type==IECore::Color3fVectorDataTypeId )
	{
		type = IECore::V3fVectorDataTypeId;
	}
	return type==pt;
}

bool Shader::uniformValueValid( GLint parameterIndex, const IECore::Data *value ) const
{
	return uniformValueValid( parameterIndex, value->typeId() );
}

bool Shader::uniformValueValid( const std::string &parameterName, const IECore::Data *value ) const
{
	return uniformValueValid( uniformParameterIndex( parameterName ), value );
}

size_t Shader::getDataSize( const IECore::Data* data )
{
	switch( data->typeId() )
	{
		case IECore::IntVectorDataTypeId :
			return IECore::runTimeCast< const IECore::IntVectorData >( data )->readable().size();
		case IECore::FloatVectorDataTypeId :
			return IECore::runTimeCast< const IECore::FloatVectorData >( data )->readable().size();
		case IECore::V2fVectorDataTypeId :
			return IECore::runTimeCast< const IECore::V2fVectorData >( data )->readable().size();
		case IECore::V2iVectorDataTypeId :
			return IECore::runTimeCast< const IECore::V2iVectorData >( data )->readable().size();
		case IECore::V3fVectorDataTypeId :
			return IECore::runTimeCast< const IECore::V3fVectorData >( data )->readable().size();
		case IECore::V3iVectorDataTypeId :
			return IECore::runTimeCast< const IECore::V3iVectorData >( data )->readable().size();
		case IECore::Color3fVectorDataTypeId :
			return IECore::runTimeCast< const IECore::Color3fVectorData >( data )->readable().size();
		case IECore::Color4fVectorDataTypeId :
			return IECore::runTimeCast< const IECore::Color4fVectorData >( data )->readable().size();
		case IECore::M33fVectorDataTypeId :
			return IECore::runTimeCast< const IECore::M33fVectorData >( data )->readable().size();
		case IECore::M44fVectorDataTypeId :
			return IECore::runTimeCast< const IECore::M44fVectorData >( data )->readable().size();
		default :
			return 1;
	}

}

void Shader::setUniformParameter( GLint parameterIndex, const IECore::Data *value )
{
	if ( !uniformValueValid( parameterIndex, value ) )
	{
		throw Exception( "Can't set uniform parameter value. Type mismatch." );
	}
	
	IECore::TypedDataAddress a;
	setUniformParameter( 
		parameterIndex, 
		value->typeId(), 
		IECore::despatchTypedData< IECore::TypedDataAddress, IECore::TypeTraits::IsTypedData >( const_cast<IECore::Data *>( value ), a )
	);
}

void Shader::setUniformParameter( GLint parameterIndex, IECore::TypeId type, const void *p )
{	
	const ParameterDescription& pd = uniformParameterDescription( parameterIndex );
	
	switch( type )
	{
		case IECore::BoolDataTypeId :
			glUniform1i( parameterIndex, *static_cast<const bool*>(p) );
			break;
		case IECore::IntDataTypeId :
			glUniform1i( parameterIndex, *static_cast<const int*>(p) );
			break;
		case IECore::FloatDataTypeId :
			glUniform1f( parameterIndex, *static_cast<const float*>(p) );
			break;
		case IECore::V2fDataTypeId :
			glUniform2fv( parameterIndex, 1, static_cast<const float *>(p) );
			break;
		case IECore::V2iDataTypeId :
			{
				const int *vv = static_cast<const int *>(p);
				GLint v[2]; v[0] = vv[0]; v[1] = vv[1];
				glUniform2iv( parameterIndex, 1, v );
			}
			break;
		case IECore::V3fDataTypeId :
			glUniform3fv( parameterIndex, 1, static_cast<const float *>(p) );
			break;
		case IECore::V3iDataTypeId :
			{
				const int *vv = static_cast<const int *>(p);
				GLint v[3]; v[0] = vv[0]; v[1] = vv[1]; v[2] = vv[2];
				glUniform3iv( parameterIndex, 1, v );
			}
			break;
		case IECore::Color3fDataTypeId :
			glUniform3fv( parameterIndex, 1, static_cast<const float *>(p) );
			break;
		case IECore::Color4fDataTypeId :
			glUniform4fv( parameterIndex, 1, static_cast<const float *>(p) );
			break;
		case IECore::M33fDataTypeId :
			glUniformMatrix3fv( parameterIndex, 1, GL_FALSE, static_cast<const float *>(p) );
			break;
		case IECore::M44fDataTypeId :
			glUniformMatrix4fv( parameterIndex, 1, GL_FALSE, static_cast<const float *>(p) );
			break;
			
		// TODO: Handle the case for BoolVectorDataTypeId
		
		case IECore::IntVectorDataTypeId :
			glUniform1iv( parameterIndex, pd.size, static_cast<const GLint *>(p) );
			break;
		case IECore::FloatVectorDataTypeId :
			glUniform1fv( parameterIndex, pd.size, static_cast<const GLfloat *>(p) );
			break;
		case IECore::V2fVectorDataTypeId :
			glUniform2fv( parameterIndex, pd.size, static_cast<const GLfloat *>(p) );
			break;
		case IECore::V2iVectorDataTypeId :
			{
				const Imath::V2i* incomingData = static_cast<const Imath::V2i *>(p);
				std::vector<GLint> rawData( 2 * pd.size );
				for( int i=0; i < pd.size; ++i )
				{
					rawData[2 * i]     = incomingData[i].x;
					rawData[2 * i + 1] = incomingData[i].y;
				}
				
				glUniform2iv( parameterIndex, pd.size, &rawData.front() );
			}
			break;
		case IECore::V3fVectorDataTypeId :
			glUniform3fv( parameterIndex, pd.size, static_cast<const float *>(p) );
			break;
		case IECore::V3iVectorDataTypeId :
			{
				const Imath::V3i* incomingData = static_cast<const Imath::V3i *>(p);
				std::vector<GLint> rawData( 3 * pd.size );
				for( int i=0; i < pd.size; ++i )
				{
					rawData[3 * i]     = incomingData[i].x;
					rawData[3 * i + 1] = incomingData[i].y;
					rawData[3 * i + 2] = incomingData[i].z;
				}
				
				glUniform3iv( parameterIndex, pd.size, &rawData.front() );
			}
			break;
		case IECore::Color3fVectorDataTypeId :
			glUniform3fv( parameterIndex, pd.size, static_cast<const float *>(p) );
			break;
		case IECore::Color4fVectorDataTypeId :
			glUniform4fv( parameterIndex, pd.size, static_cast<const float *>(p) );
			break;
		case IECore::M33fVectorDataTypeId :
			glUniformMatrix3fv( parameterIndex, pd.size, GL_FALSE, static_cast<const float *>(p) );
			break;
		case IECore::M44fVectorDataTypeId :
			glUniformMatrix4fv( parameterIndex, pd.size, GL_FALSE, static_cast<const float *>(p) );
			break;
		default :
			throw Exception( boost::str( boost::format( "Unsupported uniform parameter type \"%s\"." ) % IECore::RunTimeTyped::typeNameFromTypeId( type ) ) );
	}
	/// \todo Might it be quicker to check the gl type ourselves beforehand rather than checking
	/// for errors here?
	Exception::throwIfError();
}

void Shader::setUniformParameter( const std::string &parameterName, const IECore::Data *value )
{
	setUniformParameter( uniformParameterIndex( parameterName ), value );
}

void Shader::setUniformParameter( GLint parameterIndex, unsigned int textureUnit )
{
	glUniform1i( parameterIndex, textureUnit );
	/// \todo Might it be quicker to check the gl type ourselves beforehand rather than checking
	/// for errors here?
	Exception::throwIfError();
}

void Shader::setUniformParameter( const std::string &parameterName, unsigned int textureUnit )
{
	setUniformParameter( uniformParameterIndex( parameterName ), textureUnit );
}

void Shader::setUniformParameter( GLint parameterIndex, int value )
{
	glUniform1i( parameterIndex, value );
	Exception::throwIfError();
}

void Shader::setUniformParameter( const std::string &parameterName, int value )
{
	setUniformParameter( uniformParameterIndex( parameterName ), value );
}

struct Shader::VectorValueValid
{
	template< typename T > 
	struct IsIndexableVectorTypedData : boost::mpl::and_< IECore::TypeTraits::IsVectorTypedData<T>, boost::mpl::not_< boost::is_same< typename IECore::TypeTraits::VectorValueType<T>::type, bool > > > {};

	typedef const bool ReturnType;

	const Shader *m_shader;
	GLint m_paramIndex;

	VectorValueValid( const Shader *s, GLint paramIndex ) : m_shader(s), m_paramIndex( paramIndex )
	{
	}

	template<typename T>
	ReturnType operator() ( typename T::Ptr data )
	{
		if ( !data->readable().size() )
		{
			return false;
		}
		return m_shader->uniformValueValid( m_paramIndex, IECore::TypedData< typename T::ValueType::value_type >::staticTypeId() );
	}
};

bool Shader::uniformVectorValueValid( GLint parameterIndex, const IECore::Data *value ) const
{
	VectorValueValid valueValid( this, parameterIndex );
	return IECore::despatchTypedData< VectorValueValid, VectorValueValid::IsIndexableVectorTypedData >( const_cast<IECore::Data *>( value ), valueValid ); 
}

bool Shader::uniformVectorValueValid( const std::string &parameterName, const IECore::Data *value ) const
{
	return uniformVectorValueValid( uniformParameterIndex(parameterName), value );
}

struct Shader::VectorSetValue
{

	typedef const void ReturnType;

	Shader *m_shader;
	GLint m_paramIndex;
	unsigned m_vectorItem;

	VectorSetValue( Shader *s, GLint paramIndex, unsigned vectorItem ) : m_shader(s), m_paramIndex( paramIndex ), m_vectorItem( vectorItem )
	{
	}

	template<typename T>
	ReturnType operator() ( typename T::Ptr data )
	{
		m_shader->setUniformParameter( m_paramIndex, IECore::TypedData< typename T::ValueType::value_type >::staticTypeId(), &(data->readable()[ m_vectorItem ]) );
	}
};

void Shader::setUniformParameterFromVector( GLint parameterIndex, const IECore::Data *vector, unsigned int item )
{
	if ( !uniformVectorValueValid( parameterIndex, vector ) )
	{
		throw Exception( "Can't set uniform parameter value from vector. Type mismatch." );
	}
	VectorSetValue setValue( this, parameterIndex, item );
	IECore::despatchTypedData< VectorSetValue, VectorValueValid::IsIndexableVectorTypedData >( const_cast<IECore::Data *>( vector ), setValue );
}

void Shader::setUniformParameterFromVector( const std::string &parameterName, const IECore::Data *vector, unsigned int item )
{
	setUniformParameterFromVector( uniformParameterIndex( parameterName ), vector, item );
}

struct Shader::VectorSetup
{
	typedef VertexToUniform ReturnType;

	GLint m_paramIndex;

	VectorSetup( GLint paramIndex ) : m_paramIndex( paramIndex )
	{
	}

	template<typename T>
	ReturnType operator() ( typename T::Ptr data )
	{
		bool isInteger;
		if ( boost::is_same< float, typename T::BaseType >::value )
		{
			isInteger = false;
		}
		else if ( boost::is_same< int, typename T::BaseType >::value || 
					boost::is_same< unsigned int, typename T::BaseType >::value )
		{
			isInteger = true;
		}
		else
		{
			throw Exception( "Invalid vertex data type. Only float or int vectors accepted." );
		}
		return ReturnType( m_paramIndex,
							data->baseSize() / data->readable().size(),
							isInteger,
							data->baseReadable() );
	}
};

Shader::VertexToUniform Shader::uniformParameterFromVectorSetup( GLint parameterIndex, const IECore::Data *vector ) const
{
	VectorSetup setup( parameterIndex );
	return IECore::despatchTypedData< VectorSetup, IECore::TypeTraits::IsVectorTypedData >( const_cast<IECore::Data *>( vector ), setup );
}

Shader::VertexToUniform::VertexToUniform( ) :
	m_paramId(0), m_dimensions(0), m_isInteger(0), m_array(0)
{
}

Shader::VertexToUniform::VertexToUniform( GLint p, unsigned char d, bool i, const void *a ) :
	m_paramId(p), m_dimensions(d), m_isInteger(i), m_array(a)
{
}

void Shader::VertexToUniform::operator() ( int index ) const
{
	if ( m_isInteger )
	{
		uniformIntFunctions()[m_dimensions]( m_paramId, 1, ((const GLint *)m_array) + index * (int)m_dimensions );
	}
	else
	{
		uniformFloatFunctions()[m_dimensions]( m_paramId, 1, ((const float *)m_array) + index * (int)m_dimensions );
	}
}

const Shader::ParameterDescription &Shader::uniformParameterDescription( GLint parameterIndex ) const
{
	ParameterMap::const_iterator it = m_uniformParameters.find( parameterIndex );
	if( it==m_uniformParameters.end() )
	{
		throw Exception( "Uniform parameter doesn't exist." );
	}
	return it->second;
}

///////////////////////////////////////////////////////////////////////////////
// functions for vertex parameters
///////////////////////////////////////////////////////////////////////////////

void Shader::vertexParameterNames( std::vector<std::string> &names ) const
{
	for( ParameterMap::const_iterator it = m_vertexParameters.begin(); it != m_vertexParameters.end(); it++ )
	{
		names.push_back( it->second.name );
	}
}
/*
GLint Shader::vertexParameterIndex( const std::string &parameterName ) const
{
	for( ParameterMap::const_iterator it = m_vertexParameters.begin(); it != m_vertexParameters.end(); it++ )
	{
		if( !strcmp( parameterName.c_str(), it->second.name.c_str() ) )
		{
			return it->first;
		}
	}

	throw( Exception( boost::str( boost::format( "No vertex parameter named \"%s\"." ) % parameterName ) ) );
}

bool Shader::hasVertexParameter( const std::string &parameterName ) const
{
	for( ParameterMap::const_iterator it = m_vertexParameters.begin(); it != m_vertexParameters.end(); it++ )
	{
		if( !strcmp( parameterName.c_str(), it->second.name.c_str() ) )
		{
			return true;
		}
	}

	return false;
}

bool Shader::vertexValueValid( GLint parameterIndex, const IECore::Data *value ) const
{	
	IECore::TypeId t = value->typeId();

	const ParameterDescription &p = vertexParameterDescription( parameterIndex );
	if( p.size==1 )
	{
		switch( p.type )
		{
			case GL_FLOAT :

				if ( t == IECore::FloatVectorDataTypeId || t == IECore::DoubleVectorDataTypeId  || 
						t == IECore::UCharVectorDataTypeId || t == IECore::UCharVectorDataTypeId )
					return true;

				if ( (t == IECore::IntVectorDataTypeId || t == IECore::UIntVectorDataTypeId ) && sizeof(int) == sizeof(GLint) )
					return true;

				if ( (t == IECore::ShortVectorDataTypeId || t == IECore::UShortVectorDataTypeId) && sizeof(short) == sizeof(GLshort) )
					return true;

				return false;

			case GL_FLOAT_VEC2 :

				if ( t == IECore::V2fVectorDataTypeId || t == IECore::V2dVectorDataTypeId )
					return true;

				if ( t == IECore::V2iVectorDataTypeId && sizeof(int) == sizeof(GLint) )
					return true;
					
				return false;

			case GL_FLOAT_VEC3 :

				if ( t == IECore::V3fVectorDataTypeId || t == IECore::V3dVectorDataTypeId || t == IECore::Color3fVectorDataTypeId || t == IECore::Color3dVectorDataTypeId )
					return true;

				if ( t == IECore::V3iVectorDataTypeId && sizeof(int) == sizeof(GLint) )
					return true;

				// \todo: apparently shader mat3 attributes are returned as vec3. So we should accept M33f here too...

				return false;

			case GL_FLOAT_VEC4 :

				if ( t == IECore::Color4fVectorDataTypeId || t == IECore::Color4dVectorDataTypeId )
					return true;

				// \todo: apparently shader mat4 attributes are returned as vec4. So we should accept M44f here too...
				
				return false;

			default :
				// \todo: implement	other types like GL_FLOAT_MAT3 and GL_FLOAT_MAT4. Although I'm not sure they would be returned. Apparently mat3 and mat4 returns vec3 and vec4...

				return false;
		}
	}
	else
	{
		return false;
	}
}

bool Shader::vertexValueValid( const std::string &parameterName, const IECore::Data *value ) const
{
	return vertexValueValid( vertexParameterIndex( parameterName ), value );
}

void Shader::setVertexParameter( GLint parameterIndex, const IECore::Data *value, bool normalize )
{
	if ( !vertexValueValid( parameterIndex, value ) )
	{
		throw Exception( "Can't set vertex parameter value. Type mismatch." );
	}

	switch( value->typeId() )
	{
		case IECore::FloatVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 1, GL_FLOAT, false, 0, ((const IECore::FloatVectorData *)value)->baseReadable() );
			break;
		case IECore::DoubleVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 1, GL_DOUBLE, false, 0, ((const IECore::DoubleVectorData *)value)->baseReadable() );
			break;
		case IECore::CharVectorDataTypeId :
			/// \todo Is normalize really right here? It seems to be being passed into the stride argument.
			glVertexAttribPointer( parameterIndex, 1, GL_BYTE, false, normalize, ((const IECore::CharVectorData *)value)->baseReadable() );
			break;
		case IECore::UCharVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 1, GL_UNSIGNED_BYTE, false, normalize, ((const IECore::UCharVectorData *)value)->baseReadable() );
			break;
		case IECore::IntVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 1, GL_INT, false, normalize, ((const IECore::IntVectorData *)value)->baseReadable() );
			break;
		case IECore::UIntVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 1, GL_UNSIGNED_INT, false, normalize, ((const IECore::UIntVectorData *)value)->baseReadable() );
			break;
		case IECore::ShortVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 1, GL_SHORT, false, normalize, ((const IECore::ShortVectorData *)value)->baseReadable() );
			break;
		case IECore::UShortVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 1, GL_UNSIGNED_SHORT, false, normalize, ((const IECore::UShortVectorData *)value)->baseReadable() );
			break;
		case IECore::V2fVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 2, GL_FLOAT, false, 0, ((const IECore::V2fVectorData *)value)->baseReadable() );
			break;
		case IECore::V2dVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 2, GL_DOUBLE, false, 0, ((const IECore::V2dVectorData *)value)->baseReadable() );
			break;
		case IECore::V2iVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 2, GL_INT, false, normalize, ((const IECore::V2iVectorData *)value)->baseReadable() );
			break;
		case IECore::V3fVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 3, GL_FLOAT, false, 0, ((const IECore::V3fVectorData *)value)->baseReadable() );
			break;
		case IECore::V3dVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 3, GL_DOUBLE, false, 0, ((const IECore::V3dVectorData *)value)->baseReadable() );
			break;
		case IECore::V3iVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 3, GL_INT, false, normalize, ((const IECore::V3iVectorData *)value)->baseReadable() );
			break;
		case IECore::Color3fVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 3, GL_FLOAT, false, 0, ((const IECore::Color3fVectorData *)value)->baseReadable() );
			break;
		case IECore::Color3dVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 3, GL_DOUBLE, false, 0, ((const IECore::Color3dVectorData *)value)->baseReadable() );
			break;
		case IECore::Color4fVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 4, GL_FLOAT, false, 0, ((const IECore::Color4fVectorData *)value)->baseReadable() );
			break;
		case IECore::Color4dVectorDataTypeId :
			glVertexAttribPointer( parameterIndex, 4, GL_DOUBLE, false, 0, ((const IECore::Color4dVectorData *)value)->baseReadable() );
			break;
		default :
			throw Exception( boost::str( boost::format( "Unsupported vertex parameter type \"%s\"." ) % value->typeName() ) );
	}
	/// \todo Might it be quicker to check the gl type ourselves beforehand rather than checking
	/// for errors here?
	Exception::throwIfError();

	glEnableVertexAttribArray( parameterIndex );
}

void Shader::setVertexParameter( const std::string &parameterName, const IECore::Data *value, bool normalize )
{
	setVertexParameter( vertexParameterIndex( parameterName ), value, normalize );
}
*/

GLint Shader::vertexAttribute( const std::string &name, GLenum &type, GLint &size ) const
{
	ParameterMap::const_iterator it;
	for( it = m_vertexParameters.begin(); it != m_vertexParameters.end(); it++ )
	{
		if( !strcmp( name.c_str(), it->second.name.c_str() ) )
		{
			break;
		}
	}
	
	if( it == m_vertexParameters.end() )
	{
		return -1;
	}
	
	type = it->second.type;
	size = it->second.size;
	return it->first;
}

/*const Shader::ParameterDescription &Shader::vertexParameterDescription( GLint parameterIndex ) const
{
	ParameterMap::const_iterator it = m_vertexParameters.find( parameterIndex );
	if( it==m_vertexParameters.end() )
	{
		throw Exception( "Vertex parameter doesn't exist." );
	}
	return it->second;
}*/

///////////////////////////////////////////////////////////////////////////////
// Setup implementation
///////////////////////////////////////////////////////////////////////////////

struct Shader::Setup::MemberData : public IECore::RefCounted
{
	
	ConstShaderPtr shader;
	
	struct VertexAttribute
	{
		GLuint attributeIndex;
		GLenum type;
		GLint size;
		ConstBufferPtr buffer;
	};
	
	vector<VertexAttribute> vertexAttributes;
	
};

Shader::Setup::Setup( ShaderPtr shader )
	:	m_memberData( new MemberData )
{
	m_memberData->shader = shader;
}

const Shader *Shader::Setup::shader() const
{
	return m_memberData->shader.get();
}

void Shader::Setup::addVertexAttribute( const std::string &name, const IECore::ConstDataPtr &data )
{
	GLenum attributeType = 0;
	GLint attributeSize = 0;
	GLint attributeIndex = m_memberData->shader->vertexAttribute( name, attributeType, attributeSize );
	if( attributeIndex < 0 || attributeSize > 1 )
	{
		return;
	}

	bool dataTypeOK = false;
	GLint size = 0;
	switch( attributeType )
	{
		case GL_FLOAT :
			dataTypeOK = IECore::despatchTraitsTest<IECore::TypeTraits::IsNumericVectorTypedData>( data );
			size = 1;
			break;
		case GL_FLOAT_VEC2 :
			dataTypeOK = IECore::despatchTraitsTest<IECore::TypeTraits::IsVec2VectorTypedData>( data );
			size = 2;
			break;
		case GL_FLOAT_VEC3 :
			dataTypeOK = IECore::despatchTraitsTest<IECore::TypeTraits::IsVec3VectorTypedData>( data );
			size = 3;
			break;
		case GL_FLOAT_VEC4 :
			dataTypeOK = data->isInstanceOf( IECore::Color4fVectorDataTypeId ) || data->isInstanceOf( IECore::Color4dVectorDataTypeId );
			size = 4;
			break;
		default :
			dataTypeOK = false;
	}

	GLenum dataGLType = glType( data );
	if( !dataTypeOK || !dataGLType || !size )
	{
		IECore::msg( IECore::Msg::Warning, "Shader::Setup::addVertexAttribute", format( "Vertex attribute \"%s\" has unsuitable data type \%s\"" ) % name % data->typeName() );
		return;
	}

	MemberData::VertexAttribute b;
	b.attributeIndex = attributeIndex;
	b.type = dataGLType;
	b.size = size;
	CachedConverterPtr converter = CachedConverter::defaultCachedConverter();
	b.buffer = IECore::runTimeCast<const Buffer>( converter->convert( data  ) );

	m_memberData->vertexAttributes.push_back( b );
}

Shader::Setup::ScopedBinding::ScopedBinding( const Setup &setup )
	:	m_previousProgram( 0 ), m_setup( setup )
{
	glGetIntegerv( GL_CURRENT_PROGRAM, &m_previousProgram );
	glUseProgram( m_setup.shader()->m_program );

	const vector<MemberData::VertexAttribute> &vertexAttributes = m_setup.m_memberData->vertexAttributes;
	for( vector<MemberData::VertexAttribute>::const_iterator it = vertexAttributes.begin(), eIt = vertexAttributes.end(); it != eIt; it++ )
	{
		Buffer::ScopedBinding binding( *(it->buffer) );
		glEnableVertexAttribArray( it->attributeIndex );
		glVertexAttribPointer( it->attributeIndex, it->size, it->type, false, 0, 0 );
	}
}

Shader::Setup::ScopedBinding::~ScopedBinding()
{
	const vector<MemberData::VertexAttribute> &vertexAttributes = m_setup.m_memberData->vertexAttributes;
	for( vector<MemberData::VertexAttribute>::const_iterator it = vertexAttributes.begin(), eIt = vertexAttributes.end(); it != eIt; it++ )
	{
		glDisableVertexAttribArray( it->attributeIndex );
	}
	glUseProgram( m_previousProgram );
}

///////////////////////////////////////////////////////////////////////////////
// definitions for useful simple shaders
///////////////////////////////////////////////////////////////////////////////

/// \todo This needs a parameter for the colour to be passed in
ShaderPtr Shader::constant()
{
	static const char *vertexSource =
	"in vec3 P;"
	""
	"void main()"
	"{"
	"	vec4 pCam = gl_ModelViewMatrix * vec4( P, 1 );"
	"	gl_Position = gl_ProjectionMatrix * pCam;"
	"}";

	static const char *fragmentSource =
	"void main()"
	"{"
	"	gl_FragColor = vec4( 1, 1, 1, 1 );"
	"}";


	static ShaderPtr s = new Shader( vertexSource, fragmentSource );
	return s;
}

ShaderPtr Shader::facingRatio()
{
	static const char *vertexSource =
	"in vec3 P;"
	"in vec3 N;"
	""
	"varying out vec3 fI;"
	"varying out vec3 fN;"
	""
	"void main()"
	"{"
	"	vec4 pCam = gl_ModelViewMatrix * vec4( P, 1 );"
	"	gl_Position = gl_ProjectionMatrix * pCam;"
	"	fN = normalize( gl_NormalMatrix * N );"
	"	fI = normalize( -pCam.xyz );"
	"}";

	static const char *fragmentSource =
	"varying vec3 fI;"
	"varying vec3 fN;"
	""
	"void main()"
	"{"
	"	vec3 Nf = faceforward( fN, -fI, fN );"
	"	float f = dot( normalize(fI), normalize(Nf) );"
	"	gl_FragColor = vec4( f, f, f, 1 );"
	"}";


	static ShaderPtr s = new Shader( vertexSource, fragmentSource );
	return s;
}
