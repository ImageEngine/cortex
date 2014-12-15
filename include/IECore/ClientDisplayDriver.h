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

//! \file ClientDisplayDriver.h
/// Defines the ClientDisplayDriver class.

#ifndef IE_CORE_CLIENTDISPLAYDRIVER
#define IE_CORE_CLIENTDISPLAYDRIVER

#include "IECore/Export.h"
#include "IECore/DisplayDriver.h"

namespace IECore
{


/// Connects to a DisplayDriverServer and forwards the image to the server using socket messages.
/// This client class works synchronously.
/// It forwards all parameters to the server and also includes one called "clientPID" to help grouping AOVs from the same render.
/// You must set the parameter 'remoteDisplayType' with a registered display driver to be instantiated in the server side.
/// \ingroup renderingGroup
class IECORE_API ClientDisplayDriver : public DisplayDriver
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( ClientDisplayDriver, DisplayDriver );

		// Constructor.
		// Expects two StringData parameters: displayHost and displayPort.
		ClientDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, ConstCompoundDataPtr parameters );

		virtual ~ClientDisplayDriver();

		// Get the target host name
		std::string host() const;

		// Get the port number or service name
		std::string port() const;

		virtual bool scanLineOrderOnly() const;
		
		virtual bool acceptsRepeatedData() const;

		virtual void imageData( const Imath::Box2i &box, const float *data, size_t dataSize );

		virtual void imageClose();

	private:

		static const DisplayDriverDescription<ClientDisplayDriver> g_description;

		void sendHeader( int msg, size_t dataSize );
		size_t receiveHeader( int msg );

		class PrivateData;
		IE_CORE_DECLAREPTR( PrivateData );
		PrivateDataPtr m_data;

};

IE_CORE_DECLAREPTR( ClientDisplayDriver )


}  // namespace IECore

#endif // IE_CORE_CLIENTDISPLAYDRIVER
