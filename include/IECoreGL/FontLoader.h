//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, John Haddon. All rights reserved.
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

#ifndef IECOREGL_FONTLOADER_H
#define IECOREGL_FONTLOADER_H

#include "IECoreGL/Export.h"
#include "IECore/RefCounted.h"
#include "IECore/SearchPath.h"

#include <map>
#include <string>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Font );
IE_CORE_FORWARDDECLARE( FontLoader );

/// Loads Fonts from disk, using searchpaths to find them and
/// keeping a cache to reduce load times.
class IECOREGL_API FontLoader : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( FontLoader );

		FontLoader( const IECore::SearchPath &searchPaths );

		FontPtr load( const std::string &name );

		/// Removes any cached fonts.
		void clear();

		/// Returns a static FontLoader instance that everyone
		/// can use. This has searchpaths set using the
		/// IECORE_FONT_PATHS environment variable.
		static FontLoader *defaultFontLoader();

	private :

		typedef std::map<std::string, FontPtr> FontMap;
		FontMap m_fonts;

		IECore::SearchPath m_searchPaths;

};

} // namespace IECoreGL

#endif // IECOREGL_FONTLOADER_H
