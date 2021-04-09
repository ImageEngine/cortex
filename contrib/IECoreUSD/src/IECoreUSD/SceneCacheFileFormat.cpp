//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2021, Image Engine Design Inc. All rights reserved.
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

#include "SceneCacheFileFormat.h"

#include "SceneCacheData.h"
#include "USDScene.h"

#include "IECoreScene/LinkedScene.h"

#include "IECore/MessageHandler.h"

#include "pxr/base/tf/fileUtils.h"
#include "pxr/base/tf/pathUtils.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/base/tf/staticData.h"

#include "pxr/pxr.h"

#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/usd/clipsAPI.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/usdaFileFormat.h"
#include "pxr/usd/usdGeom/tokens.h"

#include "boost/assign.hpp"

#include <iostream>
#include <ostream>

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;
using namespace pxr;
using std::string;

// plugins to USD are required to use the internal pxr namespace
// specifically here for the macro TF_REGISTRY_FUNCTION
// to be accessible.
PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS( UsdSceneCacheFileFormatTokens, USD_SCENE_CACHE_FILE_FORMAT_TOKENS );

TF_REGISTRY_FUNCTION(TfType)
{
	SDF_DEFINE_FILE_FORMAT(UsdSceneCacheFileFormat, SdfFileFormat);
}

UsdSceneCacheFileFormat::UsdSceneCacheFileFormat()
	: SdfFileFormat( UsdSceneCacheFileFormatTokens->Id, UsdSceneCacheFileFormatTokens->Version, UsdSceneCacheFileFormatTokens->Target, UsdSceneCacheFileFormatTokens->Id),
	m_usda(SdfFileFormat::FindById(UsdUsdaFileFormatTokens->Id))
{
}

UsdSceneCacheFileFormat::~UsdSceneCacheFileFormat()
{
}


SdfAbstractDataRefPtr UsdSceneCacheFileFormat::InitData( const FileFormatArguments& args ) const
{
    return SceneCacheData::New( args );
}


bool UsdSceneCacheFileFormat::CanRead(const string& filePath) const
{
	auto extension = TfGetExtension(filePath);
	if (extension.empty()) {
		return false;
	}

	// use actual extensions
	return extension == this->GetFormatId();
}

bool UsdSceneCacheFileFormat::Read( SdfLayer* layer, const string& resolvedPath, bool metadataOnly) const
{
	SdfAbstractDataRefPtr data = InitData( layer->GetFileFormatArguments() );
	SceneCacheDataRefPtr sccData = TfStatic_cast<SceneCacheDataRefPtr>( data );
	if ( !sccData->Open( resolvedPath ) )
	{
		return false;
	}
	_SetLayerData( layer, data );
	return true;
}

bool UsdSceneCacheFileFormat::WriteToFile( const SdfLayer& layer, const std::string& filePath, const std::string& comment, const FileFormatArguments& args) const
{
	UsdStageRefPtr stage = UsdStage::Open( layer.GetIdentifier() );
	ConstUSDScenePtr usdScene = new USDScene( stage, IndexedIO::Read );

	auto fps = stage->GetTimeCodesPerSecond();

	SceneInterfacePtr outScene = SceneInterface::create( filePath, IndexedIO::Write );

	SceneInterface::NameList childNames;
	usdScene->childNames( childNames );
	for ( auto& childName : childNames )
	{
		writeLocation( layer, usdScene, childName, outScene, fps, stage );
	}
	return true;
}

