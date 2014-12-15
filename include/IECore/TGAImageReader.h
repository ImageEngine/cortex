//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TGAIMAGEREADER_H
#define IE_CORE_TGAIMAGEREADER_H

#include "boost/shared_ptr.hpp"

#include "IECore/Export.h"
#include "IECore/ImageReader.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

/// The TGAImageReader reads version 1 Truevision Targa files
/// \ingroup ioGroup
class IECORE_API TGAImageReader : public ImageReader
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( TGAImageReader, ImageReader );

		TGAImageReader();
		TGAImageReader( const std::string &filename );
		virtual ~TGAImageReader();

		static bool canRead( const std::string &filename );

		virtual void channelNames( std::vector<std::string> &names );
		virtual bool isComplete();
		virtual Imath::Box2i dataWindow();
		virtual Imath::Box2i displayWindow();
		virtual std::string sourceColorSpace() const ;

	private:

		virtual DataPtr readChannel( const std::string &name, const Imath::Box2i &dataWindow, bool raw );

		template<typename V>
		DataPtr readTypedChannel( const std::string &name, const Imath::Box2i &dataWindow );

		void readBuffer();

		static const ReaderDescription<TGAImageReader> m_readerDescription;

		/// Opens the file, if necessary, and fills the buffer. Throws an IOException if an error occurs.
		/// Tries to open the file, returning true on success and false on failure. On success,
		/// the member data derived from the Targa's header will be valid.
		/// If throwOnFailure is true then a descriptive Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );

		Imath::Box2i m_dataWindow;
		std::vector<char> m_buffer;

		/// The filename in effect when we filled the buffer last.
		std::string m_bufferFileName;

		/// The filename in effect when we last read the header.
		std::string m_headerFileName;

		struct Header;
		boost::shared_ptr<Header> m_header;
};

IE_CORE_DECLAREPTR( TGAImageReader );

} // namespace IECore

#endif // IE_CORE_TGAIMAGEREADER_H
