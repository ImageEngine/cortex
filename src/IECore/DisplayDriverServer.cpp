//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include <unistd.h>
#include <fcntl.h>

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "tbb/tbb_thread.h"

#include "IECore/DisplayDriverServer.h"
#include "IECore/private/DisplayDriverServerHeader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MemoryIndexedIO.h"
#include "IECore/MessageHandler.h"

using namespace IECore;
using boost::asio::ip::tcp;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( DisplayDriverServer );

class DisplayDriverServer::Session : public RefCounted
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
		void sendResult( DisplayDriverServerHeader::MessageType msg, size_t dataSize );
		void sendException( const char *message );

	private:
		boost::asio::ip::tcp::socket m_socket;
		DisplayDriverPtr m_displayDriver;
		DisplayDriverServerHeader m_header;
		CharVectorDataPtr m_buffer;
};

class DisplayDriverServer::PrivateData : public RefCounted
{
	
	public :
	
		bool m_success;
		boost::asio::ip::tcp::endpoint m_endpoint;
		boost::asio::io_service m_service;
		boost::asio::ip::tcp::acceptor m_acceptor;
		tbb::tbb_thread m_thread;

		PrivateData( int portNumber ) :
			m_success(false),
			m_endpoint(tcp::v4(), portNumber),
			m_service(),
			m_acceptor( m_service ),
			m_thread()
		{
			m_acceptor.open(  m_endpoint.protocol() );
			m_acceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address(true));
			m_acceptor.bind( m_endpoint );
			m_acceptor.listen();
			m_success = true;
		}

		~PrivateData()
		{
			if ( m_success )
			{
				m_acceptor.cancel();
				m_acceptor.close();
				m_thread.join();
			}
		}

};

/* Set the FD_CLOEXEC flag for the given socket descriptor, so that it will not exist on child processes.*/
static void fixSocketFlags( int socketDesc )
{
	int oldflags = fcntl (socketDesc, F_GETFD, 0);
	if ( oldflags >= 0 )
	{
		fcntl( socketDesc, F_SETFD, oldflags | FD_CLOEXEC );
	}
}

DisplayDriverServer::DisplayDriverServer( int portNumber ) :
		m_data( 0 )
{
	m_data = new DisplayDriverServer::PrivateData( portNumber );

	DisplayDriverServer::SessionPtr newSession( new DisplayDriverServer::Session( m_data->m_service ) );
	m_data->m_acceptor.async_accept( newSession->socket(),
			boost::bind( &DisplayDriverServer::handleAccept, this, newSession,
			boost::asio::placeholders::error));
	fixSocketFlags( m_data->m_acceptor.native() );
	tbb::tbb_thread newThread( boost::bind(&DisplayDriverServer::serverThread, this) );
	m_data->m_thread = newThread;
}

DisplayDriverServer::~DisplayDriverServer()
{
}

void DisplayDriverServer::serverThread()
{
	try
	{
		m_data->m_service.run();
	}
	catch( std::exception &e )
	{
		msg( Msg::Error, "DisplayDriverServer::serverThread", e.what() );
	}
}

void DisplayDriverServer::handleAccept( DisplayDriverServer::SessionPtr session, const boost::system::error_code& error)
{
	if (!error)
	{
		DisplayDriverServer::SessionPtr newSession( new DisplayDriverServer::Session( m_data->m_service ) );
		m_data->m_acceptor.async_accept( newSession->socket(),
				boost::bind( &DisplayDriverServer::handleAccept,  this, newSession,
				boost::asio::placeholders::error));
		session->start();
	}
}

/* 
 * DisplayDriverServer::Session functions 
 */

DisplayDriverServer::Session::Session( boost::asio::io_service& io_service ) :
	m_socket( io_service ), m_displayDriver(0), m_buffer( new CharVectorData( ) )
{
}

DisplayDriverServer::Session::~Session()
{
	m_socket.close();
}

boost::asio::ip::tcp::socket& DisplayDriverServer::Session::socket()
{
	return m_socket;
}

void DisplayDriverServer::Session::start()
{
	boost::asio::async_read( m_socket,
			boost::asio::buffer( m_header.buffer(), m_header.headerLength),
			boost::bind(
				&DisplayDriverServer::Session::handleReadHeader, SessionPtr(this),
				boost::asio::placeholders::error
			)
	);
	fixSocketFlags( m_socket.native() );
}

void DisplayDriverServer::Session::handleReadHeader( const boost::system::error_code& error )
{
	if (error)
	{
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadHeader", error.message().c_str() );
		m_socket.close();
		return;
	}

	if ( !m_header.valid() )
	{
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadHeader", "Invalid header!" );
		m_socket.close();
		return;
	}

	// get number of bytes ahead (unsigned int value)
	size_t bytesAhead = m_header.getDataSize();

	CharVectorData::ValueType &data = m_buffer->writable();
	data.resize( bytesAhead );

	// service
	switch( m_header.messageType() )
	{
	case DisplayDriverServerHeader::imageOpen:
		boost::asio::async_read( m_socket,
				boost::asio::buffer( &data[0], bytesAhead ),
				boost::bind( &DisplayDriverServer::Session::handleReadOpenParameters, SessionPtr(this), boost::asio::placeholders::error)
		);
		break;

	case DisplayDriverServerHeader::imageData:
		boost::asio::async_read( m_socket,
				boost::asio::buffer( &data[0], bytesAhead ),
				boost::bind(&DisplayDriverServer::Session::handleReadDataParameters, SessionPtr(this),
				boost::asio::placeholders::error));
		break;

	case DisplayDriverServerHeader::imageClose:
		if ( m_displayDriver )
		{
			try
			{
				m_displayDriver->imageClose();
			}
			catch ( std::exception &e )
			{
				msg( Msg::Error, "DisplayDriverServer::Session::handleReadHeader", e.what() );
				sendException( e.what() );
				m_socket.close();
				return;
			}
			try
			{
				sendResult( DisplayDriverServerHeader::imageClose, 0 );
			}
			catch( std::exception &e )
			{
				msg( Msg::Error, "DisplayDriverServer::Session::handleReadHeader", e.what() );
			}
			m_socket.close();
		}
		else
		{
			msg( Msg::Error, "DisplayDriverServer::Session::handleReadHeader", "No DisplayDriver to close." );
			m_socket.close();
		}
		break;

	default:
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadHeader", "Unrecognized message type." );
		m_socket.close();
		break;
	}
}

