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

#include "IECoreGL/TextureLoader.h"
#include "IECoreGL/ColorTexture.h"

#include "IECore/MessageHandler.h"
#include "IECore/Reader.h"

using namespace IECoreGL;

TextureLoader::TextureLoader( const IECore::SearchPath &searchPaths )
	:	m_searchPaths( searchPaths )
{
}

TexturePtr TextureLoader::load( const std::string &name )
{
	TexturesMap::const_iterator it = m_loadedTextures.find( name );
	if( it!=m_loadedTextures.end() )
	{
		return it->second;
	}

	boost::filesystem::path path = m_searchPaths.find( name );
	if( path.empty() )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Couldn't find \"%s\"." ) % name );
		m_loadedTextures[name] = 0; // to save us trying over and over again
		return 0;
	}

	IECore::ReaderPtr r = IECore::Reader::create( path.string() );
	if( !r )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Couldn't create a Reader for \"%s\"." ) % path.string() );
		m_loadedTextures[name] = 0; // to save us trying over and over again
		return 0;
	}

	IECore::ObjectPtr o = r->read();
	IECore::ImagePrimitivePtr i = IECore::runTimeCast<IECore::ImagePrimitive>( o );
	if( !i )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "\"%s\" is not an image." ) % path.string() );
		m_loadedTextures[name] = 0; // to save us trying over and over again
		return 0;
	}

	TexturePtr t = 0;
	try
	{
		t = new ColorTexture( i );
	}
	catch( const std::exception &e )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Texture conversion failed for \"%s\" ( %s )." ) % path.string() % e.what() );
	}
	m_loadedTextures[name] = t;
	return t;
}

void TextureLoader::clear()
{
	m_loadedTextures.clear();
}

TextureLoaderPtr TextureLoader::defaultTextureLoader()
{
	static TextureLoaderPtr t = 0;
	if( !t )
	{
		const char *e = getenv( "IECOREGL_TEXTURE_PATHS" );
		t = new TextureLoader( IECore::SearchPath( e ? e : "", ":" ) );
	}
	return t;
}
