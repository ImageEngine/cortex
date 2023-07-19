//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2021, Image Engine Design. All rights reserved.
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

#include "IECoreUSD/ShaderAlgo.h"

#include "IECoreUSD/DataAlgo.h"

#include "IECoreScene/ShaderNetworkAlgo.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#if PXR_VERSION >= 2111
#include "pxr/usd/usdLux/cylinderLight.h"
#include "pxr/usd/usdLux/nonboundableLightBase.h"
#include "pxr/usd/usdLux/sphereLight.h"

#include "pxr/usd/usd/schemaRegistry.h"
#endif

#include "boost/algorithm/string/replace.hpp"
#include "boost/pointer_cast.hpp"

#include <regex>

#if PXR_VERSION < 2102
#define IsContainer IsNodeGraph
#endif

namespace
{

pxr::TfToken g_adapterLabelToken( IECoreScene::ShaderNetworkAlgo::componentConnectionAdapterLabel().string() );

std::pair<pxr::TfToken, std::string> shaderIdAndType( const pxr::UsdShadeConnectableAPI &connectable )
{
	pxr::TfToken id;
	std::string type;
	if( auto shader = pxr::UsdShadeShader( connectable ) )
	{
		shader.GetShaderId( &id );
		type = "surface";
	}
#if PXR_VERSION >= 2111
	else if( auto light = pxr::UsdLuxLightAPI( connectable ) )
	{
		light.GetShaderIdAttr().Get( &id );
		type = "light";
	}
#endif

	return std::make_pair( id, type );
}

void readAdditionalLightParameters( const pxr::UsdPrim &prim, IECore::CompoundDataMap &parameters )
{
	// Just to keep us on our toes, not all light parameters are stored as UsdShade inputs,
	// so we have special-case code for loading those here.
#if PXR_VERSION >= 2111
	if( auto sphereLight = pxr::UsdLuxSphereLight( prim ) )
	{
		bool treatAsPoint = false;
		sphereLight.GetTreatAsPointAttr().Get( &treatAsPoint );
		parameters["treatAsPoint"] = new IECore::BoolData( treatAsPoint );
	}
	else if( auto cylinderLight = pxr::UsdLuxCylinderLight( prim ) )
	{
		bool treatAsLine = false;
		cylinderLight.GetTreatAsLineAttr().Get( &treatAsLine );
		parameters["treatAsLine"] = new IECore::BoolData( treatAsLine );
	}
#endif
}

const std::regex g_arrayIndexFromUSDRegex( ":i([0-9]+)$" );
const std::string g_arrayIndexFromUSDFormat( "[$1]" );
IECore::InternedString fromUSDParameterName( const pxr::TfToken &usdName )
{
	// USD doesn't support connections to array indices. So Arnold-USD emulates
	// them using its own `parameter:i<N>`syntax - see https://github.com/Autodesk/arnold-usd/pull/381.
	// We convert these to the regular `parameter[N]` syntax during loading.
	return std::regex_replace( usdName.GetString(), g_arrayIndexFromUSDRegex, g_arrayIndexFromUSDFormat );
}

const std::regex g_arrayIndexFromCortexRegex( "\\[([0-9]+)\\]$" );
const std::string g_arrayIndexFromCortexFormat( ":i$1" );
pxr::TfToken toUSDParameterName( IECore::InternedString cortexName )
{
	return pxr::TfToken(
		std::regex_replace( cortexName.string(), g_arrayIndexFromCortexRegex, g_arrayIndexFromCortexFormat )
	);
}

IECoreScene::ShaderNetwork::Parameter readShaderNetworkWalk( const pxr::SdfPath &anchorPath, const pxr::UsdShadeOutput &output, IECoreScene::ShaderNetwork &shaderNetwork );

IECore::InternedString readShaderNetworkWalk( const pxr::SdfPath &anchorPath, const pxr::UsdShadeConnectableAPI &usdShader, IECoreScene::ShaderNetwork &shaderNetwork )
{
	IECore::InternedString handle( usdShader.GetPath().MakeRelativePath( anchorPath ).GetString() );

	if( shaderNetwork.getShader( handle ) )
	{
		return handle;
	}

	auto [id, shaderType] = shaderIdAndType( usdShader );
	std::string shaderName = "defaultsurface";
	if( id.size() )
	{
		std::string name = id.GetString();
		size_t colonPos = name.find( ":" );
		if( colonPos != std::string::npos )
		{
			std::string prefix = name.substr( 0, colonPos );
			name = name.substr( colonPos + 1 );
			if( prefix == "arnold" )
			{
				prefix = "ai";
			}
			shaderType = prefix + ":shader";
		}
		shaderName = name;
	}

	IECore::CompoundDataPtr parametersData = new IECore::CompoundData();
	IECore::CompoundDataMap &parameters = parametersData->writable();
	std::vector<IECoreScene::ShaderNetwork::Connection> connections;
	for( pxr::UsdShadeInput &i : usdShader.GetInputs() )
	{
		pxr::UsdShadeConnectableAPI usdSource;
		pxr::TfToken usdSourceName;
		pxr::UsdShadeAttributeType usdSourceType;

		pxr::UsdAttribute valueAttribute = i;
		if( i.GetConnectedSource( &usdSource, &usdSourceName, &usdSourceType ) )
		{
			if( usdSourceType == pxr::UsdShadeAttributeType::Output )
			{
				const IECoreScene::ShaderNetwork::Parameter sourceHandle = readShaderNetworkWalk(
					anchorPath, usdSource.GetOutput( usdSourceName ), shaderNetwork
				);
				connections.push_back( {
					sourceHandle, { handle, fromUSDParameterName( i.GetBaseName() ) }
				} );
			}
			else
			{
				// Connected to an exposed input on the material container. We don't
				// have an equivalent in IECoreScene::ShaderNetwork yet, so just take
				// the parameter value from the exposed input.
				valueAttribute = usdSource.GetInput( usdSourceName );
			}
		}

		if( IECore::DataPtr d = IECoreUSD::DataAlgo::fromUSD( pxr::UsdAttribute( valueAttribute ) ) )
		{
			parameters[fromUSDParameterName( i.GetBaseName() )] = d;
		}
	}

	readAdditionalLightParameters( usdShader.GetPrim(), parameters );

	IECoreScene::ShaderPtr newShader = new IECoreScene::Shader( shaderName, shaderType, parametersData );
	pxr::VtValue metadataValue;
	if( usdShader.GetPrim().GetMetadata( g_adapterLabelToken, &metadataValue ) && metadataValue.Get<bool>() )
	{
		newShader->blindData()->writable()[ IECoreScene::ShaderNetworkAlgo::componentConnectionAdapterLabel() ] = new IECore::BoolData( true );
	}
	shaderNetwork.addShader( handle, std::move( newShader ) );

	// Can only add connections after we've added the shader.
	for( const auto &c : connections )
	{
		shaderNetwork.addConnection( c );
	}

	return handle;
}

IECoreScene::ShaderNetwork::Parameter readShaderNetworkWalk( const pxr::SdfPath &anchorPath, const pxr::UsdShadeOutput &output, IECoreScene::ShaderNetwork &shaderNetwork )
{
	IECore::InternedString shaderHandle = readShaderNetworkWalk( anchorPath, pxr::UsdShadeConnectableAPI( output.GetPrim() ), shaderNetwork );
	if( output.GetBaseName() != "DEFAULT_OUTPUT" )
	{
		return IECoreScene::ShaderNetwork::Parameter( shaderHandle, output.GetBaseName().GetString() );
	}
	else
	{
		return IECoreScene::ShaderNetwork::Parameter( shaderHandle );
	}
}

IECoreScene::ConstShaderNetworkPtr adaptShaderNetworkForWriting( const IECoreScene::ShaderNetwork *shaderNetwork )
{
	IECoreScene::ShaderNetworkPtr result = shaderNetwork->copy();
	IECoreScene::ShaderNetworkAlgo::expandSplines( result.get() );
	IECoreScene::ShaderNetworkAlgo::addComponentConnectionAdapters( result.get() );
	return result;
}

pxr::UsdShadeConnectableAPI createShaderPrim( const IECoreScene::Shader *shader, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path )
{
	pxr::UsdShadeShader usdShader = pxr::UsdShadeShader::Define( stage, path );
	if( !usdShader )
	{
		throw IECore::Exception( "Could not create shader at " + path.GetAsString() );
	}
	const std::string type = shader->getType();
	std::string typePrefix;
	size_t typeColonPos = type.find( ":" );
	if( typeColonPos != std::string::npos )
	{
		typePrefix = type.substr( 0, typeColonPos ) + ":";
		if( typePrefix == "ai:" )
		{
			typePrefix = "arnold:";
		}
	}
	usdShader.SetShaderId( pxr::TfToken( typePrefix + shader->getName() ) );

	return usdShader.ConnectableAPI();
}

void writeShaderParameterValues( const IECoreScene::Shader *shader, pxr::UsdShadeConnectableAPI usdShader )
{
	for( const auto &p : shader->parametersData()->readable() )
	{
		pxr::UsdShadeInput input = usdShader.CreateInput(
			toUSDParameterName( p.first ),
			IECoreUSD::DataAlgo::valueTypeName( p.second.get() )
		);
		input.Set( IECoreUSD::DataAlgo::toUSD( p.second.get() ) );
	}

	const IECore::BoolData *adapterMeta = shader->blindData()->member<IECore::BoolData>( IECoreScene::ShaderNetworkAlgo::componentConnectionAdapterLabel() );
	if( adapterMeta && adapterMeta->readable() )
	{
		usdShader.GetPrim().SetMetadata( g_adapterLabelToken, true );
	}
}

using ShaderMap = std::unordered_map<IECore::InternedString, pxr::UsdShadeConnectableAPI>;
void writeShaderConnections( const IECoreScene::ShaderNetwork *shaderNetwork, const ShaderMap &usdShaders )
{
	for( const auto &shader : shaderNetwork->shaders() )
	{
		pxr::UsdShadeConnectableAPI usdShader = usdShaders.at( shader.first );
		for( const auto &c : shaderNetwork->inputConnections( shader.first ) )
		{
			pxr::UsdShadeInput dest = usdShader.GetInput( pxr::TfToken( c.destination.name.string() ) );
			if( !dest )
			{
				dest = usdShader.CreateInput( toUSDParameterName( c.destination.name ), pxr::SdfValueTypeNames->Token );
			}

			pxr::UsdShadeShader sourceUsdShader = usdShaders.at( c.source.shader );
			std::string sourceOutputName = c.source.name.string();
			if( sourceOutputName.size() == 0 )
			{
				sourceOutputName = "DEFAULT_OUTPUT";
			}
			pxr::UsdShadeOutput source = sourceUsdShader.CreateOutput( pxr::TfToken( sourceOutputName ), dest.GetTypeName() );
			dest.ConnectToSource( source );
		}
	}
}

} // namespace

