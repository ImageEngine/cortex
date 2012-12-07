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
#include "IECoreGL/TextureUnits.h"

using namespace std;
using namespace boost;
using namespace IECore;
using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( Shader );

Shader::Shader( const std::string &vertexSource, const std::string &fragmentSource )
	:	m_vertexShader( 0 ), m_fragmentShader( 0 ), m_program( 0 )
{
	if( !GLEW_VERSION_2_0 )
	{
		throw Exception( "OpenGL version < 2" );
	}

	string actualVertexSource = vertexSource;
	string actualFragmentSource = fragmentSource;
	if( vertexSource == "" )
	{
		actualVertexSource = defaultVertexSource();
	}
	if( fragmentSource == "" )
	{
		actualFragmentSource = defaultFragmentSource();
	}

	compile( actualVertexSource, GL_VERTEX_SHADER, m_vertexShader );
	compile( actualFragmentSource, GL_FRAGMENT_SHADER, m_fragmentShader );

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
		int textureUnit = 0;
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
			
			if( d.type == GL_SAMPLER_2D )
			{
				// we assign a specific texture unit to each individual
				// sampler parameter - this makes it much easier to save
				// and restore state when applying nested Setups.
				d.textureUnit = textureUnit++;
			}
			else
			{
				d.textureUnit = -1;
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

/*
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
*/

/*void Shader::setUniformParameter( GLint parameterIndex, const IECore::Data *value )
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
*/

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

*/

GLint Shader::uniformParameter( const std::string &name, GLenum &type, GLint &size, size_t &textureUnit ) const
{
	ParameterMap::const_iterator it;
	for( it = m_uniformParameters.begin(); it != m_uniformParameters.end(); it++ )
	{
		if( !strcmp( name.c_str(), it->second.name.c_str() ) )
		{
			break;
		}
	}
	
	if( it == m_uniformParameters.end() )
	{
		return -1;
	}
	
	type = it->second.type;
	size = it->second.size;
	textureUnit = it->second.textureUnit;
	return it->first;
}

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
	
	struct UniformParameter
	{
		GLuint uniformIndex;
		unsigned char dimensions;
		GLint size;
		// we only use one of these two vectors, depending on the type
		// of the parameter. the first half of the vector is used to store
		// the shader values, and the second half is used to store the previous
		// shader values in a ScopedBinding.
		mutable std::vector<GLfloat> floats;
		mutable std::vector<GLint> ints;
	};
	
	struct TextureParameter
	{
		GLuint uniformIndex;
		size_t textureUnit;
		ConstTexturePtr texture;
		GLint previousTexture;
	};
	
	vector<VertexAttribute> vertexAttributes;
	vector<UniformParameter> uniformParameters;
	vector<TextureParameter> textureParameters;
	
};

Shader::Setup::Setup( ConstShaderPtr shader )
	:	m_memberData( new MemberData )
{
	m_memberData->shader = shader;
}

const Shader *Shader::Setup::shader() const
{
	return m_memberData->shader.get();
}

void Shader::Setup::addUniformParameter( const std::string &name, ConstTexturePtr value )
{
	GLenum uniformType = 0;
	GLint uniformSize = 0;
	size_t textureUnit = 0;
	GLint uniformIndex = m_memberData->shader->uniformParameter( name, uniformType, uniformSize, textureUnit );
	if( uniformIndex < 0 || uniformType != GL_SAMPLER_2D )
	{
		return;
	}
	
	MemberData::TextureParameter p;
	p.uniformIndex = uniformIndex;
	p.textureUnit = textureUnit;
	p.texture = value;
	m_memberData->textureParameters.push_back( p );
}

template<typename Container>
struct UniformDataConverter
{
	typedef bool ReturnType;
	
	UniformDataConverter( Container &container )
		:	m_container( container )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::ConstPtr data ) const
	{
		const typename T::BaseType *begin = data->baseReadable();
		const typename T::BaseType *end = begin + data->baseSize();
		for( ; begin != end; begin++ )
		{
			m_container.push_back( (typename Container::value_type)*begin );
		}
		m_container.resize( m_container.size() * 2 ); // make enough extra room to store pushed values in ScopedBinding
		return true;
	}

	private :
		
		Container &m_container;

};

