//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PNGIMAGEREADER_H
#define IE_CORE_PNGIMAGEREADER_H

#include "IECore/Export.h"
#include "IECore/ImageReader.h"

struct PNGImageData;

namespace IECore
{

/// The PNGImageReader reads Portable Network Graphics (PNG) files
/// \ingroup ioGroup
class IECORE_API PNGImageReader : public ImageReader
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( PNGImageReader, ImageReader );

		PNGImageReader();
		PNGImageReader( const std::string &filename );
		virtual ~PNGImageReader();

		static bool canRead( const std::string &filename );

		//! @name Image specific reading functions
		///////////////////////////////////////////////////////////////
		//@{
		/// Fills the passed vector with the names of all channels within the file.
		virtual void channelNames( std::vector<std::string> &names );

		/// Returns true if the file is complete. This intended as a cheaper alternative to loading the
		/// whole file to determine completeness.
		virtual bool isComplete();

		/// Returns the dataWindow contained in the file. This is the dataWindow that
		/// will be loaded if the dataWindowParameter() is left at its default value.
		virtual Imath::Box2i dataWindow();

		/// Returns the displayWindow contained in the file. This is the displayWindow
		/// that will be loaded if the displayWindowParameter() is left at its default value.
		virtual Imath::Box2i displayWindow();

		/// Returns the name of default colorspace in which the Reader returns images.
		virtual std::string sourceColorSpace() const ;
		//@}



	private:

		virtual DataPtr readChannel( const std::string &name, const Imath::Box2i &dataWindow, bool raw );

		/// Registers this reader with system
		static const ReaderDescription<PNGImageReader> m_readerDescription;

		/// Opens the file, if necessary, and fills the buffer. Throws an IOException if an error occurs.
		/// Tries to open the file, returning true on success and false on failure. On success,
		/// m_pngImageData will be filled out with relevant data and will be accurate.
		/// If throwOnFailure is true then a descriptive Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );
		
		/// The filename we filled the buffer from
		std::string m_bufferFileName;

		struct PNGImageData*	m_pngImageData;


		template<typename FromType, typename ToType>
		DataPtr readTypedChannel( const Imath::Box2i &dataWindow, int pixelOffset );
		
		void preMultiplyAlphas();
		void constructParameters();
		
		BoolParameterPtr	m_convertGreyToRGB;

};

IE_CORE_DECLAREPTR( PNGImageReader );

} // namespace IECore

#endif // IE_CORE_PNGIMAGEREADER_H
