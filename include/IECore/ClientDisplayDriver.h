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

//! \file ClientDisplayDriver.h
/// Defines the ClientDisplayDriver class.

#ifndef IE_CORE_CLIENTDISPLAYDRIVER
#define IE_CORE_CLIENTDISPLAYDRIVER

#include <boost/asio.hpp>

#include "IECore/DisplayDriver.h"
#include "IECore/DisplayDriverServer.h"

namespace IECore
{

/*
* Connects to a DisplayDriverServer and pass every call do DisplayDriver functions to it as socket messages.
* The protocol is explained in DisplayDriverServer class. This client class works synchronously.
*/
class ClientDisplayDriver : public DisplayDriver
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( ClientDisplayDriver, DisplayDriver );

		// Constructor.
		// Expects two StringData parameters: host and port.
		ClientDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, ConstCompoundDataPtr parameters );

		virtual ~ClientDisplayDriver();

		// Get the target host name
		std::string host() const;

		// Get the port number or service name
		std::string port() const;

		virtual bool scanLineOrderOnly() const;

		virtual void imageData( const Imath::Box2i &box, const float *data, size_t dataSize );

		virtual void imageClose();

	private:

		void sendHeader( DisplayDriverServer::MessageType msg, size_t dataSize );
		size_t receiveHeader( DisplayDriverServer::MessageType msg );

		boost::asio::io_service m_service;
		std::string m_host;
		std::string m_port;
		bool m_scanLineOrderOnly;
		boost::asio::ip::tcp::socket m_socket;

};

}  // namespace IECore

#endif // IE_CORE_CLIENTDISPLAYDRIVER
