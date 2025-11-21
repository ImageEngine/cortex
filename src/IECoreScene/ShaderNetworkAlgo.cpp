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

#include "IECore/DataAlgo.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/StringAlgo.h"
#include "IECore/RampData.h"
#include "IECore/TypeTraits.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/container/flat_map.hpp"
#include "boost/regex.hpp"

#include <array>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// `addShaders()`
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
// `removeUnusedShaders()`
//////////////////////////////////////////////////////////////////////////

namespace
{

void visitInputs( const ShaderNetwork *network, InternedString handle, std::unordered_set<InternedString> &visited )
{
	if( visited.insert( handle ).second )
	{
		for( const auto &c : network->inputConnections( handle ) )
		{
			visitInputs( network, c.source.shader, visited );
		}
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

//////////////////////////////////////////////////////////////////////////
// Component connection adapters
//////////////////////////////////////////////////////////////////////////

namespace
{

const InternedString g_splitAdapterHandle( "splitAdapter" );
const InternedString g_splitAdapterComponent( "splitAdapter:component" );
const InternedString g_splitAdapterInParameter( "splitAdapter:inParameter" );
const InternedString g_splitAdapterOutParameter( "splitAdapter:outParameter" );

const InternedString g_joinAdapterHandle( "joinAdapter" );
const InternedString g_joinAdapterInParameters( "joinAdapter:inParameters" );
const InternedString g_joinAdapterOutParameter( "joinAdapter:outParameter" );

const boost::regex g_componentRegex( "^(.*)\\.([rgbaxyz])$" );
array<string, 3> g_vectorComponents = { "x", "y", "z" };
array<string, 4> g_colorComponents = { "r", "g", "b", "a" };
BoolDataPtr g_trueData( new BoolData( true ) );

const InternedString g_inParameterName( "in" );
const InternedString g_outParameterName( "out" );
array<InternedString, 4> g_packInParameterNames = { "in1", "in2", "in3", "in4" };

struct SplitAdapter
{
	InternedString component;
	ConstShaderPtr shader;
	InternedString inParameter;
	InternedString outParameter;
};

// One adapter for each output component.
using ComponentsToSplitAdapters = boost::container::flat_map<InternedString, SplitAdapter>;
using SplitAdapterMap = std::unordered_map<string, ComponentsToSplitAdapters>;

SplitAdapterMap &splitAdapters()
{
	static SplitAdapterMap g_map;
	return g_map;
}

const SplitAdapter &findSplitAdapter( const std::string &destinationShaderType, InternedString component )
{
	const auto &map = splitAdapters();
	const string typePrefix = destinationShaderType.substr( 0, destinationShaderType.find_first_of( ':' ) );

	for( const auto &key : { typePrefix, string( "*" ) } )
	{
		auto it = map.find( key );
		if( it != map.end() )
		{
			auto cIt = it->second.find( component );
			if( cIt != it->second.end() )
			{
				return cIt->second;
			}
		}
	}

	throw IECore::Exception(
		"No component split adapter registered"
	);
}

struct JoinAdapter
{
	ConstShaderPtr shader;
	std::array<InternedString, 4> inParameters;
	InternedString outParameter;
};

using TypesToJoinAdapters = boost::container::flat_map<IECore::TypeId, JoinAdapter>;
using JoinAdapterMap = std::unordered_map<string, TypesToJoinAdapters>;

JoinAdapterMap &joinAdapters()
{
	static JoinAdapterMap g_map;
	return g_map;
}

const JoinAdapter &findJoinAdapter( const std::string &destinationShaderType, IECore::TypeId destinationParameterType )
{
	const auto &map = joinAdapters();
	const string typePrefix = destinationShaderType.substr( 0, destinationShaderType.find_first_of( ':' ) );

	for( const auto &key : { typePrefix, string( "*" ) } )
	{
		auto it = map.find( key );
		if( it != map.end() )
		{
			auto tIt = it->second.find( destinationParameterType );
			if( tIt != it->second.end() )
			{
				return tIt->second;
			}
		}
	}

	throw IECore::Exception(
		"No component join adapter registered"
	);
}

const bool g_defaultAdapterRegistrations = [] () {

	ShaderPtr splitter = new Shader(
		"MaterialX/mx_swizzle_color_float", "osl:shader"
	);

	for( auto c : string( "rgbaxyz" ) )
	{
		splitter->parameters()["channels"] = new StringData( { c } );
		ShaderNetworkAlgo::registerSplitAdapter(
			"*", string( { c } ),
			splitter.get(),
			g_inParameterName,
			g_outParameterName
		);
	}

	ShaderPtr joiner = new Shader( "MaterialX/mx_pack_color", "osl:shader" );
	for( auto t : { V2iDataTypeId, V3iDataTypeId, V2fDataTypeId, V3fDataTypeId, Color3fDataTypeId, Color4fDataTypeId } )
	{
		ShaderNetworkAlgo::registerJoinAdapter(
			"*", t,
			joiner.get(),
			g_packInParameterNames,
			g_outParameterName
		);
	}

	return true;
} ();

const InternedString &componentConnectionAdapterLabel()
{
	static InternedString ret( "cortex_autoAdapter" );
	return ret;
}

bool isSplitAdapter( const Shader *shader, InternedString &component, InternedString &inParameter, InternedString &outParameter )
{
	if( auto *d = shader->blindData()->member<InternedStringData>( g_splitAdapterComponent ) )
	{
		auto inP = shader->blindData()->member<InternedStringData>( g_splitAdapterInParameter );
		auto outP = shader->blindData()->member<InternedStringData>( g_splitAdapterOutParameter );
		if( inP && outP )
		{
			component = d->readable();
			inParameter = inP->readable();
			outParameter = outP->readable();
		}
		return true;
	}
	else if( auto *b = shader->blindData()->member<BoolData>( componentConnectionAdapterLabel() ) )
	{
		// Legacy format.
		if( b->readable() && shader->getName() == "MaterialX/mx_swizzle_color_float" )
		{
			component = shader->parametersData()->member<StringData>( "channels" )->readable();
			inParameter = g_inParameterName;
			outParameter = g_outParameterName;
			return true;
		}
	}

	return false;
}

bool isJoinAdapter( const Shader *shader, std::array<InternedString, 4> &inParameters, InternedString &outParameter )
{
	if( auto d = shader->blindData()->member<InternedStringVectorData>( g_joinAdapterInParameters ) )
	{
		auto o = shader->blindData()->member<InternedStringData>( g_joinAdapterOutParameter );
		if( o )
		{
			for( size_t i = 0; i < inParameters.size(); ++i )
			{
				inParameters[i] = i < d->readable().size() ? d->readable()[i] : InternedString();
			}
			outParameter = o->readable();
			return true;
		}
	}
	else if( auto *b = shader->blindData()->member<BoolData>( componentConnectionAdapterLabel() ) )
	{
		// Legacy format.
		if( b->readable() && shader->getName() == "MaterialX/mx_pack_color" )
		{
			inParameters = g_packInParameterNames;
			outParameter = g_outParameterName;
			return true;
		}
	}

	return false;
}

} // namespace

void ShaderNetworkAlgo::addComponentConnectionAdapters( ShaderNetwork *network, std::string targetPrefix )
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
			if( !boost::starts_with( sourceShader->getType(), targetPrefix ) )
			{
				continue;
			}

			boost::cmatch match;
			if( boost::regex_match( connection.source.name.c_str(), match, g_componentRegex ) )
			{
				// Insert a conversion shader to handle connection from component.
				auto inserted = outputConversions.insert( { connection.source, ShaderNetwork::Parameter() } );
				if( inserted.second )
				{
					InternedString component = match[2].str();
					const SplitAdapter &adapter = findSplitAdapter( sourceShader->getType(), component );

					ShaderPtr adapterShader = adapter.shader->copy();
					adapterShader->blindData()->writable()[g_splitAdapterComponent] = new InternedStringData( component );
					adapterShader->blindData()->writable()[g_splitAdapterInParameter] = new InternedStringData( adapter.inParameter );
					adapterShader->blindData()->writable()[g_splitAdapterOutParameter] = new InternedStringData( adapter.outParameter );

					const InternedString adapterHandle = network->addShader( g_splitAdapterHandle, std::move( adapterShader ) );
					network->addConnection( ShaderNetwork::Connection(
						ShaderNetwork::Parameter{ connection.source.shader, InternedString( match[1] ) },
						ShaderNetwork::Parameter{ adapterHandle, adapter.inParameter }
					) );
					inserted.first->second = { adapterHandle, adapter.outParameter };
				}
				network->removeConnection( connection );
				network->addConnection( { inserted.first->second, connection.destination } );
			}
		}
	}

