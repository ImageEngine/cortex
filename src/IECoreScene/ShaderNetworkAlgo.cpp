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
#include "IECore/SplineData.h"
#include "IECore/VectorTypedData.h"

#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/regex.hpp"

#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

namespace {

BoolDataPtr g_trueData( new BoolData( true ) );

}

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
const InternedString g_packInParameterNames[4] = { "in1", "in2", "in3", "in4" };
const boost::regex g_componentRegex( "^(.*)\\.([rgbaxyz])$" );
const char *g_vectorComponents[3] = { "x", "y", "z" };
const char *g_colorComponents[4] = { "r", "g", "b", "a" };

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
				// Insert a conversion shader to handle connection to component
				auto inserted = outputConversions.insert( { connection.source, ShaderNetwork::Parameter() } );
				if( inserted.second )
				{
					ShaderPtr swizzle = new Shader( "MaterialX/mx_swizzle_color_float", "osl:shader" );

					swizzle->blindData()->writable()[ componentConnectionAdapterLabel() ] = g_trueData;

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

				// Insert a conversion shader to handle connection from component

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
				Color4f value( 0 );
				const Data *d = shader.second->parametersData()->member<Data>( parameterName );
				if( const V3fData *vd = runTimeCast<const V3fData>( d ) )
				{
					value = Color4f( vd->readable()[0], vd->readable()[1], vd->readable()[2], 0.0f );
				}
				else if( const Color3fData *cd = runTimeCast<const Color3fData>( d ) )
				{
					value = Color4f( cd->readable()[0], cd->readable()[1], cd->readable()[2], 0.0f );
				}
				else if( auto c4d = runTimeCast<const Color4fData>( d ) )
				{
					value = c4d->readable();
				}

				// Make shader and set fallback values

				ShaderPtr packShader = new Shader( "MaterialX/mx_pack_color", "osl:shader" );
				packShader->blindData()->writable()[ componentConnectionAdapterLabel() ] = g_trueData;
				for( int i = 0; i < 4; ++i )
				{
					packShader->parameters()[g_packInParameterNames[i]] = new FloatData( value[i] );
				}

				const InternedString packHandle = network->addShader( g_packHandle, std::move( packShader ) );

				// Make connections

				network->addConnection( { { packHandle, g_outParameterName }, { shader.first, parameterName } } );

				for( int i = 0; i < 4; ++i )
				{
					ShaderNetwork::Parameter source = network->input( { shader.first, parameterName.string() + "." + g_colorComponents[i] } );
					if( !source && i < 3 )
					{
						source = network->input( { shader.first, parameterName.string() + "." + g_vectorComponents[i] } );
					}
					if( source )
					{
						network->addConnection( { source, { packHandle, g_packInParameterNames[i] } } );
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

	using ParameterMap = std::unordered_map<ShaderNetwork::Parameter, ShaderNetwork::Parameter>;
	ParameterMap outputConversions;
	for( const auto &s : network->shaders() )
	{
		ConstBoolDataPtr labelValue = s.second->blindData()->member<BoolData>( componentConnectionAdapterLabel() );
		if( !labelValue || !labelValue->readable() )
		{
			continue;
		}

		bool isPack = s.second->getName() == "MaterialX/mx_pack_color";
		bool isSwizzle = s.second->getName() == "MaterialX/mx_swizzle_color_float";

		if( !( s.second->getType() == "osl:shader" && ( isSwizzle || isPack ) ) )
		{
			throw IECore::Exception( boost::str(
				boost::format( "removeComponentConnectionAdapters : adapter is not of supported type and name: '%s' %s : %s" ) %
				s.first % s.second->getType() % s.second->getName()
			) );
		}

		toRemove.push_back( s.first );

		ShaderNetwork::ConnectionRange outputConnections = network->outputConnections( s.first );

		for( ShaderNetwork::ConnectionIterator it = outputConnections.begin(); it != outputConnections.end(); )
		{
			// Copy and increment now so we still have a valid iterator
			// if we remove the connection.
			const ShaderNetwork::Connection connection = *it++;
			network->removeConnection( connection );

			if( isPack )
			{
				const Shader *targetShader = network->getShader( connection.destination.shader );

				ShaderNetwork::ConnectionRange inputConnections = network->inputConnections( s.first );
				for( ShaderNetwork::ConnectionIterator inputIt = inputConnections.begin(); inputIt != inputConnections.end(); inputIt++ )
				{
					const IECore::InternedString &inputName = inputIt->destination.name;
					int inputIndex = -1;
					for( int i = 0; i < 4; i++ )
					{
						if( inputName == g_packInParameterNames[i] )
						{
							inputIndex = i;
						}
					}

					if( inputIndex == -1 )
					{
						throw IECore::Exception( boost::str(
							boost::format(
								"removeComponentConnectionAdapters : Unrecognized input for mx_pack_color \"%1%\""
							) % inputName
						) );
					}

					ShaderNetwork::Parameter componentDest;
					if(
						targetShader->parametersData()->member<Color4fData>( connection.destination.name ) ||
						targetShader->parametersData()->member<Color3fData>( connection.destination.name )
					)
					{
						componentDest = { connection.destination.shader, IECore::InternedString( connection.destination.name.string() + "." + g_colorComponents[inputIndex] ) };
					}
					else if( targetShader->parametersData()->member<V3fData>( connection.destination.name ) )
					{
						componentDest = { connection.destination.shader, IECore::InternedString( connection.destination.name.string() + "." + g_vectorComponents[inputIndex] ) };
					}
					else
					{
						throw IECore::Exception( boost::str(
							boost::format(
								"removeComponentConnectionAdapters : Unrecognized type for target parameter \"%1%.%2%\""
							) % connection.destination.shader.string() % connection.destination.name.string()
						) );
					}

					network->addConnection( { inputIt->source, componentDest } );
				}
			}
			else
			{
				const StringData *channelsData = s.second->parametersData()->member<StringData>( "channels" );
				if( !channelsData )
				{
					throw IECore::Exception( boost::str(
						boost::format(
							"removeComponentConnectionAdapters : mx_swizzle_color_float \"%1%\"should have \"channels\" parameter"
						) % s.first.string()
					) );
				}

				ShaderNetwork::Parameter componentSource = network->input( ShaderNetwork::Parameter( s.first, "in" ) );
				if( !componentSource )
				{
					throw IECore::Exception( boost::str(
						boost::format(
							"removeComponentConnectionAdapters : mx_swizzle_color_float \"%1%\" must have an input"
						) % s.first.string()
					) );
				}
				componentSource.name = componentSource.name.string() + "." + channelsData->readable();

				network->addConnection( { componentSource, connection.destination } );
			}
		}
	}

	for( IECore::InternedString &handle : toRemove )
	{
		network->removeShader( handle );
	}
}

const InternedString &ShaderNetworkAlgo::componentConnectionAdapterLabel()
{
	static InternedString ret( "cortex_autoAdapter" );
	return ret;
}


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

	using ParameterMap = std::unordered_map<ShaderNetwork::Parameter, ShaderNetwork::Parameter>;
	ParameterMap outputConversions;
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

namespace
{

template<typename Spline>
void expandSpline( const InternedString &name, const Spline &spline, CompoundDataMap &newParameters )
{
	const char *basis = "catmull-rom";
	bool duplicateEndPoints = false;
	if( spline.basis == Spline::Basis::bezier() )
	{
		basis = "bezier";
	}
	else if( spline.basis == Spline::Basis::bSpline() )
	{
		basis = "bspline";
	}
	else if( spline.basis == Spline::Basis::linear() )
	{
		// OSL discards the first and last segment of linear curves
		// "To maintain consistency with the other spline types"
		// so we need to duplicate the end points to preserve all provided segments
		duplicateEndPoints = true;
		basis = "linear";
	}

	typedef TypedData< vector<typename Spline::XType> > XTypedVectorData;
	typename XTypedVectorData::Ptr positionsData = new XTypedVectorData();
	auto &positions = positionsData->writable();
	positions.reserve( spline.points.size() );
	typedef TypedData< vector<typename Spline::YType> > YTypedVectorData;
	typename YTypedVectorData::Ptr valuesData = new YTypedVectorData();
	auto &values = valuesData->writable();
	values.reserve( spline.points.size() + 2 * duplicateEndPoints );

	if( duplicateEndPoints && spline.points.size() )
	{
		positions.push_back( spline.points.begin()->first );
		values.push_back( spline.points.begin()->second );
	}
	for( typename Spline::PointContainer::const_iterator it = spline.points.begin(), eIt = spline.points.end(); it != eIt; ++it )
	{
		positions.push_back( it->first );
		values.push_back( it->second );
	}
	if( duplicateEndPoints && spline.points.size() )
	{
		positions.push_back( spline.points.rbegin()->first );
		values.push_back( spline.points.rbegin()->second );
	}

	newParameters[ name.string() + "Positions" ] = positionsData;
	newParameters[ name.string() + "Values" ] = valuesData;
	newParameters[ name.string() + "Basis" ] = new StringData( basis );
}

template<typename SplineData>
IECore::DataPtr loadSpline(
	const StringData *basisData,
	const IECore::TypedData< std::vector< typename SplineData::ValueType::XType > > *positionsData,
	const IECore::TypedData< std::vector< typename SplineData::ValueType::YType > > *valuesData
)
{
	typename SplineData::Ptr resultData = new SplineData();
	auto &result = resultData->writable();

	bool unduplicateEndPoints = false;

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
		unduplicateEndPoints = true;
		result.basis = SplineData::ValueType::Basis::linear();
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
		if( unduplicateEndPoints && ( i == 0 || i == n - 1 ) )
		{
			continue;
		}

		result.points.insert( typename SplineData::ValueType::Point( positions[i], values[i] ) );
	}

	return resultData;
}

} // namespace

IECore::ConstCompoundDataPtr ShaderNetworkAlgo::collapseSplineParameters( const IECore::ConstCompoundDataPtr &parameters )
{
	CompoundDataPtr newParametersData;

	const auto &parms = parameters->readable();
	for( const auto &maybeBasis : parameters->readable() )
	{
		if( !boost::ends_with( maybeBasis.first.string(), "Basis" ) )
		{
			continue;
		}
		const StringData *basis = runTimeCast<const StringData>( maybeBasis.second.get() );
		if( !basis )
		{
			continue;
		}


		std::string prefix = maybeBasis.first.string().substr( 0, maybeBasis.first.string().size() - 5 );
		IECore::InternedString positionsName = prefix + "Positions";
		const auto positionsIter = parms.find( positionsName );
		const FloatVectorData *floatPositions = nullptr;

		if( positionsIter != parms.end() )
		{
			floatPositions = runTimeCast<const FloatVectorData>( positionsIter->second.get() );
		}

		if( !floatPositions )
		{
			continue;
		}

		IECore::InternedString valuesName = prefix + "Values";
		const auto valuesIter = parms.find( valuesName );

		IECore::DataPtr foundSpline;
		if( valuesIter != parms.end() )
		{
			if( const FloatVectorData *floatValues = runTimeCast<const FloatVectorData>( valuesIter->second.get() ) )
			{
				foundSpline = loadSpline<SplineffData>( basis, floatPositions, floatValues );
			}
			else if( const Color3fVectorData *color3Values = runTimeCast<const Color3fVectorData>( valuesIter->second.get() ) )
			{
				foundSpline = loadSpline<SplinefColor3fData>( basis, floatPositions, color3Values );
			}
			else if( const Color4fVectorData *color4Values = runTimeCast<const Color4fVectorData>( valuesIter->second.get() ) )
			{
				foundSpline = loadSpline<SplinefColor4fData>( basis, floatPositions, color4Values );
			}
		}

		if( foundSpline )
		{
			if( !newParametersData )
			{
				newParametersData = new IECore::CompoundData();
				newParametersData->writable() = parameters->readable();
			}
			auto &newParameters = newParametersData->writable();

			newParameters[prefix] = foundSpline;
			newParameters.erase( maybeBasis.first );
			newParameters.erase( positionsName );
			newParameters.erase( valuesName );
		}
	}

	if( newParametersData )
	{
		return newParametersData;
	}
	else
	{
		return parameters;
	}
}

IECore::ConstCompoundDataPtr ShaderNetworkAlgo::expandSplineParameters( const IECore::ConstCompoundDataPtr &parameters )
{
	bool hasSplines = false;
	for( const auto &i : parameters->readable() )
	{
		if( runTimeCast<const SplinefColor3fData>( i.second.get() ) ||
			runTimeCast<const SplineffData>( i.second.get() ) )
		{
			hasSplines = true;
			break;
		}
	}

	if( !hasSplines )
	{
		return parameters;
	}

	CompoundDataPtr newParametersData = new CompoundData();
	CompoundDataMap &newParameters = newParametersData->writable();

	for( const auto &i : parameters->readable() )
	{
		if( const SplinefColor3fData *colorSpline = runTimeCast<const SplinefColor3fData>( i.second.get() ) )
		{
			expandSpline( i.first, colorSpline->readable(), newParameters );
		}
		else if( const SplineffData *floatSpline = runTimeCast<const SplineffData>( i.second.get() ) )
		{
			expandSpline( i.first, floatSpline->readable(), newParameters );
		}
		else
		{
			newParameters[ i.first ] = i.second;
		}
	}

	return newParametersData;
}
