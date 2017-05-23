//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_DISPLAYDRIVERSERVERHEADER
#define IE_CORE_DISPLAYDRIVERSERVERHEADER

#include "IECore/DisplayDriverServer.h"

namespace IECore
{

/* Header block used by back and forth messages with the server.
* 7 bytes long:
* [0] - magic number ( 0x82 )
* [1] - protocol version ( 1 )
* [2] - message type ( imageOpen, imageData, imageClose )
* [3-6] - length of following data block.
*/
class DisplayDriverServerHeader
{
	public:

		enum MessageType { imageOpen = 1, imageData = 2, imageClose = 3, exception = 4 };

		static const unsigned char headerLength = 7;
		static const unsigned char magicNumber = 0x82;
		static const unsigned char currentProtocolVersion = 2;

		DisplayDriverServerHeader();
		DisplayDriverServerHeader( MessageType msg, size_t dataSize );

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

} // namespace IECore

#endif // IE_CORE_DISPLAYDRIVERSERVERHEADER