	// Input parameters

	std::unordered_set<InternedString> convertedParameters;
	for( const auto &shader : network->shaders() )
	{
		if( !boost::starts_with( shader.second->getType(), targetPrefix ) )
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
				const InternedString parameterName = match[1].str();

				auto inserted = convertedParameters.insert( parameterName );
				if( !inserted.second )
				{
					// Dealt with already, when we visited a different component of the same
					// parameter.
					network->removeConnection( connection );
					continue;
				}

				// Insert a conversion shader to handle connection from component

				const Data *parameterValue = shader.second->parametersData()->member<Data>( parameterName );
				if( !parameterValue )
				{
					throw IECore::Exception(
						boost::str( boost::format(
							"No value found for parameter `%1%.%2%`"
						) % shader.first % parameterName )
					);
				}

				// Make adapter shader.

				const JoinAdapter &adapter = findJoinAdapter( shader.second->getType(), parameterValue->typeId() );
				ShaderPtr adapterShader = adapter.shader->copy();
				adapterShader->blindData()->writable()[g_joinAdapterInParameters] = new InternedStringVectorData(
					vector<InternedString>( adapter.inParameters.begin(), adapter.inParameters.end() )
				);
				adapterShader->blindData()->writable()[g_joinAdapterOutParameter] = new InternedStringData( adapter.outParameter );

				// Set fallback values for adapter input parameters (since all may not receive connections).

				dispatch(
					parameterValue,
					[&] ( auto *d ) {
						using DataType = typename std::remove_const_t<std::remove_pointer_t<decltype( d )>>;
						if constexpr(
							TypeTraits::IsVecTypedData<DataType>::value ||
							std::is_same_v<DataType, Color3fData> ||
							std::is_same_v<DataType, Color4fData>
						)
						{
							using ValueType = typename DataType::ValueType;
							using BaseType = typename ValueType::BaseType;
							for( size_t i = 0; i < ValueType::dimensions(); ++i )
							{
								if( !adapter.inParameters[i].string().empty() )
								{
									adapterShader->parameters()[adapter.inParameters[i]] = new TypedData<BaseType>(
										d->readable()[i]
									);
								}
							}
						}
					}
				);

				// Add shader to network and make connections.

				const InternedString adapterHandle = network->addShader( g_joinAdapterHandle, std::move( adapterShader ) );
				network->addConnection( { { adapterHandle, adapter.outParameter }, { shader.first, parameterName } } );

				for( int i = 0; i < 4; ++i )
				{
					if( adapter.inParameters[i].string().empty() )
					{
						continue;
					}

					ShaderNetwork::Parameter source = network->input( { shader.first, parameterName.string() + "." + g_colorComponents[i] } );
					if( !source && i < 3 )
					{
						source = network->input( { shader.first, parameterName.string() + "." + g_vectorComponents[i] } );
					}
					if( source )
					{
						network->addConnection( { source, { adapterHandle, adapter.inParameters[i] } } );
					}
				}

				network->removeConnection( connection );
			}
		}
	}
}

