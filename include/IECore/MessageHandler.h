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

#ifndef IE_CORE_MESSAGEHANDLER_H
#define IE_CORE_MESSAGEHANDLER_H

#include "IECore/RefCounted.h"

#include "boost/format.hpp"

#include <stack>

namespace IECore
{

class MessageHandler;
IE_CORE_DECLAREPTR( MessageHandler );

/// The MessageHandler class should be used for all logging
/// within IECore code, and code using IECore. It provides a
/// uniform interface for outputting messages, with the possibility
/// to implement multiple message handlers appropriate to
/// specific application contexts.
/// \todo Thread safety
class MessageHandler : public RefCounted
{

	public :
	
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
		///////////////////////////////////////////////////////
		//@{
		/// Output a message to the current handler.
		static void output( Level level, const std::string &context, const std::string &message );
		/// Output a message to the current handler.
		static void output( Level level, const std::string &context, const boost::format &message );
		//@}
		
		//! @name Handler stack manipulation
		///
		/// A stack of message handlers is provided to make it
		/// easy to install a different message handler appropriate
		/// to a new application context, and then revert to the
		/// previous handler upon exiting that context. At startup
		/// a single default handler resides on the stack - it
		/// is an error to try to pop this.
		///////////////////////////////////////////////////////
		//@{
		/// Pushes a new MessageHandler onto the stack. This will
		/// be the current handler until popHandler() is called.
		static void pushHandler( MessageHandlerPtr handler );
		/// Pops the current MessageHandler from the stack. The current
		/// handler will now be the one that was current prior to the
		/// last pushHandler() call.
		static MessageHandlerPtr popHandler();
		//@}
		
		//! @name Conversions between Level and string
		////////////////////////////////////////////////////////
		//@{
		/// Returns a readable string representation of the specified message level.
		static std::string levelAsString( Level level );
		/// Returns a message level based on the specified string (case is ignored).
		static Level stringAsLevel( const std::string &level );
		//@}
		
		/// Must be implemented by subclasses to output the message appropriately.	
		virtual void handle( Level level, const std::string &context, const std::string &message ) = 0;	

	private :

		/// Returns the current handler.
		static MessageHandlerPtr currentHandler();
		/// Returns the stack of message handlers.
		static std::stack<MessageHandlerPtr> *handlerStack();

};

/// typedef for brevity.
typedef MessageHandler Msg;

/// Free functions which calls MessageHandler::output() with their arguments. These are provided
/// for brevity.
void msg( MessageHandler::Level level, const std::string &context, const std::string &message );
void msg( MessageHandler::Level level, const std::string &context, const boost::format &message );

}; // namespace IECore

#endif // IE_CORE_MESSAGEHANDLER_H
