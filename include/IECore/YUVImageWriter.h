//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_YUVIMAGEWRITER_H
#define IE_CORE_YUVIMAGEWRITER_H

#include "IECore/ImageWriter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"

namespace IECore
{

/// The YUVImageWriter class serializes images to raw 8-bit YUV (actually Y'CbCr) files. This format is supported by ffmpeg.
/// Further YUV formats will be added in the future.
class YUVImageWriter : public ImageWriter
{
	public:
	
		typedef enum
		{
			/// Write a YUV 4:2:0 file as 3 separate image planes, Y followed by U, then V
			YUV420P,
			
			/// Write a YUV 4:2:2 file as 3 separate image planes, Y followed by U, then V
			YUV422P,
			
			/// Write a YUV 4:4:4 file as 3 separate image planes, Y followed by U, then V
			YUV444P,
			
		} Format;

		IE_CORE_DECLARERUNTIMETYPED( YUVImageWriter, ImageWriter )

		YUVImageWriter();

		/// Construct an YUVImageWriter for the given image and output filename
		YUVImageWriter(ObjectPtr object, const std::string & fileName);
		virtual ~YUVImageWriter();
		
		virtual std::string defaultColorSpace() const ;		

		IntParameterPtr formatParameter();
		ConstIntParameterPtr formatParameter() const;
		
		V2fParameterPtr kBkRParameter();
		ConstV2fParameterPtr kBkRParameter() const;
		
		Box3fParameterPtr rangeParameter();		
		ConstBox3fParameterPtr rangeParameter() const;	

	private:

		static const WriterDescription<YUVImageWriter> m_writerDescription;
		
		struct ChannelConverter;

		/// write the image
		virtual void writeImage( const std::vector<std::string> &names,
		                         ConstImagePrimitivePtr image,
		                         const Imath::Box2i &dw ) const;

		void constructParameters();
		
		IntParameterPtr m_formatParameter;
		V2fParameterPtr m_kBkRParameter;
		Box3fParameterPtr m_rangeParameter;

};

IE_CORE_DECLAREPTR(YUVImageWriter);

} // namespace IECore

#endif // IE_CORE_YUVIMAGEWRITER_H
