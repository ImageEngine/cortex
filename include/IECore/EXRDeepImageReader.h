//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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
#ifndef IECORE_EXRDEEPIMAGEREADER_H
#define IECORE_EXRDEEPIMAGEREADER_H

#include "OpenEXR/ImfDeepScanLineInputFile.h"

#include "IECore/DeepImageReader.h"
#include "IECore/LRUCache.h"
#include "IECore/TypeIds.h"

namespace IECore
{

/// The EXRDeepImageReader class reads EXR 2.0 deep image files.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class EXRDeepImageReader : public DeepImageReader
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( EXRDeepImageReader, EXRDeepImageReaderTypeId, DeepImageReader );

		EXRDeepImageReader();
		EXRDeepImageReader( const std::string &filename );

		virtual ~EXRDeepImageReader();

		static bool canRead( const std::string &filename );

		virtual void channelNames( std::vector<std::string> &names );
		virtual bool isComplete();
		virtual Imath::Box2i dataWindow();
		virtual Imath::Box2i displayWindow();
		virtual Imath::M44f worldToNDCMatrix();
		virtual Imath::M44f worldToCameraMatrix();

	protected :

		virtual DeepPixelPtr doReadPixel( int x, int y );

	private :

		static const ReaderDescription<EXRDeepImageReader> g_readerDescription;

		/// Tries to open the file, returning true on success and false on failure. On success,
		/// all of the private members will be valid. If throwOnFailure is true then a descriptive
		/// Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );
		
		class Scanline : public RefCounted
		{
			public :
				
				IE_CORE_DECLAREMEMBERPTR( Scanline );
				
				Scanline( size_t width, size_t numChannels );
				
				std::vector<unsigned> sampleCount;
				std::vector<void *> pointers;
				std::vector<char> data;
		
		};
		
		struct Getter;
		typedef LRUCache<int, Scanline::Ptr> Cache;
		
		Cache *m_cache;
		Imf::DeepScanLineInputFile *m_inputFile;
		
		int m_depthChannel;
		std::vector<std::string> m_channelNames;
		std::vector<Imf::PixelType> m_channelTypes;

};

IE_CORE_DECLAREPTR( EXRDeepImageReader );

} // namespace IECore

#endif // IECORE_EXRDEEPIMAGEREADER_H
