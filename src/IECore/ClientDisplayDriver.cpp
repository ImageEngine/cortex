//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "boost/asio.hpp"
#include "boost/bind.hpp"

#include "IECore/ClientDisplayDriver.h"
#include "IECore/private/DisplayDriverServerHeader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MemoryIndexedIO.h"

using namespace boost;
using namespace std;
using namespace Imath;
using namespace IECore;
using boost::asio::ip::tcp;

class ClientDisplayDriver::PrivateData : public RefCounted
{
	public :
		PrivateData() :
		m_service(), m_host(""), m_port(""), m_scanLineOrderOnly(false), m_acceptsRepeatedData(false), m_socket( m_service )
		{
		}

		~PrivateData()
		{
			m_socket.close();
		}

		boost::asio::io_service m_service;
		std::string m_host;
		std::string m_port;
		bool m_scanLineOrderOnly;
		bool m_acceptsRepeatedData;
		boost::asio::ip::tcp::socket m_socket;
};

IE_CORE_DEFINERUNTIMETYPED( ClientDisplayDriver );

const DisplayDriver::DisplayDriverDescription<ClientDisplayDriver> ClientDisplayDriver::g_description;

ClientDisplayDriver::ClientDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters ) :
		DisplayDriver( displayWindow, dataWindow, channelNames, parameters ),
		m_data( new PrivateData() )
{
	// expects three custom StringData parameters : displayHost, displayPort and displayType
	const StringData *displayHostData = parameters->member<StringData>( "displayHost", true /* throw if missing */ );
	const StringData *displayPortData = parameters->member<StringData>( "displayPort", true /* throw if missing */ );
	
	m_data->m_host = displayHostData->readable();
	m_data->m_port = displayPortData->readable();
	
	tcp::resolver resolver(m_data->m_service);
	tcp::resolver::query query(m_data->m_host, m_data->m_port);

	boost::system::error_code error;
	tcp::resolver::iterator iterator = resolver.resolve( query, error );	
	if( !error )
	{
		error = boost::asio::error::host_not_found;
		while( error && iterator != tcp::resolver::iterator() )
		{
			m_data->m_socket.close();
			m_data->m_socket.connect( *iterator++, error );
		}
	}
	if( error )
	{
		throw Exception( std::string( "Could not connect to remote display driver server : " ) + error.message() );
	}

	MemoryIndexedIOPtr io;
	ConstCharVectorDataPtr buf;
	Box2iDataPtr displayWindowData = new Box2iData( displayWindow );
	Box2iDataPtr dataWindowData = new Box2iData( dataWindow );
	StringVectorDataPtr channelNamesData = new StringVectorData( channelNames );

	IECore::CompoundDataPtr tmpParameters = parameters->copy();
	tmpParameters->writable()[ "clientPID" ] = new IntData( getpid() );

	// build the data block
	io = new MemoryIndexedIO( ConstCharVectorDataPtr(), IndexedIO::rootPath, IndexedIO::Exclusive | IndexedIO::Write );
	displayWindowData->Object::save( io, "displayWindow" );
	dataWindowData->Object::save( io, "dataWindow" );
	channelNamesData->Object::save( io, "channelNames" );
	tmpParameters->Object::save( io, "parameters" );
	buf = io->buffer();

	size_t dataSize = buf->readable().size();

	sendHeader( DisplayDriverServerHeader::imageOpen, dataSize );

	m_data->m_socket.send( boost::asio::buffer( &(buf->readable()[0]), dataSize ) );

	if ( receiveHeader( DisplayDriverServerHeader::imageOpen ) != sizeof(m_data->m_scanLineOrderOnly) )
	{
		throw Exception( "Invalid returned scanLineOrder from display driver server!" );
	}
	m_data->m_socket.receive( boost::asio::buffer( &m_data->m_scanLineOrderOnly, sizeof(m_data->m_scanLineOrderOnly) ) );
	
	if ( receiveHeader( DisplayDriverServerHeader::imageOpen ) != sizeof(m_data->m_acceptsRepeatedData) )
	{
		throw Exception( "Invalid returned acceptsRepeatedData from display driver server!" );
	}
	m_data->m_socket.receive( boost::asio::buffer( &m_data->m_acceptsRepeatedData, sizeof(m_data->m_acceptsRepeatedData) ) );
}

ClientDisplayDriver::~ClientDisplayDriver()
{
}

std::string ClientDisplayDriver::host() const
{
	return m_data->m_host;
}

std::string ClientDisplayDriver::port() const
{
	return m_data->m_port;
}

bool ClientDisplayDriver::scanLineOrderOnly() const
{
	return m_data->m_scanLineOrderOnly;
}

bool ClientDisplayDriver::acceptsRepeatedData() const
{
	return m_data->m_acceptsRepeatedData;
}

void ClientDisplayDriver::sendHeader( int msg, size_t dataSize )
{
	DisplayDriverServerHeader header( (DisplayDriverServerHeader::MessageType)msg, dataSize );
	m_data->m_socket.send( boost::asio::buffer( header.buffer(), header.headerLength ) );
}

size_t ClientDisplayDriver::receiveHeader( int msg )
{
	DisplayDriverServerHeader header;
	m_data->m_socket.receive( boost::asio::buffer( header.buffer(), header.headerLength ) );
	if ( !header.valid() )
	{
		throw Exception( "Invalid display driver header block on socket package." );
	}
	size_t bytesAhead = header.getDataSize();

	if ( header.messageType() == DisplayDriverServerHeader::exception )
	{
		vector<char> txt;
		txt.resize( bytesAhead );
		m_data->m_socket.receive( boost::asio::buffer( &(txt[0]), bytesAhead ) );
		throw Exception( std::string("Error on remote display driver: ") + &(txt[0]) );
	}
	if ( header.messageType() != msg )
	{
		throw Exception( "Unexpected message type on display driver socket package." );
	}
	return bytesAhead;
}

void ClientDisplayDriver::imageData( const Box2i &box, const float *data, size_t dataSize )
{
	MemoryIndexedIOPtr io;
	ConstCharVectorDataPtr buf;

	// build the data block
	Box2iDataPtr boxData = new Box2iData( box );
	FloatVectorDataPtr dataData = new FloatVectorData( std::vector<float>( data, data+dataSize ) );

	io = new MemoryIndexedIO( ConstCharVectorDataPtr(), IndexedIO::rootPath, IndexedIO::Exclusive | IndexedIO::Write );
	boost::static_pointer_cast<Object>(boxData)->save( io, "box" );
	boost::static_pointer_cast<Object>(dataData)->save( io, "data" );
	buf = io->buffer();
	size_t blockSize = buf->readable().size();

	sendHeader( DisplayDriverServerHeader::imageData, blockSize );

	m_data->m_socket.send( boost::asio::buffer( &(buf->readable()[0]), blockSize) );
}

void ClientDisplayDriver::imageClose()
{
	sendHeader( DisplayDriverServerHeader::imageClose, 0 );
	receiveHeader( DisplayDriverServerHeader::imageClose );
	m_data->m_socket.close();
}

