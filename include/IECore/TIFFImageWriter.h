//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TIFFIMAGEWRITER_H
#define IE_CORE_TIFFIMAGEWRITER_H

#include <vector>

#include "IECore/Export.h"
#include "IECore/ImageWriter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/NumericParameter.h"
#include "IECore/CompoundParameter.h"

struct tiff;

namespace IECore
{

/// The TIFFImageWriter class serializes images to the Tagged Image File Format (TIFF) format
///
/// The tags which may be written into the files are:
///
/// TIFFTAG_PHOTOMETRIC<br>
/// TIFFTAG_SAMPLESPERPIXEL<br>
/// TIFFTAG_EXTRASAMPLES<br>
/// TIFFTAG_COMPRESSION<br>
/// TIFFTAG_SAMPLEFORMAT<br>
/// TIFFTAG_IMAGEWIDTH<br>
/// TIFFTAG_IMAGELENGTH<br>
/// TIFFTAG_XPOSITION<br>
/// TIFFTAG_YPOSITION<br>
/// TIFFTAG_PIXAR_IMAGEFULLWIDTH<br>
/// TIFFTAG_PIXAR_IMAGEFULLLENGTH<br>
/// TIFFTAG_BITSPERSAMPLE<br>
/// TIFFTAG_ROWSPERSTRIP<br>
/// TIFFTAG_FILLORDER<br>
/// TIFFTAG_PLANARCONFIG<br>
/// TIFFTAG_XRESOLUTION<br>
/// TIFFTAG_YRESOLUTION<br>
/// TIFFTAG_RESOLUTIONUNIT<br>
///
/// \ingroup ioGroup
class IECORE_API TIFFImageWriter : public ImageWriter
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( TIFFImageWriter, ImageWriter )

		TIFFImageWriter();

		/// construct an TIFFImageWriter for the given image and output filename
		TIFFImageWriter( ObjectPtr object, const std::string &fileName );
		virtual ~TIFFImageWriter();

		virtual std::string destinationColorSpace() const ;

	private:

		static const WriterDescription<TIFFImageWriter> m_writerDescription;

		virtual void writeImage( const std::vector<std::string> &names,
		                         const ImagePrimitive * image,
		                         const Imath::Box2i &dataWindow	) const;

		template<typename ChannelData>
		struct ChannelConverter;

		template<typename T>
		void encodeChannels( const ImagePrimitive * image, const std::vector<std::string> &names,
		                     const Imath::Box2i &dw, tiff *tiffImage, size_t bufSize, unsigned int numStrips ) const;

		IntParameterPtr m_compressionParameter;
		IntParameterPtr m_bitDepthParameter;

		void constructParameters();
};

IE_CORE_DECLAREPTR( TIFFImageWriter );

} // namespace IECore

#endif // IE_CORE_TIFFIMAGEWRITER_H
