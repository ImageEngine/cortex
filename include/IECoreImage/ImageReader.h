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

#include "IECore/Reader.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );

}

namespace IECoreImage
{

/// The ImageReader class defines an abstract base class for reading sampled images.
/// ImageReader's main purpose is to define a standard set of parameters
/// which all concrete ImageReader implementations obey.  It also defines some pure virtual functions
/// which allow interface implementors to focus on image-specific code for loading channels.
/// The ImageReader will return by default an ImagePrimitive in linear colorspace with all channels
/// converted to FloatVectorData.
/// If 'rawChannels' is On, then it will return an ImagePrimitive with channels that are the close as 
/// possible to the original data type stored on the file. Note that most image Ops available on IECore
/// will only work on float data channels.
/// \ingroup ioGroup
class IECOREIMAGE_API ImageReader : public IECore::Reader
{

	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ImageReader, ImageReaderTypeId, IECore::Reader );

		ImageReader( const std::string &description );

		virtual IECore::CompoundObjectPtr readHeader();

		//! @name Parameter accessors
		/// These provide convenient access to the parameters controlling
		/// image loading.
		///////////////////////////////////////////////////////////////
		//@{
		/// The parameter specifying the dataWindow of the loaded image.
		IECore::Box2iParameter *dataWindowParameter();
		const IECore::Box2iParameter *dataWindowParameter() const;
		/// The parameter specifying the displayWindow of the loaded image.
		IECore::Box2iParameter *displayWindowParameter();
		const IECore::Box2iParameter *displayWindowParameter() const;
		/// The parameter specifying the channels to load.
		IECore::StringVectorParameter *channelNamesParameter();
		const IECore::StringVectorParameter *channelNamesParameter() const;
		/// The parameter specifying if the returned data channels should be
		/// exactly or as close as possible to what's stored in the file. 
		IECore::BoolParameter *rawChannelsParameter();
		const IECore::BoolParameter *rawChannelsParameter() const;
		//@}

		//! @name Image specific reading functions
		///////////////////////////////////////////////////////////////
		//@{
		/// Fills the passed vector with the names of all channels within the file.
		virtual void channelNames( std::vector<std::string> &names ) = 0;
		/// Returns true if the file is complete. Implementations of this function should
		/// be quick - it's intended as a cheaper alternative to loading the
		/// whole file to determine completeness.
		virtual bool isComplete() = 0;
		/// Returns the dataWindow contained in the file. This is the dataWindow that
		/// will be loaded if the dataWindowParameter() is left at its default value.
		virtual Imath::Box2i dataWindow() = 0;
		/// Returns the displayWindow contained in the file. This is the displayWindow
		/// that will be loaded if the displayWindowParameter() is left at its default value.
		virtual Imath::Box2i displayWindow() = 0;
		/// Reads the specified channel. This function obeys the dataWindowParameter(), so
		/// that a subsection of the channel will be loaded if requested.
		/// If raw is false it should return a FloatVectorData, otherwise
		/// it returns the raw data. It must return a vector data type and
		/// each element corresponds to a pixel. If that does not correspond
		/// to the native file format, then it should return a FloatVectorData.
		IECore::DataPtr readChannel( const std::string &name, bool raw = false );
		//@}

	protected:

		/// Fills the passed vector with the intersection of channelNames() and
		/// the channels requested by the user in channelNamesParameter().
		void channelsToRead( std::vector<std::string> &names );
		/// Returns the data window that should be loaded, throwing an Exception if it
		/// isn't wholly inside the available dataWindow().
		Imath::Box2i dataWindowToRead();

		/// Implemented using displayWindow(), dataWindow(), channelNames() and readChannel().
		/// Derived classes should implement those methods rather than reimplement this function.
		virtual IECore::ObjectPtr doOperation( const IECore::CompoundObject *operands );

		/// Read the specified area from the channel with the specified name - this is called
		/// by the public readChannel() method and the doOperation() method, and must be implemented
		/// in all derived classes. It is guaranteed that this function will not be called with 
		/// invalid names or dataWindows which are not wholly within the dataWindow in the file.
		virtual IECore::DataPtr readChannel( const std::string &name, const Imath::Box2i &dataWindow, bool raw ) = 0;

	private :

		IECore::Box2iParameterPtr m_dataWindowParameter;
		IECore::Box2iParameterPtr m_displayWindowParameter;
		IECore::StringVectorParameterPtr m_channelNamesParameter;
		IECore::BoolParameterPtr m_rawChannelsParameter;
};

IE_CORE_DECLAREPTR(ImageReader);

} // namespace IECoreImage

#endif // IECOREIMAGE_IMAGEREADER_H