void ShaderNetworkAlgo::removeComponentConnectionAdapters( ShaderNetwork *network )
{
	std::vector< IECore::InternedString > toRemove;

	InternedString component;
	InternedString inParameter;
	std::array<InternedString, 4> inParameters;
	InternedString outParameter;

	for( const auto &s : network->shaders() )
	{
		if( isSplitAdapter( s.second.get(), component, inParameter, outParameter ) )
		{
			ShaderNetwork::Parameter source = network->input( ShaderNetwork::Parameter( s.first, inParameter ) );
			if( !source )
			{
				throw IECore::Exception( boost::str(
					boost::format(
						"removeComponentConnectionAdapters : \"%1%.%2%\" has no input"
					) % s.first.string() % inParameter.string()
				) );
			}
			source.name = source.name.string() + "." + component.string();

			const ShaderNetwork::ConnectionRange outputConnections = network->outputConnections( s.first );
			for( auto connectionIt = outputConnections.begin(); connectionIt != outputConnections.end(); )
			{
				// Copy and increment now so we still have a valid iterator when we
				// remove the connection.
				const ShaderNetwork::Connection connection = *connectionIt++;
				network->removeConnection( connection );
				network->addConnection( { source, connection.destination } );
			}

			toRemove.push_back( s.first );
		}
		else if( isJoinAdapter( s.second.get(), inParameters, outParameter ) )
		{
			std::array<ShaderNetwork::Parameter, 4> componentInputs;
			for( size_t i = 0; i < inParameters.size(); ++i )
			{
				if( !inParameters[i].string().empty() )
				{
					componentInputs[i] = network->input( { s.first, inParameters[i] } );
				}
			}

			const ShaderNetwork::ConnectionRange outputConnections = network->outputConnections( s.first );
			for( auto connectionIt = outputConnections.begin(); connectionIt != outputConnections.end(); )
			{
				// Copy and increment now so we still have a valid iterator when we
				// remove the connection.
				const ShaderNetwork::Connection connection = *connectionIt++;
				network->removeConnection( connection );

				const Data *destinationValue = network->getShader( connection.destination.shader )->parametersData()->member<Data>( connection.destination.name );
				const bool isColor = runTimeCast<const Color3fData>( destinationValue ) || runTimeCast<const Color4fData>( destinationValue );

				for( size_t i = 0; i < componentInputs.size(); ++i )
				{
					if( !componentInputs[i] )
					{
						continue;
					}

					InternedString component = isColor ? g_colorComponents.at( i ) : g_vectorComponents.at( i );
					network->addConnection(
						{ componentInputs[i], { connection.destination.shader, connection.destination.name.string() + "." + component.string() } }
					);
				}
			}

			toRemove.push_back( s.first );
		}
	}

	for( IECore::InternedString &handle : toRemove )
	{
		network->removeShader( handle );
	}
}

void ShaderNetworkAlgo::registerSplitAdapter( const std::string &destinationShaderType, IECore::InternedString component, const IECoreScene::Shader *adapter, IECore::InternedString inParameter, IECore::InternedString outParameter )
{
	splitAdapters()[destinationShaderType][component] = { component, adapter->copy(), inParameter, outParameter };
}

void ShaderNetworkAlgo::deregisterSplitAdapter( const std::string &destinationShaderType, IECore::InternedString component )
{
	splitAdapters()[destinationShaderType].erase( component );
}

void ShaderNetworkAlgo::registerJoinAdapter( const std::string &destinationShaderType, IECore::TypeId destinationParameterType, const IECoreScene::Shader *adapter, const std::array<IECore::InternedString, 4> &inParameters, IECore::InternedString outParameter )
{
	joinAdapters()[destinationShaderType][destinationParameterType] = { adapter->copy(), inParameters, outParameter };
}

void ShaderNetworkAlgo::deregisterJoinAdapter( const std::string &destinationShaderType, IECore::TypeId destinationParameterType )
{
	joinAdapters()[destinationShaderType].erase( destinationParameterType );
}

