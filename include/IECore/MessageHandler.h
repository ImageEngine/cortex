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

#ifndef IECORE_MESSAGEHANDLER_H
#define IECORE_MESSAGEHANDLER_H

#include <stack>

#include "boost/format.hpp"
#include "boost/noncopyable.hpp"

#include "IECore/Export.h"
#include "IECore/RefCounted.h"

namespace IECore
{

class MessageHandler;
IE_CORE_DECLAREPTR( MessageHandler );

/// The MessageHandler class should be used for all logging
/// within IECore code, and code using IECore. It provides a
/// uniform interface for outputting messages, with the possibility
/// to implement multiple message handlers appropriate to
/// specific application contexts.
/// \ingroup utilityGroup
class IECORE_API MessageHandler : public RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( MessageHandler );

		enum Level
		{
			Error = 0,
			Warning = 1,
			Info = 2,
			Debug = 3,
			Invalid = 4
		};

		//! @name Message output
		/// These functions all output a message via the current
		/// MessageHandler object.
		/// \threading These functions are threadsafe provided that
		/// the current handler's handle() method is also threadsafe.
		///////////////////////////////////////////////////////
		//@{
		/// Output a message to the current handler.
		static void output( Level level, const std::string &context, const std::string &message );
		/// Output a message to the current handler.
		static void output( Level level, const std::string &context, const boost::format &message );
		//@}

		//! @name Default handler
		/// There is a single default MessageHandler specified
		/// globally. Typically this will be set once on application
		/// startup and then be left in place. This message handler
		/// is used whenever there is no locally specified handler
		/// installed using the Scope class below.
		/// \threading These functions are not threadsafe - it is
		/// expected that setDefaultHandler() will be called once
		/// from the main thread on application startup.
		///////////////////////////////////////////////////////
		//@{
		static void setDefaultHandler( const MessageHandlerPtr &handler );
		static MessageHandler *getDefaultHandler();
		//@}
		
		/// Each thread has its own stack of message handlers which
		/// may be pushed and popped to provide message handling specific
		/// to particular contexts. The Scope class is used to install
		/// these local handlers on construction and uninstall them on
		/// destruction.
		/// \threading The Scope class provides a threadsafe means of
		/// installing and uninstalling MessageHandlers.
		class IECORE_API Scope : boost::noncopyable
		{
		
			public :
				
				/// Pushes the specified handler, making it
				/// the currentHandler() for this thread. It is
				/// the caller's responsibility to keep the handler
				/// alive for the lifetime of the Scope class.
				Scope( MessageHandler *handler );
				/// Pops the handler pushed in the constructor,
				/// reverting to the previous handler.
				~Scope();
			
			private :
				
				MessageHandler *m_handler;
		
		};
		
		/// Returns the current handler for this thread, reverting
		/// to the result of getDefaultHandler() if no thread-local
		/// handler has been installed.
		/// \threading This is threadsafe with respect to handlers installed
		/// by the Scope class.
		static MessageHandler *currentHandler();

		//! @name Conversions between Level and string
		////////////////////////////////////////////////////////
		//@{
		/// Returns a readable string representation of the specified message level.
		static std::string levelAsString( Level level );
		/// Returns a message level based on the specified string (case is ignored).
		static Level stringAsLevel( const std::string &level );
		//@}

		/// Must be implemented by subclasses to output the message appropriately. Client code
		/// should use MessageHandler::output() rather than call this directly.
		virtual void handle( Level level, const std::string &context, const std::string &message ) = 0;

};

/// typedef for brevity.
typedef MessageHandler Msg;

/// Free functions which calls MessageHandler::output() with their arguments. These are provided
/// for brevity.
IECORE_API void msg( MessageHandler::Level level, const std::string &context, const std::string &message );
IECORE_API void msg( MessageHandler::Level level, const std::string &context, const boost::format &message );

}; // namespace IECore

#endif // IECORE_MESSAGEHANDLER_H
