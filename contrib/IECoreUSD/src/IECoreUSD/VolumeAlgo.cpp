//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2023, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreUSD/DataAlgo.h"
#include "IECoreUSD/ObjectAlgo.h"

#include "IECoreVDB/VDBObject.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdVol/volume.h"
#include "pxr/usd/usdVol/openVDBAsset.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace pxr;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

// Overview
// ========
//
// The closest analogue to `UsdVolVolume` in Cortex is `IECoreVDB::VDBObject`,
// but the two classes have significant differences.
//
// - UsdVol provides schemas for referencing volume files on disk, but currently
//   provides no access to the volume data itself. On the other hand, VDBObject
//   provides direct access to data via `openvdb::GridBase::Ptr`, making it
//   suitable for use in live volume generation and processing. We take
//   advantage of this in Gaffer by providing various nodes for manipulating
//   volumes.
// - UsdVolVolume allows the referencing of multiple fields (grids in VDB
//   parlance) from multiple different files, and allows the fields to be given
//   names that are distinct from the grid names themselves. While a VDBObject
//   _can_ be composed of multiple grids from arbitrary sources, this is done by
//   calling `insertGrid()`, and doesn't track the file of origin (if there even
//   was one). VDBObject is only guaranteed to reference file-backed data when
//   constructed from a single filename and when the loaded grids have not been
//   modified subsequently.
//
// While there is scope for extending VDBObject to provide a cleaner mapping, at
// present this would have limited benefit. All our supported VDB-consuming
// renderers have volume objects that reference only a single file, so we would
// just be pushing the mismatch further down the pipeline. So for now we provide
// the closest mapping we can, and issue warnings for any loss of data.

// Reading
// =======

namespace
{

IECore::ObjectPtr readVolume( pxr::UsdVolVolume &volume, pxr::UsdTimeCode time, const Canceller *canceller )
{
	std::string fileName;
	for( const auto &[fieldName, fieldPath] : volume.GetFieldPaths() )
	{
		UsdVolOpenVDBAsset fieldAsset( volume.GetPrim().GetPrimAtPath( fieldPath ) );
		if( !fieldAsset )
		{
			IECore::msg(
				IECore::Msg::Warning, "IECoreVDB::VolumeAlgo::readVolume",
				"Ignoring \"" + fieldPath.GetAsString() + "\" because it is not an OpenVDBAsset"
			);
			continue;
		}

		ConstDataPtr fieldFileNameData = DataAlgo::fromUSD( fieldAsset.GetFilePathAttr(), time );
		const std::string fieldFileName = static_cast<const StringData *>( fieldFileNameData.get() )->readable();

		if( fileName.empty() )
		{
			fileName = fieldFileName;
		}
		else if( fieldFileName != fileName )
		{
			IECore::msg(
				IECore::Msg::Warning, "IECoreVDB::VolumeAlgo::readVolume",
				"Ignoring file \"" + fieldFileName + "\" from field \"" + fieldPath.GetAsString() + "\""
			);
		}
	}

	if( fileName.empty() )
	{
		IECore::msg(
			IECore::Msg::Warning, "IECoreVDB::VolumeAlgo::readVolume",
			"No file found for \"" + volume.GetPrim().GetPath().GetAsString() + "\""
		);
		return nullptr;
	}

	return new IECoreVDB::VDBObject( fileName );
}

bool volumeMightBeTimeVarying( pxr::UsdVolVolume &volume )
{
	for( const auto &[fieldName, fieldPath] : volume.GetFieldPaths() )
	{
		UsdVolOpenVDBAsset fieldAsset( volume.GetPrim().GetPrimAtPath( fieldPath ) );
		if( !fieldAsset )
		{
			continue;
		}

		if( fieldAsset.GetFilePathAttr().ValueMightBeTimeVarying() )
		{
			return true;
		}
	}
	return false;
}

ObjectAlgo::ReaderDescription<pxr::UsdVolVolume> g_volumeReaderDescription( pxr::TfToken( "Volume" ), readVolume, volumeMightBeTimeVarying );

} // namespace

//////////////////////////////////////////////////////////////////////////
// Writing
//////////////////////////////////////////////////////////////////////////

namespace
{

pxr::TfToken gridClass( const openvdb::GridBase *grid )
{
	switch( grid->getGridClass() )
	{
		case openvdb::GRID_LEVEL_SET :
			return pxr::TfToken( "GRID_LEVEL_SET" );
		case openvdb::GRID_FOG_VOLUME :
			return pxr::TfToken( "GRID_FOG_VOLUME" );
		case openvdb::GRID_STAGGERED :
			return pxr::TfToken( "GRID_STAGGERED" );
		default :
			return pxr::TfToken( "GRID_UNKNOWN" );
	}
}

bool writeVolume( const IECoreVDB::VDBObject *object, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )
{
	if( !object->unmodifiedFromFile() )
	{
		IECore::msg(
			IECore::Msg::Warning, "IECoreVDB::VolumeAlgo::writeVolume",
			"Not writing \"" + path.GetAsString() + "\"because VDBObject is not backed by a file"
		);
		return false;
	}

	auto volume = UsdVolVolume::Define( stage, path );

	for( const auto &gridName : object->gridNames() )
	{
		const TfToken gridNameToken( gridName );
		auto fieldPath = path.AppendChild( gridNameToken );
		auto fieldAsset = UsdVolOpenVDBAsset::Define( stage, fieldPath );
		fieldAsset.CreateFilePathAttr().Set( SdfAssetPath( object->fileName() ), time );
		fieldAsset.CreateFieldNameAttr().Set( gridNameToken );
		fieldAsset.CreateFieldClassAttr().Set( gridClass( object->findGrid( gridName ).get() ) );
		volume.CreateFieldRelationship( gridNameToken, fieldPath );
	}

	return true;
}

ObjectAlgo::WriterDescription<IECoreVDB::VDBObject> g_volumeWriterDescription( writeVolume );

} // namespace