//////////////////////////////////////////////////////////////////////////
// OSL Utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

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
		auto cIt = find( begin( g_colorComponents ), end( g_colorComponents ), suffix );
		assert( cIt != end( g_colorComponents ) );
		index = cIt - begin( g_colorComponents );
	}

	return ShaderNetwork::Parameter(
		parameter.shader,
		boost::replace_last_copy( parameter.name.string(), "." + suffix, "[" + to_string( index ) + "]" )
	);
}

void convertOSLComponentConnections( ShaderNetwork *network, int oslVersion )
{
	if( oslVersion < 11000 )
	{
		// OSL doesn't support component-level connections,
		// so we emulate them by inserting conversion shaders for OSL nodes.
		ShaderNetworkAlgo::addComponentConnectionAdapters( network, "osl:" );
		return;
	}

	// We have an OSL version that supports component connections.
	// But OSL uses `[0]` rather than `.r` suffix style, so translate the connection names

	for( const auto &s : network->shaders() )
	{
		bool destIsOSL = boost::starts_with( s.second->getType(), "osl:" );

		ShaderNetwork::ConnectionRange inputConnections = network->inputConnections( s.first );
		for( ShaderNetwork::ConnectionIterator it = inputConnections.begin(); it != inputConnections.end(); )
		{
			// Copy and increment now so we still have a valid iterator
			// if we remove the connection.
			const ShaderNetwork::Connection connection = *it++;

			const Shader *sourceShader = network->getShader( connection.source.shader );

			bool sourceIsOSL = boost::starts_with( sourceShader->getType(), "osl:" );
			ShaderNetwork::Parameter newSourceName, newDestName;

			boost::cmatch match;
			if( sourceIsOSL && boost::regex_match( connection.source.name.c_str(), match, g_componentRegex ) )
			{
				newSourceName = convertComponentSuffix( connection.source, match[2] );
			}
			if( destIsOSL && boost::regex_match( connection.destination.name.c_str(), match, g_componentRegex ) )
			{
				newDestName = convertComponentSuffix( connection.destination, match[2] );
			}
			if( newSourceName.shader.string().size() || newDestName.shader.string().size() )
			{
				network->removeConnection( connection );
				network->addConnection( {
					newSourceName.shader.string().size() ? newSourceName : connection.source,
					newDestName.shader.string().size() ? newDestName : connection.destination
				} );
			}
		}
	}
}

} // namespace


void ShaderNetworkAlgo::convertToOSLConventions( ShaderNetwork *network, int oslVersion )
{
	expandRamps( network, "osl:" );

	// \todo - it would be a bit more efficient to integrate this, and only traverse the network once,
	// but I don't think it's worth duplicated the code - fix this up once this call is standard and we
	// deprecate and remove convertOSLComponentConnections
	convertOSLComponentConnections( network, oslVersion);

}

//////////////////////////////////////////////////////////////////////////
// `convertObjectVector()`
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
// Ramp handling
//////////////////////////////////////////////////////////////////////////

