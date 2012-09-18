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

#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/Texture.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/TextureUnits.h"
#include "IECoreGL/ToGLTextureConverter.h"
#include "IECoreGL/SplineToGLTextureConverter.h"

#include "IECore/SplineData.h"
#include "IECore/MessageHandler.h"

#include <iostream>

using namespace IECoreGL;
using namespace std;

StateComponent::Description<ShaderStateComponent> ShaderStateComponent::g_description;

ShaderStateComponent::ShaderStateComponent()
	:	m_shaderManager(0), m_textureLoader(0), m_fragmentShader(""), m_vertexShader( "" ), 
		m_parameterMap( IECore::CompoundObjectPtr( new IECore::CompoundObject() ) ), m_shader( 0 )
{
}

ShaderStateComponent::ShaderStateComponent( const ShaderStateComponent &other ) :
	m_shaderManager( other.m_shaderManager ), m_textureLoader( other.m_textureLoader ), m_fragmentShader( other.m_fragmentShader ), 
	m_vertexShader( other.m_vertexShader ), m_parameterMap( other.m_parameterMap ? other.m_parameterMap->copy() : IECore::CompoundObjectPtr( new IECore::CompoundObject() ) ), 
	m_shader( other.m_shader )
{
}

ShaderStateComponent::ShaderStateComponent( ShaderManagerPtr shaderManager, TextureLoaderPtr textureLoader, const std::string vertexShader, const std::string fragmentShader, IECore::ConstCompoundObjectPtr parameterValues ) :
	m_shaderManager( shaderManager ), m_textureLoader( textureLoader ), m_fragmentShader( fragmentShader ), 
	m_vertexShader( vertexShader ), m_parameterMap( parameterValues ? parameterValues->copy() : IECore::CompoundObjectPtr(new IECore::CompoundObject()) ), 
	m_shader( 0 )
{
}

void ShaderStateComponent::bind() const
{
	ConstShaderPtr s = shader();

	if ( s )
	{
		s->bind();

		// deallocate dirty textures.
		for ( std::set<std::string>::const_iterator sit = m_dirtyTextures.begin(); sit != m_dirtyTextures.end(); sit++ )
		{
			m_textureParameters.erase( *sit );
		}
		m_dirtyTextures.clear();

		const IECore::CompoundObject::ObjectMap &d = m_parameterMap->members();

		for( IECore::CompoundObject::ObjectMap::const_iterator it = d.begin(); it!=d.end(); it++ )
		{
			// \todo: Consider shader caching the parameter list and returning a reference for the internal cached list.
			GLint paramIndex;
			try 
			{
				paramIndex = s->uniformParameterIndex( it->first );
			}
			catch( ... )
			{
				// silently ignore non-existent parameters.
				// \todo: Maybe it should raise exceptions for parameters that were defined on the shader call like the original implementation...
				continue;
			}

			// test if a given parameter is supposed to be a texture and do the proper conversion
			if ( s->uniformParameterType( paramIndex ) == Texture::staticTypeId() )
			{
				// make sure our texture local cache has the texture
				if ( m_textureParameters.find( it->first ) == m_textureParameters.end() )
				{
					if ( it->second->typeId() == IECore::ImagePrimitiveTypeId || it->second->typeId() == IECore::CompoundDataTypeId )
					{
						m_textureParameters[ it->first ] = IECore::staticPointerCast<Texture>( ToGLTextureConverter( IECore::staticPointerCast<IECore::ImagePrimitive>(it->second) ).convert() );
					}
					else if ( it->second->typeId() == IECore::SplineffData::staticTypeId() || it->second->typeId() == IECore::SplinefColor3fData::staticTypeId() )
					{
						m_textureParameters[ it->first ] = IECore::staticPointerCast<Texture>( SplineToGLTextureConverter( it->second ).convert() );
					}
					else if ( it->second->typeId() == IECore::StringData::staticTypeId() )
					{
						const std::string &fileName = IECore::staticPointerCast<IECore::StringData>(it->second)->readable();
						if( fileName!="" )
						{
							TexturePtr texture = m_textureLoader->load( fileName );
							if( texture )
							{
								m_textureParameters[ it->first ] = texture;
							}
						}
					}
					else
					{
						throw Exception( "Invalid data type!" );
					}
				}
			}
			else
			{
				IECore::DataPtr data = IECore::dynamicPointerCast< IECore::Data >( it->second );
				if ( data )
				{
					m_shader->setUniformParameter( paramIndex, data );
				}
				else
				{
					throw Exception( boost::str( boost::format( "Non-Data type assigned to parameter %s!" ) % it->first.value()) );
				}
			}
		}

		if( m_textureParameters.size() )
		{
			unsigned int i=0;
			const std::vector<GLenum> &texUnits = textureUnits();

			glEnable( GL_TEXTURE_2D );

			for( TexturesMap::const_iterator it=m_textureParameters.begin(); it!=m_textureParameters.end(); it++ )
			{
				if( i>=texUnits.size() )
				{
					IECore::msg( IECore::Msg::Warning, "ShaderStateComponent::bind", boost::format( "Not enough texture units - skipping texture for \"%s\"." ) % it->first );
					continue;
				}

				glActiveTexture( texUnits[i] );
				it->second->bind();
				m_shader->setUniformParameter( it->first, i );
				i++;
			}
		}
		else
		{
			glDisable( GL_TEXTURE_2D );
		}
	}
	else
	{
		if( GLEW_VERSION_2_0 )
		{
			glUseProgram( 0 );
		}
		glDisable( GL_TEXTURE_2D );
	}
}

void ShaderStateComponent::addShaderParameterValue( const std::string &paramName, IECore::ConstObjectPtr paramValue )
{
	if ( m_parameterMap )
	{
		// add to internal dict.
		m_parameterMap->members()[ paramName ] = paramValue->copy();

		// also check if the old parameter being replaced was already bound as a texture and mark the texture for removal in that case.
		TexturesMap::const_iterator it = m_textureParameters.find( paramName );
		if ( it != m_textureParameters.end() )
		{
			m_dirtyTextures.insert( paramName );
		}
	}
}

ShaderPtr ShaderStateComponent::shader() const
{
	// \todo Can we break const? Necessary on the bind() const method...
	return const_cast< ShaderStateComponent * >(this)->shader();
}

ShaderPtr ShaderStateComponent::shader()
{
	if( !m_shader )
	{
		if ( m_shaderManager )
		{
			// load the shader
			m_shader = m_shaderManager->create( m_vertexShader, m_fragmentShader );

			// query default parameters
			vector<string> allParameters;
			m_shader->uniformParameterNames( allParameters );

			const IECore::CompoundObject::ObjectMap &d = m_parameterMap->members();

			// Add shader parameters not provided before with default values.
			for ( vector<string>::const_iterator it = allParameters.begin(); it != allParameters.end(); it++ )
			{
				// if the parameter name is already defined, then skip it.
				if ( d.find( *it ) != d.end() )
					continue;

				IECore::DataPtr paramValue;
				try
				{
					paramValue = m_shader->getUniformParameterDefault( *it );
				}
				catch( ... )
				{
					// ignore unsupported parameters.
					continue;
				}
				m_parameterMap->members()[ *it ] = paramValue;
			}
		}
		else if ( m_vertexShader.size() == 0 && m_fragmentShader.size() == 0 )
		{
			// get the default shader.
			m_shader = Shader::facingRatio();
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ShaderStateComponent::shader", "No ShaderManager defined while requesting for shader." );
		}
	}
	return m_shader;
}
