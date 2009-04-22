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

#include "boost/bind.hpp"

#include "IECore/ClientDisplayDriver.h"
#include "IECore/DisplayDriverServer.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MemoryIndexedIO.h"

using namespace boost;
using namespace std;
using namespace Imath;
using namespace IECore;
using boost::asio::ip::tcp;

IE_CORE_DEFINERUNTIMETYPED( ClientDisplayDriver );

ClientDisplayDriver::ClientDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters ) : 
		DisplayDriver( displayWindow, dataWindow, channelNames, parameters ),
		m_service(), m_host(""), m_port(""), m_scanLineOrderOnly(false), m_socket( m_service )
{
	// expects two custom StringData parameters: displayHost and displayPort
	CompoundDataMap::const_iterator it = parameters->readable().find("displayHost");
	if ( it == parameters->readable().end() )
	{
		// for backward compatibility...
		it = parameters->readable().find("host");
	}
	if ( it == parameters->readable().end() )
	{
		throw Exception( "Could not find 'host' parameter!" );
	}
	DataPtr data = it->second;
	if ( data->typeId() != StringDataTypeId )
	{
		throw Exception( "Invalid 'host' type parameter. Should be StringData." );
	}
	m_host = static_pointer_cast<const StringData>(data)->readable();
	it = parameters->readable().find("displayPort");
	if ( it == parameters->readable().end() )
	{
		// for backward compatibility...
		it = parameters->readable().find("port");
	}	
	if ( it == parameters->readable().end() )
	{
		throw Exception( "Could not find 'port' parameter!" );
	}
	data = it->second;
	if ( data->typeId() != StringDataTypeId )
	{
		throw Exception( "Invalid 'port' parameter. Should be a StringData." );
	}
	m_port = static_pointer_cast<const StringData>(data)->readable();

	tcp::resolver resolver(m_service);
	tcp::resolver::query query(m_host, m_port);
	tcp::resolver::iterator iterator = resolver.resolve(query);

	try
	{
		m_socket.connect( *iterator );
	}
	catch( std::exception &e )
	{
		throw Exception( std::string("Could not connect to remote display driver server: ") + e.what() );
	}


	MemoryIndexedIOPtr io;
	ConstCharVectorDataPtr buf;
	Box2iDataPtr displayWindowData = new Box2iData( displayWindow );
	Box2iDataPtr dataWindowData = new Box2iData( dataWindow );
	StringVectorDataPtr channelNamesData = new StringVectorData( channelNames );

	// build the data block
	io = new MemoryIndexedIO( ConstCharVectorDataPtr(), "/", IndexedIO::Exclusive | IndexedIO::Write );
	displayWindowData->Object::save( io, "displayWindow" );
	dataWindowData->Object::save( io, "dataWindow" );
	channelNamesData->Object::save( io, "channelNames" );
	parameters->Object::save( io, "parameters" );
	buf = io->buffer();

	size_t dataSize = buf->readable().size();

	sendHeader( DisplayDriverServer::imageOpen, dataSize );

	m_socket.send( boost::asio::buffer( &(buf->readable()[0]), dataSize ) );

	if ( receiveHeader( DisplayDriverServer::imageOpen ) != sizeof(m_scanLineOrderOnly) )
	{
		throw Exception( "Invalid returned scanLineOrder from display driver server!" );
	}
	m_socket.receive( boost::asio::buffer( &m_scanLineOrderOnly, sizeof(m_scanLineOrderOnly) ) );

}

ClientDisplayDriver::~ClientDisplayDriver()
{
	m_socket.close();
}

std::string ClientDisplayDriver::host() const
{
	return m_host;
}

std::string ClientDisplayDriver::port() const
{
	return m_port;
}

bool ClientDisplayDriver::scanLineOrderOnly() const
{
	return m_scanLineOrderOnly;
}

void ClientDisplayDriver::sendHeader( DisplayDriverServer::MessageType msg, size_t dataSize )
{
	DisplayDriverServer::Header header( msg, dataSize );
	m_socket.send( boost::asio::buffer( header.buffer(), header.headerLength ) );
}

size_t ClientDisplayDriver::receiveHeader( DisplayDriverServer::MessageType msg )
{
	DisplayDriverServer::Header header;
	m_socket.receive( boost::asio::buffer( header.buffer(), header.headerLength ) );
	if ( !header.valid() )
	{
		throw Exception( "Invalid display driver header block on socket package." );
	}
	size_t bytesAhead = header.getDataSize();
	
	if ( header.messageType() == DisplayDriverServer::exception )
	{
		vector<char> txt;
		txt.resize( bytesAhead );
		m_socket.receive( boost::asio::buffer( &(txt[0]), bytesAhead ) );
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

	io = new MemoryIndexedIO( ConstCharVectorDataPtr(), "/", IndexedIO::Exclusive | IndexedIO::Write );
	static_pointer_cast<Object>(boxData)->save( io, "box" );
	static_pointer_cast<Object>(dataData)->save( io, "data" );
	buf = io->buffer();
	size_t blockSize = buf->readable().size();

	sendHeader( DisplayDriverServer::imageData, blockSize );

	m_socket.send( boost::asio::buffer( &(buf->readable()[0]), blockSize) );
}

void ClientDisplayDriver::imageClose()
{
	sendHeader( DisplayDriverServer::imageClose, 0 );
	receiveHeader( DisplayDriverServer::imageClose );
	m_socket.close();
}