pxr::UsdShadeOutput IECoreUSD::ShaderAlgo::writeShaderNetwork( const IECoreScene::ShaderNetwork *shaderNetwork, pxr::UsdPrim shaderContainer )
{
	IECoreScene::ConstShaderNetworkPtr adaptedNetwork = adaptShaderNetworkForWriting( shaderNetwork );
	shaderNetwork = adaptedNetwork.get();

	IECoreScene::ShaderNetwork::Parameter networkOutput = shaderNetwork->getOutput();
	if( networkOutput.shader.string() == "" )
	{
		// This could theoretically happen, but a shader network with no output is not useful in any way
		IECore::msg(
			IECore::Msg::Warning, "IECoreUSD::ShaderAlgo::writeShaderNetwork",
			"No output shader in network"
		);
	}

	ShaderMap usdShaders;
	pxr::UsdShadeOutput networkOutUsd;
	for( const auto &shader : shaderNetwork->shaders() )
	{
		const pxr::SdfPath usdShaderPath = shaderContainer.GetPath().AppendChild( pxr::TfToken( pxr::TfMakeValidIdentifier( shader.first.string() ) ) );
		pxr::UsdShadeConnectableAPI usdShader = createShaderPrim( shader.second.get(), shaderContainer.GetStage(), usdShaderPath );
		writeShaderParameterValues( shader.second.get(), usdShader );
		usdShaders[shader.first] = usdShader;

		if( networkOutput.shader == shader.first )
		{
			pxr::TfToken outName( networkOutput.name.string() );
			if( outName.GetString().size() == 0 )
			{
				outName = pxr::TfToken( "DEFAULT_OUTPUT" );
			}

			// \todo - we should probably be correctly tracking the output type if it is typed?
			// Currently, we don't really track output types in Gaffer.
			networkOutUsd = usdShader.CreateOutput( outName, pxr::SdfValueTypeNames->Token );
		}
	}

	writeShaderConnections( shaderNetwork, usdShaders );

	return networkOutUsd;
}

