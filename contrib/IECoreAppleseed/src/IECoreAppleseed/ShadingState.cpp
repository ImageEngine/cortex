//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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


#include <string>

#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/classification.hpp"

#include "renderer/api/material.h"
#include "renderer/api/surfaceshader.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAppleseed/private/ShadingState.h"
#include "IECoreAppleseed/private/AppleseedUtil.h"

using namespace IECore;
using namespace std;
using namespace boost;

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::ShadingState::ShadingState()
{
	m_shadingSamples = 1;
}

void IECoreAppleseed::ShadingState::setShadingSamples(int samples)
{
	m_shadingSamples = samples;
	updateHashes();
}

void IECoreAppleseed::ShadingState::addOSLShader( ConstShaderPtr shader )
{
	if( m_surfaceShader )
	{
		m_shaders.clear();
		m_surfaceShader = 0;
	}

	m_shaders.push_back( shader );
	updateHashes();
}

void IECoreAppleseed::ShadingState::setOSLSurface( ConstShaderPtr surface )
{
	if( m_surfaceShader )
	{
		m_shaders.clear();
		m_surfaceShader = 0;
	}

	m_surfaceShader = surface;
	updateHashes();
}

const MurmurHash &IECoreAppleseed::ShadingState::shaderGroupHash() const
{
	return m_shaderGroupHash;
}

string IECoreAppleseed::ShadingState::createShaderGroup( asr::Assembly& assembly )
{
	string shaderGroupName;

	asf::auto_release_ptr<asr::ShaderGroup> sg( asr::ShaderGroupFactory::create( "shadergroup" ) );

	for( int i = 0, e = m_shaders.size(); i < e; ++i)
	{
		asr::ParamArray params = convertParameters( m_shaders[i]->parameters() );
		const StringData *handle = m_shaders[i]->parametersData()->member<StringData>( "__handle" );
		sg->add_shader( "shader", m_shaders[i]->getName().c_str(), handle ? handle->readable().c_str() : m_shaders[i]->getName().c_str(), params );

		if( handle )
		{
			addConnections( handle->readable(), m_shaders[i]->parameters(), sg.get() );
		}
	}

	// surface
	{
		asr::ParamArray params = convertParameters( m_surfaceShader->parameters() );
		sg->add_shader( "surface", m_surfaceShader->getName().c_str(), "appleseedRenderer:surface", params );
		addConnections( "appleseedRenderer:surface", m_surfaceShader->parameters(), sg.get() );
		shaderGroupName = insertEntityWithUniqueName( assembly.shader_groups(), sg, m_surfaceShader->getName() + "_shadergroup" );
	}

	return shaderGroupName;
}


const MurmurHash&IECoreAppleseed::ShadingState::materialHash() const
{
	return m_materialHash;
}

string IECoreAppleseed::ShadingState::createMaterial( asr::Assembly &assembly, const std::string &shaderGroupName, const asf::SearchPaths &searchPaths, const string &alphaMap )
{
	asr::ParamArray params;

	params.insert( "front_lighting_samples", m_shadingSamples );
	params.insert( "back_lighting_samples", m_shadingSamples );
	asf::auto_release_ptr<asr::SurfaceShader> surfaceShader( asr::PhysicalSurfaceShaderFactory().create( "surface_shader", params ) );
	string surfaceShaderName = insertEntityWithUniqueName( assembly.surface_shaders(), surfaceShader, m_surfaceShader->getName() + "_surface_shader" );

	params.clear();
	params.insert( "surface_shader", surfaceShaderName.c_str() );
	params.insert( "osl_surface", shaderGroupName.c_str() );
	asf::auto_release_ptr<asr::Material> mat( asr::OSLMaterialFactory().create( "material", params ) );

	if( !alphaMap.empty() )
	{
		string alphaMapTextureInstanceName = createAlphaMapTextureEntity( assembly.textures(), assembly.texture_instances(), searchPaths, m_surfaceShader->getName() + "_alpha_map", alphaMap );
		mat->get_parameters().insert( "alpha_map", alphaMapTextureInstanceName.c_str() );
	}

	string materialName = insertEntityWithUniqueName( assembly.materials(), mat, m_surfaceShader->getName() + "_material" );
	return materialName;
}

