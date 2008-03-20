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

#ifndef IE_CORE_JPEGIMAGEWRITER_H
#define IE_CORE_JPEGIMAGEWRITER_H

#include "IECore/ImageWriter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

/// The JPEGImageWriter class serializes images to the Joint Photographic Experts Group (JPEG) format
class JPEGImageWriter : public ImageWriter
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( JPEGImageWriter, ImageWriter )

		JPEGImageWriter();

		/// construct an JPEGImageWriter for the given image and output filename
		JPEGImageWriter(ObjectPtr object, const std::string & fileName);

		/// free any resources consumed in writing the associated image
		virtual ~JPEGImageWriter();

		IntParameterPtr qualityParameter();
		ConstIntParameterPtr qualityParameter() const;

	private:

		static const WriterDescription<JPEGImageWriter> m_writerDescription;

		/// write the image
		virtual void writeImage(
		        std::vector<std::string> &names,
		        ConstImagePrimitivePtr image,
		        const Imath::Box2i &dw
		);

		void constructParameters();
		
		template<typename T>
		void encodeChannel( ConstDataPtr dataContainer, const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, int numChannels, int channelOffset, std::vector<unsigned char> &imageBuffer );

	public:

		/// \todo Move all ExtraData members to here on next major version change
		struct ExtraData;

};

IE_CORE_DECLAREPTR(JPEGImageWriter);

} // namespace IECore

#endif // IE_CORE_JPEGIMAGEWRITER_H