IECoreScene::ShaderNetworkPtr IECoreUSD::ShaderAlgo::readShaderNetwork( const pxr::UsdShadeOutput &output  )
{
	pxr::UsdShadeConnectableAPI usdSource;
	pxr::TfToken usdSourceName;
	pxr::UsdShadeAttributeType usdSourceType;
	if(
		!output.GetConnectedSource( &usdSource, &usdSourceName, &usdSourceType ) ||
		usdSourceType != pxr::UsdShadeAttributeType::Output
	)
	{
		return new IECoreScene::ShaderNetwork();
	}

	IECoreScene::ShaderNetworkPtr result = new IECoreScene::ShaderNetwork();
	IECoreScene::ShaderNetwork::Parameter outputHandle = readShaderNetworkWalk( usdSource.GetPrim().GetParent().GetPath(), usdSource.GetOutput( usdSourceName ), *result );

	// For the output shader, set the type to "ai:surface" if it is "ai:shader".
	// This is complete nonsense - there is nothing to suggest that this shader is
	// of type surface - it could be a simple texture or noise, or even a
	// displacement or volume shader.
	//
	// But arbitrarily setting the type on the output to "ai:surface" matches our
	// current Gaffer convention, so it allows round-tripping.
	// In the long run, the fact this is working at all appears to indicate that we
	// don't use the suffix of the shader type for anything, and we should just set
	// everything to prefix:shader ( aside from lights, which are a bit of a
	// different question )
	const IECoreScene::Shader *outputShader = result->getShader( outputHandle.shader );
	if( outputShader->getType() == "ai:shader" )
	{
		IECoreScene::ShaderPtr o = outputShader->copy();
		o->setType( "ai:surface" );
		result->setShader( outputHandle.shader, std::move( o ) );
	}

	result->setOutput( outputHandle );

	IECoreScene::ShaderNetworkAlgo::removeComponentConnectionAdapters( result.get() );
	IECoreScene::ShaderNetworkAlgo::collapseSplines( result.get() );

	return result;
}

