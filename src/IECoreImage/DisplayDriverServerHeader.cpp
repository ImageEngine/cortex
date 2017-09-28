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

#include "IECoreImage/Private/DisplayDriverServerHeader.h"

using namespace IECore;
using namespace IECoreImage;

enum byteOrder {
	orderMagicNumber = 0,
	orderProtocolVersion,
	orderMessageType,
	orderDataSize1,
	orderDataSize2,
	orderDataSize3,
	orderDataSize4
};

DisplayDriverServerHeader::DisplayDriverServerHeader()
{
	memset( &m_header[0], 0, sizeof(m_header) );
}

DisplayDriverServerHeader::DisplayDriverServerHeader( MessageType msg, size_t dataSize )
{
	m_header[orderMagicNumber] = magicNumber;
	m_header[orderProtocolVersion] = currentProtocolVersion;
	m_header[orderMessageType] = msg;
	setDataSize( dataSize );
}

unsigned char *DisplayDriverServerHeader::buffer()
{
	return &m_header[0];
}

bool DisplayDriverServerHeader::valid()
{
	if ( m_header[orderMagicNumber] != magicNumber ||
		 m_header[orderProtocolVersion] != currentProtocolVersion ||
		( m_header[orderMessageType] != imageOpen &&
			m_header[orderMessageType] != imageData &&
			m_header[orderMessageType] != imageClose &&
			m_header[orderMessageType] != exception ) )
	{
		return false;
	}
	return true;
}

size_t DisplayDriverServerHeader::getDataSize()
{
	return (unsigned int)m_header[orderDataSize1] | ((unsigned int)m_header[orderDataSize2] << 8) |
				((unsigned int)m_header[orderDataSize3] << 16) | ((unsigned int)m_header[orderDataSize4] << 24);
}

void DisplayDriverServerHeader::setDataSize( size_t dataSize )
{
	m_header[orderDataSize1] = dataSize & 0xff;
	m_header[orderDataSize2] = ( dataSize >> 8 ) & 0xff;
	m_header[orderDataSize3] = ( dataSize >> 16 ) & 0xff;
	m_header[orderDataSize4] = ( dataSize >> 24 ) & 0xff;
}

DisplayDriverServerHeader::MessageType DisplayDriverServerHeader::messageType()
{
	return (MessageType)m_header[2];
}
