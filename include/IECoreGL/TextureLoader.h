//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_TEXTURELOADER_H
#define IECOREGL_TEXTURELOADER_H

#include "IECoreGL/Export.h"
#include "IECoreGL/Texture.h"

#include "IECore/RefCounted.h"
#include "IECore/SearchPath.h"

#include <map>
#include <string>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Texture );
IE_CORE_FORWARDDECLARE( TextureLoader );

/// \todo At some point we'll need to deal with the fact that
/// there's limited texture memory and we can't just keep loading
/// things forever without getting rid of something.
class IECOREGL_API TextureLoader : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( TextureLoader );

		TextureLoader( const IECore::SearchPath &searchPaths );

		TexturePtr load( const std::string &name, int maximumResolution = std::numeric_limits<int>::max() );

		/// Removes any cached textures.
		void clear();

		/// Returns a static TextureLoader instance that everyone
		/// can use. This has searchpaths set using the
		/// IECOREGL_TEXTURE_PATHS environment variable.
		static TextureLoader *defaultTextureLoader();

	private :

		void freeUnusedTextures();

		using TexturesMapKey = std::pair<std::string, int>;
		using TexturesMap = std::map<TexturesMapKey, TexturePtr>;
		TexturesMap m_loadedTextures;

		IECore::SearchPath m_searchPaths;

};

} // namespace IECoreGL

#endif // IECOREGL_TEXTURELOADER_H
