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

//////////////////////////////////////////////////////////////////////////
// Shader::Implementation
//////////////////////////////////////////////////////////////////////////

class Shader::Implementation : public IECore::RefCounted
{

	public :

		Implementation( const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource )
			:	m_vertexSource( vertexSource ), m_geometrySource( geometrySource ), m_fragmentSource( fragmentSource ),
				m_vertexShader( 0 ), m_geometryShader( 0 ), m_fragmentShader( 0 ), m_program( 0 )
		{		
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
			compile( geometrySource, GL_GEOMETRY_SHADER, m_geometryShader );
			compile( actualFragmentSource, GL_FRAGMENT_SHADER, m_fragmentShader );


			m_program = glCreateProgram();
			glAttachShader( m_program, m_vertexShader );
			if( m_geometryShader )
			{
				glAttachShader( m_program, m_geometryShader );
			}
			glAttachShader( m_program, m_fragmentShader );

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
		
		virtual ~Implementation()
		{
			release();
		}
		
		GLuint program() const
		{
			return m_program;
		}
		
		const std::string &vertexSource() const
		{
			return m_vertexSource;
		}
		
		const std::string &geometrySource() const
		{
			return m_geometrySource;
		}
		
		const std::string &fragmentSource() const
		{
			return m_fragmentSource;
		}
		
		void uniformParameterNames( std::vector<std::string> &names ) const
		{
			for( ParameterMap::const_iterator it = m_uniformParameters.begin(); it != m_uniformParameters.end(); it++ )
			{
				names.push_back( it->second.name );
			}
		}
		
		GLint uniformParameter( const std::string &name, GLenum &type, GLint &size, size_t &textureUnit ) const
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

		void vertexParameterNames( std::vector<std::string> &names ) const
		{	
			for( ParameterMap::const_iterator it = m_vertexParameters.begin(); it != m_vertexParameters.end(); it++ )
			{
				names.push_back( it->second.name );
			}
		}
		
		GLint vertexAttribute( const std::string &name, GLenum &type, GLint &size ) const
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
	
	private :
	
		friend class Shader::Setup;
		
		std::string m_vertexSource;
		std::string m_geometrySource;
		std::string m_fragmentSource;

		GLuint m_vertexShader;
		GLuint m_geometryShader;
		GLuint m_fragmentShader;
		GLuint m_program;

		struct ParameterDescription
		{
			std::string name;
			GLenum type;
			GLint size;
			size_t textureUnit; // only used for uniform parameters
		};
		/// Maps from the uniform location to the parameter details.
		typedef std::map<GLint, ParameterDescription> ParameterMap;
		ParameterMap m_uniformParameters;
		ParameterMap m_vertexParameters;
		
		void compile( const std::string &source, GLenum type, GLuint &shader )
		{
			if( source == "" )
			{
				return;
			}
			
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

		void release()
		{
			glDeleteShader( m_vertexShader );
			glDeleteShader( m_geometryShader );
			glDeleteShader( m_fragmentShader );
			glDeleteProgram( m_program );
		}

};
	
//////////////////////////////////////////////////////////////////////////
// Shader
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( Shader );

Shader::Shader( const std::string &vertexSource, const std::string &fragmentSource )
	:	m_implementation( new Implementation( vertexSource, "", fragmentSource ) )
{
}

Shader::Shader( const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource )
	:	m_implementation( new Implementation( vertexSource, geometrySource, fragmentSource ) )
{
}

Shader::~Shader()
{
}

GLuint Shader::program() const
{
	return m_implementation->program();
}

const std::string &Shader::vertexSource() const
{
	return m_implementation->vertexSource();
}

const std::string &Shader::geometrySource() const
{
	return m_implementation->geometrySource();
}

const std::string &Shader::fragmentSource() const
{
	return m_implementation->fragmentSource();
}
		
void Shader::uniformParameterNames( std::vector<std::string> &names ) const
{
	m_implementation->uniformParameterNames( names );
}

void Shader::vertexParameterNames( std::vector<std::string> &names ) const
{
	m_implementation->vertexParameterNames( names );
}

GLint Shader::uniformParameter( const std::string &name, GLenum &type, GLint &size, size_t &textureUnit ) const
{
	return m_implementation->uniformParameter( name, type, size, textureUnit );
}

GLint Shader::vertexAttribute( const std::string &name, GLenum &type, GLint &size ) const
{
	return m_implementation->vertexAttribute( name, type, size );
}

///////////////////////////////////////////////////////////////////////////////
// Setup implementation
///////////////////////////////////////////////////////////////////////////////

struct Shader::Setup::MemberData : public IECore::RefCounted
{
	
	ConstShaderPtr shader;
	
	// base class for objects which can bind a value of some sort
	// to a shader, and later unbind it.
	struct Value : public IECore::RefCounted
	{
		virtual void bind() = 0;
		virtual void unbind() = 0;
	};
	IE_CORE_DECLAREPTR( Value );
	
	// value class for specifying vertex attributes
	struct VertexValue : public Value
	{
	
		VertexValue( GLuint attributeIndex, GLenum type, GLint size, ConstBufferPtr buffer, GLuint divisor )
			:	m_attributeIndex( attributeIndex ), m_type( type ), m_size( size ), m_buffer( buffer ), m_divisor( divisor )
		{
		}
		
		virtual void bind()
		{
			Buffer::ScopedBinding binding( *m_buffer );
			glEnableVertexAttribArray( m_attributeIndex );
			glVertexAttribPointer( m_attributeIndex, m_size, m_type, false, 0, 0 );
			glVertexAttribDivisor( m_attributeIndex, m_divisor );
		}
		
		virtual void unbind()
		{
			glVertexAttribDivisor( m_attributeIndex, 0 );
			glDisableVertexAttribArray( m_attributeIndex );
		}
		
		private :
		
			GLuint m_attributeIndex;
			GLenum m_type;
			GLint m_size;
			ConstBufferPtr m_buffer;
			GLuint m_divisor;
			
	};
	
	// value class for specifying textures
	struct TextureValue : public Value
	{
	
		TextureValue( GLuint uniformIndex, GLuint textureUnit, ConstTexturePtr texture )
			:	m_uniformIndex( uniformIndex ), m_textureUnit( textureUnit ), m_texture( texture )
		{
		}
		
		virtual void bind()
		{
			glActiveTexture( textureUnits()[m_textureUnit] );
			glGetIntegerv( GL_TEXTURE_BINDING_2D, &m_previousTexture );
			if( m_texture )
			{
				m_texture->bind();
			}
			else
			{
				glBindTexture( GL_TEXTURE_2D, 0 );
			}
			glUniform1i( m_uniformIndex, m_textureUnit );
		}
		
		virtual void unbind()
		{
			glActiveTexture( textureUnits()[m_textureUnit] );
			glBindTexture( GL_TEXTURE_2D, m_previousTexture );
		}
		
		private :
		
			GLuint m_uniformIndex;
			size_t m_textureUnit;
			ConstTexturePtr m_texture;
			GLint m_previousTexture;
	
	};
	
	// value class for specifying uniform values
	struct UniformFloatValue : public Value
	{
	
		UniformFloatValue( GLuint program, GLuint uniformIndex, unsigned char dimensions, std::vector<GLfloat> &values )
			:	m_program( program ), m_uniformIndex( uniformIndex ), m_dimensions( dimensions ), m_values( values )
		{
			m_previousValues.resize( m_values.size() );
		}
	
		virtual void bind()
		{
			glGetUniformfv( m_program, m_uniformIndex, &(m_previousValues[0]) );
			uniformFloatFunctions()[m_dimensions]( m_uniformIndex, 1, &(m_values[0]) );
		}
		
		virtual void unbind()
		{
			uniformFloatFunctions()[m_dimensions]( m_uniformIndex, 1, &(m_previousValues[0]) );
		}
	
		private :
		
			GLuint m_program;
			GLuint m_uniformIndex;
			unsigned char m_dimensions;
			std::vector<GLfloat> m_values;
			std::vector<GLfloat> m_previousValues;
			
	};
	
	// value class for specifying uniform values
	struct UniformIntegerValue : public Value
	{
	
		UniformIntegerValue( GLuint program, GLuint uniformIndex, unsigned char dimensions, std::vector<GLint> &values )
			:	m_program( program ), m_uniformIndex( uniformIndex ), m_dimensions( dimensions ), m_values( values )
		{
			m_previousValues.resize( m_values.size() );
		}
	
		virtual void bind()
		{
			glGetUniformiv( m_program, m_uniformIndex, &(m_previousValues[0]) );
			uniformIntFunctions()[m_dimensions]( m_uniformIndex, 1, &(m_values[0]) );
		}
		
		virtual void unbind()
		{
			uniformIntFunctions()[m_dimensions]( m_uniformIndex, 1, &(m_previousValues[0]) );
		}
	
		private :
		
			GLuint m_program;
			GLuint m_uniformIndex;
			unsigned char m_dimensions;
			std::vector<GLint> m_values;
			std::vector<GLint> m_previousValues;
			
	};
	
	struct UniformMatrixValue : public Value
	{
		UniformMatrixValue( GLuint program, GLuint uniformIndex, unsigned char dimensions0, unsigned char dimensions1, std::vector<GLfloat> &values )
			:	m_program( program ), m_uniformIndex( uniformIndex ), m_dimensions0( dimensions0 ), m_dimensions1( dimensions1 ), m_values( values )
		{
			m_previousValues.resize( m_values.size(), 0 );
		}
		
		virtual void bind()
		{
			glGetUniformfv( m_program, m_uniformIndex, &(m_previousValues[0]) );
			uniformMatrixFunctions()[m_dimensions0][m_dimensions1]( m_uniformIndex, 1, GL_FALSE, &(m_values[0]) );
		}
		
		virtual void unbind()
		{
			uniformMatrixFunctions()[m_dimensions0][m_dimensions1]( m_uniformIndex, 1, GL_FALSE, &(m_previousValues[0]) );
		}
		
		private :
		
			GLuint m_program;
			GLuint m_uniformIndex;
			unsigned char m_dimensions0;
			unsigned char m_dimensions1;
			std::vector<GLfloat> m_values;
			std::vector<GLfloat> m_previousValues;
				
	};
	
	vector<ValuePtr> values;
	
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
	
	m_memberData->values.push_back( new MemberData::TextureValue( uniformIndex, textureUnit, value ) );
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
	if( uniformIndex < 0 )
	{
		return;
	}
	
	if( uniformSize > 1 )
	{
		IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Array parameter \"%s\" is currently unsupported." ) % name );
		return;
	}
	
