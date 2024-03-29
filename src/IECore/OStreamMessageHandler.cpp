//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/OStreamMessageHandler.h"

#include <iostream>

using namespace std;
using namespace IECore;

///////////////////////////////////////////////////////////////////////////////////////
// structors
///////////////////////////////////////////////////////////////////////////////////////

OStreamMessageHandler::OStreamMessageHandler( std::ostream &stream )
	:	m_stream( &stream ), m_ownStream( false )
{
}

OStreamMessageHandler::OStreamMessageHandler( std::ostream *stream )
	:	m_stream( stream ), m_ownStream( true )
{
}

OStreamMessageHandler::~OStreamMessageHandler()
{
	if( m_ownStream )
	{
		delete m_stream;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// handler function
///////////////////////////////////////////////////////////////////////////////////////

void OStreamMessageHandler::handle( Level level, const std::string &context, const std::string &message )
{
	const string levelString = levelAsString( level );
	// Output the message a line at a time.
	for( size_t lineBegin = 0; lineBegin < message.size(); )
	{
		// Find span to the next newline.
		const size_t f = message.find( '\n', lineBegin );
		const size_t lineEnd = f == string::npos ? message.size() : f;
		// Prefix every line with the level
		*m_stream << levelString << " : ";
		// Only prefix the first line with the context
		if( lineBegin == 0 )
		{
			*m_stream << context << " : ";
		}
		// Output line and set up for next one.
		*m_stream << std::string_view( message.data() + lineBegin, lineEnd - lineBegin ) << endl;
		lineBegin = lineEnd + 1;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// shared fellahs
///////////////////////////////////////////////////////////////////////////////////////

OStreamMessageHandler *OStreamMessageHandler::cErrHandler()
{
	static OStreamMessageHandlerPtr s = new OStreamMessageHandler( cerr );
	return s.get();
}

OStreamMessageHandler *OStreamMessageHandler::cOutHandler()
{
	static OStreamMessageHandlerPtr s = new OStreamMessageHandler( cout );
	return s.get();
}
