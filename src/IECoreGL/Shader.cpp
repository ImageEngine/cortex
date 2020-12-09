//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/Buffer.h"
#include "IECoreGL/CachedConverter.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/NumericTraits.h"
#include "IECoreGL/Selector.h"
#include "IECoreGL/Texture.h"
#include "IECoreGL/UniformFunctions.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TypeTraits.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/format.hpp"
#include "boost/tokenizer.hpp"

#include <iostream>
#include <vector>

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
				m_vertexShader( 0 ), m_geometryShader( 0 ), m_fragmentShader( 0 ), m_program( 0 ),
				m_csParameter( nullptr )
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
					glGetProgramInfoLog( m_program, logLength, nullptr, &log[0] );
					message = &log[0];
				}
				release();
				throw IECoreGL::Exception( message );
			}
			else if ( logLength > 1 )
			{
				std::string message;
				vector<char> log( logLength, ' ' );
				glGetProgramInfoLog( m_program, logLength, nullptr, &log[0] );
				message = &log[0];

				// os x spews warnings rather overzealously, so we split them
				// into warnings and debug messages. the debug messages will generally
				// be filtered out by the message level, but they're still there
				// if someone really wants them.
				std::string warning;
				std::string debug;
				typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
				Tokenizer lines( message, boost::char_separator<char>( "\n" ) );
				for( Tokenizer::iterator it = lines.begin(); it != lines.end(); ++it )
				{
					if( starts_with( *it, "WARNING: Output of vertex shader" ) && ends_with( *it, "not read by fragment shader" ) )
					{
						debug += *it + "\n";
					}
					else
					{
						warning += *it + "\n";
					}
				}

				if( debug.size() )
				{
					IECore::msg( IECore::Msg::Debug, "IECoreGL::Shader", debug );
				}
				if( warning.size() )
				{
					IECore::msg( IECore::Msg::Warning, "IECoreGL::Shader", warning );
				}
			}
			{
				// build the uniform parameter description map
				GLint numUniforms = 0;
				glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &numUniforms );
				GLint maxUniformNameLength = 0;
				glGetProgramiv( m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength );
				vector<char> nameChars( maxUniformNameLength );
				GLuint textureUnit = 0;
				for( int i=0; i<numUniforms; i++ )
				{
					Parameter p;
					glGetActiveUniform( m_program, i, maxUniformNameLength, nullptr, &p.size, &p.type, &nameChars[0] );
					p.location = glGetUniformLocation( m_program, &nameChars[0] );

					std::string name = &nameChars[0];

					// ignore native parameters
					if( 0 == name.compare( 0, 3, "gl_" ) )
					{
						continue;
					}

					if( p.size > 1 )
					{
						// remove the "[0]" from the end of the string
						size_t bracketPos = name.rfind( "[" );
						if( bracketPos != std::string::npos )
						{
							name = name.substr( 0, bracketPos );
						}
					}

					if( p.type == GL_SAMPLER_2D || p.type == GL_SAMPLER_3D )
					{
						// we assign a specific texture unit to each individual
						// sampler parameter - this makes it much easier to save
						// and restore state when applying nested Setups.
						p.textureUnit = textureUnit++;
					}
					else
					{
						p.textureUnit = 0;
					}

					m_uniformParameters[name] = p;

					if( name == "Cs" )
					{
						m_csParameter = &(m_uniformParameters[name]);
					}
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
					vector<char> nameChars( maxVertexNameLength );
					for( int i=0; i<numVertexs; i++ )
					{
						Parameter p;
						glGetActiveAttrib( m_program, i, maxVertexNameLength, nullptr, &p.size, &p.type, &nameChars[0] );
						p.location = glGetAttribLocation( m_program, &nameChars[0] );

						std::string name = &nameChars[0];

						// ignore native parameters
						if( 0 == name.compare( 0, 3, "gl_" ) )
						{
							continue;
						}

						/// \todo implement arrays
						if( p.size != 1 )
						{
							continue;
						}

						m_vertexAttributes[name] = p;
					}
				}
			}
		}

		~Implementation() override
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
				names.push_back( it->first );
			}
		}

		const Shader::Parameter *uniformParameter( const std::string &name ) const
		{
			ParameterMap::const_iterator it = m_uniformParameters.find( name );
			if( it != m_uniformParameters.end() )
			{
				return &(it->second);
			}
			return nullptr;
		}

		void vertexAttributeNames( std::vector<std::string> &names ) const
		{
			for( ParameterMap::const_iterator it = m_vertexAttributes.begin(); it != m_vertexAttributes.end(); it++ )
			{
				names.push_back( it->first );
			}
		}

		const Shader::Parameter *vertexAttribute( const std::string &name ) const
		{
			ParameterMap::const_iterator it = m_vertexAttributes.find( name );
			if( it != m_vertexAttributes.end() )
			{
				return &(it->second);
			}
			return nullptr;
		}

		const Shader::Parameter *csParameter() const
		{
			return m_csParameter;
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

		/// Maps from the uniform location to the parameter details.
		typedef std::map<std::string, Shader::Parameter> ParameterMap;
		ParameterMap m_uniformParameters;
		ParameterMap m_vertexAttributes;

		const Shader::Parameter *m_csParameter;

		void compile( const std::string &source, GLenum type, GLuint &shader )
		{
			if( source == "" )
			{
				return;
			}

			const char *s = source.c_str();
			shader = glCreateShader( type );
			glShaderSource( shader, 1, &s, nullptr );
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
				throw IECoreGL::Exception( message );
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

void Shader::vertexAttributeNames( std::vector<std::string> &names ) const
{
	m_implementation->vertexAttributeNames( names );
}

const Shader::Parameter *Shader::uniformParameter( const std::string &name ) const
{
	return m_implementation->uniformParameter( name );
}

const Shader::Parameter *Shader::vertexAttribute( const std::string &name ) const
{
	return m_implementation->vertexAttribute( name );
}

const Shader::Parameter *Shader::csParameter() const
{
	return m_implementation->csParameter();
}

bool Shader::Parameter::operator == ( const Shader::Parameter &other ) const
{
	return
		type == other.type &&
		size == other.size &&
		location == other.location &&
		textureUnit == other.textureUnit;
}

///////////////////////////////////////////////////////////////////////////////
// Setup implementation
///////////////////////////////////////////////////////////////////////////////

class Shader::Setup::MemberData : public IECore::RefCounted
{

	public :

		MemberData( ConstShaderPtr &s )
			:	shader( s ), hasCsValue( false )
		{
		}

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

			void bind() override
			{
				Buffer::ScopedBinding binding( *m_buffer );
				glEnableVertexAttribArrayARB( m_attributeIndex );
				glVertexAttribPointerARB( m_attributeIndex, m_size, m_type, false, 0, nullptr );
				glVertexAttribDivisorARB( m_attributeIndex, m_divisor );
			}

			void unbind() override
			{
				glVertexAttribDivisorARB( m_attributeIndex, 0 );
				glDisableVertexAttribArrayARB( m_attributeIndex );
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

			void bind() override
			{
				glActiveTexture( GL_TEXTURE0 + m_textureUnit );
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

			void unbind() override
			{
				glActiveTexture( GL_TEXTURE0 + m_textureUnit );
				glBindTexture( GL_TEXTURE_2D, m_previousTexture );
			}

			private :

				GLuint m_uniformIndex;
				GLuint m_textureUnit;
				ConstTexturePtr m_texture;
				GLint m_previousTexture;

		};

		// value class for specifying uniform values
		struct UniformFloatValue : public Value
		{

			UniformFloatValue( GLuint program, GLuint uniformIndex, unsigned char dimensions, GLsizei count, std::vector<GLfloat> &values )
				:	m_program( program ), m_uniformIndex( uniformIndex ), m_dimensions( dimensions ), m_count( count ), m_values( values )
			{
				m_previousValues.resize( m_values.size() );
			}

			void bind() override
			{
				glGetUniformfv( m_program, m_uniformIndex, &(m_previousValues[0]) );
				uniformFloatFunctions()[m_dimensions]( m_uniformIndex, m_count, &(m_values[0]) );
			}

			void unbind() override
			{
				uniformFloatFunctions()[m_dimensions]( m_uniformIndex, m_count, &(m_previousValues[0]) );
			}

			private :

				GLuint m_program;
				GLuint m_uniformIndex;
				unsigned char m_dimensions;
				GLsizei m_count;
				std::vector<GLfloat> m_values;
				std::vector<GLfloat> m_previousValues;

		};

		// value class for specifying uniform values
		struct UniformIntegerValue : public Value
		{

			UniformIntegerValue( GLuint program, GLuint uniformIndex, unsigned char dimensions, GLsizei count, std::vector<GLint> &values )
				:	m_program( program ), m_uniformIndex( uniformIndex ), m_dimensions( dimensions ), m_count( count ), m_values( values )
			{
				m_previousValues.resize( m_values.size() );
			}

			void bind() override
			{
				glGetUniformiv( m_program, m_uniformIndex, &(m_previousValues[0]) );
				uniformIntFunctions()[m_dimensions]( m_uniformIndex, m_count, &(m_values[0]) );
			}

			void unbind() override
			{
				uniformIntFunctions()[m_dimensions]( m_uniformIndex, m_count, &(m_previousValues[0]) );
			}

			private :

				GLuint m_program;
				GLuint m_uniformIndex;
				unsigned char m_dimensions;
				GLsizei m_count;
				std::vector<GLint> m_values;
				std::vector<GLint> m_previousValues;

		};

		struct UniformMatrixValue : public Value
		{
			UniformMatrixValue( GLuint program, GLuint uniformIndex, unsigned char dimensions0, unsigned char dimensions1, GLsizei count, std::vector<GLfloat> &values )
				:	m_program( program ), m_uniformIndex( uniformIndex ), m_dimensions0( dimensions0 ), m_dimensions1( dimensions1 ), m_count( count ), m_values( values )
			{
				m_previousValues.resize( m_values.size(), 0 );
			}

			void bind() override
			{
				glGetUniformfv( m_program, m_uniformIndex, &(m_previousValues[0]) );
				uniformMatrixFunctions()[m_dimensions0][m_dimensions1]( m_uniformIndex, m_count, GL_FALSE, &(m_values[0]) );
			}

			void unbind() override
			{
				uniformMatrixFunctions()[m_dimensions0][m_dimensions1]( m_uniformIndex, m_count, GL_FALSE, &(m_previousValues[0]) );
			}

			private :

				GLuint m_program;
				GLuint m_uniformIndex;
				unsigned char m_dimensions0;
				unsigned char m_dimensions1;
				GLsizei m_count;
				std::vector<GLfloat> m_values;
				std::vector<GLfloat> m_previousValues;

		};

		ConstShaderPtr shader;
		vector<ValuePtr> values;
		bool hasCsValue;

};

Shader::Setup::Setup( ConstShaderPtr shader )
	:	m_memberData( new MemberData( shader ) )
{
}

Shader::Setup::~Setup()
{
}

const Shader *Shader::Setup::shader() const
{
	return m_memberData->shader.get();
}

void Shader::Setup::addUniformParameter( const std::string &name, ConstTexturePtr value )
{
	const Parameter *p = m_memberData->shader->uniformParameter( name );
	if( !p || p->type != GL_SAMPLER_2D )
	{
		return;
	}

	m_memberData->values.push_back( new MemberData::TextureValue( p->location, p->textureUnit, value ) );
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
	const Parameter *p = m_memberData->shader->uniformParameter( name );
	if( !p )
	{
		return;
	}


	if( p->type == GL_BOOL || p->type == GL_INT || p->type == GL_INT_VEC2 || p->type == GL_INT_VEC3 )
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
			IECore::despatchTypedData< UniformDataConverter<vector<GLint> >, IECore::TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( const_cast<IECore::Data *>( value.get() ), converter );
		}
		if( !integers.size() )
		{
			IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Uniform parameter \"%s\" has unsuitable data type \%s\"" ) % name % value->typeName() );
		}
		else
		{
			unsigned char dimensions = 0;
			switch( p->type )
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
			m_memberData->values.push_back( new MemberData::UniformIntegerValue( m_memberData->shader->program(), p->location, dimensions, p->size, integers ) );
		}
	}
	else if( p->type == GL_FLOAT || p->type == GL_FLOAT_VEC2 || p->type == GL_FLOAT_VEC3 || p->type == GL_FLOAT_VEC4 )
	{
		// float value
		vector<GLfloat> floats;
		UniformDataConverter<vector<GLfloat> > converter( floats );

		IECore::despatchTypedData< UniformDataConverter<vector<GLfloat> >, IECore::TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( const_cast<IECore::Data *>( value.get() ), converter );
		if( !floats.size() )
		{
			IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Uniform parameter \"%s\" has unsuitable data type \%s\"" ) % name % value->typeName() );
			return;
		}
		else
		{
			unsigned char dimensions = 0;
			switch( p->type )
			{
				case GL_FLOAT :
					dimensions = 1;
					break;
				case GL_FLOAT_VEC2 :
					dimensions = 2;
					break;
				case GL_FLOAT_VEC3 :
					dimensions = 3;
					break;
				case GL_FLOAT_VEC4 :
					dimensions = 4;
					break;
			}
			m_memberData->values.push_back( new MemberData::UniformFloatValue( m_memberData->shader->program(), p->location, dimensions, p->size, floats ) );
		}

		if( name == "Cs" )
		{
			m_memberData->hasCsValue = true;
		}
	}
	else if( p->type == GL_FLOAT_MAT3 || p->type == GL_FLOAT_MAT4 )
	{
		// matrix value
		unsigned char dimensions0 = 0;
		unsigned char dimensions1 = 0;
		switch( p->type )
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
		IECore::despatchTypedData< UniformDataConverter<vector<GLfloat> >, IECore::TypeTraits::IsNumericBasedTypedData, DespatchTypedDataIgnoreError>( const_cast<IECore::Data *>( value.get() ), converter );
		if( int( floats.size() ) != dimensions0 * dimensions1 * p->size )
		{
			IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Matrix parameter \"%s\" requires %d values but value of type \%s\" provided %d" ) % name % (dimensions0 * dimensions1) % value->typeName() % floats.size() );
			return;
		}

		m_memberData->values.push_back( new MemberData::UniformMatrixValue( m_memberData->shader->program(), p->location, dimensions0, dimensions1, p->size, floats ) );
	}
	else
	{
		IECore::msg( IECore::Msg::Warning, "Shader::Setup::addUniformParameter", format( "Uniform parameter \"%s\" has unsupported OpenGL type \%d\"" ) % name % p->type );
	}
}

