//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_DTEXDEEPIMAGEREADER_H
#define IECORERI_DTEXDEEPIMAGEREADER_H

#include "RixDeepTexture.h"

#include "IECore/CompoundObject.h"
#include "IECore/DeepImageReader.h"

#include "IECoreRI/Export.h"
#include "IECoreRI/TypeIds.h"

namespace IECoreRI
{

/// The DTEXDeepImageReader class reads PRMan deep texture files. Note that it will only
/// read the first RixDeepTexture::DeepImage in the RixDeepTexture::DeepFile.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class IECORERI_API DTEXDeepImageReader : public IECore::DeepImageReader
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( DTEXDeepImageReader, DTEXDeepImageReaderTypeId, IECore::DeepImageReader );

		DTEXDeepImageReader();
		DTEXDeepImageReader( const std::string &filename );

		virtual ~DTEXDeepImageReader();
		
		static bool canRead( const std::string &filename );

		virtual void channelNames( std::vector<std::string> &names );
		virtual bool isComplete();
		virtual Imath::Box2i dataWindow();
		virtual Imath::Box2i displayWindow();
		virtual Imath::M44f worldToCameraMatrix();
		virtual Imath::M44f worldToNDCMatrix();

	protected :

		virtual IECore::DeepPixelPtr doReadPixel( int x, int y );

	private :

		static const ReaderDescription<DTEXDeepImageReader> g_readerDescription;

		/// Tries to open the file, returning true on success and false on failure. On success,
		/// all of the private members will be valid. If throwOnFailure is true then a descriptive
		/// Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );
		void cleanRixInterface();
		
		RixDeepTexture::DeepFile *m_inputFile;
		RixDeepTexture::DeepCache *m_dtexCache;
		RixDeepTexture::DeepImage *m_dtexImage;
		RixDeepTexture::DeepPixel *m_dtexPixel;
		Imath::Box2i m_dataWindow;
		Imath::M44f m_worldToCamera;
		Imath::M44f m_worldToNDC;
		std::string m_inputFileName;
		std::string m_channelNames;

};

IE_CORE_DECLAREPTR( DTEXDeepImageReader );

} // namespace IECoreRI

#endif // IECORERI_DTEXDEEPIMAGEREADER_H
