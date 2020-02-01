//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
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

#include "IECoreScene/ShaderNetworkAlgo.h"

#include "IECore/SimpleTypedData.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/regex.hpp"

#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

ShaderNetwork::Parameter ShaderNetworkAlgo::addShaders( ShaderNetwork *network, const ShaderNetwork *sourceNetwork, bool connections )
{
	std::unordered_map<InternedString, InternedString> handleMap;

	for( const auto &s : sourceNetwork->shaders() )
	{
		handleMap[s.first] = network->addShader( s.first, s.second.get() );
	}

	if( connections )
	{
		for( const auto &s : sourceNetwork->shaders() )
		{
			for( const auto &c : sourceNetwork->inputConnections( s.first ) )
			{
				network->addConnection(
					ShaderNetwork::Connection(
						ShaderNetwork::Parameter( handleMap[c.source.shader], c.source.name ),
						ShaderNetwork::Parameter( handleMap[c.destination.shader], c.destination.name )
					)
				);
			}
		}
	}

	return ShaderNetwork::Parameter(
		handleMap[sourceNetwork->getOutput().shader],
		sourceNetwork->getOutput().name
	);
}

namespace
{

void visitInputs( const ShaderNetwork *network, InternedString handle, std::unordered_set<InternedString> &visited )
{
	visited.insert( handle );
	for( const auto &c : network->inputConnections( handle ) )
	{
		visitInputs( network, c.source.shader, visited );
	}
}

} // namespace

void ShaderNetworkAlgo::removeUnusedShaders( ShaderNetwork *network )
{
	std::unordered_set<InternedString> visited;
	visitInputs( network, network->getOutput().shader, visited );

	auto shaders = network->shaders();
	for( auto it = shaders.begin(); it != shaders.end(); )
	{
		if( visited.find( it->first ) == visited.end() )
		{
			it = network->removeShader( it );
		}
		else
		{
			++it;
		}
	}
}

namespace
{

const InternedString g_swizzleHandle( "swizzle" );
const InternedString g_packHandle( "pack" );
const InternedString g_inParameterName( "in" );
const InternedString g_outParameterName( "out" );
const InternedString g_packInParameterNames[3] = { "in1", "in2", "in3" };
const boost::regex g_componentRegex( "^(.*)\\.([rgbxyz])$" );
static const char *g_vectorComponents[3] = { "x", "y", "z" };
static const char *g_colorComponents[3] = { "r", "g", "b" };

ShaderNetwork::Parameter convertComponentSuffix( const ShaderNetwork::Parameter &parameter, const std::string &suffix )
{
	int index;
	auto it = find( begin( g_vectorComponents ), end( g_vectorComponents ), suffix );
	if( it != end( g_vectorComponents ) )
	{
		index = it - begin( g_vectorComponents );
	}
	else
	{
		it = find( begin( g_colorComponents ), end( g_colorComponents ), suffix );
		assert( it != end( g_colorComponents ) );
		index = it - begin( g_colorComponents );
	}

	return ShaderNetwork::Parameter(
		parameter.shader,
		boost::replace_last_copy( parameter.name.string(), "." + suffix, "[" + to_string( index ) + "]" )
	);
}

} // namespace

void ShaderNetworkAlgo::convertOSLComponentConnections( ShaderNetwork *network )
{
	convertOSLComponentConnections( network, 10900 /* OSL 1.9 */ );
}

