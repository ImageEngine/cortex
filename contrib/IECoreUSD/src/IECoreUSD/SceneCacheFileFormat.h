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

#ifndef IECOREUSD_SCENECACHEFILEFORMAT_H
#define IECOREUSD_SCENECACHEFILEFORMAT_H

#include "USDScene.h"

#include "IECoreScene/SceneInterface.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/staticTokens.h"

#include "pxr/pxr.h"

#include "pxr/usd/sdf/fileFormat.h"

#include <iosfwd>
#include <string>

PXR_NAMESPACE_OPEN_SCOPE

#define USD_SCENE_CACHE_FILE_FORMAT_TOKENS \
	((Id, "scc")) \
	((Version, "1.0")) \
	((Target, "usd")) \

TF_DECLARE_PUBLIC_TOKENS( UsdSceneCacheFileFormatTokens, USD_SCENE_CACHE_FILE_FORMAT_TOKENS );

TF_DECLARE_WEAK_AND_REF_PTRS( UsdSceneCacheFileFormat );

// We support FileFormatArguments to control the behaviour of the plugin for writing.
// We can filter time samples within a frame range and support per frame writing.
//
// Support per frame write requires all of the following FileFormatArguments:
//  - perFrameWrite: supported value is "1" when we want to use the per frame writing behaviour
//   and "0" when write the file by calling WriteToFile only once.
//  - currentFrame: string encoding the floating point value for the current frame number being written
//  - firstFrame: string encoding the floating point value for the first frame to be written.
//  - lastFrame: string encoding the floating point value for the last frame to be written.
// `firstFrame` and `lastFrame` are used to figure out then we should open the file for writing ( currentFrame == firstFrame ) when using the per frame writing
//   and when to close the file ( currentFrame == lastFrame ).
//
// `firstFrame` and `lastFrame` are also used to filter the time samples to be written ( only the time sample within the range are written ) both when using per frame writing and single write mode.
class UsdSceneCacheFileFormat : public SdfFileFormat
{
	public:
		SdfAbstractDataRefPtr InitData( const FileFormatArguments& ) const override;

		bool CanRead( const std::string& filePath ) const override;
		bool Read( SdfLayer* layer, const std::string& resolvedPath, bool metadataOnly ) const override;
		bool ReadFromString( SdfLayer* layer, const std::string& str ) const override;

		bool WriteToFile( const SdfLayer& layer, const std::string& filePath, const std::string& comment = std::string(), const FileFormatArguments& args = FileFormatArguments() ) const override;
		bool WriteToString( const SdfLayer& layer, std::string* str, const std::string& comment = std::string() ) const override;
		bool WriteToStream( const SdfSpecHandle& spec, std::ostream& out, size_t indent ) const override;

	protected:
		SDF_FILE_FORMAT_FACTORY_ACCESS;

		~UsdSceneCacheFileFormat() override;
		UsdSceneCacheFileFormat();

		void writeLocation(
			const SdfLayer& layer,
			IECoreScene::ConstSceneInterfacePtr inScene,
			const IECore::InternedString & childName,
			IECoreScene::SceneInterfacePtr outScene,
			double fps,
			UsdStageRefPtr stage,
			std::set<double> frames
		) const;

	private:
		SdfFileFormatConstPtr m_usda;
};

PXR_NAMESPACE_CLOSE_SCOPE
#endif // IECOREUSD_SCENECACHEFILEFORMAT_H

