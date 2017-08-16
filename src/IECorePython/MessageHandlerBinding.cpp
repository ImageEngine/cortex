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

#include "boost/python.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/NullMessageHandler.h"
#include "IECore/OStreamMessageHandler.h"
#include "IECore/CompoundMessageHandler.h"
#include "IECore/FilteredMessageHandler.h"
#include "IECore/LevelFilteredMessageHandler.h"
#include "IECorePython/MessageHandlerBinding.h"
#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/ExceptionAlgo.h"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;

namespace
{

class MessageHandlerWrapper : public RefCountedWrapper<MessageHandler>
{
	public :

		MessageHandlerWrapper( PyObject *self ) : RefCountedWrapper<MessageHandler>( self )
		{
		}

		virtual void handle( MessageHandler::Level level, const std::string &context, const std::string &message )
		{
			ScopedGILLock gilLock;
			try
			{
				this->methodOverride( "handle" )( level, context, message );
			}
			catch( const error_already_set &e )
			{
				IECorePython::ExceptionAlgo::translatePythonException();
			}
		}

};

void addHandler( CompoundMessageHandler &h, MessageHandlerPtr hh )
{
	h.handlers.insert( hh );
}

void removeHandler( CompoundMessageHandler &h, MessageHandlerPtr hh )
{
	h.handlers.erase( hh );
}

LevelFilteredMessageHandlerPtr levelFilteredMessageHandlerConstructor(MessageHandlerPtr handle, MessageHandler::Level level)
{
	return new LevelFilteredMessageHandler( handle, level );
}

} // namespace

void IECorePython::bindMessageHandler()
{

	def( "msg", (void (*)( MessageHandler::Level, const std::string &, const std::string &))&msg );

	object mh = RefCountedClass<MessageHandler, RefCounted, MessageHandlerWrapper>( "MessageHandler" )
		.def( init<>() )
		.def( "handle", pure_virtual( &MessageHandler::handle ) )
		.def( "setDefaultHandler", &MessageHandler::setDefaultHandler )
		.staticmethod( "setDefaultHandler" )
		.def( "getDefaultHandler", &MessageHandler::getDefaultHandler, return_value_policy<CastToIntrusivePtr>() )
		.staticmethod( "getDefaultHandler" )
		.def( "currentHandler", &MessageHandler::currentHandler, return_value_policy<CastToIntrusivePtr>() )
		.staticmethod( "currentHandler" )
		.def( "output", (void (*)( MessageHandler::Level, const std::string &, const std::string &))&MessageHandler::output )
		.staticmethod( "output" )
		.def( "levelAsString", MessageHandler::levelAsString )
		.staticmethod( "levelAsString" )
		.def( "stringAsLevel", MessageHandler::stringAsLevel )
		.staticmethod( "stringAsLevel" )
	;

	RefCountedClass<NullMessageHandler, MessageHandler>( "NullMessageHandler" )
	.	def( init<>() )
	;

	RefCountedClass<OStreamMessageHandler, MessageHandler>( "OStreamMessageHandler" )
		.def( "cErrHandler", &OStreamMessageHandler::cErrHandler, return_value_policy<CastToIntrusivePtr>() )
		.staticmethod( "cErrHandler" )
		.def( "cOutHandler", &OStreamMessageHandler::cOutHandler, return_value_policy<CastToIntrusivePtr>() )
		.staticmethod( "cOutHandler" )
	;

	RefCountedClass<CompoundMessageHandler, MessageHandler>( "CompoundMessageHandler" )
		.def( init<>() )
		.def( "addHandler", &addHandler )
		.def( "removeHandler", &removeHandler )
	;

	RefCountedClass<FilteredMessageHandler, MessageHandler>( "FilteredMessageHandler" )
	;

	RefCountedClass<LevelFilteredMessageHandler, FilteredMessageHandler>( "LevelFilteredMessageHandler" )
		.def( "__init__", make_constructor( &levelFilteredMessageHandlerConstructor ) )
		.def( "setLevel", &LevelFilteredMessageHandler::setLevel )
		.def( "getLevel", &LevelFilteredMessageHandler::getLevel )
		.def( "defaultLevel", &LevelFilteredMessageHandler::defaultLevel ).staticmethod( "defaultLevel" )
	;

	scope mhS( mh );

	enum_<MessageHandler::Level>( "Level" )
		.value( "Error", MessageHandler::Error )
		.value( "Warning", MessageHandler::Warning )
		.value( "Info", MessageHandler::Info )
		.value( "Debug", MessageHandler::Debug )
		.value( "Invalid", MessageHandler::Invalid )
	;

	class_<MessageHandler::Scope, boost::noncopyable>( "_Scope", init<MessageHandler *>() )
	;

}
