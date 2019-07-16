//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/Texture.h"
#include "IECoreGL/ToGLTextureConverter.h"

#include "IECoreImage/ImageReader.h"

#include "IECore/NumericParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/Reader.h"

#include <limits>

using namespace IECoreGL;

TextureLoader::TextureLoader( const IECore::SearchPath &searchPaths )
	:	m_searchPaths( searchPaths )
{
}

TexturePtr TextureLoader::load( const std::string &name, int maximumResolution )
{
	TexturesMapKey key( name, maximumResolution );
	TexturesMap::const_iterator it = m_loadedTextures.find( key );
	if( it!=m_loadedTextures.end() )
	{
		return it->second;
	}

	boost::filesystem::path path = m_searchPaths.find( name );
	if( path.empty() )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Couldn't find \"%s\"." ) % name );
		m_loadedTextures[key] = nullptr; // to save us trying over and over again
		return nullptr;
	}

	if( !IECoreImage::ImageReader::canRead( path.string() ) )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Couldn't create an ImageReader for \"%s\"." ) % path.string() );
		m_loadedTextures[key] = nullptr; // to save us trying over and over again
		return nullptr;
	}
	IECoreImage::ImageReaderPtr imageReader = new IECoreImage::ImageReader( path.string() );

	// Set miplevel if texture resolution is limited
	if( maximumResolution < std::numeric_limits<int>::max() )
	{
		int miplevel = 0;

		Imath::V2i dataWindowSize = imageReader->dataWindow().size();
		int currentMax = std::max( dataWindowSize.x, dataWindowSize.y ) + 1;
		IECore::IntParameter *miplevelParameter = imageReader->mipLevelParameter();
		while( currentMax > maximumResolution )
		{
			miplevelParameter->setNumericValue( ++miplevel );

			dataWindowSize = imageReader->dataWindow().size();
			currentMax = std::max( dataWindowSize.x, dataWindowSize.y ) + 1;
		}
	}

	IECore::ObjectPtr o = imageReader->read();
	IECoreImage::ImagePrimitivePtr i = IECore::runTimeCast<IECoreImage::ImagePrimitive>( o.get() );
	if( !i )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "\"%s\" is not an image." ) % path.string() );
		m_loadedTextures[key] = nullptr; // to save us trying over and over again
		return nullptr;
	}

	TexturePtr t = nullptr;
	try
	{
		ToGLTextureConverterPtr converter = new ToGLTextureConverter( i );
		t = IECore::runTimeCast<Texture>( converter->convert() );
	}
	catch( const std::exception &e )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Texture conversion failed for \"%s\" ( %s )." ) % path.string() % e.what() );
	}
	m_loadedTextures[key] = t;
	return t;
}

void TextureLoader::clear()
{
	m_loadedTextures.clear();
}

TextureLoader *TextureLoader::defaultTextureLoader()
{
	static TextureLoaderPtr t = nullptr;
	if( !t )
	{
		const char *e = getenv( "IECOREGL_TEXTURE_PATHS" );
		t = new TextureLoader( IECore::SearchPath( e ? e : "" ) );
	}
	return t.get();
}