void Shader::Setup::addVertexAttribute( const std::string &name, IECore::ConstDataPtr value, GLuint divisor )
{
	const Parameter *p = m_memberData->shader->vertexAttribute( name );
	if( !p )
	{
		return;
	}

	if( p->size > 1 )
	{
		IECore::msg( IECore::Msg::Warning, "Shader::Setup::addVertexAttribute", format( "Array attribute \"%s\" is currently unsupported." ) % name );
	}

	GLenum dataGLType = glType( value.get() );
	if( !dataGLType )
	{
		IECore::msg( IECore::Msg::Warning, "Shader::Setup::addVertexAttribute", format( "Vertex attribute \"%s\" has unsuitable data type \%s\"" ) % name % value->typeName() );
	}

	GLint size = 0;
	switch( p->type )
	{
		case GL_INT :
		case GL_FLOAT :
			size = 1;
			break;
		case GL_INT_VEC2 :
		case GL_FLOAT_VEC2 :
			size = 2;
			break;
		case GL_INT_VEC3 :
		case GL_FLOAT_VEC3 :
			size = 3;
			break;
		case GL_FLOAT_VEC4 :
			size = 4;
			break;
		default :
			IECore::msg( IECore::Msg::Warning, "Shader::Setup::addVertexAttribute", format( "Vertex attribute \"%s\" has unsupported OpenGL type \%d\"" ) % name % p->type );
			return;
	}

	CachedConverterPtr converter = CachedConverter::defaultCachedConverter();
	ConstBufferPtr buffer = IECore::runTimeCast<const Buffer>( converter->convert( value.get() ) );

	m_memberData->values.push_back( new MemberData::VertexValue( p->location, dataGLType, size, buffer, divisor ) );
}

