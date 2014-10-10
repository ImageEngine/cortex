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

#include <iostream>

#include "IECore/SplineData.h"
#include "IECore/MessageHandler.h"

#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/Texture.h"
#include "IECoreGL/TextureLoader.h"
#include "IECoreGL/CachedConverter.h"

using namespace std;
using namespace IECoreGL;

//////////////////////////////////////////////////////////////////////////
// ShaderStateComponent::Implementation
//////////////////////////////////////////////////////////////////////////

class ShaderStateComponent::Implementation : public IECore::RefCounted
{

	public :
	
		Implementation()
			:	m_shaderLoader( ShaderLoader::defaultShaderLoader() ), m_textureLoader( TextureLoader::defaultTextureLoader() ), m_fragmentSource( "" ), m_geometrySource( "" ), m_vertexSource( "" ), 
				m_parameterMap( 0 ), m_shaderSetup( 0 )
		{
		}

		Implementation( ShaderLoaderPtr shaderLoader, TextureLoaderPtr textureLoader, const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource, IECore::ConstCompoundObjectPtr parameterValues )
			:	m_shaderLoader( shaderLoader ), m_textureLoader( textureLoader ), m_fragmentSource( fragmentSource ), m_geometrySource( geometrySource ),
				m_vertexSource( vertexSource ), m_parameterMap( parameterValues->copy() ), m_shaderSetup( 0 )
		{
		}

		ShaderLoader *shaderLoader()
		{
			return m_shaderLoader.get();
		}

		TextureLoader *textureLoader()
		{
			return m_textureLoader.get();
		}
		
		Shader::Setup *shaderSetup()
		{
			ensureShaderSetup();
			return m_shaderSetup.get();
		}
		
		const Shader::Setup *shaderSetup() const
		{
			ensureShaderSetup();
			return m_shaderSetup.get();
		}

		void addParametersToShaderSetup( Shader::Setup *shaderSetup ) const
		{
			const IECore::CompoundObject::ObjectMap &d = m_parameterMap->members();
			for( IECore::CompoundObject::ObjectMap::const_iterator it = d.begin(), eIt = d.end(); it != eIt; it++ )
			{
				const Shader::Parameter *p = shaderSetup->shader()->uniformParameter( it->first );
				if( !p )
				{
					continue;
				}

				if( p->type == GL_SAMPLER_2D )
				{
					ConstTexturePtr texture = 0;
					if(
						it->second->typeId() == IECore::ImagePrimitiveTypeId ||
						it->second->typeId() == IECore::CompoundDataTypeId ||
						it->second->typeId() == IECore::SplineffData::staticTypeId() ||
						it->second->typeId() == IECore::SplinefColor3fData::staticTypeId()
					)
					{
						texture = IECore::runTimeCast<const Texture>( CachedConverter::defaultCachedConverter()->convert( it->second.get() ) );
					}
					else if( it->second->typeId() == IECore::StringData::staticTypeId() )
					{
						const std::string &fileName = static_cast<const IECore::StringData *>( it->second.get() )->readable();
						if( fileName!="" )
						{
							texture = m_textureLoader->load( fileName );
						}
					}

					shaderSetup->addUniformParameter( it->first.value(), texture );
				}
				else if( it->second->isInstanceOf( IECore::DataTypeId ) )
				{
					shaderSetup->addUniformParameter( it->first.value(), boost::static_pointer_cast<const IECore::Data>( it->second ) );
				}
			}
		
		}

	private :

		ShaderLoaderPtr m_shaderLoader;
		TextureLoaderPtr m_textureLoader;
		std::string m_fragmentSource;
		std::string m_geometrySource;
		std::string m_vertexSource;
		IECore::CompoundObjectPtr m_parameterMap;		
		mutable Shader::SetupPtr m_shaderSetup;

		void ensureShaderSetup() const
		{
			if( m_shaderSetup )
			{
				return;
			}

			if( !m_parameterMap )
			{
				// we were default constructed, so we're just a facing ratio shader.
				m_shaderSetup = new Shader::Setup( Shader::facingRatio() );
				return;
			}

			// load a shader, create a setup, and add our parameters to it.
			ShaderPtr shader = m_shaderLoader->create( m_vertexSource, m_geometrySource, m_fragmentSource );
			m_shaderSetup = new Shader::Setup( shader );

			addParametersToShaderSetup( m_shaderSetup.get() );
		}

};

//////////////////////////////////////////////////////////////////////////
// ShaderStateComponent
//////////////////////////////////////////////////////////////////////////

StateComponent::Description<ShaderStateComponent> ShaderStateComponent::g_description;

ShaderStateComponent::ShaderStateComponent()
	:	m_implementation( new Implementation() )
{
}

ShaderStateComponent::ShaderStateComponent( ShaderLoaderPtr shaderLoader, TextureLoaderPtr textureLoader, const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource, IECore::ConstCompoundObjectPtr parameterValues )
	:	m_implementation( new Implementation( shaderLoader, textureLoader, vertexSource, geometrySource, fragmentSource, parameterValues ) )
{
}

void ShaderStateComponent::bind() const
{
}

ShaderLoader *ShaderStateComponent::shaderLoader()
{
	return m_implementation->shaderLoader();
}

TextureLoader *ShaderStateComponent::textureLoader()
{
	return m_implementation->textureLoader();
}
		
Shader::Setup *ShaderStateComponent::shaderSetup()
{
	return m_implementation->shaderSetup();
}

const Shader::Setup *ShaderStateComponent::shaderSetup() const
{
	return m_implementation->shaderSetup();
}

void ShaderStateComponent::addParametersToShaderSetup( Shader::Setup *shaderSetup ) const
{
	m_implementation->addParametersToShaderSetup( shaderSetup );
}
