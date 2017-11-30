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
#include "boost/format.hpp"

#include "renderer/api/material.h"
#include "renderer/api/scene.h"
#include "renderer/api/shadergroup.h"
#include "renderer/api/surfaceshader.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAppleseed/private/ShadingState.h"

using namespace IECore;
using namespace IECoreScene;
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
}

void IECoreAppleseed::ShadingState::addOSLShader( ConstShaderPtr shader )
{
	if( m_surfaceShader )
	{
		m_shaders.clear();
		m_surfaceShader = nullptr;
	}

	m_shaders.push_back( shader );
}

void IECoreAppleseed::ShadingState::setOSLSurface( ConstShaderPtr surface )
{
	if( m_surfaceShader )
	{
		m_shaders.clear();
		m_surfaceShader = nullptr;
	}

	m_surfaceShader = surface;
}

void IECoreAppleseed::ShadingState::shaderGroupHash( MurmurHash &hash ) const
{
	for( int i = 0, e = m_shaders.size(); i < e; ++i)
	{
		m_shaders[i]->hash( hash );
	}

	if( m_surfaceShader )
	{
		m_surfaceShader->hash( hash );
	}
}

string IECoreAppleseed::ShadingState::createShaderGroup( asr::Assembly& assembly, const string &name )
{
	string shaderGroupName = name + "_shader_group";
	if( !assembly.shader_groups().get_by_name( shaderGroupName.c_str() ) )
	{
		asf::auto_release_ptr<asr::ShaderGroup> sg( asr::ShaderGroupFactory::create( shaderGroupName.c_str() ) );
		assembly.shader_groups().insert( sg );
		editShaderGroup( assembly, name );
	}

	return shaderGroupName;
}

void IECoreAppleseed::ShadingState::editShaderGroup( asr::Assembly& assembly, const string &name )
{
	string shaderGroupName = name + "_shader_group";

	if( asr::ShaderGroup *sg = assembly.shader_groups().get_by_name( shaderGroupName.c_str() ) )
	{
		sg->clear();

		for( int i = 0, e = m_shaders.size(); i < e; ++i )
		{
			asr::ParamArray params = convertParameters( m_shaders[i]->parameters() );
			const StringData *handle = m_shaders[i]->parametersData()->member<StringData>( "__handle" );
			sg->add_shader( "shader", m_shaders[i]->getName().c_str(), handle ? handle->readable().c_str() : m_shaders[i]->getName().c_str(), params );

			if( handle )
			{
				addConnections( handle->readable(), m_shaders[i]->parameters(), sg );
			}
		}

		// surface
		{
			asr::ParamArray params = convertParameters( m_surfaceShader->parameters() );
			sg->add_shader( "surface", m_surfaceShader->getName().c_str(), "appleseedRenderer:surface", params );
			addConnections( "appleseedRenderer:surface", m_surfaceShader->parameters(), sg );
		}
	}
}

void IECoreAppleseed::ShadingState::materialHash( MurmurHash &hash ) const
{
	shaderGroupHash( hash );
	hash.append( m_shadingSamples );
}

string IECoreAppleseed::ShadingState::createMaterial( asr::Assembly &assembly, const string &name, const string &shaderGroupName )
{
	string materialName = name + "_material";
	if( !assembly.materials().get_by_name( materialName.c_str() ) )
	{
		asr::ParamArray params;

		string surfaceShaderName = name + "_surface_shader";
		if( !assembly.surface_shaders().get_by_name( surfaceShaderName.c_str() ) )
		{
			params.insert( "front_lighting_samples", m_shadingSamples );
			params.insert( "back_lighting_samples", m_shadingSamples );
			asf::auto_release_ptr<asr::SurfaceShader> surfaceShader( asr::PhysicalSurfaceShaderFactory().create( surfaceShaderName.c_str(), params ) );
			assembly.surface_shaders().insert( surfaceShader );
		}

		params.clear();
		params.insert( "surface_shader", surfaceShaderName.c_str() );
		params.insert( "osl_surface", shaderGroupName.c_str() );
		asf::auto_release_ptr<asr::Material> mat( asr::OSLMaterialFactory().create( materialName.c_str(), params ) );
		assembly.materials().insert( mat );
	}

	return materialName;
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
			size_t dot_pos = value.find_first_of( '.' );

			if( dot_pos == string::npos )
			{
				msg( Msg::Warning, "AppleseedRenderer", boost::format( "Parameter \"%s\" has unexpected value \"%s\" - expected value of the form \"link:sourceShader.sourceParameter" ) % it->first.string() % value );
				continue;
			}

			shaderGroup->add_connection( string( value, 5, dot_pos - 5 ).c_str(),
				string( value, dot_pos + 1).c_str(), shaderHandle.c_str(),
				it->first.c_str() );
		}
	}
}

bool IECoreAppleseed::ShadingState::valid() const
{
	return m_surfaceShader.get() != nullptr;
}