void Shader::Setup::addUniformParameter( const std::string &name, IECore::ConstDataPtr value )
{
	GLenum uniformType = 0;
	GLint uniformSize = 0;
	size_t textureUnit = 0;
	GLint uniformIndex = m_memberData->shader->uniformParameter( name, uniformType, uniformSize, textureUnit );
	if( uniformIndex < 0 || uniformSize > 1 )
	{
		return;
	}
	
	GLint dimensions = 0;
	switch( uniformType )
	{
		case GL_BOOL :
		case GL_INT :
		case GL_FLOAT :
			dimensions = 1;
			break;
		case GL_BOOL_VEC2 :
		case GL_INT_VEC2 :
		case GL_FLOAT_VEC2 :
			dimensions = 2;
			break;
		case GL_BOOL_VEC3 :
		case GL_INT_VEC3 :
		case GL_FLOAT_VEC3 :
			dimensions = 3;
			break;
		default :
			dimensions = 0;
	}
	
	if( !dimensions )
	{
		return;
	}
	
	bool integer = uniformType != GL_FLOAT && uniformType != GL_FLOAT_VEC2 && uniformType != GL_FLOAT_VEC3;
	
	MemberData::UniformParameter p;
	p.uniformIndex = uniformIndex;
	p.dimensions = dimensions;
	p.size = uniformSize;
	
	if( integer )
	{
		UniformDataConverter<vector<GLint> > converter( p.ints );
		IECore::despatchTypedData< UniformDataConverter<vector<GLint> >, IECore::TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( IECore::constPointerCast<IECore::Data>( value ), converter );
	}
	else
	{
		UniformDataConverter<vector<GLfloat> > converter( p.floats );
		IECore::despatchTypedData< UniformDataConverter<vector<GLfloat> >, IECore::TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( IECore::constPointerCast<IECore::Data>( value ), converter );	
	}
	
	m_memberData->uniformParameters.push_back( p );
}
				
void Shader::Setup::addVertexAttribute( const std::string &name, IECore::ConstDataPtr value )
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
			dataTypeOK = IECore::despatchTraitsTest<IECore::TypeTraits::IsNumericVectorTypedData>( value );
			size = 1;
			break;
		case GL_FLOAT_VEC2 :
			dataTypeOK = IECore::despatchTraitsTest<IECore::TypeTraits::IsVec2VectorTypedData>( value );
			size = 2;
			break;
		case GL_FLOAT_VEC3 :
			dataTypeOK = IECore::despatchTraitsTest<IECore::TypeTraits::IsVec3VectorTypedData>( value );
			size = 3;
			break;
		case GL_FLOAT_VEC4 :
			dataTypeOK = value->isInstanceOf( IECore::Color4fVectorDataTypeId ) || value->isInstanceOf( IECore::Color4dVectorDataTypeId );
			size = 4;
			break;
		default :
			dataTypeOK = false;
	}

	GLenum dataGLType = glType( value );
	if( !dataTypeOK || !dataGLType || !size )
	{
		IECore::msg( IECore::Msg::Warning, "Shader::Setup::addVertexAttribute", format( "Vertex attribute \"%s\" has unsuitable data type \%s\"" ) % name % value->typeName() );
		return;
	}

	MemberData::VertexAttribute b;
	b.attributeIndex = attributeIndex;
	b.type = dataGLType;
	b.size = size;
	CachedConverterPtr converter = CachedConverter::defaultCachedConverter();
	b.buffer = IECore::runTimeCast<const Buffer>( converter->convert( value  ) );

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
	
	const vector<MemberData::UniformParameter> &uniformParameters = m_setup.m_memberData->uniformParameters;
	for( vector<MemberData::UniformParameter>::const_iterator it = uniformParameters.begin(), eIt = uniformParameters.end(); it != eIt; it++ )
	{
		if( it->floats.size() )
		{
			// save the current value into the back half of our storage
			glGetUniformfv( m_setup.shader()->m_program, it->uniformIndex, &(it->floats[it->floats.size()/2]) );
			// load the new value from the front half of our storage.
			uniformFloatFunctions()[it->dimensions]( it->uniformIndex, it->size, &(it->floats[0]) );
		}
		else
		{
			// save the current value into the back half of our storage
			glGetUniformiv( m_setup.shader()->m_program, it->uniformIndex, &(it->ints[it->ints.size()/2]) );
			// load the new value from the front half of our storage.
			uniformIntFunctions()[it->dimensions]( it->uniformIndex, it->size, &(it->ints[0]) );
		}
	}
	
	vector<MemberData::TextureParameter> &textureParameters = m_setup.m_memberData->textureParameters;
	for( vector<MemberData::TextureParameter>::iterator it = textureParameters.begin(), eIt = textureParameters.end(); it != eIt; it++ )
	{
		glActiveTexture( textureUnits()[it->textureUnit] );
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &(it->previousTexture) );
		if( it->texture )
		{
			it->texture->bind();
		}
		else
		{
			glBindTexture( GL_TEXTURE_2D, 0 );
		}
		glUniform1i( it->uniformIndex, it->textureUnit );
	}
}

