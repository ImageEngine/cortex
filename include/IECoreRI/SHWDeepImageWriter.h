//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_SHWDEEPIMAGEWRITER_H
#define IECORERI_SHWDEEPIMAGEWRITER_H

#include "dtex.h"

#include "IECore/DeepImageWriter.h"

#include "IECoreRI/TypeIds.h"

namespace IECoreRI
{

/// The SHWDeepImageWriter class writes 3delight deep shadow files. As this is an Alpha-only format,
/// only the A channel will be used and the rest will be ignored. If A does not exist, then the first
/// channel will be used in its place, regardless of name.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class SHWDeepImageWriter : public IECore::DeepImageWriter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( SHWDeepImageWriter, SHWDeepImageWriterTypeId, IECore::DeepImageWriter );

		SHWDeepImageWriter();
		SHWDeepImageWriter( const std::string &filename );

		virtual ~SHWDeepImageWriter();

		static bool canWrite( const std::string &filename );

	private :

		static const DeepImageWriterDescription<SHWDeepImageWriter> g_writerDescription;

		virtual void doWritePixel( int x, int y, const IECore::DeepPixel *pixel );

		/// Tries to open the file for writing, throwing on failure. On success,
		/// all of the private members will be valid.
		void open();
		void clean();
		
		IECore::V2iParameterPtr m_tileSizeParameter;
		
		DtexFile *m_outputFile;
		DtexCache *m_dtexCache;
		DtexImage *m_dtexImage;
		DtexPixel *m_dtexPixel;

		Imath::M44f m_NDCToCamera;
		std::string m_outputFileName;
		int m_alphaOffset;

};

IE_CORE_DECLAREPTR( SHWDeepImageWriter );

} // namespace IECoreRI

#endif // IECORERI_SHWDEEPIMAGEWRITER_H