void UsdSceneCacheFileFormat::writeLocation( const SdfLayer& layer, ConstSceneInterfacePtr inScene, const InternedString & childName, SceneInterfacePtr outScene, double fps, UsdStageRefPtr stage ) const
{
	ConstSceneInterfacePtr inChild = inScene->child( childName, SceneInterface::MissingBehaviour::ThrowIfMissing );
	SceneInterfacePtr outChild = outScene->createChild( childName );

	auto frames = layer.ListAllTimeSamples();
	// static write a single sample.
	if( frames.empty() )
	{
		frames.insert( 0. );
	}

	//transform
	for ( auto& frame : frames )
	{
		outChild->writeTransform( inChild->readTransform( frame / fps ).get(), frame / fps );
	}
	// location path
	SceneInterface::Path currentPath;
	inChild->path( currentPath );
	SdfPath primPath = USDScene::toUSD(currentPath);

	if( !primPath.IsAbsoluteRootPath() )
	{
		if( const auto linkedOutScene = runTimeCast<LinkedScene>( outChild.get() ) )
		{
			const auto& root = layer.GetPseudoRoot();
			const auto& primSpec = root->GetPrimAtPath( primPath );

			const auto& referenceListOp = primSpec->GetReferenceList();
			std::vector<SdfReference> references;
			referenceListOp.ApplyEditsToList( &references );

			if ( references.size() > 1 )
			{
				IECore::msg(
					IECore::Msg::Warning,
					"SceneCacheFileFormat::writeLocation",
					boost::format( "Unsupported multiple reference at location \"%s\", writing only the first reference." ) % primPath
					);
			}

			if ( ! references.empty() )
			{
				// cortex only support a single reference per location so we only use the first reference.
				const auto& ref = references[0];

				// file path
				const auto& filePath = ref.GetAssetPath();

				// root path
				const auto& rootPath = ref.GetPrimPath();

				// read scene to link
				ConstSceneInterfacePtr sceneToLink;
				try
				{
					sceneToLink = SharedSceneInterfaces::get( filePath );
				}
				catch( IOException )
				{
					IECore::msg(
						IECore::Msg::Warning,
						"SceneCacheFileFormat::writeLocation",
						boost::format( "Unsupported file extension \"%s\" for reference at location \"%s\"." ) % filePath % primPath
					);
					return;
				}
				const auto& locationToLink = sceneToLink->scene( USDScene::fromUSD( rootPath ) );
				const auto& clips = UsdClipsAPI::Get( stage, primPath );

				if ( clips )
				{
					VtVec2dArray times;
					if( clips.GetClipTimes( &times ) )
					{
						for( const auto& time : times )
						{
							const auto& linkData = IECoreScene::LinkedScene::linkAttributeData( locationToLink.get(), time[1] / fps );
							linkedOutScene->writeAttribute( IECoreScene::LinkedScene::linkAttribute, linkData.get(), time[0] / fps );
						}
						return;
					}
					else
					{
						// write link
						linkedOutScene->writeLink( locationToLink.get() );
						return;
					}
				}
				else
				{
					// write link
					linkedOutScene->writeLink( locationToLink.get() );
					return;
				}
			}
		}
	}

	// tags
	SceneInterface::NameList tags;
	inChild->readTags( tags );
	outChild->writeTags( tags );

	// object
	if ( inChild->hasObject() )
	{
		for ( auto& frame : frames )
		{
			outChild->writeObject( inChild->readObject( frame / fps ).get(), frame / fps );
		}
	}

	// attributes
	SceneInterface::NameList attributeNames;
	inChild->attributeNames( attributeNames );
	for ( auto& attributeName : attributeNames )
	{
		for ( auto& frame : frames )
		{
			if ( inChild->readAttribute( attributeName, frame/ fps ) )
			{
				outChild->writeAttribute( attributeName, inChild->readAttribute( attributeName, frame / fps ).get(), frame / fps );
			}
		}
	}
	// recursion
	SceneInterface::NameList grandChildNames;
	inChild->childNames( grandChildNames );
	for ( auto& grandChildName : grandChildNames )
	{
		writeLocation( layer, inChild, grandChildName, outChild, fps, stage );
	}
}

bool UsdSceneCacheFileFormat::ReadFromString( SdfLayer* layer, const std::string& str) const
{
    return m_usda->ReadFromString(layer, str);
}

bool UsdSceneCacheFileFormat::WriteToString( const SdfLayer& layer, std::string* str, const std::string& comment) const
{
    return m_usda->WriteToString(layer, str, comment);
}

bool UsdSceneCacheFileFormat::WriteToStream( const SdfSpecHandle &spec, std::ostream& out, size_t indent) const
{
    return m_usda->WriteToStream(spec, out, indent);
}

PXR_NAMESPACE_CLOSE_SCOPE