void DisplayDriverServer::Session::handleReadOpenParameters( const boost::system::error_code& error )
{
	if (error)
	{
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadOpenParameters", error.message().c_str() );
		m_socket.close();
		return;
	}

	StringDataPtr displayDriverType;
	Box2iDataPtr displayWindow, dataWindow;
	StringVectorDataPtr channelNames;
	CompoundDataPtr parameters;
	bool scanLineOrder = false;
	bool acceptsRepeatedData = false;

	// handle imageOpen parameters.
	try
	{
		MemoryIndexedIOPtr io = new MemoryIndexedIO( m_buffer, IndexedIO::rootPath, IndexedIO::Exclusive | IndexedIO::Read );
		displayWindow = staticPointerCast<Box2iData>( Object::load( io, "displayWindow" ) );
		dataWindow = staticPointerCast<Box2iData>( Object::load( io, "dataWindow" ) );
		channelNames = staticPointerCast<StringVectorData>( Object::load( io, "channelNames" ) );
		parameters = staticPointerCast<CompoundData>( Object::load( io, "parameters" ) );

		const StringData *displayType = parameters->member<StringData>( "remoteDisplayType", true /* throw if missing */ );

		// create a displayDriver using the factory function.
		m_displayDriver = DisplayDriver::create( displayType->readable(), displayWindow->readable(), dataWindow->readable(), channelNames->readable(), parameters );

		scanLineOrder = m_displayDriver->scanLineOrderOnly();
		acceptsRepeatedData = m_displayDriver->acceptsRepeatedData();
	}
	catch( std::exception &e )
	{
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadOpenParameters", e.what() );
		sendException( e.what() );
		m_socket.close();
		return;
	}

	try
	{
		// send the result back.
		sendResult( DisplayDriverServerHeader::imageOpen, sizeof(scanLineOrder) );
		m_socket.send( boost::asio::buffer( &scanLineOrder, sizeof(scanLineOrder) ) );
		
		sendResult( DisplayDriverServerHeader::imageOpen, sizeof(acceptsRepeatedData) );
		m_socket.send( boost::asio::buffer( &acceptsRepeatedData, sizeof(acceptsRepeatedData) ) );

		// prepare for getting imageData packages
		boost::asio::async_read( m_socket,
			boost::asio::buffer( m_header.buffer(), m_header.headerLength),
			boost::bind(
				&DisplayDriverServer::Session::handleReadHeader, SessionPtr(this),
				boost::asio::placeholders::error
			)
		);
	}
	catch( std::exception &e )
	{
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadOpenParameters", e.what() );
		m_socket.close();
	}

}

void DisplayDriverServer::Session::handleReadDataParameters( const boost::system::error_code& error )
{
	if (error)
	{
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadDataParameters", error.message().c_str() );
		m_socket.close();
		return;
	}

	// sanity check: check DisplayDriver object
	if (! m_displayDriver )
	{
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadDataParameters", "No display drivers!" );
		m_socket.close();
		return;
	}

	// get imageData parameters
	Box2iDataPtr box;
	FloatVectorDataPtr data;

	try
	{
		MemoryIndexedIOPtr io = new MemoryIndexedIO( m_buffer, IndexedIO::rootPath, IndexedIO::Exclusive | IndexedIO::Read );
		box = staticPointerCast<Box2iData>( Object::load( io, "box" ) );
		data = staticPointerCast<FloatVectorData>( Object::load( io, "data" ) );

		// call imageData passing the data
		m_displayDriver->imageData( box->readable(), &(data->readable()[0]), data->readable().size() );

		// prepare for getting more imageData packages or a imageClose.
		boost::asio::async_read( m_socket,
			boost::asio::buffer( m_header.buffer(), m_header.headerLength),
			boost::bind(
				&DisplayDriverServer::Session::handleReadHeader, SessionPtr(this),
				boost::asio::placeholders::error
			)
		);
	}
	catch( std::exception &e )
	{
		msg( Msg::Error, "DisplayDriverServer::Session::handleReadDataParameters", e.what() );
		m_socket.close();
		return;
	}
}

void DisplayDriverServer::Session::sendResult( DisplayDriverServerHeader::MessageType msg, size_t dataSize )
{
	DisplayDriverServerHeader header( msg, dataSize );
	m_socket.send( boost::asio::buffer( header.buffer(), header.headerLength ) );
}

void DisplayDriverServer::Session::sendException( const char *message )
{
	size_t msgLen = strlen( message ) + 1;
	sendResult( DisplayDriverServerHeader::exception, msgLen );
	m_socket.send( boost::asio::buffer( message, msgLen ) );
}
