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

//! \file DisplayDriver.h
/// Defines the DisplayDriver abstract base class.

#ifndef IE_CORE_DISPLAYDRIVER_H
#define IE_CORE_DISPLAYDRIVER_H

#include <vector>
#include <boost/function.hpp>
#include <OpenEXR/ImathBox.h>

#include <IECore/RunTimeTyped.h>
#include <IECore/CompoundData.h>

namespace IECore
{


IE_CORE_FORWARDDECLARE( DisplayDriver );

/*
* Abstract base class for generic image display drivers.
* Derived classes should implement the virtual methods that are used as callbacks for opening a new pass, updating an image block and closing the image.
* \todo derive from RunTimeTyped
* \todo factory function
*/
class DisplayDriver : public RunTimeTyped
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( DisplayDriver, RunTimeTyped );

		// Initializes the display driver for showing a new image according to the given parameters.
		DisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, ConstCompoundDataPtr parameters );

		virtual ~DisplayDriver();

		// Defines a sub-region of the image.
		// Called multiple times depending on how the channels are being computed.
		// \param box Defines the area on which the given data should be written on the channel.
		// \param data Points to a block of float values with interleaved channel data. Each pixel will have the same number of floats as the channelNames parameter in the constructor.
		// \param dataSize Is the number of float values in the data. It must be box.width * box.height * numberOfChannels.
		virtual void imageData( const Imath::Box2i &box, const float *data, size_t dataSize ) = 0;

		// Finalizes the display driver for the current image being constructed.
		virtual void imageClose() = 0;

		// Indicates whether this display driver only accepts data one line each time.
		virtual bool scanLineOrderOnly() const = 0;

		// returns display window size.
		Imath::Box2i displayWindow() const;

		// returns data window size.
		Imath::Box2i dataWindow() const;

		// returns channelNames.
		const std::vector<std::string> &channelNames() const;

		// Factory class for DisplayDrivers
		class DisplayDriverCreator : public RunTimeTyped
		{
			public:

				IE_CORE_DECLARERUNTIMETYPED( DisplayDriverCreator, RunTimeTyped );

				// Should return a null pointer if it cannot instantiate a DisplayDriver for the given parameters.
				virtual DisplayDriverPtr create( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters ) = 0;
		};
		IE_CORE_DECLAREPTR( DisplayDriverCreator )

		// factory function for display drivers. 
		static DisplayDriverPtr create( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, ConstCompoundDataPtr parameters );

		// Registers a factory function for display drivers.
		static bool registerFactory( DisplayDriverCreatorPtr creator );

		// Unregisters a factory function
		static bool unregisterFactory( DisplayDriverCreatorPtr creator );

	private:

		static std::vector< DisplayDriverCreatorPtr > &factoryList();

		Imath::Box2i m_displayWindow;
		Imath::Box2i m_dataWindow;
		std::vector<std::string> m_channelNames;
};

}  // namespace IECore

#endif // IE_CORE_DISPLAYDRIVER_H
