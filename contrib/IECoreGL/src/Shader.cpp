//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/Shader.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/Texture.h"

#include "IECore/SimpleTypedData.h"

#include "boost/format.hpp"

#include <vector>
#include <iostream>

using namespace IECoreGL;
using namespace std;

Shader::Shader( const std::string &vertexSource, const std::string &fragmentSource )
	:	m_vertexShader( 0 ), m_fragmentShader( 0 ), m_program( 0 )
{
	if( !GLEW_VERSION_2_1 )
	{
		throw Exception( "OpenGL version < 2.1" );
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
	if( !linkStatus )
	{
		GLint logLength = 0;
		glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &logLength );
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

	// build the parameter description map
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
		m_parameters[location] = d; 
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

GLbitfield Shader::mask() const
{
	return 0;
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
		if( !compileStatus )
		{
			GLint logLength = 0;
			glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );
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
	}
	
}

void Shader::release()
{
	glDeleteShader( m_vertexShader );
	glDeleteShader( m_fragmentShader );
	glDeleteProgram( m_program );
}

void Shader::parameterNames( std::vector<std::string> &names ) const
{
	GLint numUniforms = 0;
	glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &numUniforms );
	GLint maxUniformNameLength = 0;
	glGetProgramiv( m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength ); 
	vector<char> name( maxUniformNameLength );
	for( int i=0; i<numUniforms; i++ )
	{
		GLint size = 0;
		GLenum type = 0;
		glGetActiveUniform( m_program, i, maxUniformNameLength, 0, &size, &type, &name[0] );
		if( strncmp( "gl_", &name[0], 3 ) )
		{
			names.push_back( &name[0] );
		}
	}
}

GLint Shader::parameterIndex( const std::string &parameterName ) const
{
	GLint location = glGetUniformLocation( m_program, parameterName.c_str() );
	if( location==-1 )
	{
		throw( Exception( boost::str( boost::format( "No parameter named \"%s\"." ) % parameterName ) ) );
	}
	return location;
}

bool Shader::hasParameter( const std::string &parameterName ) const
{
	GLint location = glGetUniformLocation( m_program, parameterName.c_str() );
	return location != -1;
}

IECore::TypeId Shader::parameterType( GLint parameterIndex ) const
{
	const ParameterDescription &p = parameterDescription( parameterIndex );
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
				throw Exception( "Unsupported parameter type." );
		}

	}
	else
	{
		throw Exception( "Array parameters not supported yet." );
	}
}

IECore::TypeId Shader::parameterType( const std::string &parameterName ) const
{
	return parameterType( parameterIndex( parameterName ) );
}
		
IECore::DataPtr Shader::getParameter( GLint parameterIndex ) const
{
	const ParameterDescription &p = parameterDescription( parameterIndex );
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
				throw Exception( "Unsupported parameter type." );
		}
	}
	else
	{
		throw Exception( "Array parameters not supported yet." );
	}
}

IECore::DataPtr Shader::getParameter( const std::string &parameterName ) const
{
	return getParameter( parameterIndex( parameterName ) );
}

bool Shader::valueValid( GLint parameterIndex, IECore::ConstDataPtr value ) const
{
	IECore::TypeId pt = parameterType( parameterIndex );
	if( pt==Texture::staticTypeId() )
	{
		return false;
	}
	
	IECore::TypeId t = value->typeId();
	if( t==IECore::BoolDataTypeId && pt!=IECore::BoolDataTypeId )
	{
		t = IECore::IntDataTypeId;
	}
	else if( t==IECore::Color3fDataTypeId )
	{
		t = IECore::V3fDataTypeId;
	}
	return t==pt;
}

bool Shader::valueValid( const std::string &parameterName, IECore::ConstDataPtr value ) const
{
	return valueValid( parameterIndex( parameterName ), value );
}
				
