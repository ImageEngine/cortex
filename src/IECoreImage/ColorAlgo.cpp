//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECoreImage/ColorAlgo.h"

#include "IECoreImage/ImagePrimitive.h"
#include "IECoreImage/OpenImageIOAlgo.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/VectorTypedData.h"

#include "OpenImageIO/imagebufalgo.h"
#include "OpenImageIO/imageio.h"

OIIO_NAMESPACE_USING

using namespace IECore;
using namespace IECoreImage;

namespace
{

struct ColorTransformer
{
	typedef void ReturnType;

	ColorTransformer( const std::string &inputSpace, const std::string &outputSpace, int width, int height )
		: m_inputSpace( inputSpace ), m_outputSpace( outputSpace ), m_width( width ), m_height( height )
	{
	}

	template<typename T>
	ReturnType operator()( T *data )
	{
		// \todo: remove this logic once `transformChannel` has been removed
		int width = ( m_width == 0 ) ? data->readable().size() : m_width;
		int height = ( m_height == 0 ) ? 1 : m_height;

		// present it as a single channel, single scanline image
		OpenImageIOAlgo::DataView dataView( data );
		ImageSpec spec( width, height, 1, dataView.type.elementtype() );
		ImageBuf buffer( spec, data->baseWritable() );

		ROI roi(
			/* xbegin */ spec.x, /* xend */ spec.width,
			/* ybegin */ spec.y, /* yend */ spec.height,
			/* zbegin */ 0, /* zend */ 1,
			/* chbegin */ 0, /* chend */ 1
		);

		// convert in-place
#if OIIO_VERSION > 10800
		bool status = ImageBufAlgo::colorconvert(
			/* dst */ buffer, /* src */ buffer,
			/* from */ m_inputSpace, /* to */ m_outputSpace,
			/* unpremult */ false,
			/* context_key */ "",
			/* context_value */ "",
			/* colorconfig */ OpenImageIOAlgo::colorConfig(),
			/* roi */ roi
		);
#elif OIIO_VERSION > 10701 
		bool status = ImageBufAlgo::colorconvert(
			/* dst */ buffer, /* src */ buffer,
			/* from */ m_inputSpace, /* to */ m_outputSpace,
			/* unpremult */ false,
			/* colorconfig */ OpenImageIOAlgo::colorConfig(),
			/* roi */ roi
		);
#else
		bool status = ImageBufAlgo::colorconvert(
			/* dst */ buffer, /* src */ buffer,
			/* from */ m_inputSpace, /* to */ m_outputSpace,
			/* unpremult */ false,
			/* roi */ roi
		);
#endif

		if( !status )
		{
			throw Exception( std::string( "ColorAlgo::transformImage : " + buffer.geterror() ) );
		}
	}

	const std::string &m_inputSpace;
	const std::string &m_outputSpace;
	const int m_width;
	const int m_height;
};

} // namespace

namespace IECoreImage
{

namespace ColorAlgo
{

void transformChannel( Data *channel, const std::string &inputSpace, const std::string &outputSpace )
{
	if( outputSpace == inputSpace )
	{
		return;
	}

	ColorTransformer transformer( inputSpace, outputSpace, 0, 0 );
	IECore::despatchTypedData<ColorTransformer, IECore::TypeTraits::IsNumericVectorTypedData>( channel, transformer );
}

void transformImage( ImagePrimitive *image, const std::string &inputSpace, const std::string &outputSpace )
{
	if( outputSpace == inputSpace )
	{
		return;
	}

	ColorTransformer transformer( inputSpace, outputSpace, image->getDataWindow().size().x + 1, image->getDataWindow().size().y + 1 );

	for( auto &channel : image->channels )
	{
		if( channel.first == "A" || channel.first == "Z" )
		{
			continue;
		}

		IECore::despatchTypedData<ColorTransformer, IECore::TypeTraits::IsNumericVectorTypedData>( channel.second.get(), transformer );
	}
}

} // namespace ColorAlgo

} // namespace IECoreImage
