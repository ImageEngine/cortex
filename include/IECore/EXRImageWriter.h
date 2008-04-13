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

#ifndef IE_CORE_EXRIMAGEWRITER_H
#define IE_CORE_EXRIMAGEWRITER_H

#include "IECore/ImageWriter.h"

// ILM
#include "OpenEXR/Iex.h"
#include "OpenEXR/ImfOutputFile.h"
#include "OpenEXR/ImfArray.h"
#include "OpenEXR/ImfHeader.h"
#include "OpenEXR/ImfCompression.h"
#include "OpenEXR/ImfChannelList.h"

namespace IECore
{

/// The EXRImageWriter class serializes images to the OpenEXR HDR image format.
/// N.B Both Shake and Nuke seem to assume channel names "R", "G", "B", and "A"
/// - lowercase do not work as expected.
class EXRImageWriter : public ImageWriter
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( EXRImageWriter, ImageWriter )

		EXRImageWriter();

		/// construct an EXRImageWriter for the given image and output filename
		EXRImageWriter( ObjectPtr object, const std::string & fileName );

	private:

		static const WriterDescription<EXRImageWriter> m_writerDescription;

		virtual void writeImage(const std::vector<std::string> &names, 
		                        ConstImagePrimitivePtr image,
		                        const Imath::Box2i &dw) const;

		template<typename T>
		void writeTypedChannel(const char *name,
		                       const Imath::Box2i &dw, const std::vector<T> &channel,
		                       const Imf::PixelType TYPE, Imf::Header &header,
		                       Imf::FrameBuffer &fb) const;

};

IE_CORE_DECLAREPTR(EXRImageWriter);

} // namespace IECore

#endif // IE_CORE_EXRIMAGEWRITER_H