void Shader::setParameter( GLint parameterIndex, IECore::ConstDataPtr value )
{
	/// \todo We either need to do type checking below before all those casts, or
	/// state in the documentation that this function will blow up with invalid data.
	switch( value->typeId() )
	{
		case IECore::BoolDataTypeId :
			glUniform1i( parameterIndex, ((const IECore::BoolData *)value.get())->readable() );
			break;
		case IECore::IntDataTypeId :
			glUniform1i( parameterIndex, ((const IECore::IntData *)value.get())->readable() );	
			break;
		case IECore::FloatDataTypeId :
			glUniform1f( parameterIndex, ((const IECore::FloatData *)value.get())->readable() );
			break;
		case IECore::V2fDataTypeId :
			glUniform2fv( parameterIndex, 1, &((const IECore::V3fData *)value.get())->readable().x );	
			break;
		case IECore::V2iDataTypeId :
			{
				const Imath::V2i &vv = ((const IECore::V2iData *)value.get())->readable();
				GLint v[2]; v[0] = vv[0]; v[1] = vv[1];
				glUniform2iv( parameterIndex, 1, v );	
			}
			break;	
		case IECore::V3fDataTypeId :
			glUniform3fv( parameterIndex, 1, &((const IECore::V3fData *)value.get())->readable().x );	
			break;
		case IECore::V3iDataTypeId :
			{
				const Imath::V3i &vv = ((const IECore::V3iData *)value.get())->readable();
				GLint v[3]; v[0] = vv[0]; v[1] = vv[1]; v[2] = vv[2];
				glUniform3iv( parameterIndex, 1, v );	
			}
			break;
		case IECore::Color3fDataTypeId :
			glUniform3fv( parameterIndex, 1, &((const IECore::Color3fData *)value.get())->readable().x );	
			break;
		case IECore::Color4fDataTypeId :
			glUniform4fv( parameterIndex, 1, &((const IECore::Color4fData *)value.get())->readable().r );	
			break;
		case IECore::M33fDataTypeId :
			glUniformMatrix3fv( parameterIndex, 1, GL_FALSE, ((const IECore::M33fData *)value.get())->readable().getValue() );
			break;	
		case IECore::M44fDataTypeId :
			glUniformMatrix4fv( parameterIndex, 1, GL_FALSE, ((const IECore::M44fData *)value.get())->readable().getValue() );
			break;
		default :
			throw Exception( boost::str( boost::format( "Unsupported parameter type \"%s\"." ) % value->typeName() ) );
	}
	/// \todo Might it be quicker to check the gl type ourselves beforehand rather than checking
	/// for errors here?
	Exception::throwIfError();
}
		
void Shader::setParameter( const std::string &parameterName, IECore::ConstDataPtr value )
{
	setParameter( parameterIndex( parameterName ), value );
}

void Shader::setParameter( GLint parameterIndex, unsigned int textureUnit )
{
	glUniform1i( parameterIndex, textureUnit );
	/// \todo Might it be quicker to check the gl type ourselves beforehand rather than checking
	/// for errors here?
	Exception::throwIfError();
}

void Shader::setParameter( const std::string &parameterName, unsigned int textureUnit )
{
	setParameter( parameterIndex( parameterName ), textureUnit );
}

void Shader::setParameter( GLint parameterIndex, int value )
{
	glUniform1i( parameterIndex, value );
	Exception::throwIfError();
}

void Shader::setParameter( const std::string &parameterName, int value )
{
	setParameter( parameterIndex( parameterName ), value );
}
		
const Shader::ParameterDescription &Shader::parameterDescription( GLint parameterIndex ) const
{
	ParameterMap::const_iterator it = m_parameters.find( parameterIndex );
	if( it==m_parameters.end() )
	{
		throw Exception( "Parameter doesn't exist." );
	}
	return it->second;
}
		
///////////////////////////////////////////////////////////////////////////////
// definitions for useful simple shaders
///////////////////////////////////////////////////////////////////////////////

ShaderPtr Shader::constant()
{
	static const char *vertexSource = 
	"void main()"
	"{"
	"	gl_Position = ftransform();"
	"	gl_FrontColor = gl_Color;"
	"	gl_BackColor = gl_Color;"
	"}";

	static ShaderPtr s = new Shader( vertexSource, "" );
	return s;
}

ShaderPtr Shader::facingRatio()
{
	static const char *vertexSource = 
	"varying vec3 I;"
	"varying vec3 N;"
	""
	"void main()"
	"{"
	"	gl_Position = ftransform();"
	"	N = normalize( gl_NormalMatrix * gl_Normal );"
	"	I = normalize( -gl_Position.xyz );"
	"}";

	static const char *fragmentSource = 
	"varying vec3 I;"
	"varying vec3 N;"
	""
	"void main()"
	"{"
	"	N = faceforward( N, -I, N );"
	"	float f = dot( I, N );"
	"	gl_FragColor = vec4( f, f, f, 1 );"
	"}";


	static ShaderPtr s = new Shader( vertexSource, fragmentSource );
	return s;
}