void IECoreAppleseed::ShadingState::updateHashes()
{
	m_shaderGroupHash = MurmurHash();

	for( int i = 0, e = m_shaders.size(); i < e; ++i)
	{
		m_shaders[i]->hash( m_shaderGroupHash );
	}

	if( m_surfaceShader )
	{
		m_surfaceShader->hash( m_shaderGroupHash );
	}

	m_materialHash = m_shaderGroupHash;
	m_materialHash.append( m_shadingSamples );
}

asr::ParamArray IECoreAppleseed::ShadingState::convertParameters( const CompoundDataMap &parameters )
{
	asr::ParamArray params;

	for( CompoundDataMap::const_iterator it = parameters.begin(), eIt = parameters.end(); it != eIt; ++it )
	{
		if( it->first == "__handle" )
		{
			continue;
		}

		if( it->second->isInstanceOf( StringDataTypeId ) )
		{
			const std::string &value = static_cast<const StringData *>( it->second.get() )->readable();
			if( boost::starts_with( value, "link:" ) )
			{
				// this will be handled in declareConnections()
				continue;
			}
		}

		std::stringstream ss;

		const Data *data = it->second.get();
		switch( data->typeId() )
		{
			case FloatDataTypeId :
			{
				const float *p = static_cast<const FloatData *>( data )->baseReadable();
				ss << "float " << *p;
			}
			break;

			case IntDataTypeId :
			{
				const int *p = static_cast<const IntData *>( data )->baseReadable();
				ss << "int " << *p;
			}
			break;

			case V3fDataTypeId :
			{
				const float *p = static_cast<const V3fData *>( data )->baseReadable();
				ss << "vector " << p[0] << " " << p[1] << " " << p[2];
			}
			break;

			case Color3fDataTypeId :
			{
				const float *p = static_cast<const Color3fData *>( data )->baseReadable();
				ss << "color " << p[0] << " " << p[1] << " " << p[2];
			}
			break;

			case StringDataTypeId :
			{
				const std::string *p = &(static_cast<const StringData *>( data )->readable() );
				ss << "string " << *p;
			}
			break;

			default:
				msg( Msg::Warning, "AppleseedRenderer", boost::format( "Parameter \"%s\" has unsupported type \"%s\"" ) % it->first.string() % it->second->typeName() );
			break;
		}

		if( !ss.str().empty() )
		{
			params.insert( it->first.c_str(), ss.str() );
		}
	}

	return params;
}

void IECoreAppleseed::ShadingState::addConnections( const std::string &shaderHandle, const CompoundDataMap &parameters, asr::ShaderGroup *shaderGroup )
{
	for( CompoundDataMap::const_iterator it = parameters.begin(), eIt = parameters.end(); it != eIt; ++it )
	{
		if( it->second->typeId() != StringDataTypeId )
		{
			continue;
		}

		const std::string &value = static_cast<const StringData *>( it->second.get() )->readable();
		if( boost::starts_with( value, "link:" ) )
		{
			vector<string> splitValue;
			boost::algorithm::split( splitValue, value, is_any_of( "." ), token_compress_on );
			if( splitValue.size() != 2 )
			{
				msg( Msg::Warning, "AppleseedRenderer", boost::format( "Parameter \"%s\" has unexpected value \"%s\" - expected value of the form \"link:sourceShader.sourceParameter" ) % it->first.string() % value );
				continue;
			}

			shaderGroup->add_connection( splitValue[0].c_str() + 5, splitValue[1].c_str(), shaderHandle.c_str(), it->first.c_str() );
		}
	}
}

bool IECoreAppleseed::ShadingState::valid() const
{
	return m_surfaceShader.get() != 0;
}