#if PXR_VERSION >= 2111

// This is very similar to `writeShaderNetwork` but with these key differences :
//
// - The output shader is written as a UsdLight-derived prim rather than a UsdShadeShader.
// - The other shaders are parented inside the light.
// - We don't need to create a UsdShadeOutput to return.
void IECoreUSD::ShaderAlgo::writeLight( const IECoreScene::ShaderNetwork *shaderNetwork, pxr::UsdPrim prim )
{
	IECoreScene::ConstShaderNetworkPtr adaptedNetwork = adaptShaderNetworkForWriting( shaderNetwork );
	shaderNetwork = adaptedNetwork.get();

	// Verify that the light shader corresponds to a valid USD light type.

	const IECoreScene::Shader *outputShader = shaderNetwork->outputShader();
	if( !outputShader )
	{
		IECore::msg( IECore::Msg::Warning, "ShaderAlgo::writeLight", "No output shader" );
		return;
	}

	pxr::TfType type = pxr::UsdSchemaRegistry::GetInstance().GetTypeFromName( pxr::TfToken( outputShader->getName() ) );
	if(
		!type.IsA<pxr::UsdLuxBoundableLightBase>() &&
		!type.IsA<pxr::UsdLuxNonboundableLightBase>()
	)
	{
		IECore::msg( IECore::Msg::Warning, "ShaderAlgo::writeLight", boost::format( "Shader `%1%` is not a valid UsdLux light type" ) % outputShader->getName() );
		return;
	}

	// Write the light itself onto the prim we've been given.

	ShaderMap usdShaders;
	prim.SetTypeName( pxr::TfToken( outputShader->getName() ) );
	writeShaderParameterValues( outputShader, pxr::UsdShadeConnectableAPI( prim ) );
	usdShaders[shaderNetwork->getOutput().shader] = pxr::UsdShadeConnectableAPI( prim );

	// Then write any other shaders as child prims so they are
	// encapsulated within the light.

	for( const auto &shader : shaderNetwork->shaders() )
	{
		if( shader.second == outputShader )
		{
			continue;
		}
		const pxr::SdfPath usdShaderPath = prim.GetPath().AppendChild( pxr::TfToken( pxr::TfMakeValidIdentifier( shader.first.string() ) ) );
		pxr::UsdShadeConnectableAPI usdShader = createShaderPrim( shader.second.get(), prim.GetStage(), usdShaderPath );
		writeShaderParameterValues( shader.second.get(), usdShader );
		usdShaders[shader.first] = usdShader;
	}

	// Finally, connect everything up.

	writeShaderConnections( shaderNetwork, usdShaders );
}

IECoreScene::ShaderNetworkPtr IECoreUSD::ShaderAlgo::readLight( const pxr::UsdLuxLightAPI &light )
{
	IECoreScene::ShaderNetworkPtr result = new IECoreScene::ShaderNetwork();
	IECoreScene::ShaderNetwork::Parameter lightHandle = readShaderNetworkWalk( light.GetPath().GetParentPath(), pxr::UsdShadeConnectableAPI( light ), *result );
	result->setOutput( lightHandle );
	IECoreScene::ShaderNetworkAlgo::removeComponentConnectionAdapters( result.get() );
	return result;
}

#endif
