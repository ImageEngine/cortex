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

#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/Texture.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/TextureUnits.h"

#include "IECore/MessageHandler.h"

#include <iostream>

using namespace IECoreGL;
using namespace std;

StateComponent::Description<ShaderStateComponent> ShaderStateComponent::g_description;

ShaderStateComponent::ShaderStateComponent()
	:	m_shader( 0 ), m_parameterData( 0 )
{
}

ShaderStateComponent::ShaderStateComponent( ShaderPtr shader, IECore::ConstCompoundDataPtr parameterValues,
	const TexturesMap *textureParameterValues )
	:	m_shader( shader ), m_parameterData( parameterValues ? parameterValues->copy() : 0 )
{
	if( textureParameterValues )
	{
		m_textureParameters = *textureParameterValues;
	}
}
		
void ShaderStateComponent::bind() const
{
	if( m_shader )
	{
		m_shader->bind();
		if( m_parameterData )
		{
			const IECore::CompoundDataMap &d = m_parameterData->readable();
			for( IECore::CompoundDataMap::const_iterator it = d.begin(); it!=d.end(); it++ )
			{
				m_shader->setParameter( it->first, it->second );
			}
		}
		if( m_textureParameters.size() )
		{
			unsigned int i=0;
			const std::vector<GLenum> &texUnits = textureUnits();
			for( TexturesMap::const_iterator it=m_textureParameters.begin(); it!=m_textureParameters.end(); it++ )
			{
				glEnable( GL_TEXTURE_2D );
				if( i>=texUnits.size() )
				{
					IECore::msg( IECore::Msg::Warning, "ShaderStateComponent::bind", boost::format( "Not enough texture units - skipping texture for \"%s\"." ) % it->first );
					continue;					
				}
				
				glActiveTexture( texUnits[i] );
				it->second->bind();
				m_shader->setParameter( it->first, i );
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
	}
}

GLbitfield ShaderStateComponent::mask() const
{
	if( m_shader ) 
	{
		return m_shader->mask() | GL_TEXTURE_BIT;
	}
	else
	{
		return 0;
	}
}

ConstShaderPtr ShaderStateComponent::shader() const
{
	return m_shader;
}

ShaderPtr ShaderStateComponent::shader()
{
	return m_shader;
}

IECore::ConstCompoundDataPtr ShaderStateComponent::parameterValues() const
{
	return m_parameterData;
}

IECore::CompoundDataPtr ShaderStateComponent::parameterValues()
{
	return m_parameterData;
}

const ShaderStateComponent::TexturesMap &ShaderStateComponent::textureValues() const
{
	return m_textureParameters;
}

ShaderStateComponent::TexturesMap &ShaderStateComponent::textureValues()
{
	return m_textureParameters;
}
