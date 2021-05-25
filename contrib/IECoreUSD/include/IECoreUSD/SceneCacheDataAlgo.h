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

#ifndef IECOREUSD_SCENECACHEDATAALGO_H
#define IECOREUSD_SCENECACHEDATAALGO_H

#include "IECoreUSD/Export.h"

#include "IECoreScene/SceneInterface.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/tf/token.h"
IECORE_POP_DEFAULT_VISIBILITY

// SceneCacheDataAlgo is suite of utilities used by SceneCacheData and SceneCacheFileFormat
// classes responsible for loading Cortex Scene Cache Data into USD.
// This header can be ignored in most other usage of IECoreUSD.

namespace IECoreUSD
{

namespace SceneCacheDataAlgo
{

IECOREUSD_API pxr::TfToken internalRootNameToken();
IECOREUSD_API IECore::InternedString internalRootName();
IECOREUSD_API IECoreScene::SceneInterface::Path fromInternalPath( const IECoreScene::SceneInterface::Path& scenePath );
IECOREUSD_API IECoreScene::SceneInterface::Path toInternalPath( const IECoreScene::SceneInterface::Path& scenePath );
IECOREUSD_API std::string fromInternalName( const IECore::InternedString& name );
IECOREUSD_API std::string toInternalName( const IECore::InternedString& name );

} // namespace SceneCacheDataAlgo

} // namespace IECoreUSD


#endif // IECOREUSD_SCENECACHEDATAALGO_H