namespace
{

std::string_view stringViewFromMatch( const std::string &s, const boost::smatch &match, int index )
{
	return std::string_view( s.data() + match.position( index ), match.length( index ) );
}

std::tuple< const std::string*, const std::string*, const std::string*, const std::string*, const std::string* >
lookupRampParameterSuffixes( const std::string &shaderName )
{
	// We seem to be able to identify shaders that should use the PRMan convention by whether they start
	// with one of the PRMan prefixes.
	// NOTE : This will fail if a shader is loaded from an explicit path, rather than being found in the
	// search path, because the shader name will include the full file path. We consider this an
	// acceptable failure, because shaders should be found in the search paths.
	if( boost::starts_with( shaderName, "Pxr" ) ||  boost::starts_with( shaderName, "Lama" ) )
	{
		// The convention used by the PRMan shader library.
		static const std::string positions( "_Knots" );
		static const std::string floatValues( "_Floats" );
		static const std::string colorValues( "_Colors" );
		static const std::string basis( "_Interpolation" );
		static const std::string count( "" );
		return { &positions, &floatValues, &colorValues, &basis, &count };
	}
	else
	{
		// The convention used by the OSL shaders that we ship with Gaffer.
		static const std::string positions( "Positions" );
		static const std::string values( "Values" );
		static const std::string basis( "Basis" );
		return { &positions, &values, &values, &basis, nullptr };
	}
}

template<typename Ramp>
int expandRamp( const InternedString &name, const Ramp &ramp, CompoundDataMap &newParameters, const std::string &shaderName )
{
	StringDataPtr basisData = new StringData();
	std::string &basis = basisData->writable();

	typedef TypedData< vector<typename Ramp::XType> > XTypedVectorData;
	typename XTypedVectorData::Ptr positionsData = new XTypedVectorData();
	auto &positions = positionsData->writable();
	positions.reserve( ramp.points.size() );
	typedef TypedData< vector<typename Ramp::YType> > YTypedVectorData;
	typename YTypedVectorData::Ptr valuesData = new YTypedVectorData();
	auto &values = valuesData->writable();

	ramp.toOSL( basis, positions, values );

	auto [ positionsSuffix, floatValuesSuffix, colorValuesSuffix, basisSuffix, countSuffix ] = lookupRampParameterSuffixes( shaderName );

	newParameters[ name.string() + *positionsSuffix ] = positionsData;
	if constexpr( std::is_same_v< typename Ramp::YType, float > )
	{
		newParameters[ name.string() + *floatValuesSuffix ] = valuesData;
	}
	else
	{
		newParameters[ name.string() + *colorValuesSuffix ] = valuesData;
	}
	newParameters[ name.string() + *basisSuffix ] = basisData;

	if( countSuffix )
	{
		newParameters[ name.string() + *countSuffix ] = new IntData( positionsData->readable().size() );
	}

	return positionsData->readable().size();
}

void ensureParametersCopy(
	const IECore::CompoundDataMap &parameters,
	IECore::CompoundDataPtr &parametersDataCopy, CompoundDataMap *&parametersCopy
)
{
	if( !parametersDataCopy )
	{
		parametersDataCopy = new CompoundData();
		parametersCopy = &parametersDataCopy->writable();
		*parametersCopy = parameters;
	}
}

IECore::ConstCompoundDataPtr collapseRampParametersInternal( const IECore::ConstCompoundDataPtr &parametersData, const std::string &shaderName )
{

	auto [ positionsSuffix, floatValuesSuffix, colorValuesSuffix, basisSuffix, countSuffix ] = lookupRampParameterSuffixes( shaderName );

	const CompoundDataMap &parameters( parametersData->readable() );
	CompoundDataPtr newParametersData;
	CompoundDataMap *newParameters = nullptr;

	for( const auto &maybeBasis : parameters )
	{
		if( !boost::ends_with( maybeBasis.first.string(), *basisSuffix ) )
		{
			continue;
		}
		const StringData *basis = runTimeCast<const StringData>( maybeBasis.second.get() );
		if( !basis )
		{
			continue;
		}


		std::string prefix = maybeBasis.first.string().substr( 0, maybeBasis.first.string().size() - basisSuffix->size() );
		const IECore::InternedString positionsName = prefix + *positionsSuffix;
		const FloatVectorData *floatPositions = parametersData->member<const FloatVectorData>( positionsName );
		if( !floatPositions )
		{
			continue;
		}

		IECore::InternedString countName;
		const IntData *countData = nullptr;

		if( countSuffix )
		{
			countName = prefix + *countSuffix;
			countData = parametersData->member<const IntData>( countName );

			if( !countData )
			{
				IECore::msg(
					Msg::Error, "ShaderNetworkAlgo",
					"Using spline format that expects count parameter, but no int count parameter found matching \"" + countName.string() + "\""
				);
			}
			else
			{
				if( (int)floatPositions->readable().size() != countData->readable() )
				{
					IECore::msg(
						Msg::Error, "ShaderNetworkAlgo",
						"Spline count \"" + countName.string() + "\" does not match length of data: " + std::to_string( countData->readable() ) + " != " + std::to_string( floatPositions->readable().size() ) + "\""
					);
				}
			}
		}

		IECore::InternedString valuesName = prefix + *floatValuesSuffix;
		IECore::DataPtr foundRamp;
		if( const FloatVectorData *floatValues = parametersData->member<const FloatVectorData>( valuesName ) )
		{
			RampffData::Ptr rampData = new RampffData();
			rampData->writable().fromOSL(
				basis->readable(), floatPositions->readable(), floatValues->readable(), prefix
			);
			foundRamp = rampData;
		}
		else
		{
			valuesName = prefix + *colorValuesSuffix;
			if( const Color3fVectorData *color3Values = parametersData->member<const Color3fVectorData>( valuesName ) )
			{
				RampfColor3fData::Ptr rampData = new RampfColor3fData();
				rampData->writable().fromOSL(
					basis->readable(), floatPositions->readable(), color3Values->readable(), prefix
				);
				foundRamp = rampData;
			}
			else if( const Color4fVectorData *color4Values = parametersData->member<const Color4fVectorData>( valuesName ) )
			{
				RampfColor4fData::Ptr rampData = new RampfColor4fData();
				rampData->writable().fromOSL(
					basis->readable(), floatPositions->readable(), color4Values->readable(), prefix
				);
				foundRamp = rampData;
			}

		}

		if( foundRamp )
		{
			ensureParametersCopy( parameters, newParametersData, newParameters );
			newParameters->erase( maybeBasis.first );
			newParameters->erase( positionsName );
			newParameters->erase( valuesName );
			if( countData )
			{
				newParameters->erase( countName );
			}

			(*newParameters)[prefix] = foundRamp;

		}
	}

	if( newParametersData )
	{
		return newParametersData;
	}
	else
	{
		return parametersData;
	}
}

const std::string g_oslShader( "osl:shader" );

const std::string g_colorToArrayAdapter( "Utility/__ColorToArray" );
const std::string g_floatToArrayAdapter( "Utility/__FloatToArray" );

const int maxArrayInputAdapterSize = 32;
const InternedString g_arrayInputNames[maxArrayInputAdapterSize] = {
	"in0", "in1", "in2", "in3", "in4", "in5", "in6", "in7", "in8", "in9",
	"in10", "in11", "in12", "in13", "in14", "in15", "in16", "in17", "in18", "in19",
	"in20", "in21", "in22", "in23", "in24", "in25", "in26", "in27", "in28", "in29",
	"in30", "in31"
};
const InternedString g_arrayOutputNames[maxArrayInputAdapterSize + 1] = {
	"unused", "out1", "out2", "out3", "out4", "out5", "out6", "out7", "out8", "out9",
	"out10", "out11", "out12", "out13", "out14", "out15", "out16", "out17", "out18", "out19",
	"out20", "out21", "out22", "out23", "out24", "out25", "out26", "out27", "out28", "out29",
	"out30", "out31", "out32"
};

const boost::regex g_rampElementRegex( "^(.*)\\[(.*)\\]\\.y(.*)$" );
const boost::regex g_splineAdapterInRegex( "^in([0-9]+)(\\..*)?$" );

struct RampInputAdapterParameters
{
	InternedString adapterHandle;
	size_t origSize;
	size_t expandedSize;
	int knotOffset;
};

template< typename TypedRamp >
RampInputAdapterParameters createRampInputAdapter(
	ShaderNetwork *network, const TypedRamp &ramp,
	const IECore::CompoundDataMap &newParameters, const IECore::InternedString &rampParameterName,
	const ShaderNetwork::Parameter &destination
)
{
	using ValueVectorData = TypedData< std::vector< typename TypedRamp::YType > >;

	if( ramp.interpolation == RampInterpolation::MonotoneCubic )
	{
		IECore::msg(
			Msg::Error, "ShaderNetworkAlgo",
			"Cannot connect adaptors to ramp when using monotoneCubic interpolation: " +
			destination.shader.string() + "." + destination.name.string()
		);
		return { "", 0, 0, 0 };
	}

	IECore::InternedString splineValuesName = rampParameterName.string() + "Values";
	auto findValues = newParameters.find( splineValuesName );
	const ValueVectorData *splineValuesData = findValues != newParameters.end() ? runTimeCast<const ValueVectorData>( findValues->second.get() ) : nullptr;
	if( !splineValuesData )
	{
		throw IECore::Exception( "Internal failure in convertToOSLConventions - expandRamp did not create values." );
	}

	const std::vector< typename TypedRamp::YType > &splineValues = splineValuesData->readable();

	if( splineValues.size() > maxArrayInputAdapterSize )
	{
		IECore::msg(
			Msg::Error, "ShaderNetworkAlgo",
			"Cannot handle input to " + destination.shader.string() + "." + destination.name.string() +
			" : expanded spline has " + std::to_string( splineValues.size() ) +
			" control points, but max input adapter size is " + std::to_string( maxArrayInputAdapterSize )
		);
		return { "", 0, 0, 0 };
	}

	// Using this adapter depends on Gaffer being available, but I guess we don't really
	// care about use cases outside Gaffer ( and in terms of using exported USD elsewhere,
	// this ramp representation is only used in Gaffer's ramp shaders, so it's not very
	// useful if you don't have access to Gaffer shaders anyway ).
	ShaderPtr adapter = new Shader(
		std::is_same< TypedRamp, RampfColor3f >::value ? g_colorToArrayAdapter : g_floatToArrayAdapter,
		g_oslShader
	);

	for( unsigned int i = 0; i < splineValues.size(); i++ )
	{
		adapter->parameters()[ g_arrayInputNames[i] ] = new TypedData< typename TypedRamp::YType >( splineValues[i] );
	}

	InternedString adapterHandle = network->addShader( destination.shader.string() + "_" + rampParameterName.string() + "InputArrayAdapter", std::move( adapter ) );
	network->addConnection( ShaderNetwork::Connection(
		{ adapterHandle, g_arrayOutputNames[ splineValues.size() ] },
		{ destination.shader, splineValuesName }
	) );

	return { adapterHandle, ramp.points.size(), splineValues.size(), ramp.oslStartPointMultiplicity() - 1 };
}

} // namespace

