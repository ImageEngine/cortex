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
#include "pxr/usd/usdLux/sphereLight.h"
#endif

#include "boost/algorithm/string/replace.hpp"
#include "boost/pointer_cast.hpp"

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
					sourceHandle, { handle, IECore::InternedString( i.GetBaseName().GetString() ) }
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
			parameters[ i.GetBaseName().GetString() ] = d;
		}
	}

	readAdditionalLightParameters( usdShader.GetPrim(), parameters );

	parametersData = boost::const_pointer_cast< IECore::CompoundData >( IECoreScene::ShaderNetworkAlgo::collapseSplineParameters( parametersData ) );

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

} // namespace

pxr::UsdShadeOutput IECoreUSD::ShaderAlgo::writeShaderNetwork( const IECoreScene::ShaderNetwork *shaderNetwork, pxr::UsdPrim shaderContainer )
{
	IECoreScene::ShaderNetworkPtr shaderNetworkWithAdapters = shaderNetwork->copy();
	IECoreScene::ShaderNetworkAlgo::addComponentConnectionAdapters( shaderNetworkWithAdapters.get() );

	IECoreScene::ShaderNetwork::Parameter networkOutput = shaderNetworkWithAdapters->getOutput();
	if( networkOutput.shader.string() == "" )
	{
		// This could theoretically happen, but a shader network with no output is not useful in any way
		IECore::msg(
			IECore::Msg::Warning, "IECoreUSD::ShaderAlgo::writeShaderNetwork",
			"No output shader in network"
		);
	}

	pxr::UsdShadeOutput networkOutUsd;
	for( const auto &shader : shaderNetworkWithAdapters->shaders() )
	{
		pxr::SdfPath usdShaderPath = shaderContainer.GetPath().AppendChild( pxr::TfToken( pxr::TfMakeValidIdentifier( shader.first.string() ) ) );
		pxr::UsdShadeShader usdShader = pxr::UsdShadeShader::Define( shaderContainer.GetStage(), usdShaderPath );
		if( !usdShader )
		{
			throw IECore::Exception( "Could not create shader at: " + shaderContainer.GetPath().GetString() + " / " + shader.first.string() );
		}
		std::string type = shader.second->getType();
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
		usdShader.SetShaderId( pxr::TfToken( typePrefix + shader.second->getName() ) );


		IECore::ConstCompoundDataPtr expandedParameters = IECoreScene::ShaderNetworkAlgo::expandSplineParameters(
			shader.second->parametersData()
		);
		for( const auto &p : expandedParameters->readable() )
		{
			pxr::UsdShadeInput input = usdShader.CreateInput(
				pxr::TfToken( p.first.string() ),
				DataAlgo::valueTypeName( p.second.get() )
			);
			input.Set( DataAlgo::toUSD( p.second.get() ) );
		}

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

		const IECore::BoolData* adapterMeta = shader.second->blindData()->member<IECore::BoolData>( IECoreScene::ShaderNetworkAlgo::componentConnectionAdapterLabel() );
		if( adapterMeta && adapterMeta->readable() )
		{
			usdShader.GetPrim().SetMetadata( g_adapterLabelToken, true );
		}
	}

	for( const auto &shader : shaderNetworkWithAdapters->shaders() )
	{
		pxr::UsdShadeShader usdShader = pxr::UsdShadeShader::Get( shaderContainer.GetStage(), shaderContainer.GetPath().AppendChild( pxr::TfToken( pxr::TfMakeValidIdentifier( shader.first.string() ) ) ) );
		for( const auto &c : shaderNetworkWithAdapters->inputConnections( shader.first ) )
		{
			pxr::UsdShadeInput dest = usdShader.GetInput( pxr::TfToken( c.destination.name.string() ) );
			if( ! dest.GetPrim().IsValid() )
			{
				dest = usdShader.CreateInput( pxr::TfToken( c.destination.name.string() ), pxr::SdfValueTypeNames->Token );
			}

			pxr::UsdShadeShader sourceUsdShader = pxr::UsdShadeShader::Get( shaderContainer.GetStage(), shaderContainer.GetPath().AppendChild( pxr::TfToken( pxr::TfMakeValidIdentifier( c.source.shader.string() ) ) ) );
			std::string sourceOutputName = c.source.name.string();
			if( sourceOutputName.size() == 0 )
			{
				sourceOutputName = "DEFAULT_OUTPUT";
			}
			pxr::UsdShadeOutput source = sourceUsdShader.CreateOutput( pxr::TfToken( sourceOutputName ), dest.GetTypeName() );
			dest.ConnectToSource( source );
		}

	}

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

	return result;
}

#if PXR_VERSION >= 2111

IECoreScene::ShaderNetworkPtr IECoreUSD::ShaderAlgo::readShaderNetwork( const pxr::UsdLuxLightAPI &light )
{
	IECoreScene::ShaderNetworkPtr result = new IECoreScene::ShaderNetwork();
	IECoreScene::ShaderNetwork::Parameter lightHandle = readShaderNetworkWalk( light.GetPath().GetParentPath(), pxr::UsdShadeConnectableAPI( light ), *result );
	result->setOutput( lightHandle );
	IECoreScene::ShaderNetworkAlgo::removeComponentConnectionAdapters( result.get() );
	return result;
}

#endif
