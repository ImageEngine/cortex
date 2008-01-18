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

#include "IECore/MessageHandler.h"
#include "IECore/OStreamMessageHandler.h"

#include "boost/algorithm/string/case_conv.hpp"

#include <iostream>

using namespace std;
using namespace IECore;

///////////////////////////////////////////////////////////////////////////////////////
// output functions
///////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::output( Level level, const std::string &context, const std::string &message )
{
	currentHandler()->handle( level, context, message );
}

void MessageHandler::output( Level level, const std::string &context, const boost::format &message )
{
	string m = message.str();
	output( level, context, m );
}

///////////////////////////////////////////////////////////////////////////////////////
// handler stack functions
///////////////////////////////////////////////////////////////////////////////////////

void MessageHandler::pushHandler( MessageHandlerPtr handler )
{
	handlerStack()->push( handler );
}

MessageHandlerPtr MessageHandler::popHandler()
{
	MessageHandlerPtr h = currentHandler();
	handlerStack()->pop();
	return h;
}

MessageHandlerPtr MessageHandler::currentHandler()
{
	return handlerStack()->top();
}

std::stack<MessageHandlerPtr> *MessageHandler::handlerStack()
{
	static std::stack<MessageHandlerPtr> *s = new std::stack<MessageHandlerPtr>;
	if( !s->size() )
	{
		s->push( OStreamMessageHandler::cErrHandler() );
	}
	return s;
}

///////////////////////////////////////////////////////////////////////////////////////
// conversions between level and string
///////////////////////////////////////////////////////////////////////////////////////

std::string MessageHandler::levelAsString( Level level )
{
	switch( level )
	{
		case Error :
			return "ERROR";
		case Warning :
			return "WARNING";
		case Info :
			return "INFO";
		case Debug :
			return "DEBUG";
		default :
			return "INVALID";
	}
}

MessageHandler::Level MessageHandler::stringAsLevel( const std::string &level )
{
	string l = level;
	boost::to_lower( l );
	if( l=="error" )
	{
		return Error;
	}
	else if( l=="warning" )
	{
		return Warning;
	}
	else if( l=="info" )
	{
		return Info;
	}
	else if( l=="debug" )
	{
		return Debug;
	}
	return Invalid;
}

///////////////////////////////////////////////////////////////////////////////////////
// message output shortcuts
///////////////////////////////////////////////////////////////////////////////////////

void IECore::msg( MessageHandler::Level level, const std::string &context, const std::string &message )
{
	MessageHandler::output( level, context, message );
}

void IECore::msg( MessageHandler::Level level, const std::string &context, const boost::format &message )
{
	MessageHandler::output( level, context, message );
}
