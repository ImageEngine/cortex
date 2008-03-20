//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_EXRIMAGEREADER_H
#define IE_CORE_EXRIMAGEREADER_H

#include "IECore/ImageReader.h"

namespace Imf
{
	class Channel;
	class InputFile;
};

namespace IECore
{

/// The EXRImageReader class reads OpenEXR files.
class EXRImageReader : public ImageReader
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( EXRImageReader, ImageReader );

		EXRImageReader();
		EXRImageReader( const std::string &filename );

		virtual ~EXRImageReader();

		static bool canRead( const std::string &filename );

		virtual void channelNames( std::vector<std::string> &names );
		virtual bool isComplete();
		virtual Imath::Box2i dataWindow();
		virtual Imath::Box2i displayWindow();

	private:

		template<class T>
		DataPtr readTypedChannel( const std::string &name, const Imath::Box2i &dataWindow, const Imf::Channel *channel );
		
		virtual DataPtr readChannel( const std::string &name, const Imath::Box2i &dataWindow );

		static const ReaderDescription<EXRImageReader> g_readerDescription;

		/// Tries to open the file, returning true on success and false on failure. On success,
		/// m_header and m_inputFile will be valid. If throwOnFailure is true then a descriptive
		/// Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );
		Imf::InputFile *m_inputFile;

};

IE_CORE_DECLAREPTR( EXRImageReader );

} // namespace IECore

#endif // IE_CORE_EXRIMAGEREADER_H