Shader::Setup::ScopedBinding::~ScopedBinding()
{
	const vector<MemberData::TextureParameter> &textureParameters = m_setup.m_memberData->textureParameters;
	for( vector<MemberData::TextureParameter>::const_iterator it = textureParameters.begin(), eIt = textureParameters.end(); it != eIt; it++ )
	{
		glActiveTexture( textureUnits()[it->textureUnit] );
		glBindTexture( GL_TEXTURE_2D, it->previousTexture );
	}
	
	const vector<MemberData::UniformParameter> &uniformParameters = m_setup.m_memberData->uniformParameters;
	for( vector<MemberData::UniformParameter>::const_iterator it = uniformParameters.begin(), eIt = uniformParameters.end(); it != eIt; it++ )
	{
		if( it->floats.size() )
		{
			// load the previously saved value from the back half of our storage.
			uniformFloatFunctions()[it->dimensions]( it->uniformIndex, it->size, &(it->floats[it->floats.size()/2]) );
		}
		else
		{
			// load the previously saved value from the back half of our storage.
			uniformIntFunctions()[it->dimensions]( it->uniformIndex, it->size, &(it->ints[it->ints.size()/2]) );
		}
	}
	
	const vector<MemberData::VertexAttribute> &vertexAttributes = m_setup.m_memberData->vertexAttributes;
	for( vector<MemberData::VertexAttribute>::const_iterator it = vertexAttributes.begin(), eIt = vertexAttributes.end(); it != eIt; it++ )
	{
		glDisableVertexAttribArray( it->attributeIndex );
	}
	glUseProgram( m_previousProgram );
}

///////////////////////////////////////////////////////////////////////////////
// default shader source
///////////////////////////////////////////////////////////////////////////////

const std::string &Shader::defaultVertexSource()
{
	static string s =
		
		"in vec3 P;"
		"in vec3 N;"
		""
		"varying out vec3 fragmentI;"
		"varying out vec3 fragmentN;"
		""
		"void main()"
		"{"
		"	vec4 pCam = gl_ModelViewMatrix * vec4( P, 1 );"
		"	gl_Position = gl_ProjectionMatrix * pCam;"
		"	fragmentN = normalize( gl_NormalMatrix * N );"
		"	if( gl_ProjectionMatrix[2][3] != 0.0 )"
		"	{"
		"		fragmentI = normalize( -pCam.xyz );"
		"	}"
		"	else"
		"	{"
		"		fragmentI = vec3( 0, 0, -1 );"
		"	}"
		"}";
		
	return s;
}

const std::string &Shader::defaultFragmentSource()
{
	static string s = 
	
		"varying vec3 fragmentI;"
		"varying vec3 fragmentN;"
		""
		"void main()"
		"{"
		"	vec3 Nf = faceforward( fragmentN, -fragmentI, fragmentN );"
		"	float f = dot( normalize( fragmentI ), normalize(Nf) );"
		"	gl_FragColor = vec4( f, f, f, 1 );"
		"}";
		
	return s;
}

///////////////////////////////////////////////////////////////////////////////
// definitions for useful simple shaders
///////////////////////////////////////////////////////////////////////////////

ShaderPtr Shader::constant()
{
	static const char *fragmentSource =
	
		"uniform vec3 Cs;"
		""
		"void main()"
		"{"
		"	gl_FragColor = vec4( Cs, 1 );"
		"}";

	static ShaderPtr s = new Shader( defaultVertexSource(), fragmentSource );
	return s;
}

ShaderPtr Shader::facingRatio()
{
	static ShaderPtr s = new Shader( defaultVertexSource(), defaultFragmentSource() );
	return s;
}