bool Shader::Setup::hasCsValue() const
{
	return m_memberData->hasCsValue;
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


	if( Selector *currentSelector = Selector::currentSelector() )
	{
		if( currentSelector->mode() == Selector::IDRender )
		{
			currentSelector->pushIDShader( m_setup.shader() );
		}
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
	if( Selector *currentSelector = Selector::currentSelector() )
	{
		if( currentSelector->mode() == Selector::IDRender )
		{
			currentSelector->popIDShader();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// common shader sources
///////////////////////////////////////////////////////////////////////////////

const std::string &Shader::defaultVertexSource()
{
	static string s =

		"#version 120\n"
		""
		"#if __VERSION__ <= 120\n"
		"#define in attribute\n"
		"#define out varying\n"
		"#endif\n"
		""
		"uniform vec3 Cs = vec3( 1, 1, 1 );"
		"uniform bool vertexCsActive = false;"
		"uniform bool vertexNActive = false;"
		""
		"in vec3 vertexP;"
		"in vec3 vertexN;"
		"in vec2 vertexuv;"
		"in vec3 vertexCs;"
		""
		"out vec3 geometryI;"
		"out vec3 geometryP;"
		"out vec3 geometryN;"
		"out vec2 geometryuv;"
		"out vec3 geometryCs;"
		""
		"out vec3 fragmentI;"
		"out vec3 fragmentP;"
		"out vec3 fragmentN;"
		"out vec2 fragmentuv;"
		"out vec3 fragmentCs;"
		""
		"void main()"
		"{"
		"	vec4 pCam = gl_ModelViewMatrix * vec4( vertexP, 1 );"
		"	gl_Position = gl_ProjectionMatrix * pCam;"
		"	geometryP = pCam.xyz;"
		""
		"	if( vertexNActive )"
		"	{"
		"		geometryN = normalize( gl_NormalMatrix * vertexN );"
		"	}"
		"	else"
		"	{"
		"		geometryN = vec3( 0.0, 0.0, 1.0 );"
		"	}"
		""
		"	if( gl_ProjectionMatrix[2][3] != 0.0 )"
		"	{"
		"		geometryI = normalize( -pCam.xyz );"
		"	}"
		"	else"
		"	{"
		"		geometryI = vec3( 0, 0, -1 );"
		"	}"
		""
		"	geometryuv = vertexuv;"
		"	geometryCs = mix( Cs, vertexCs, float( vertexCsActive ) );"
		""
		"	fragmentI = geometryI;"
		"	fragmentP = geometryP;"
		"	fragmentN = geometryN;"
		"	fragmentuv = geometryuv;"
		"	fragmentCs = geometryCs;"
		"}";

	return s;
}

const std::string &Shader::defaultGeometrySource()
{
	static string s = "";
	return s;
}

const std::string &Shader::defaultFragmentSource()
{
	static string s =

		"#if __VERSION__ <= 120\n"
		"#define in varying\n"
		"#endif\n"
		""
		"in vec3 fragmentI;"
		"in vec3 fragmentN;"
		"in vec3 fragmentCs;"
		""
		"void main()"
		"{"
		"	vec3 Nf = faceforward( fragmentN, -fragmentI, fragmentN );"
		"	float f = dot( normalize( fragmentI ), normalize(Nf) );"
		"	gl_FragColor = vec4( f * fragmentCs, 1 );"
		"}";

	return s;
}

const std::string &Shader::constantFragmentSource()
{
	static string s =

		"#if __VERSION__ <= 120\n"
		"#define in varying\n"
		"#endif\n"
		""
		"in vec3 fragmentCs;"
		""
		"void main()"
		"{"
		"	gl_FragColor = vec4( fragmentCs, 1 );"
		"}";

	return s;
}

const std::string &Shader::lambertFragmentSource()
{
	static string s =

		"#if __VERSION__ <= 120\n"
		"#define in varying\n"
		"#endif\n"
		""
		"#include \"IECoreGL/Lights.h\"\n"
		"#include \"IECoreGL/ColorAlgo.h\"\n"
		"#include \"IECoreGL/Diffuse.h\"\n"

		"in vec3 fragmentP;"
		"in vec3 fragmentN;"
		"in vec3 fragmentCs;"
		""
		"void main()"
		"{"
		"	vec3 n = normalize( fragmentN );"
		""
		"	vec3 L[ gl_MaxLights ];"
		"	vec3 Cl[ gl_MaxLights ];"
		""
		"	lights( fragmentP, Cl, L, gl_MaxLights );"
		""
		"	vec3 Cdiffuse = ieDiffuse( fragmentP, n, Cl, L, gl_MaxLights );"
		""
		"	gl_FragColor = vec4( Cdiffuse, 1.0 );"
		"}";

	return s;
}

///////////////////////////////////////////////////////////////////////////////
// definitions for useful simple shaders
///////////////////////////////////////////////////////////////////////////////

Shader *Shader::constant()
{
	static ShaderPtr s = new Shader( "", constantFragmentSource() );
	return s.get();
}

Shader *Shader::facingRatio()
{
	// the empty strings will be replaced with defaultVertexSource() and
	// defaultFragmentSource() in the constructor. it's important that we
	// pass the empty string here, so that Shader::vertexSource() will return
	// the empty string too, signifying to the funkier primitives (PointsPrimitive
	// being a good example) that it's ok to replace the vertex shader.
	static ShaderPtr s = new Shader( "", "" );
	return s.get();
}
