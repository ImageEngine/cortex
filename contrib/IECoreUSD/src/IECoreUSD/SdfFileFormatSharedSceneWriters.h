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

#ifndef IECOREUSD_SDFFILEFORMATSHAREDSCENEWRITERS_H
#define IECOREUSD_SDFFILEFORMATSHAREDSCENEWRITERS_H

#include "IECoreScene/SceneInterface.h"

using namespace IECoreScene;

namespace IECoreUSD
{

// Class used to support per frame write when writing USD data from Houdini's Solaris context.
// We open the scene cache file for writing on the first frame of writing and close it for the last frame of writing.
class SdfFileFormatSharedSceneWriters
{
	public :

		static SceneInterfacePtr get( const std::string &fileName );

		/// Close a single file from the cache
		static void close( const std::string &fileName );

		/// Close all the scene from the cache
		static void closeAll();

		/// Sets the limit for the number of scene interfaces that will
		/// be cached internally.
		static void setMaxScenes( size_t numScenes );
		/// Returns the limit for the number of scene interfaces that will
		/// be cached internally.
		static size_t getMaxScenes();
		/// Returns the number of scene interfaces currently in the cache.
		static size_t numScenes();

};

} // namespace IECoreUSD

#endif // IECOREUSD_SDFFILEFORMATSHAREDSCENEWRITERS_H

