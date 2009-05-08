//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_IMAGEREADER_H
#define IE_CORE_IMAGEREADER_H

#include "IECore/Reader.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );


/// The ImageReader class defines an abstract base class for reading sampled images.
/// ImageReader's main purpose is to define a standard set of parameters
/// which all concrete ImageReader implementations obey.  It also defines some pure virtual functions
/// which allow interface implementors to focus on image-specific code for loading channels.
/// \todo Define and support a parameter for conversion of image data type during loading
/// \todo Define and support a parameter for conversion of colourspace during loading
class ImageReader : public Reader
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( ImageReader, Reader );

		ImageReader( const std::string &name, const std::string &description );

		virtual CompoundObjectPtr readHeader();

		//! @name Parameter accessors
		/// These provide convenient access to the parameters controlling
		/// image loading.
		///////////////////////////////////////////////////////////////
		//@{
		/// The parameter specifying the dataWindow of the loaded image.
		Box2iParameterPtr dataWindowParameter();
		ConstBox2iParameterPtr dataWindowParameter() const;
		/// The parameter specifying the displayWindow of the loaded image.
		Box2iParameterPtr displayWindowParameter();
		ConstBox2iParameterPtr displayWindowParameter() const;
		/// The parameter specifying the channels to load.
		StringVectorParameterPtr channelNamesParameter();
		ConstStringVectorParameterPtr channelNamesParameter() const;
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
		/// Returns the displayWIndow contained in the file. This is the displayWindow
		/// that will be loaded if the displayWindowParameter() is left at its default value.
		virtual Imath::Box2i displayWindow() = 0;
		/// Reads the specified channel. This function obeys the dataWindowParameter(), so
		/// that a subsection of the channel will be loaded if requested.
		DataPtr readChannel( const std::string &name );

		/// Returns the name of the colorspace in which the image is stored. For example,
		/// this would usually be "srgb" for a JPEG file or "linear" for an EXR file.
		virtual std::string sourceColorSpace() const = 0;

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
		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );

		/// Read the specified area from the channel with the specified name - this is called
		/// by the public readChannel() method and the doOperation() method, and must be implemented
		/// in all derived classes. It is guaranteed that this function will not be called with
		/// invalid names or dataWindows which are not wholly within the dataWindow in the file.
		virtual DataPtr readChannel( const std::string &name, const Imath::Box2i &dataWindow ) = 0;

	private :

		Box2iParameterPtr m_dataWindowParameter;
		Box2iParameterPtr m_displayWindowParameter;
		StringVectorParameterPtr m_channelNamesParameter;

};

IE_CORE_DECLAREPTR(ImageReader);

} // namespace IECore

#endif // IE_CORE_IMAGEREADER_H