void ShaderNetworkAlgo::collapseRamps( ShaderNetwork *network, std::string targetPrefix )
{
	std::vector< IECore::InternedString > adapters;

	for( auto [name, shader] : network->shaders() )
	{
		if( !boost::starts_with( shader->getType(), targetPrefix ) )
		{
			continue;
		}

		bool isRampAdapter = shader->getType() == g_oslShader && (
			shader->getName() == g_colorToArrayAdapter || shader->getName() == g_floatToArrayAdapter
		);

		if( isRampAdapter )
		{
			adapters.push_back( name );
			continue;
		}

		// For nodes which aren't spline adapters, we just need to deal with any parameters that can become ramps
		ConstCompoundDataPtr collapsed = collapseRampParametersInternal( shader->parametersData(), shader->getName() );
		if( collapsed != shader->parametersData() )
		{
			// \todo - this const_cast is ugly, although safe because if the return from collapseRampParameterInternals
			// doesn't match the input, it is freshly allocated. Now that collapseRampParameters is fully
			// deprecated, and no longer visible publicly, collapseRampParametersInternal could
			// just return a non-const new parameter data, or nullptr if no changes are needed.
			network->setShader( name, std::move( new Shader( shader->getName(), shader->getType(), const_cast< CompoundData *>( collapsed.get() ) ) ) );
		}
	}

	for( InternedString &name : adapters )
	{
		// For all adapters we create, there will be a single output, but it doesn't hurt to have the
		// generality of this being a loop just in case.
		for( auto output : network->outputConnections( name ) )
		{
			const std::string &splineValuesName = output.destination.name.string();
			if( !boost::ends_with( splineValuesName, "Values" ) )
			{
				IECore::msg(
					Msg::Error, "ShaderNetworkAlgo", "Invalid spline parameter name \"" + splineValuesName + "\""
				);
				continue;
			}

			InternedString rampName = string_view( splineValuesName ).substr( 0, splineValuesName.size() - 6 );

			const IECoreScene::Shader *targetShader = network->getShader( output.destination.shader );
			if( !targetShader )
			{
				throw IECore::Exception(
					"Invalid connection to shader that doesn't exist \"" + output.destination.shader.string() + "\""
				);
			}
			const IECore::CompoundDataMap &targetParameters = targetShader->parameters();

			int targetRampKnotOffset = -1;
			int targetRampSize = -1;
			auto targetParameterIt = targetParameters.find( rampName );
			if( targetParameterIt != targetParameters.end() )
			{
				if( const RampffData *findRampffData = runTimeCast<const RampffData>( targetParameterIt->second.get() ) )
				{
					const Rampff &ramp = findRampffData->readable();
					if( ramp.interpolation != RampInterpolation::MonotoneCubic )
					{
						targetRampKnotOffset = ramp.oslStartPointMultiplicity() - 1;
						targetRampSize = ramp.points.size();
					}
				}
				else if( const RampfColor3fData *findRampfColor3fData = runTimeCast<const RampfColor3fData>( targetParameterIt->second.get() ) )
				{
					const RampfColor3f &ramp = findRampfColor3fData->readable();
					if( ramp.interpolation != RampInterpolation::MonotoneCubic )
					{
						targetRampKnotOffset = ramp.oslStartPointMultiplicity() - 1;
						targetRampSize = ramp.points.size();
					}
				}
				else if( const RampfColor4fData *findRampfColor4fData = runTimeCast<const RampfColor4fData>( targetParameterIt->second.get() ) )
				{
					const RampfColor4f &ramp = findRampfColor4fData->readable();
					if( ramp.interpolation != RampInterpolation::MonotoneCubic )
					{
						targetRampKnotOffset = ramp.oslStartPointMultiplicity() - 1;
						targetRampSize = ramp.points.size();
					}
				}
			}

			if( targetRampKnotOffset == -1 )
			{
				IECore::msg(
					Msg::Error, "ShaderNetworkAlgo",
					"Invalid connection to spline parameter that doesn't exist or can't accept connections \"" +
					output.destination.shader.string() + "." + output.destination.name.string() + "\""
				);
				continue;
			}

			for( auto input : network->inputConnections( name ) )
			{

				const std::string &adapterDestName = input.destination.name.string();
				boost::smatch match;
				if( !boost::regex_match( adapterDestName, match, g_splineAdapterInRegex ) )
				{
					IECore::msg(
						Msg::Error, "ShaderNetworkAlgo", "Invalid spline adapter input name \"" + adapterDestName + "\""
					);
					continue;
				}

				int elementId =
					StringAlgo::toInt( stringViewFromMatch( adapterDestName, match, 1 ) )
					- targetRampKnotOffset;

				if( elementId < 0 || elementId >= targetRampSize )
				{
					// The likely cause of elements that don't map to the collapsed ramp is that this connection
					// was created to handle endpoint duplication.
					continue;
				}

				InternedString origDestName;
				if( match[2].matched )
				{
					origDestName = StringAlgo::concat(
						rampName.string(), "[", std::to_string( elementId ), "].y",
						stringViewFromMatch( adapterDestName, match, 2 )
					);
				}
				else
				{
					origDestName = StringAlgo::concat(
						rampName.string(), "[", std::to_string( elementId ), "].y"
					);
				}

				network->addConnection( {
					{ input.source.shader, input.source.name},
					{ output.destination.shader, origDestName }
				} );
			}
		}
		network->removeShader( name );
	}
}

