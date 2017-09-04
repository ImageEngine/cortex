//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

//! \file DisplayDriver.h
/// Defines the DisplayDriver abstract base class.

#ifndef IECOREIMAGE_DISPLAYDRIVER_H
#define IECOREIMAGE_DISPLAYDRIVER_H

#include <vector>

#include "boost/function.hpp"

#include "OpenEXR/ImathBox.h"

#include "IECore/RunTimeTyped.h"
#include "IECore/CompoundData.h"

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECoreImage
{

IE_CORE_FORWARDDECLARE( DisplayDriver );

/// Abstract base class for generic image display drivers.
/// Derived classes should implement the virtual methods that are used as callbacks for opening a new pass, updating an image block and closing the image.
/// \ingroup renderingGroup
class IECOREIMAGE_API DisplayDriver : public IECore::RunTimeTyped
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( DisplayDriver, DisplayDriverTypeId, IECore::RunTimeTyped );

		/// Initializes the display driver for showing a new image according to the given parameters.
		DisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters );

		virtual ~DisplayDriver();

		/// Defines a sub-region of the image.
		/// Called multiple times depending on how the channels are being computed.
		/// \param box Defines the area on which the given data should be written on the channel.
		/// \param data Points to a block of float values with interleaved channel data. Each pixel will have the same number of floats as the channelNames parameter in the constructor.
		/// \param dataSize Is the number of float values in the data. It must be box.width * box.height * numberOfChannels.
		virtual void imageData( const Imath::Box2i &box, const float *data, size_t dataSize ) = 0;

		/// Finalizes the display driver for the current image being constructed.
		virtual void imageClose() = 0;

		/// Indicates whether this display driver only accepts data one line each time.
		virtual bool scanLineOrderOnly() const = 0;

		/// Indicates whether this display driver will accept calls to
		/// imageData() that respecify data sent by a previous
		/// call. By accepting repeated data, it is possible to receive images
		/// from progressive and/or rerendering renders. 
		virtual bool acceptsRepeatedData() const = 0;

		/// Returns display window size.
		Imath::Box2i displayWindow() const;

		/// Returns data window size.
		Imath::Box2i dataWindow() const;

		/// Returns channelNames.
		const std::vector<std::string> &channelNames() const;

		//! @name Factory functions
		/////////////////////////////////////////////////////////////////////
		//@{
		/// Factory function for creating display drivers. Returns a new display driver
		/// of the specified type, constructed using the specified parameters.
		static DisplayDriverPtr create( const std::string &typeName, const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters );
		/// Definition of a function which can create a display driver.
		typedef boost::function<DisplayDriverPtr ( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters )> CreatorFn;
		/// Registers a new display driver type.
		static void registerType( const std::string &typeName, CreatorFn creator );
		//@}
	
	protected :
	
		/// Create a static const instance of one of these to automatically register a display driver type. Should
		/// be templated on the type of the DisplayDriver.
		template<typename T>
		class DisplayDriverDescription
		{
			public :
				DisplayDriverDescription();
			private :			
				static DisplayDriverPtr creator( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters );
		};

	private :

		typedef std::map<std::string, CreatorFn > TypeNamesToCreators;
		static TypeNamesToCreators &typeNamesToCreators();

		Imath::Box2i m_displayWindow;
		Imath::Box2i m_dataWindow;
		std::vector<std::string> m_channelNames;
		
};

}  // namespace IECoreImage

#include "IECoreImage/DisplayDriver.inl"

#endif // IECOREIMAGE_DISPLAYDRIVER_H
