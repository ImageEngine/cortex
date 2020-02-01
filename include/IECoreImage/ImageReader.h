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

#ifndef IECOREIMAGE_IMAGEREADER_H
#define IECOREIMAGE_IMAGEREADER_H

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

#include "IECore/Reader.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECoreImage
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );

/// The ImageReader will return an ImagePrimitive in linear colorspace with all channels
/// converted to FloatVectorData. If 'rawChannels' is On, then it will return an
/// ImagePrimitive with channels that are the close as possible to the original data
/// type stored on the file.
/// \ingroup ioGroup
class IECOREIMAGE_API ImageReader : public IECore::Reader
{

	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ImageReader, ImageReaderTypeId, IECore::Reader );

		ImageReader();
		ImageReader( const std::string &filename );

		~ImageReader() override;

		static bool canRead( const std::string &filename );

		IECore::CompoundObjectPtr readHeader() override;

		//! @name Parameter accessors
		/// These provide convenient access to the parameters controlling
		/// image loading.
		///////////////////////////////////////////////////////////////
		//@{
		/// The parameter specifying the channels to load.
		IECore::StringVectorParameter *channelNamesParameter();
		const IECore::StringVectorParameter *channelNamesParameter() const;
		/// The parameter specifying if the returned data channels should be
		/// exactly or as close as possible to what's stored in the file.
		/// If true, the values will not be linearized nor converted to float.
		IECore::BoolParameter *rawChannelsParameter();
		const IECore::BoolParameter *rawChannelsParameter() const;
		/// The parameter specifying the miplevel to be read from the image file.
		IECore::IntParameter *mipLevelParameter();
		const IECore::IntParameter *mipLevelParameter() const;
		//@}

		//! @name Image specific reading functions
		///////////////////////////////////////////////////////////////
		//@{
		/// Fills the passed vector with the names of all channels within the file.
		void channelNames( std::vector<std::string> &names );
		/// Returns true if the file contains a valid image.
		bool isComplete();
		/// Returns the dataWindow contained in the file.
		Imath::Box2i dataWindow();
		/// Returns the displayWindow contained in the file.
		Imath::Box2i displayWindow();
		/// Reads the specified channel. This function obeys the dataWindowParameter(), so
		/// that a subsection of the channel will be loaded if requested.
		/// If raw is false it should return a FloatVectorData, otherwise
		/// it returns the raw data. It must return a vector data type and
		/// each element corresponds to a pixel. If that does not correspond
		/// to the native file format, then it should return a FloatVectorData.
		IECore::DataPtr readChannel( const std::string &name, bool raw = false );
		//@}

	protected :

		/// Implemented using displayWindow(), dataWindow(), channelNames() and readChannel().
		IECore::ObjectPtr doOperation( const IECore::CompoundObject *operands ) override;

	private :

		/// Fills the passed vector with the intersection of channelNames() and
		/// the channels requested by the user in channelNamesParameter().
		void channelsToRead( std::vector<std::string> &names );

		static const ReaderDescription<ImageReader> g_readerDescription;

		IECore::StringVectorParameterPtr m_channelNamesParameter;
		IECore::BoolParameterPtr m_rawChannelsParameter;
		IECore::IntParameterPtr m_miplevelParameter;

		class Implementation;
		std::unique_ptr<Implementation> m_implementation;

};

IE_CORE_DECLAREPTR(ImageReader);

} // namespace IECoreImage

#endif // IECOREIMAGE_IMAGEREADER_H
