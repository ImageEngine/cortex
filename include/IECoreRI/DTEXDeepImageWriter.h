//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_DTEXDEEPIMAGEWRITER_H
#define IECORERI_DTEXDEEPIMAGEWRITER_H

#include "RixDeepTexture.h"

#include "IECore/DeepImageWriter.h"

#include "IECoreRI/Export.h"
#include "IECoreRI/TypeIds.h"

namespace IECoreRI
{

/// The DTEXDeepImageWriter class writes PRMan deep texture files.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class IECORERI_API DTEXDeepImageWriter : public IECore::DeepImageWriter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( DTEXDeepImageWriter, DTEXDeepImageWriterTypeId, IECore::DeepImageWriter );

		DTEXDeepImageWriter();
		DTEXDeepImageWriter( const std::string &filename );

		virtual ~DTEXDeepImageWriter();

		static bool canWrite( const std::string &filename );

	private :

		static const DeepImageWriterDescription<DTEXDeepImageWriter> g_writerDescription;

		virtual void doWritePixel( int x, int y, const IECore::DeepPixel *pixel );

		/// Tries to open the file for writing, throwing on failure. On success,
		/// all of the private members will be valid.
		void open();
		void cleanRixInterface();
		
		IECore::V2iParameterPtr m_tileSizeParameter;
		
		RixDeepTexture::DeepFile *m_outputFile;
		RixDeepTexture::DeepCache *m_dtexCache;
		RixDeepTexture::DeepImage *m_dtexImage;
		RixDeepTexture::DeepPixel *m_dtexPixel;
		std::string m_outputFileName;

};

IE_CORE_DECLAREPTR( DTEXDeepImageWriter );

} // namespace IECoreRI

#endif // IECORERI_DTEXDEEPIMAGEWRITER_H
