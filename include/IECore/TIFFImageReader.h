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

#ifndef IE_CORE_TIFFIMAGEREADER_H
#define IE_CORE_TIFFIMAGEREADER_H

#include "IECore/ImageReader.h"
#include "IECore/VectorTypedData.h"

// forward declaration
struct tiff;

namespace IECore
{	
  
/// The TIFFImageReader reads Tagged Image File Format (TIFF) files
/// \todo Support RGBA images
/// \bug This doesn't set the display window of the loaded image
class TIFFImageReader : public ImageReader 
{
    
	public:
	
		IE_CORE_DECLARERUNTIMETYPED( TIFFImageReader, ImageReader );

		TIFFImageReader();
		TIFFImageReader(const std::string & filename);
		virtual ~TIFFImageReader();
	
		static bool canRead(const std::string & filename);
	
		/// give the channel names into the vector
		virtual void channelNames(std::vector<std::string> & names);

	private:
	
		virtual void readChannel(std::string name, ImagePrimitivePtr image, const Imath::Box2i & dataWindow);

		// filename associator
		static const ReaderDescription<TIFFImageReader> m_readerDescription;

		/// read the image into the buffer with a single pass
		bool open();
	
		// tiff image pointer
		tiff *m_tiffImage;
		std::string m_tiffImageFileName;
		unsigned char *m_buffer; /// \todo Use a std::vector instead
	
		// reads the interlaced data into the buffer
		void read_buffer();
	
};
	
IE_CORE_DECLAREPTR(TIFFImageReader);
	
} // namespace IECore

#endif // IE_CORE_TIFFIMAGEREADER_H
