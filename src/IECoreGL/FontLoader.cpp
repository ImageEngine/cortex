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

#include "IECoreGL/FontLoader.h"
#include "IECoreGL/Font.h"

#include "IECore/MessageHandler.h"
#include "IECore/Font.h"

using namespace IECoreGL;

FontLoader::FontLoader( const IECore::SearchPath &searchPaths )
	:	m_searchPaths( searchPaths )
{
}

FontPtr FontLoader::load( const std::string &name )
{
	FontMap::const_iterator it = m_fonts.find( name );
	if( it!=m_fonts.end() )
	{
		return it->second;
	}

	boost::filesystem::path path = m_searchPaths.find( name );
	if( path.empty() )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::FontLoader::load", boost::format( "Couldn't find \"%s\"." ) % name );
		m_fonts[name] = 0; // to save us trying over and over again
		return 0;
	}

	IECore::FontPtr coreFont = 0;
	try
	{
		coreFont = new IECore::Font( path.string() );
	}
	catch( const std::exception &e )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::Font::load", boost::format( "Failed to load \"%s\" ( %s )." ) % path.string() % e.what() );
		m_fonts[name] = 0; // to save us trying over and over again
		return 0;
	}

	FontPtr result = new Font( coreFont );
	m_fonts[name] = result;

	return result;
}

void FontLoader::clear()
{
	m_fonts.clear();
}

FontLoader *FontLoader::defaultFontLoader()
{
	static FontLoaderPtr l = 0;
	if( !l )
	{
		const char *e = getenv( "IECORE_FONT_PATHS" );
		l = new FontLoader( IECore::SearchPath( e ? e : "", ":" ) );
	}
	return l.get();
}