void ShaderNetworkAlgo::expandRamps( ShaderNetwork *network, std::string targetPrefix )
{
	for( const auto &s : network->shaders() )
	{
		if( !boost::starts_with( s.second->getType(), targetPrefix ) )
		{
			continue;
		}

		const CompoundDataMap &origParameters = s.second->parameters();

		CompoundDataPtr newParametersData;
		CompoundDataMap *newParameters = nullptr;

		for( const auto &[name, value] : origParameters )
		{
			if( const RampfColor3fData *colorRamp = runTimeCast<const RampfColor3fData>( value.get() ) )
			{
				ensureParametersCopy( origParameters, newParametersData, newParameters );
				newParameters->erase( name );
				expandRamp( name, colorRamp->readable(), *newParameters, s.second->getName() );
			}
			else if( const RampffData *floatRamp = runTimeCast<const RampffData>( value.get() ) )
			{
				ensureParametersCopy( origParameters, newParametersData, newParameters );
				newParameters->erase( name );
				expandRamp( name, floatRamp->readable(), *newParameters, s.second->getName() );
			}
		}

		if( !newParameters )
		{
			// No ramps to convert
			continue;
		}

		// currentRampArrayAdapters holds array adapters that we need to use to hook up inputs to
		// spline parameters that were converted from ramp. It is indexed by the name of a ramp
		// parameter for the shader, and holds the name of the adapter shader, and the offset we
		// need to use when accessing the knot vector.

		std::map< IECore::InternedString, RampInputAdapterParameters > currentRampArrayAdapters;

		std::vector< ShaderNetwork::Connection > connectionsToAdd;
		ShaderNetwork::ConnectionRange inputConnections = network->inputConnections( s.first );
		for( ShaderNetwork::ConnectionIterator it = inputConnections.begin(); it != inputConnections.end(); )
		{
			// Copy and increment now so we still have a valid iterator
			// if we remove the connection.
			const ShaderNetwork::Connection connection = *it++;

			const std::string destName = connection.destination.name.string();
			boost::smatch rampElementMatch;
			if( !boost::regex_match( destName, rampElementMatch, g_rampElementRegex) )
			{
				continue;
			}

			IECore::InternedString parameterName( stringViewFromMatch( destName, rampElementMatch, 1 ) );
			auto findParameter = origParameters.find( parameterName );
			if( findParameter == origParameters.end() )
			{
				continue;
			}

			const RampfColor3fData* colorRampData = runTimeCast<const RampfColor3fData>( findParameter->second.get() );
			const RampffData* floatRampData = runTimeCast<const RampffData>( findParameter->second.get() );

			if( !( colorRampData || floatRampData ) )
			{
				continue;
			}

			// Insert a conversion shader to handle connection to component
			auto [ adapterIter, newlyInserted ] = currentRampArrayAdapters.insert( { parameterName, RampInputAdapterParameters() } );
			if( newlyInserted )
			{
				if( colorRampData )
				{
					adapterIter->second = createRampInputAdapter(
						network, colorRampData->readable(), *newParameters, parameterName, connection.destination
					);
				}
				else
				{
					adapterIter->second = createRampInputAdapter(
						network, floatRampData->readable(), *newParameters, parameterName, connection.destination
					);
				}
			}

			network->removeConnection( connection );

			const RampInputAdapterParameters &adapterParms = adapterIter->second;

			if( adapterParms.adapterHandle.string().size() == 0 )
			{
				// Can't form new connection, createRampInputAdapter should have already printed an error
				continue;
			}

			int elementId;
			std::string_view elementIdString( stringViewFromMatch( destName, rampElementMatch, 2 ) );
			try
			{
				elementId = StringAlgo::toInt( elementIdString );
			}
			catch( ... )
			{
				throw IECore::Exception( StringAlgo::concat( "Invalid ramp point index ", elementIdString ) );
			}

			if( elementId < 0 || elementId >= (int)adapterParms.origSize )
			{
				throw IECore::Exception( "Connection to ramp index " + std::to_string( elementId ) + " is out of range in ramp with " + std::to_string( adapterParms.origSize ) + " points." );
			}

			// Map connections to the corresponding parameters of the expanded ramp. When mapping the
			// first or last point, the value may need to be connected multiple times to match the end
			// point duplication. This is needed in order to actually reach the end point value when using
			// BSpline or CatmullRom interpolation. It doesn't actually matter for Linear or Constant, which
			// have duplicated end points that aren't used, just because OSL thought it would be good idea to
			// specify unused duplicated end points for "consistency", but for simplicity, we always connect
			// the first or last control point to the duplicated end points.

			int outIndexMin, outIndexMax;

			if( elementId == 0 )
			{
				outIndexMin = 0;
				outIndexMax = adapterParms.knotOffset;
			}
			else if( elementId == (int)( adapterParms.origSize - 1 ) )
			{
				outIndexMin = elementId + adapterParms.knotOffset;
				outIndexMax = adapterParms.expandedSize - 1;
			}
			else
			{
				outIndexMin = outIndexMax = elementId + adapterParms.knotOffset;
			}

			for( int i = outIndexMin; i <= outIndexMax; i++ )
			{
				InternedString destinationName = g_arrayInputNames[i];
				if( rampElementMatch.length( 3 ) )
				{
					destinationName = StringAlgo::concat( destinationName.string(), stringViewFromMatch( destName, rampElementMatch, 3 ) );
				}

				network->addConnection( { connection.source, { adapterParms.adapterHandle, destinationName } } );
			}
		}

		network->setShader( s.first, std::move( new Shader( s.second->getName(), s.second->getType(), newParametersData.get() ) ) );

	}
}

