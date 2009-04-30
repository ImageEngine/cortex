//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_CINIMAGEREADER_H
#define IE_CORE_CINIMAGEREADER_H

#include "IECore/ImageReader.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

/// The CINImageReader reads Kodak Cineon (CIN) files.
/// Currently, only the overwhelmingly popular 10-bit log-encoded pixel-interlaced
/// 32-bit word boundary format is loaded.
class CINImageReader : public ImageReader
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( CINImageReader, ImageReader );

		CINImageReader();
		CINImageReader( const std::string &filename );
		virtual ~CINImageReader();

		static bool canRead( const std::string &filename );

		virtual void channelNames( std::vector<std::string> &names );
		virtual bool isComplete();
		virtual Imath::Box2i dataWindow();
		virtual Imath::Box2i displayWindow();
		virtual std::string sourceColorSpace() const ;		

	private:


		virtual DataPtr readChannel( const std::string &name, const Imath::Box2i &dataWindow );

		// filename associator
		static const ReaderDescription<CINImageReader> m_readerDescription;

		/// Opens the file, if necessary, and fills the buffer. Throws an IOException if an error occurs.
		/// Tries to open the file, returning true on success and false on failure. On success,
		/// the member data derived from the Cineons's header will be valid.
		/// If throwOnFailure is true then a descriptive Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );

		/// CIN image memory buffer - cineon image data is typically found in pixel-interlaced
		/// format, so we will make one I/O pass and cache the image in memory while striping
		/// off channels / planes
		std::vector<unsigned int> m_buffer;

		/// the filename in effect when we filled the buffer last.
		std::string m_bufferFileName;
		unsigned int m_bufferWidth, m_bufferHeight;
		bool m_reverseBytes;
		
		struct Header;
		Header *m_header;
};

IE_CORE_DECLAREPTR(CINImageReader);

} // namespace IECore

#endif // IE_CORE_CINIMAGEREADER_H
