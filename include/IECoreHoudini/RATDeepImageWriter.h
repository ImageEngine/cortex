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

#ifndef IECOREHOUDINI_RATDEEPIMAGEWRITER_H
#define IECOREHOUDINI_RATDEEPIMAGEWRITER_H

#include "IMG/IMG_DeepShadow.h"
#include "UT/UT_Version.h"

#include "IECore/DeepImageWriter.h"

#include "IECoreHoudini/TypeIds.h"

namespace IECoreHoudini
{

/// The RATDeepImageWriter class writes Houdini deep texture files. Note that C will
/// only be added to the RAT file if RGBA channels are specified in order. As Of is
/// required by the format, A will always be converted to an Of triple. If A is not
/// provided, a value of 1.0 will be used for all Of sub-channels.
/// \todo: verify that arbitrary channels can be written once the RATDeepImageReader
/// supports reading them.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class RATDeepImageWriter : public IECore::DeepImageWriter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( RATDeepImageWriter, RATDeepImageWriterTypeId, IECore::DeepImageWriter );

		RATDeepImageWriter();
		RATDeepImageWriter( const std::string &filename );

		virtual ~RATDeepImageWriter();

		static bool canWrite( const std::string &filename );

	private :

		static const DeepImageWriterDescription<RATDeepImageWriter> g_writerDescription;

		virtual void doWritePixel( int x, int y, const IECore::DeepPixel *pixel );

		/// Tries to open the file for writing, throwing on failure. On success,
		/// all of the private members will be valid.
		void open();
		
		IMG_DeepShadow *m_outputFile;

#if UT_MAJOR_VERSION_INT >= 13

		IMG_DeepPixelWriter *m_ratPixel;

#endif

		std::string m_outputFileName;
		int m_dataSize;
		int m_alphaOffset;
		int m_extraOffset;

};

IE_CORE_DECLAREPTR( RATDeepImageWriter );

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_RATDEEPIMAGEWRITER_H
