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

#include <boost/python.hpp>

#include "IECore/MessageHandler.h"
#include "IECore/NullMessageHandler.h"
#include "IECore/OStreamMessageHandler.h"
#include "IECore/CompoundMessageHandler.h"
#include "IECore/FilteredMessageHandler.h"
#include "IECore/LevelFilteredMessageHandler.h"
#include "IECore/bindings/MessageHandlerBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/Wrapper.h"
#include "IECore/bindings/WrapperToPython.h"

using namespace boost::python;

namespace IECore
{

class MessageHandlerWrap : public MessageHandler, public Wrapper<MessageHandler>
{
	public :

		MessageHandlerWrap( PyObject *self ) : Wrapper<MessageHandler>( self, this ) {}

		virtual void handle( MessageHandler::Level level, const std::string &context, const std::string &message ) 
		{
			this->get_override( "handle" )( level, context, message ); 
		}

};
IE_CORE_DECLAREPTR( MessageHandlerWrap );


static void addHandler( CompoundMessageHandler &h, MessageHandlerPtr hh )
{
	h.handlers.insert( hh );
}

static void removeHandler( CompoundMessageHandler &h, MessageHandlerPtr hh )
{
	h.handlers.erase( hh );
}

static LevelFilteredMessageHandlerPtr levelFilteredMessageHandlerConstructor(MessageHandlerPtr handle, MessageHandler::Level level)
{
	return new LevelFilteredMessageHandler( handle, level );
}

void bindMessageHandler()
{
	
	def( "msg", (void (*)( MessageHandler::Level, const std::string &, const std::string &))&msg );
	
	typedef class_<MessageHandler, boost::noncopyable, MessageHandlerWrapPtr> MessageHandlerPyClass;
	object mh = MessageHandlerPyClass( "MessageHandler", no_init )
		.def( init<>() )
		.def( "handle", pure_virtual( &MessageHandler::handle ) )
		.def( "pushHandler", &MessageHandler::pushHandler )
		.staticmethod( "pushHandler" )
		.def( "popHandler", &MessageHandler::popHandler )
		.staticmethod( "popHandler" )
		.def( "output", (void (*)( MessageHandler::Level, const std::string &, const std::string &))&MessageHandler::output )
		.staticmethod( "output" )
	;
	
	WrapperToPython<MessageHandlerPtr>();

	INTRUSIVE_PTR_PATCH( MessageHandler, MessageHandlerPyClass );

	def( "Msg", mh );
	
	typedef class_<NullMessageHandler, boost::noncopyable, NullMessageHandlerPtr, bases<MessageHandler> > NullMessageHandlerPyClass;
	NullMessageHandlerPyClass( "NullMessageHandler" )
	;
	INTRUSIVE_PTR_PATCH( NullMessageHandler, NullMessageHandlerPyClass );
	implicitly_convertible<NullMessageHandlerPtr, MessageHandlerPtr>();
	
	typedef class_<OStreamMessageHandler, boost::noncopyable, OStreamMessageHandlerPtr, bases<MessageHandler> > OStreamMessageHandlerPyClass;
	OStreamMessageHandlerPyClass( "OStreamMessageHandler", no_init )
		.def( "cErrHandler", &OStreamMessageHandler::cErrHandler )
		.staticmethod( "cErrHandler" )
		.def( "cOutHandler", &OStreamMessageHandler::cOutHandler )
		.staticmethod( "cOutHandler" )
	;
	INTRUSIVE_PTR_PATCH( OStreamMessageHandler, OStreamMessageHandlerPyClass );
	implicitly_convertible<OStreamMessageHandlerPtr, MessageHandlerPtr>();
	
	typedef class_<CompoundMessageHandler, boost::noncopyable, CompoundMessageHandlerPtr, bases<MessageHandler> > CompoundMessageHandlerPyClass;
	CompoundMessageHandlerPyClass( "CompoundMessageHandler" )
		.def( "addHandler", &addHandler )
		.def( "removeHandler", &removeHandler )
	;
	INTRUSIVE_PTR_PATCH( CompoundMessageHandler, CompoundMessageHandlerPyClass );
	implicitly_convertible<CompoundMessageHandlerPtr, MessageHandlerPtr>();
	
	typedef class_<FilteredMessageHandler, boost::noncopyable, FilteredMessageHandlerPtr, bases<MessageHandler> > FilteredMessageHandlerPyClass;
	FilteredMessageHandlerPyClass( "FilteredMessageHandler", no_init )
	;
	INTRUSIVE_PTR_PATCH( FilteredMessageHandler, FilteredMessageHandlerPyClass );
	implicitly_convertible<FilteredMessageHandlerPtr, MessageHandlerPtr>();

	typedef class_<LevelFilteredMessageHandler, boost::noncopyable, LevelFilteredMessageHandlerPtr, bases<FilteredMessageHandler> > LevelFilteredMessageHandlerPyClass;
	LevelFilteredMessageHandlerPyClass( "LevelFilteredMessageHandler", no_init )
		.def( "__init__", make_constructor( &levelFilteredMessageHandlerConstructor ) )
	;
	INTRUSIVE_PTR_PATCH( LevelFilteredMessageHandler, LevelFilteredMessageHandlerPyClass );
	implicitly_convertible<LevelFilteredMessageHandlerPtr, FilteredMessageHandlerPtr>();

	scope mhS( mh );

	enum_<MessageHandler::Level>( "Level" )
		.value( "Error", MessageHandler::Error )
		.value( "Warning", MessageHandler::Warning )
		.value( "Info", MessageHandler::Info )
		.value( "Debug", MessageHandler::Debug )
	;
		
}

}