	if( uniformType == GL_BOOL || uniformType == GL_INT || uniformType == GL_INT_VEC2 || uniformType == GL_INT_VEC3 )
	{
		// integer value
		
		vector<GLint> integers;
		if( value->isInstanceOf( IECore::BoolDataTypeId ) )
		{
			integers.push_back( static_cast<const IECore::BoolData *>( value.get() )->readable() );
		}
		else
		{
			UniformDataConverter<vector<GLint> > converter( integers );
			IECore::despatchTypedData< UniformDataConverter<vector<GLint> >, IECore::TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( IECore::constPointerCast<IECore::Data>( value ), converter );
		}
		if( !integers.size() )
		{
			IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Uniform parameter \"%s\" has unsuitable data type \%s\"" ) % name % value->typeName() );
		}
		else
		{
			unsigned char dimensions = 0;
			switch( uniformType )
			{
				case GL_BOOL :
				case GL_INT :
					dimensions = 1;
					break;
				case GL_INT_VEC2 :
					dimensions = 2;
					break;
				case GL_INT_VEC3 :
					dimensions = 3;	
			}
			m_memberData->values.push_back( new MemberData::UniformIntegerValue( m_memberData->shader->program(), uniformIndex, dimensions, integers ) );
		}
	}
	else if( uniformType == GL_FLOAT || uniformType == GL_FLOAT_VEC2 || uniformType == GL_FLOAT_VEC3 )
	{
		// float value
		
		vector<GLfloat> floats;
		UniformDataConverter<vector<GLfloat> > converter( floats );
		IECore::despatchTypedData< UniformDataConverter<vector<GLfloat> >, IECore::TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( IECore::constPointerCast<IECore::Data>( value ), converter );	
		if( !floats.size() )
		{
			IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Uniform parameter \"%s\" has unsuitable data type \%s\"" ) % name % value->typeName() );
			return;
		}
		else
		{
			unsigned char dimensions = 0;
			switch( uniformType )
			{
				case GL_FLOAT :
					dimensions = 1;
					break;
				case GL_FLOAT_VEC2 :
					dimensions = 2;
					break;
				case GL_FLOAT_VEC3 :
					dimensions = 3;	
			}
			m_memberData->values.push_back( new MemberData::UniformFloatValue( m_memberData->shader->program(), uniformIndex, dimensions, floats ) );
		}
	}
	else if( uniformType == GL_FLOAT_MAT3 || uniformType == GL_FLOAT_MAT4 )
	{
		// matrix value
		unsigned char dimensions0 = 0;
		unsigned char dimensions1 = 0;
		switch( uniformType )
		{
			case GL_FLOAT_MAT3 :
				dimensions0 = dimensions1 = 3;
				break;
			case GL_FLOAT_MAT4 :
				dimensions0 = dimensions1 = 4;
				break;
		};
				
		vector<GLfloat> floats;
		UniformDataConverter<vector<GLfloat> > converter( floats );
		IECore::despatchTypedData< UniformDataConverter<vector<GLfloat> >, IECore::TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( IECore::constPointerCast<IECore::Data>( value ), converter );	
		if( floats.size() != dimensions0 * dimensions1 )
		{
			IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Matrix parameter \"%s\" requires %d values but value of type \%s\" provided %d" ) % name % (dimensions0 * dimensions1) % value->typeName() % floats.size() );
			return;
		}
		
		m_memberData->values.push_back( new MemberData::UniformMatrixValue( m_memberData->shader->program(), uniformIndex, dimensions0, dimensions1, floats ) );
	}
	else
	{
		IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Uniform parameter \"%s\" has unsupported OpenGL type \%d\"" ) % name % uniformType );
	}
}
				
