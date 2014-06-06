//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_SHWDEEPIMAGEREADER_H
#define IECORERI_SHWDEEPIMAGEREADER_H

#include "dtex.h"

#include "IECore/DeepImageReader.h"

#include "IECoreRI/TypeIds.h"

namespace IECoreRI
{

/// The SHWDeepImageReader class reads 3delight deep shadow files. Note that this is an Alpha-only format.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class SHWDeepImageReader : public IECore::DeepImageReader
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( SHWDeepImageReader, SHWDeepImageReaderTypeId, IECore::DeepImageReader );

		SHWDeepImageReader();
		SHWDeepImageReader( const std::string &filename );

		virtual ~SHWDeepImageReader();
		
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

		static const ReaderDescription<SHWDeepImageReader> g_readerDescription;

		/// Tries to open the file, returning true on success and false on failure. On success,
		/// all of the private members will be valid. If throwOnFailure is true then a descriptive
		/// Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );
		void clean();
		
		DtexFile *m_inputFile;
		DtexCache *m_dtexCache;
		DtexImage *m_dtexImage;
		DtexPixel *m_dtexPixel;
		
		Imath::Box2i m_dataWindow;
		Imath::M44f m_worldToCamera;
		Imath::M44f m_worldToNDC;
		Imath::M44f &m_NDCToCamera();
		std::string m_inputFileName;
		std::string m_channelNames;

};

IE_CORE_DECLAREPTR( SHWDeepImageReader );

} // namespace IECoreRI

#endif // IECORERI_SHWDEEPIMAGEREADER_H
