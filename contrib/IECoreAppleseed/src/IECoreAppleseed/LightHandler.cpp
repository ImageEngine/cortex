//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#include <cassert>

#include "boost/format.hpp"

#include "IECoreAppleseed/private/LightHandler.h"
#include "IECoreAppleseed/ColorAlgo.h"
#include "IECoreAppleseed/ParameterAlgo.h"
#include "IECoreAppleseed/TextureAlgo.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "renderer/api/environment.h"
#include "renderer/api/environmentedf.h"
#include "renderer/api/environmentshader.h"
#include "renderer/api/light.h"

using namespace std;
using namespace boost;
using namespace IECore;
using namespace Imath;

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::LightHandler::LightHandler( asr::Scene &scene,
	const asf::SearchPaths &searchPaths ) : m_scene( scene ), m_searchPaths( searchPaths )
{
	m_mainAssembly = m_scene.assemblies().get_by_name( "assembly" );
	assert( m_mainAssembly );
}

void IECoreAppleseed::LightHandler::environment( const string &name, const string &handle,
	bool visible, const CompoundDataMap &parameters )
{
	m_environmentHandle = handle;
	m_environmentModel = name;
	m_environmentParams = convertParams( handle, parameters, true );
	m_environmentVisible = visible;
	createOrUpdateEnvironment();
}

void IECoreAppleseed::LightHandler::light( const string &name, const string &handle,
	const foundation::Transformd &transform, const CompoundDataMap &parameters )
{
	LightEntry light;
	light.model = name;
	light.parameters= convertParams( handle, parameters, false );
	light.transform = transform;
	m_lightMap[handle] = light;
	createOrUpdateLight( handle, light );
}

void IECoreAppleseed::LightHandler::illuminate( const string &lightHandle, bool on )
{
	// lights cannot be enabled or disabled in appleseed.
	// we just delete them or recreate them using the information
	// we saved when the light was declared.
	if( lightHandle == m_environmentHandle )
	{
		if( on )
		{
			// Check if light is already on.
			if(  !m_scene.environment_edfs().empty() )
			{
				return;
			}

			createOrUpdateEnvironment();
		}
		else
		{
			// Check if light is already off.
			if(  m_scene.environment_edfs().empty() )
			{
				return;
			}

			// Remove the environment light.
			m_scene.environment_edfs().clear();
			m_scene.environment_shaders().clear();
			m_scene.set_environment( asr::EnvironmentFactory().create( "environment", asr::ParamArray() ) );
		}
	}
	else
	{
		if( on )
		{
			// Check if light is already on.
			if( m_mainAssembly->lights().get_by_name( lightHandle.c_str() ) )
			{
				return;
			}

			LightMap::const_iterator lightIt = m_lightMap.find( lightHandle );

			if( lightIt == m_lightMap.end() )
			{
				return;
			}

			createOrUpdateLight( lightHandle, lightIt->second );
		}
		else
		{
			// remove the light from the project.
			if( asr::Light *light = m_mainAssembly->lights().get_by_name( lightHandle.c_str() ) )
			{
				m_mainAssembly->lights().remove( light );
			}
		}
	}
}

asr::ParamArray IECoreAppleseed::LightHandler::convertParams( const string &handle,
	const CompoundDataMap &parameters, bool isEnvironment ) const
{
	asr::ParamArray params;
	for( CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
	{
		string paramName = it->first.value();
		ConstDataPtr paramValue = it->second;

		// for environment lights convert the radiance_map parameter to a texture, instead of a color.
		if( isEnvironment && paramName == "radiance_map" )
		{
			if( paramValue->typeId() != StringDataTypeId )
			{
				msg( MessageHandler::Warning, "IECoreAppleseed::RendererImplementation::light", "Expected radianceMap parameter to be a string" );
				continue;
			}

			const string &fileName = static_cast<const StringData*>( paramValue.get() )->readable();
			string textureName = handle + "." + paramName;
			string textureInstanceName = TextureAlgo::createTextureEntity( m_scene.textures(), m_scene.texture_instances(), m_searchPaths, textureName, fileName );
			params.insert( "radiance", textureInstanceName.c_str() );
		}
		else
		{
			if( paramValue->typeId() == Color3fDataTypeId )
			{
				string colorName = handle + "." + paramName;
				const Color3f &col = static_cast<const Color3fData*>( paramValue.get() )->readable();
				colorName = ColorAlgo::createColorEntity( m_scene.colors(), col, colorName.c_str() );
				params.insert( paramName.c_str(), colorName.c_str() );
			}
			else
			{
				params.insert( paramName.c_str(), ParameterAlgo::dataToString( paramValue ) );
			}
		}
	}

	return params;
}

void IECoreAppleseed::LightHandler::createOrUpdateLight( const string &handle , const LightEntry &lightEntry )
{
	if( asr::Light *light = m_mainAssembly->lights().get_by_name( handle.c_str() ) )
	{
		light->get_parameters() = lightEntry.parameters;
		light->set_transform( lightEntry.transform );
	}
	else
	{
		asr::LightFactoryRegistrar factoryRegistrar;
		const asr::ILightFactory *factory = factoryRegistrar.lookup( lightEntry.model.c_str() );

		if( !factory )
		{
			msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::light", format( "Unknown light model \"%s\"." ) % lightEntry.model );
			return;
		}

		asf::auto_release_ptr<asr::Light> l( factory->create( handle.c_str(), lightEntry.parameters ) );
		l->set_transform( lightEntry.transform );
		m_mainAssembly->lights().insert( l );
	}
}

void IECoreAppleseed::LightHandler::createOrUpdateEnvironment()
{
	if( !m_scene.environment_edfs().empty() )
	{
		asr::EnvironmentEDF *light = m_scene.environment_edfs().get_by_index( 0 );
		light->get_parameters() = m_environmentParams;
	}
	else
	{
		asr::EnvironmentEDFFactoryRegistrar factoryRegistrar;
		const asr::IEnvironmentEDFFactory *factory = factoryRegistrar.lookup( m_environmentModel.c_str() );

		if( !factory )
		{
			msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::light", format( "Unknown light model \"%s\"." ) % m_environmentModel );
			return;
		}

		asf::auto_release_ptr<asr::EnvironmentEDF> l( factory->create( m_environmentHandle.c_str(), m_environmentParams ) );
		m_scene.environment_edfs().insert( l );
		m_scene.get_environment()->get_parameters().insert( "environment_edf", m_environmentHandle.c_str() );

		if( m_environmentVisible )
		{
			asr::EnvironmentShaderFactoryRegistrar factoryRegistrar;
			const asr::IEnvironmentShaderFactory *factory = factoryRegistrar.lookup( "edf_environment_shader" );
			asf::auto_release_ptr<asr::EnvironmentShader> envShader( factory->create( "environment_shader",
				asr::ParamArray().insert( "environment_edf", m_environmentHandle.c_str() ) ) );
			m_scene.environment_shaders().insert( envShader );

			m_scene.get_environment()->get_parameters().insert( "environment_shader", "environment_shader" );
		}
	}
}