void ShaderNetworkAlgo::convertOSLComponentConnections( ShaderNetwork *network, int oslVersion )
{
	// Output parameters

	using ParameterMap = std::unordered_map<ShaderNetwork::Parameter, ShaderNetwork::Parameter>;
	ParameterMap outputConversions;
	for( const auto &s : network->shaders() )
	{
		ShaderNetwork::ConnectionRange inputConnections = network->inputConnections( s.first );
		for( ShaderNetwork::ConnectionIterator it = inputConnections.begin(); it != inputConnections.end(); )
		{
			// Copy and increment now so we still have a valid iterator
			// if we remove the connection.
			const ShaderNetwork::Connection connection = *it++;

			const Shader *sourceShader = network->getShader( connection.source.shader );
			if( !boost::starts_with( sourceShader->getType(), "osl:" ) )
			{
				continue;
			}

			boost::cmatch match;
			if( boost::regex_match( connection.source.name.c_str(), match, g_componentRegex ) )
			{
				if( oslVersion < 11000 )
				{
					// OSL doesn't support component-level connections,
					// so we emulate them by inserting a conversion shader.
					auto inserted = outputConversions.insert( { connection.source, ShaderNetwork::Parameter() } );
					if( inserted.second )
					{
						ShaderPtr swizzle = new Shader( "MaterialX/mx_swizzle_color_float", "osl:shader" );
						swizzle->parameters()["channels"] = new StringData( match[2] );
						const InternedString swizzleHandle = network->addShader( g_swizzleHandle, std::move( swizzle ) );
						network->addConnection( ShaderNetwork::Connection(
							ShaderNetwork::Parameter{ connection.source.shader, InternedString( match[1] ) },
							ShaderNetwork::Parameter{ swizzleHandle, g_inParameterName }
						) );
						inserted.first->second = { swizzleHandle, g_outParameterName };
					}
					network->removeConnection( connection );
					network->addConnection( { inserted.first->second, connection.destination } );
				}
				else
				{
					// OSL supports component-level connections, but uses `[0]`
					// rather than `.r` suffix style.
					network->removeConnection( connection );
					network->addConnection( { convertComponentSuffix( connection.source, match[2] ), connection.destination } );
				}
			}
		}
	}

	// Input parameters

	std::unordered_set<InternedString> convertedParameters;
	for( const auto &shader : network->shaders() )
	{
		if( !boost::starts_with( shader.second->getType(), "osl:" ) )
		{
			continue;
		}

		convertedParameters.clear();
		ShaderNetwork::ConnectionRange inputConnections = network->inputConnections( shader.first );
		for( ShaderNetwork::ConnectionIterator it = inputConnections.begin(); it != inputConnections.end(); )
		{
			// Copy and increment now so we still have a valid iterator
			// if we remove the connection.
			const ShaderNetwork::Connection connection = *it++;

			boost::cmatch match;
			if( boost::regex_match( connection.destination.name.c_str(), match, g_componentRegex ) )
			{
				// Connection into a color/vector component

				if( oslVersion < 11000 )
				{
					// OSL doesn't support component-level connections, so we must emulate
					// them by inserting shaders.

					const InternedString parameterName = match[1].str();
					auto inserted = convertedParameters.insert( parameterName );
					if( !inserted.second )
					{
						// Dealt with already, when we visited a different component of the same
						// parameter.
						network->removeConnection( connection );
						continue;
					}

					// All components won't necessarily have connections, so get
					// the values to fall back on for those that don't.
					V3f value( 0 );
					const Data *d = shader.second->parametersData()->member<Data>( parameterName );
					if( const V3fData *vd = runTimeCast<const V3fData>( d ) )
					{
						value = vd->readable();
					}
					else if( const Color3fData *cd = runTimeCast<const Color3fData>( d ) )
					{
						value = cd->readable();
					}

					// Make shader and set fallback values

					ShaderPtr packShader = new Shader( "MaterialX/mx_pack_color", "osl:shader" );
					for( int i = 0; i < 3; ++i )
					{
						packShader->parameters()[g_packInParameterNames[i]] = new FloatData( value[i] );
					}

					const InternedString packHandle = network->addShader( g_packHandle, std::move( packShader ) );

					// Make connections

					network->addConnection( { { packHandle, g_outParameterName }, { shader.first, parameterName } } );

					for( int i = 0; i < 3; ++i )
					{
						ShaderNetwork::Parameter source = network->input( { shader.first, parameterName.string() + "." + g_vectorComponents[i] } );
						if( !source )
						{
							source = network->input( { shader.first, parameterName.string() + "." + g_colorComponents[i] } );
						}
						if( source )
						{
							network->addConnection( { source, { packHandle, g_packInParameterNames[i] } } );
						}
					}

					network->removeConnection( connection );
				}
				else
				{
					// OSL supports component-level connections, but uses `[0]`
					// rather than `.r` suffix style.
					network->removeConnection( connection );
					network->addConnection( { connection.source, convertComponentSuffix( connection.destination, match[2] ) } );
				}
			}
		}
	}
}

namespace
{

const InternedString g_handle( "__handle" );
const InternedString g_defaultHandle( "shader" );
const std::string g_linkPrefix( "link:" );

ShaderNetwork::Parameter linkedParameter( const std::string &s )
{
	if( !boost::starts_with( s, g_linkPrefix ) )
	{
		return ShaderNetwork::Parameter();
	}

	size_t i = s.find( '.' );
	if( i == std::string::npos )
	{
		return ShaderNetwork::Parameter( s.substr( g_linkPrefix.size() ) );
	}
	else
	{
		return ShaderNetwork::Parameter( s.substr( g_linkPrefix.size(), i - g_linkPrefix.size() ), s.substr( i + 1 ) );
	}
}

} // namespace

ShaderNetworkPtr ShaderNetworkAlgo::convertObjectVector( const ObjectVector *network )
{
	ShaderNetworkPtr result = new ShaderNetwork;
	for( const auto &member : network->members() )
	{
		const Shader *shader = runTimeCast<const Shader>( member.get() );
		if( !shader )
		{
			continue;
		}

		InternedString handle = g_defaultHandle;
		ShaderPtr shaderCopy = shader->copy();
		std::vector<ShaderNetwork::Connection> connections;

		for( auto it = shaderCopy->parameters().begin(); it != shaderCopy->parameters().end(); )
		{
			bool erase = false;
			if( const auto *stringData = runTimeCast<const StringData>( it->second.get() ) )
			{
				if( it->first == g_handle )
				{
					handle = stringData->readable();
					erase = true;
				}
				else if( auto p = linkedParameter( stringData->readable() ) )
				{
					connections.push_back( { p, { InternedString(), it->first } } );
					erase = true;
				}
			}

			if( erase )
			{
				it = shaderCopy->parameters().erase( it );
			}
			else
			{
				++it;
			}
		}

		result->addShader( handle, std::move( shaderCopy ) );
		for( const auto &c : connections )
		{
			result->addConnection( { c.source, { handle, c.destination.name } } );
		}

		if( &member == &network->members().back() )
		{
			result->setOutput( { handle } );
		}
	}

	return result;
}
