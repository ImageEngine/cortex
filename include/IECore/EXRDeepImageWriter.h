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
#ifndef IECORE_EXRDEEPIMAGEWRITER_H
#define IECORE_EXRDEEPIMAGEWRITER_H

#include "OpenEXR/ImfDeepScanLineOutputFile.h"
#include "OpenEXR/ImfCompression.h"
#include "OpenEXR/half.h"

#include "IECore/Export.h"
#include "IECore/DeepImageWriter.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypeIds.h"

namespace IECore
{

/// The EXRDeepImageWriter class writes EXR 2.0 deep image files.
/// \todo Currently we only support the writing of pixels by scanline in ascending order. Add random access.
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class IECORE_API EXRDeepImageWriter : public DeepImageWriter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( EXRDeepImageWriter, EXRDeepImageWriterTypeId, DeepImageWriter );

		EXRDeepImageWriter();
		EXRDeepImageWriter( const std::string &filename );

		virtual ~EXRDeepImageWriter();

		static bool canWrite( const std::string &filename );

		/// The different modes of compression that are available.
		enum Compression
		{
			None = Imf::NO_COMPRESSION,
			RLE = Imf::RLE_COMPRESSION,
			ZIPS = Imf::ZIPS_COMPRESSION
		};
	
	protected :
		
		virtual void doWritePixel( int x, int y, const DeepPixel *pixel );
		
		Imf::Compression compression() const;

	private :
		
		void clearScanlineBuffer();
		void appendParameters();
		void writeScanline();
		unsigned int numberOfChannels() const;
		const std::string &channelName( unsigned int index ) const;

		static const DeepImageWriterDescription<EXRDeepImageWriter> g_writerDescription;

		/// Tries to open the file for writing, throwing on failure. On success,
		/// all of the private members will be valid.
		void open();
		
		Imf::DeepScanLineOutputFile *m_outputFile;
		IntParameterPtr m_compressionParameter;
		StringVectorParameterPtr m_halfChannelsParameter;
	
		unsigned int m_numberOfFloatChannels;	
		unsigned int m_numberOfHalfChannels;	
		
		std::vector< unsigned > m_sampleCount;
		std::vector< const void * > m_samplePointers;

		std::vector< std::vector< float > > m_floatSamples;
		std::vector< std::vector< half > > m_halfSamples;

		std::vector< std::vector< float > > m_depthSamples;
		std::vector< const float * > m_depthPointers;
		
		std::vector<Imf::PixelType> m_channelTypes;

		int m_width, m_height;
		int m_currentSlice, m_lastSlice;
};

IE_CORE_DECLAREPTR( EXRDeepImageWriter );

} // namespace IECore

#endif // IECORE_EXRDEEPIMAGEWRITER_H
