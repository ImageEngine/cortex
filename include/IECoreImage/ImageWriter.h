//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2017, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREIMAGE_IMAGEWRITER_H
#define IECOREIMAGE_IMAGEWRITER_H

#include <vector>
#include <string>

#include "IECore/Writer.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECoreImage
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );

/// The ImageWriter serializes images to any of the various file formats
/// supported by OpenImageIO. A limited subset of format options are
/// expossed as parameters.
/// \ingroup ioGroup
class IECOREIMAGE_API ImageWriter : public IECore::Writer
{

	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ImageWriter, ImageWriterTypeId, IECore::Writer );

		ImageWriter();
		ImageWriter( IECore::ObjectPtr object, const std::string &fileName );

		virtual ~ImageWriter();

		/// Checks that object is an ImagePrimitive with consistent PrimitiveVariables
		static bool canWrite( IECore::ConstObjectPtr object, const std::string &fileName );

		/// The parameter specifying the channels to write.
		IECore::StringVectorParameter *channelNamesParameter();
		const IECore::StringVectorParameter *channelNamesParameter() const;

		/// The parameter specifying if the input channels are exactly
		/// or as close as possible to what will stored in the file.
		/// If true, the values will be assumed to be in the expected
		/// colorspace for the file format. Note that type changes may
		/// still be performed based on the format settings.
		IECore::BoolParameter *rawChannelsParameter();
		const IECore::BoolParameter *rawChannelsParameter() const;

		/// The parameter specifying the format specific settings.
		IECore::CompoundParameter *formatSettingsParameter();
		const IECore::CompoundParameter *formatSettingsParameter() const;

		/// Convenience function to access the channels that will be written
		/// to disk. This is calculated based on the requested channelNames,
		/// the channels existing in the ImagePrimitive, and the capabilities
		/// of the file format. If "operands" are provided they will override
		/// the parameter values.
		void channelsToWrite( std::vector<std::string> &channels, const IECore::CompoundObject *operands = nullptr ) const;

	protected :

		virtual void doWrite( const IECore::CompoundObject *operands );

	private :

		static const WriterDescription<ImageWriter> g_writerDescription;

		void constructCommon();

		const ImagePrimitive *getImage() const;

		IECore::StringVectorParameterPtr m_channelsParameter;
		IECore::BoolParameterPtr m_rawChannelsParameter;
		IECore::CompoundParameterPtr m_formatSettingsParameter;

};

IE_CORE_DECLAREPTR(ImageWriter);

} // namespace IECoreImage

#endif // IECOREIMAGE_IMAGEWRITER_H