void ShaderNetworkAlgo::convertDeprecatedSplines( ShaderNetwork *network )
{
	for( const auto &s : network->shaders() )
	{
		const CompoundDataMap &origParameters = s.second->parameters();

		CompoundDataPtr newParametersData;
		CompoundDataMap *newParameters = nullptr;

		for( const auto &[name, value] : origParameters )
		{
			if( const SplinefColor3fData *colorSpline = runTimeCast<const SplinefColor3fData>( value.get() ) )
			{
				ensureParametersCopy( origParameters, newParametersData, newParameters );
				RampfColor3fDataPtr rampData = new RampfColor3fData;
				rampData->writable().fromDeprecatedSpline( colorSpline->readable() );
				(*newParameters)[ name ] = rampData;
			}
			else if( const SplineffData *floatSpline = runTimeCast<const SplineffData>( value.get() ) )
			{
				ensureParametersCopy( origParameters, newParametersData, newParameters );
				RampffDataPtr rampData = new RampffData;
				rampData->writable().fromDeprecatedSpline( floatSpline->readable() );
				(*newParameters)[ name ] = rampData;
			}
		}

		if( newParameters )
		{
			network->setShader( s.first, std::move(
				new Shader( s.second->getName(), s.second->getType(), newParametersData.get() )
			) );
		}

	}
}
