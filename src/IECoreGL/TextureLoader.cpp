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
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/LuminanceTexture.h"

#include "IECoreImage/OpenImageIOAlgo.h"

#include "IECore/NumericParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/Reader.h"

#include "OpenImageIO/imagebuf.h"
#include "OpenImageIO/imagebufalgo.h"
#include "OpenImageIO/imageio.h"

#include <limits>

// Windows does a #define that mucks up the SearchPath - include SearchPath.h again so it can correct that
#include "IECore/SearchPath.h"

using namespace IECoreGL;

namespace{
	void destroyImageCache( OIIO::ImageCache *cache )
	{
		OIIO::ImageCache::destroy( cache, /* teardown */ true );
	}
}

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

	OIIO::ustring oiioPath( path.string() );
	
	// We currently use automip to downsample textures without mipmaps.  This feels pretty ineffective, it would
	// be better to just explicitly resize to maxResolution if the texture is larger than maximumResolution and
	// doesn't have mipmaps on disk ... but I'm keeping this consistent for now.
	// To set automip, we need to create an ImageCache - currently I just destroy it when this function exits:
	// we cache the GL texture ourselves, so dropping the OIIO cache should be OK.
	std::unique_ptr<OIIO::ImageCache, decltype(&destroyImageCache) > imageCache( OIIO::ImageCache::create( /* shared */ false ), &destroyImageCache );
	imageCache->attribute( "automip", 1 );

	OIIO::ImageSpec mipSpec;
	if( !imageCache->get_imagespec( oiioPath, mipSpec, 0, 0 ) )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Couldn't load \"%s\"." ) % path.string() );
		m_loadedTextures[key] = nullptr; // to save us trying over and over again
		return nullptr;
	}
	
	// Set miplevel if texture resolution is limited
	int miplevel = 0;
	if( maximumResolution < std::numeric_limits<int>::max() )
	{
		while(
			std::max( mipSpec.full_width, mipSpec.full_height ) > maximumResolution &&
			imageCache->get_imagespec( oiioPath, mipSpec, 0, miplevel + 1 ) 
		)
		{
			miplevel++;
		}
	}

	OIIO::ImageBuf imageBuf( oiioPath, 0, miplevel, imageCache.get() );
	if( imageBuf.spec().full_x != 0 || imageBuf.spec().full_y != 0 )
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Texture display window must start at origin for \"%s\"." ) % path.string() );
		m_loadedTextures[key] = nullptr; // to save us trying over and over again
		return nullptr;
	}

	// This logic feels pretty broken - why do we ask the current color config's
	// display transform to decide what colorspace a file is stored in?  Why special
	// case just png.  But I've currently copied this logic from ImageReader in the
	// name of backwards compatibility
	std::string linearColorSpace;
	std::string currentColorSpace;
	OIIO::string_view fileFormat = imageBuf.file_format_name();
	if( fileFormat == "png" )
	{
		// The most common use for loading PNGs via Cortex is for icons in Gaffer.
		// If we were to use the OCIO config to guess the colorspaces as below, we
		// would get it spectacularly wrong. For instance, with an ACES config the
		// resulting icons are so washed out as to be illegible. Instead, we hardcode
		// the rudimentary colour spaces much more likely to be associated with a PNG.
		// These are supported by OIIO regardless of what OCIO config is in use.
		/// \todo Should this apply to other formats too? Can we somehow fix
		/// `OpenImageIOAlgo::colorSpace` instead?
		linearColorSpace = "linear";
		currentColorSpace = "sRGB";
	}
	else
	{
		linearColorSpace = IECoreImage::OpenImageIOAlgo::colorSpace( "", imageBuf.spec() );
		currentColorSpace = IECoreImage::OpenImageIOAlgo::colorSpace( fileFormat, imageBuf.spec() );
	}

	if( !OIIO::ImageBufAlgo::colorconvert( imageBuf, imageBuf, currentColorSpace, linearColorSpace ) )
	{
		// This conversion is the first operation that will trigger a lazy read of the ImageBuf
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Error reading \"%s\" : %s." ) % path.string() % imageBuf.geterror() );
	}
	

	IECore::FloatVectorDataPtr y;
	IECore::FloatVectorDataPtr r;
	IECore::FloatVectorDataPtr g;
	IECore::FloatVectorDataPtr b;
	IECore::FloatVectorDataPtr a;

	// \todo uninterleaving here is pretty wasteful, since we have to interleave again to actually transfer
	// to OpenGL.  Eventually we should probably change the signature to ColorTexture so we can pass in
	// interleaved data, and then we could read the interleaved buffer straight from OIIO
	OIIO::ROI chanRoiFull = imageBuf.roi_full();
	const std::vector<std::string> &channelnames = imageBuf.spec().channelnames;
	for( unsigned int chan = 0; chan < channelnames.size(); chan++ )
	{
		IECore::FloatVectorDataPtr *dst;
		if( channelnames[chan] == "Y" ) dst = &y;
		else if( channelnames[chan] == "R" ) dst = &r;
		else if( channelnames[chan] == "G" ) dst = &g;
		else if( channelnames[chan] == "B" ) dst = &b;
		else if( channelnames[chan] == "A" ) dst = &a;
		else continue;

		*dst = new IECore::FloatVectorData();
		(*dst)->writable().resize( imageBuf.spec().full_width * imageBuf.spec().full_height );

		chanRoiFull.chbegin = chan;
		chanRoiFull.chend = chan + 1;
		if( !imageBuf.get_pixels( chanRoiFull, OIIO::TypeDesc::FLOAT, &(*dst)->writable()[0] ) )
		{
			IECore::msg(
				IECore::Msg::Error, "IECoreGL::TextureLoader::load",
				boost::format( "Failed to read channel \"%s\" for \"%s\".%s" ) %
				channelnames[chan] % path.string() % ( imageBuf.has_error() ? " " + imageBuf.geterror() : "" )
			);
		}
	}

	TexturePtr t = nullptr;
	if ( !y && r && g && b )
	{
		t = new ColorTexture( imageBuf.spec().full_width, imageBuf.spec().full_height, r.get(), g.get(), b.get(), a.get() );
	}
	else if ( y && !r && !g && !b )
	{
		t = new LuminanceTexture( imageBuf.spec().full_width, imageBuf.spec().full_height, y.get(), a.get() );
	}
	else
	{
		IECore::msg( IECore::Msg::Error, "IECoreGL::TextureLoader::load", boost::format( "Texture conversion failed for \"%s\" ( Invalid image format, ToGLTextureConverter supports RGB[A] and Y[A]. )." ) % path.string() );
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
