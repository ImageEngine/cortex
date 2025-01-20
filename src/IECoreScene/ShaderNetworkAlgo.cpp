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
#include "IECore/SplineData.h"
#include "IECore/TypeTraits.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/container/flat_map.hpp"
#include "boost/regex.hpp"

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
	else if( auto *b = shader->blindData()->member<BoolData>( ShaderNetworkAlgo::componentConnectionAdapterLabel() ) )
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
	else if( auto *b = shader->blindData()->member<BoolData>( ShaderNetworkAlgo::componentConnectionAdapterLabel() ) )
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

const InternedString &ShaderNetworkAlgo::componentConnectionAdapterLabel()
{
	static InternedString ret( "cortex_autoAdapter" );
	return ret;
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

} // namespace

void ShaderNetworkAlgo::convertOSLComponentConnections( ShaderNetwork *network )
{
	convertOSLComponentConnections( network, 10900 /* OSL 1.9 */ );
}

void ShaderNetworkAlgo::convertOSLComponentConnections( ShaderNetwork *network, int oslVersion )
{
	if( oslVersion < 11000 )
	{
		// OSL doesn't support component-level connections,
		// so we emulate them by inserting conversion shaders for OSL nodes.
		addComponentConnectionAdapters( network, "osl:" );
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

void ShaderNetworkAlgo::convertToOSLConventions( ShaderNetwork *network, int oslVersion )
{
	expandSplines( network, "osl:" );

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
// Spline handling
//////////////////////////////////////////////////////////////////////////

namespace
{

std::string_view stringViewFromMatch( const std::string &s, const boost::smatch &match, int index )
{
	return std::string_view( s.data() + match.position( index ), match.length( index ) );
}

template <typename T>
std::pair< size_t, size_t > getEndPointDuplication( const T &basis )
{
	if( basis == T::linear() )
	{
		// OSL discards the first and last segment of linear curves
		// "To maintain consistency with the other spline types"
		// so we need to duplicate the end points to preserve all provided segments
		return std::make_pair( 1, 1 );
	}
	else if( basis == T::constant() )
	{
		// Also, "To maintain consistency", "constant splines ignore the first and the two last
		// data values."
		return std::make_pair( 1, 2 );
	}

	return std::make_pair( 0, 0 );
}

template<typename Spline>
void expandSpline( const InternedString &name, const Spline &spline, CompoundDataMap &newParameters, const std::string shaderType = "", const std::string shaderName = "" )
{
	const char *basis = "catmull-rom";
	// For Renderman see https://rmanwiki-26.pixar.com/space/REN26/19661691/PxrRamp
	const char *riBasis = "catmull-rom";
	// For Arnold see https://help.autodesk.com/view/ARNOL/ENU/?guid=arnold_user_guide_ac_texture_shaders_ac_texture_ramp_html
	int aiBasisIdx = 2;
	const bool isArnold = boost::starts_with( shaderType, "ai:" );

	if( spline.basis == Spline::Basis::bezier() )
	{
		basis = "bezier";
	}
	else if( spline.basis == Spline::Basis::bSpline() )
	{
		basis = "bspline";
		riBasis = "bspline";
	}
	else if( spline.basis == Spline::Basis::linear() )
	{
		basis = "linear";
		riBasis = "linear";
		aiBasisIdx = 1;
	}
	else if( spline.basis == Spline::Basis::constant() )
	{
		// Also, "To maintain consistency", "constant splines ignore the first and the two last
		// data values."
		basis = "constant";
		riBasis = "constant";
		aiBasisIdx = 0;
	}
	auto [ duplicateStartPoints, duplicateEndPoints ] = getEndPointDuplication( spline.basis );

	typedef TypedData< vector<typename Spline::XType> > XTypedVectorData;
	typename XTypedVectorData::Ptr positionsData = new XTypedVectorData();
	auto &positions = positionsData->writable();
	positions.reserve( spline.points.size() );
	typedef TypedData< vector<typename Spline::YType> > YTypedVectorData;
	typename YTypedVectorData::Ptr valuesData = new YTypedVectorData();
	auto &values = valuesData->writable();
	values.reserve( spline.points.size() + duplicateStartPoints + duplicateEndPoints );

	if( spline.points.size() && !isArnold )
	{
		for( size_t i = 0; i < duplicateStartPoints; i++ )
		{
			positions.push_back( spline.points.begin()->first );
			values.push_back( spline.points.begin()->second );
		}
	}
	for( typename Spline::PointContainer::const_iterator it = spline.points.begin(), eIt = spline.points.end(); it != eIt; ++it )
	{
		positions.push_back( it->first );
		values.push_back( it->second );
	}
	if( spline.points.size() && !isArnold )
	{
		for( size_t i = 0; i < duplicateEndPoints; i++ )
		{
			positions.push_back( spline.points.rbegin()->first );
			values.push_back( spline.points.rbegin()->second );
		}
	}

	if( isArnold && ( shaderName == "ramp_float" || shaderName == "ramp_rgb" ) )
	{
		newParameters[ "position" ] = positionsData;
		if constexpr ( std::is_same_v<Spline, SplinefColor3f> )
		{
			newParameters[ "color" ] = valuesData;
		}
		else
		{
			newParameters[ "value" ] = valuesData;
		}
		std::vector<int> interp;
		interp.resize( spline.points.size() );
		std::fill( interp.begin(), interp.end(), aiBasisIdx );
		newParameters[ "interpolation" ] = new IntVectorData( interp );
	}
	// Intentionally OR'd here as many Renderman shaders are OSL so search for the 'Pxr' prefix.
	else if( boost::starts_with( shaderType, "ri:" ) || ( boost::starts_with( shaderName, "Pxr" ) ) )
	{
		newParameters[ name.string() + "_Knots" ] = positionsData;
		if constexpr ( std::is_same_v<Spline, SplinefColor3f> )
		{
			newParameters[ name.string() + "_Colors" ] = valuesData;
		}
		else
		{
			newParameters[ name.string() + "_Floats" ] = valuesData;
		}
		newParameters[ name.string() + "_Interpolation" ] = new StringData( riBasis );
	}
	else
	{
		newParameters[ name.string() + "Positions" ] = positionsData;
		newParameters[ name.string() + "Values" ] = valuesData;
		newParameters[ name.string() + "Basis" ] = new StringData( basis );
	}
}

template<typename SplineData>
IECore::DataPtr loadSpline(
	const StringData *basisData,
	const IECore::TypedData< std::vector< typename SplineData::ValueType::XType > > *positionsData,
	const IECore::TypedData< std::vector< typename SplineData::ValueType::YType > > *valuesData,
	const bool unduplicatePoints = true
)
{
	typename SplineData::Ptr resultData = new SplineData();
	auto &result = resultData->writable();

	size_t unduplicateStartPoints = 0;
	size_t unduplicateEndPoints = 0;

	const std::string &basis = basisData->readable();
	if( basis == "bezier" )
	{
		result.basis = SplineData::ValueType::Basis::bezier();
	}
	if( basis == "bspline" )
	{
		result.basis = SplineData::ValueType::Basis::bSpline();
	}
	else if( basis == "linear" )
	{
		// Reverse the duplication we do when expanding splines
		if( unduplicatePoints )
		{
			unduplicateStartPoints = 1;
			unduplicateEndPoints = 1;
		}
		result.basis = SplineData::ValueType::Basis::linear();
	}
	else if( basis == "constant" )
	{
		// Reverse the duplication we do when expanding splines
		if( unduplicatePoints )
		{
			unduplicateStartPoints = 1;
			unduplicateEndPoints = 2;
		}
		result.basis = SplineData::ValueType::Basis::constant();
	}
	else
	{
		result.basis = SplineData::ValueType::Basis::catmullRom();
	}

	const auto &positions = positionsData->readable();
	const auto &values = valuesData->readable();

	size_t n = std::min( positions.size(), values.size() );
	for( size_t i = 0; i < n; ++i )
	{
		if( i < unduplicateStartPoints || i >= n - unduplicateEndPoints )
		{
			continue;
		}

		result.points.insert( typename SplineData::ValueType::Point( positions[i], values[i] ) );
	}

	return resultData;
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

const boost::regex g_splineElementRegex( "^(.*)\\[(.*)\\]\\.y(.*)$" );
const boost::regex g_splineAdapterInRegex( "^in([0-9]+)(\\..*)?$" );

template< typename TypedSpline >
std::pair< InternedString, int > createSplineInputAdapter(
	ShaderNetwork *network, const TypedData<TypedSpline> *splineData,
	const IECore::CompoundDataMap &newParameters, const IECore::InternedString &splineParameterName,
	const ShaderNetwork::Parameter &destination
)
{
	using ValueVectorData = TypedData< std::vector< typename TypedSpline::YType > >;

	IECore::InternedString splineValuesName = splineParameterName.string() + "Values";
	auto findValues = newParameters.find( splineValuesName );
	const ValueVectorData *splineValuesData = findValues != newParameters.end() ? runTimeCast<const ValueVectorData>( findValues->second.get() ) : nullptr;
	if( !splineValuesData )
	{
		throw IECore::Exception( "Internal failure in convertToOSLConventions - expandSpline did not create values." );
	}

	const std::vector< typename TypedSpline::YType > &splineValues = splineValuesData->readable();

	if( splineValues.size() > maxArrayInputAdapterSize )
	{
		throw IECore::Exception(
			"Cannot handle input to " +
			destination.shader.string() + "." + destination.name.string() +
			" : expanded spline has " + std::to_string( splineValues.size() ) +
			" control points, but max input adapter size is " + std::to_string( maxArrayInputAdapterSize )
		);
	}

	// Using this adapter depends on Gaffer being available, but I guess we don't really
	// care about use cases outside Gaffer ( and in terms of using exported USD elsewhere,
	// this spline representation is only used in Gaffer's spline shaders, so it's not very
	// useful if you don't have access to Gaffer shaders anyway ).
	ShaderPtr adapter = new Shader(
		std::is_same< TypedSpline, SplinefColor3f >::value ? g_colorToArrayAdapter : g_floatToArrayAdapter,
		g_oslShader
	);

	for( unsigned int i = 0; i < splineValues.size(); i++ )
	{
		adapter->parameters()[ g_arrayInputNames[i] ] = new TypedData< typename TypedSpline::YType >( splineValues[i] );
	}

	InternedString adapterHandle = network->addShader( destination.shader.string() + "_" + splineParameterName.string() + "InputArrayAdapter", std::move( adapter ) );
	network->addConnection( ShaderNetwork::Connection(
		{ adapterHandle, g_arrayOutputNames[ splineValues.size() ] },
		{ destination.shader, splineValuesName }
	) );

	return std::make_pair( adapterHandle, getEndPointDuplication( splineData->readable().basis ).first );
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

} // namespace

void ShaderNetworkAlgo::collapseSplines( ShaderNetwork *network, std::string targetPrefix )
{
	std::vector< IECore::InternedString > adapters;

	for( auto [name, shader] : network->shaders() )
	{
		if( !boost::starts_with( shader->getType(), targetPrefix ) )
		{
			continue;
		}

		bool isSplineAdapter = shader->getType() == g_oslShader && (
			shader->getName() == g_colorToArrayAdapter || shader->getName() == g_floatToArrayAdapter
		);

		if( isSplineAdapter )
		{
			adapters.push_back( name );
			continue;
		}

		// For nodes which aren't spline adapters, we just need to deal with any parameters that are splines
		ConstCompoundDataPtr collapsed = collapseSplineParameters( shader->parametersData(), shader->getType(), shader->getName());
		if( collapsed != shader->parametersData() )
		{
			// \todo - this const_cast is ugly, although safe because if the return from collapseSplineParameters
			// doesn't match the input, it is freshly allocated. Once collapseSplineParameters is fully
			// deprecated, and no longer visible publicly, an internal version of collapseSplineParameters could
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
					Msg::Error, "ShaderNetworkAlgo", "Invalid spline plug name \"" + splineValuesName + "\""
				);
				continue;
			}

			InternedString splineName = string_view( splineValuesName ).substr( 0, splineValuesName.size() - 6 );

			const IECoreScene::Shader *targetShader = network->getShader( output.destination.shader );
			if( !targetShader )
			{
				throw IECore::Exception(
					"Invalid connection to shader that doesn't exist \"" + output.destination.shader.string() + "\""
				);
			}
			const IECore::CompoundDataMap &targetParameters = targetShader->parameters();

			int targetSplineKnotOffset = -1;
			auto targetParameterIt = targetParameters.find( splineName );
			if( targetParameterIt != targetParameters.end() )
			{
				if( const SplineffData *findSplineff = runTimeCast<const SplineffData>( targetParameterIt->second.get() ) )
				{
					targetSplineKnotOffset = getEndPointDuplication( findSplineff->readable().basis ).first;
				}
				else if( const SplinefColor3fData *findSplinefColor3f = runTimeCast<const SplinefColor3fData>( targetParameterIt->second.get() ) )
				{
					targetSplineKnotOffset = getEndPointDuplication( findSplinefColor3f->readable().basis ).first;
				}
				else if( const SplinefColor4fData *findSplinefColor4f = runTimeCast<const SplinefColor4fData>( targetParameterIt->second.get() ) )
				{
					targetSplineKnotOffset = getEndPointDuplication( findSplinefColor4f->readable().basis ).first;
				}
			}

			if( targetSplineKnotOffset == -1 )
			{
				IECore::msg(
					Msg::Error, "ShaderNetworkAlgo",
					"Invalid connection to spline parameter that doesn't exist \"" +
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
					- targetSplineKnotOffset;

				InternedString origDestName;
				if( match[2].matched )
				{
					origDestName = StringAlgo::concat(
						splineName.string(), "[", std::to_string( elementId ), "].y",
						stringViewFromMatch( adapterDestName, match, 2 )
					);
				}
				else
				{
					origDestName = StringAlgo::concat(
						splineName.string(), "[", std::to_string( elementId ), "].y"
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

void ShaderNetworkAlgo::expandSplines( ShaderNetwork *network, std::string targetPrefix )
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
			if( const SplinefColor3fData *colorSpline = runTimeCast<const SplinefColor3fData>( value.get() ) )
			{
				ensureParametersCopy( origParameters, newParametersData, newParameters );
				newParameters->erase( name );
				expandSpline( name, colorSpline->readable(), *newParameters, s.second->getType(), s.second->getName() );
			}
			else if( const SplineffData *floatSpline = runTimeCast<const SplineffData>( value.get() ) )
			{
				ensureParametersCopy( origParameters, newParametersData, newParameters );
				newParameters->erase( name );
				expandSpline( name, floatSpline->readable(), *newParameters, s.second->getType(), s.second->getName() );
			}
		}

		if( !newParameters )
		{
			// No splines to convert
			continue;
		}

		// currentSplineArrayAdapters holds array adapters that we need to use to hook up inputs to
		// spline plugs. It is indexed by the name of a spline parameter for the shader, and holds
		// the name of the adapter shader, and the offset we need to use when accessing the knot
		// vector.

		std::map< IECore::InternedString, std::pair< IECore::InternedString, size_t > > currentSplineArrayAdapters;

		std::vector< ShaderNetwork::Connection > connectionsToAdd;
		ShaderNetwork::ConnectionRange inputConnections = network->inputConnections( s.first );
		for( ShaderNetwork::ConnectionIterator it = inputConnections.begin(); it != inputConnections.end(); )
		{
			// Copy and increment now so we still have a valid iterator
			// if we remove the connection.
			const ShaderNetwork::Connection connection = *it++;

			const std::string &destName = connection.destination.name.string();
			boost::smatch splineElementMatch;
			if( !boost::regex_match( destName, splineElementMatch, g_splineElementRegex) )
			{
				continue;
			}

			IECore::InternedString parameterName( stringViewFromMatch( destName, splineElementMatch, 1 ) );
			auto findParameter = origParameters.find( parameterName );
			if( findParameter == origParameters.end() )
			{
				continue;
			}

			const SplinefColor3fData* colorSplineData = runTimeCast<const SplinefColor3fData>( findParameter->second.get() );
			const SplineffData* floatSplineData = runTimeCast<const SplineffData>( findParameter->second.get() );

			if( !( colorSplineData || floatSplineData ) )
			{
				continue;
			}

			int numPoints = colorSplineData ? colorSplineData->readable().points.size() : floatSplineData->readable().points.size();

			// Insert a conversion shader to handle connection to component
			auto [ adapterIter, newlyInserted ] = currentSplineArrayAdapters.insert( { parameterName, std::make_pair( IECore::InternedString(), 0 ) } );
			if( newlyInserted )
			{
				if( colorSplineData )
				{
					adapterIter->second = createSplineInputAdapter(
						network, colorSplineData, *newParameters, parameterName, connection.destination
					);
				}
				else
				{
					adapterIter->second = createSplineInputAdapter(
						network, floatSplineData, *newParameters, parameterName, connection.destination
					);
				}
			}

			const auto [ adapterHandle, knotOffset ] = adapterIter->second;

			int elementId;
			std::string_view elementIdString( stringViewFromMatch( destName, splineElementMatch, 2 ) );
			try
			{
				elementId = StringAlgo::toInt( elementIdString );
			}
			catch( ... )
			{
				throw IECore::Exception( StringAlgo::concat( "Invalid spline point index ", elementIdString ) );
			}

			if( elementId < 0 || elementId >= numPoints )
			{
				throw IECore::Exception( "Spline index " + std::to_string( elementId ) + " is out of range in spline with " + std::to_string( numPoints ) + " points." );
			}

			// We form only a single connection, even if we are at an endpoint which is duplicated during
			// expandSpline. This is OK because the end points that are duplicated by expandSpline are ignored
			// by OSL.
			//
			// An aside : the X values of the ignored points do need to be non-decreasing sometimes. There are
			// two contraditory claims in the OSL spec, that:
			// "Results are undefined if the knots ... not ... monotonic"
			// and
			// "constant splines ignore the first and the two last data values."
			// This statements combine to make it ambiguous whether the duplicated value is completely
			// ignored, or whether it must be monotonic ... in practice, it seems to cause problems for
			// constant, but not linear interpolation.
			//
			// In any case, we only make connections to the Y value, so there is no problem with ignoring
			// the duplicated values

			InternedString destinationName = g_arrayInputNames[elementId + knotOffset];
			if( splineElementMatch.length( 3 ) )
			{
				destinationName = StringAlgo::concat( destinationName.string(), stringViewFromMatch( destName, splineElementMatch, 3 ) );
			}

			network->removeConnection( connection );
			network->addConnection( { connection.source, { adapterHandle, destinationName } } );
		}

		network->setShader( s.first, std::move( new Shader( s.second->getName(), s.second->getType(), newParametersData.get() ) ) );

	}
}

IECore::ConstCompoundDataPtr ShaderNetworkAlgo::collapseSplineParameters( const IECore::ConstCompoundDataPtr &parametersData, const std::string shaderType, const std::string shaderName )
{
	const CompoundDataMap &parameters( parametersData->readable() );
	CompoundDataPtr newParametersData;
	CompoundDataMap *newParameters = nullptr;

	std::string basisStr = "Basis";
	std::string positionsStr = "Positions";
	std::string valuesStr = "Values";

	const bool isArnold = boost::starts_with( shaderType, "ai:" );
	const bool isRenderman = boost::starts_with( shaderType, "ri:" ) || boost::starts_with( shaderName, "Pxr" );
	const bool unduplicatePoints = !isArnold;

	if( isArnold && ( shaderName == "ramp_float" || shaderName == "ramp_rgb" ) )
	{
		basisStr = "interpolation";
		positionsStr = "position";
		if( shaderName == "ramp_rgb" )
		{
			valuesStr = "color";
		}
		else
		{
			valuesStr = "value";
		}
	}
	else if( isRenderman )
	{
		basisStr = "_Interpolation";
		positionsStr = "_Knots";
		valuesStr = "_Floats";
	}

	for( const auto &maybeBasis : parameters )
	{
		if( !boost::ends_with( maybeBasis.first.string(), basisStr ) )
		{
			continue;
		}
		StringDataPtr basisPtr;
		const StringData *basis = runTimeCast<StringData>( maybeBasis.second.get() );
		if( !basis )
		{
			const IntVectorData *intBasis = runTimeCast<const IntVectorData>( maybeBasis.second.get() );
			if( !intBasis )
			{
				continue;
			}
			// Do int to string conversion here, using the first value of the interpolation array
			if( intBasis->readable().front() == 0 )
			{
				basisPtr = new StringData( "constant" );
			}
			else if( intBasis->readable().front() == 1 )
			{
				basisPtr = new StringData( "linear" );
			}
			else if( intBasis->readable().front() == 3 )
			{
				basisPtr = new StringData( "monotonecubic" );
			}
			else
			{
				basisPtr = new StringData( "catmull-rom" );
			}
		}
		else
		{
			basisPtr = basis->copy();
		}

		std::string prefix = maybeBasis.first.string().substr( 0, maybeBasis.first.string().size() - basisStr.size() );
		IECore::InternedString positionsName = prefix + positionsStr;
		const auto positionsIter = parameters.find( positionsName );
		const FloatVectorData *floatPositions = nullptr;

		if( positionsIter != parameters.end() )
		{
			floatPositions = runTimeCast<const FloatVectorData>( positionsIter->second.get() );
		}

		if( !floatPositions )
		{
			continue;
		}

		IECore::InternedString valuesName = prefix + valuesStr;
		auto valuesIter = parameters.find( valuesName );
		if( valuesIter == parameters.end() && isRenderman )
		{
			valuesName = prefix + "_Colors";
			valuesIter = parameters.find( valuesName );
		}

		IECore::DataPtr foundSpline;
		if( valuesIter != parameters.end() )
		{
			if( const FloatVectorData *floatValues = runTimeCast<const FloatVectorData>( valuesIter->second.get() ) )
			{
				foundSpline = loadSpline<SplineffData>( basisPtr.get(), floatPositions, floatValues, unduplicatePoints );
			}
			else if( const Color3fVectorData *color3Values = runTimeCast<const Color3fVectorData>( valuesIter->second.get() ) )
			{
				foundSpline = loadSpline<SplinefColor3fData>( basisPtr.get(), floatPositions, color3Values, unduplicatePoints );
			}
			else if( const Color4fVectorData *color4Values = runTimeCast<const Color4fVectorData>( valuesIter->second.get() ) )
			{
				foundSpline = loadSpline<SplinefColor4fData>( basisPtr.get(), floatPositions, color4Values, unduplicatePoints );
			}
		}

		if( foundSpline )
		{
			ensureParametersCopy( parameters, newParametersData, newParameters );
			// Arnold ramp_rgb/ramp_float has no prefix so ensure we have a parameter name to set
			std::string newParamName( "ramp" );
			if( !prefix.empty() )
			{
				newParamName = prefix;
			}
			(*newParameters)[newParamName] = foundSpline;
			newParameters->erase( maybeBasis.first );
			newParameters->erase( positionsName );
			newParameters->erase( valuesName );
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

IECore::ConstCompoundDataPtr ShaderNetworkAlgo::expandSplineParameters( const IECore::ConstCompoundDataPtr &parametersData, const std::string shaderType, const std::string shaderName )
{
	const CompoundDataMap &parameters( parametersData->readable() );

	CompoundDataPtr newParametersData;
	CompoundDataMap *newParameters = nullptr;

	for( const auto &i : parameters )
	{
		if( const SplinefColor3fData *colorSpline = runTimeCast<const SplinefColor3fData>( i.second.get() ) )
		{
			ensureParametersCopy( parameters, newParametersData, newParameters );
			newParameters->erase( i.first );
			expandSpline( i.first, colorSpline->readable(), *newParameters, shaderType, shaderName );
		}
		else if( const SplineffData *floatSpline = runTimeCast<const SplineffData>( i.second.get() ) )
		{
			ensureParametersCopy( parameters, newParametersData, newParameters );
			newParameters->erase( i.first );
			expandSpline( i.first, floatSpline->readable(), *newParameters, shaderType, shaderName );
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
