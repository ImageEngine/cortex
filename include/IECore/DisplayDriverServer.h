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

//! \file DisplayDriverServer.h
/// Defines the DisplayDriverServer class.

#ifndef IE_CORE_DISPLAYDRIVERSERVER
#define IE_CORE_DISPLAYDRIVERSERVER

#include <map>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <IECore/RunTimeTyped.h>
#include "IECore/VectorTypedData.h"
#include "IECore/DisplayDriver.h"

namespace IECore
{

/*
* Opens a socket port and pass every socket request to a named DisplayDriver object in the display driver pool.
* The protocol is the following:
* 1. Server waits for a header block.
* 2. Depending on the message type:
*     - imageOpen : The data block followint the header is a MemoryIndexedIO buffer containing the parameters for 
*                   the imageOpen function ( displayWindow, dataWindow, channelNames, parameters )
*     - imageData : The data block is a MemoryIndexedIO buffer containing "box" and "data" parameters for imageData function.
*     - imageClose : The data block is zero bytes length.
* 3. Returns the result using the same header block structure. 
*    If there was any exception while executing the request, then the message 
*    type will be 'exception' and the data block will be a StringData object. 
*    Otherwise it will match the incomming message type.
*    In the case of imageOpen, it will return a data block of one byte containing 
*    the resulting scanLineOrderOnly value.
*    For imageData messages there will be no confirmation message to not compromise performance.
* 
* The server object creates a thread to control the socket connection. The thread dies when the object is destructed.
*/
class DisplayDriverServer : public RunTimeTyped
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( DisplayDriverServer, RunTimeTyped );

		DisplayDriverServer( int portNumber );
		~DisplayDriverServer();

	public:

		enum MessageType { imageOpen = 1, imageData = 2, imageClose = 3, exception = 4 };

		/* Header block used by back and forth messages with the server.
		* 7 bytes long:
		* [0] - magic number ( 0x82 )
		* [1] - protocol version ( 1 )
		* [2] - message type ( imageOpen, imageData, imageClose )
		* [3-6] - length of following data block.
		*/
		class Header
		{
			public:
				static const unsigned char headerLength = 7;
				static const unsigned char magicNumber = 0x82;
				static const unsigned char currentProtocolVersion = 1;

				Header();
				Header( MessageType msg, size_t dataSize );

				// returns internal buffer ( length = headerLength constant )
				unsigned char *buffer();

				// checks if the header is valid.
				bool valid();

				// returns the number of bytes is expected to follow the current header down from the socket connection.
				size_t getDataSize();

				// sets the number of bytes that will follow this header on the socket connection.
				void setDataSize( size_t dataSize );

				// returns the message type defined in the header.
				MessageType messageType();

			private:

				unsigned char m_header[ headerLength ];
		};
		
	private:

		// Session class
		// Takes care of one client connection.
		class Session : public RefCounted
		{
			public:

				Session( boost::asio::io_service& io_service );
				~Session();

				boost::asio::ip::tcp::socket& socket();
				void start();

			private:

				void handleReadHeader( const boost::system::error_code& error );
				void handleReadOpenParameters( const boost::system::error_code& error );
				void handleReadDataParameters( const boost::system::error_code& error );
				void sendResult( MessageType msg, size_t dataSize );
				void sendException( const char *message );

			private:
				boost::asio::ip::tcp::socket m_socket;
				DisplayDriverPtr m_displayDriver;
				Header m_header;
				CharVectorDataPtr m_buffer;
		};

		typedef boost::intrusive_ptr<Session> SessionPtr;

		void serverThread();
		void handleAccept( DisplayDriverServer::SessionPtr session, const boost::system::error_code& error);

		boost::asio::ip::tcp::endpoint m_endpoint;
		boost::asio::io_service m_service;
		boost::asio::ip::tcp::acceptor m_acceptor;
		boost::thread m_thread;
		bool m_startThread;
};

IE_CORE_DECLAREPTR( DisplayDriverServer )

} // namespace IECore

#endif // IE_CORE_DISPLAYDRIVERSERVER