void Shader::Setup::addVertexAttribute( const std::string &name, IECore::ConstDataPtr value, GLuint divisor )
{	
	GLenum attributeType = 0;
	GLint attributeSize = 0;
	GLint attributeIndex = m_memberData->shader->vertexAttribute( name, attributeType, attributeSize );
	if( attributeIndex < 0 )
	{
		return;
	}

	if( attributeSize > 1 )
	{
		IECore::msg( IECore::Msg::Warning, "Shader::Setup::addVertexAttribute", format( "Array attribute \"%s\" is currently unsupported." ) % name );
	}

	bool dataTypeOK = false;
	GLint size = 0;
	switch( attributeType )
	{
		case GL_INT :
		case GL_FLOAT :
			dataTypeOK = IECore::despatchTraitsTest<IECore::TypeTraits::IsNumericVectorTypedData>( value );
			size = 1;
			break;
		case GL_INT_VEC2 :
		case GL_FLOAT_VEC2 :
			dataTypeOK = IECore::despatchTraitsTest<IECore::TypeTraits::IsVec2VectorTypedData>( value );
			size = 2;
			break;
		case GL_INT_VEC3 :
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
	
	CachedConverterPtr converter = CachedConverter::defaultCachedConverter();
	ConstBufferPtr buffer = IECore::runTimeCast<const Buffer>( converter->convert( value  ) );
	
	m_memberData->values.push_back( new MemberData::VertexValue( attributeIndex, dataGLType, size, buffer, divisor ) );
}

Shader::Setup::ScopedBinding::ScopedBinding( const Setup &setup )
	:	m_previousProgram( 0 ), m_setup( setup )
{
	glGetIntegerv( GL_CURRENT_PROGRAM, &m_previousProgram );
	glUseProgram( m_setup.shader()->m_implementation->m_program );

	const vector<MemberData::ValuePtr> &values = m_setup.m_memberData->values;
	for( vector<MemberData::ValuePtr>::const_iterator it = values.begin(), eIt = values.end(); it != eIt; it++ )
	{
		(*it)->bind();
	}
}

Shader::Setup::ScopedBinding::~ScopedBinding()
{
	const vector<MemberData::ValuePtr> &values = m_setup.m_memberData->values;
	for( vector<MemberData::ValuePtr>::const_iterator it = values.begin(), eIt = values.end(); it != eIt; it++ )
	{
		(*it)->unbind();
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
		"in vec2 st;"
		""
		"varying out vec3 fragmentI;"
		"varying out vec3 fragmentN;"
		"varying out vec2 fragmentst;"
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
		""
		"	fragmentst = st;"
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

	static ShaderPtr s = new Shader( "", fragmentSource );
	return s;
}

ShaderPtr Shader::facingRatio()
{
	// the empty strings will be replaced with defaultVertexSource() and
	// defaultFragmentSource() in the constructor. it's important that we
	// pass the empty string here, so that Shader::vertexSource() will return
	// the empty string too, signifying to the funkier primitives (PointsPrimitive
	// being a good example) that it's ok to replace the vertex shader.
	static ShaderPtr s = new Shader( "", "" );
	return s;
}
