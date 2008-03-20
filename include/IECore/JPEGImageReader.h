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

#ifndef IE_CORE_JPEGIMAGEREADER_H
#define IE_CORE_JPEGIMAGEREADER_H

#include "IECore/ImageReader.h"

namespace IECore
{

/// The JPEGImageReader reads Joint Photographic Experts Group (JPEG) files
class JPEGImageReader : public ImageReader
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( JPEGImageReader, ImageReader );

		JPEGImageReader();
		JPEGImageReader( const std::string & filename );
		virtual ~JPEGImageReader();

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
		//@}

	private:

		virtual DataPtr readChannel( const std::string &name, const Imath::Box2i &dataWindow );

		/// Registers this reader with system
		static const ReaderDescription<JPEGImageReader> m_readerDescription;

		/// Opens the file, if necessary, and fills the buffer. Throws an IOException if an error occurs.
		/// Tries to open the file, returning true on success and false on failure. On success,
                /// m_buffer, m_bufferWidth, m_bufferHeight, m_bufferFileName, and m_numChannels will be valid. 
		/// If throwOnFailure is true then a descriptive Exception is thrown rather than false being returned.
                bool open( bool throwOnFailure = false );

		/// The filename we filled the buffer from
		std::string m_bufferFileName;

		/// Decompressed image data buffer 
		std::vector<unsigned char> m_buffer;
		
		/// Information gathered from header
		int m_bufferWidth;
		int m_bufferHeight;		
		int m_numChannels;
};

IE_CORE_DECLAREPTR( JPEGImageReader );

} // namespace IECore

#endif // IE_CORE_JPEGIMAGEREADER_H
