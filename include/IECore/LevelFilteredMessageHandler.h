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

#ifndef IE_CORE_LEVELFILTEREDMESSAGEHANDLER_H
#define IE_CORE_LEVELFILTEREDMESSAGEHANDLER_H

#include "IECore/Export.h"
#include "IECore/FilteredMessageHandler.h"

namespace IECore
{

class LevelFilteredMessageHandler;
IE_CORE_DECLAREPTR( LevelFilteredMessageHandler );

/// \addtogroup environmentGroup
///
/// <b>IECORE_LOG_LEVEL</b><br>
/// Specifies the default filtering level for messages. Valid values are :
///
///   - ERROR
///   - WARNING
///   - INFO
///   - DEBUG
		
/// This class implements a FilteredMessageHandler that only passes
/// messages which have a Level below a certain threshold.
/// \ingroup utilityGroup
class IECORE_API LevelFilteredMessageHandler : public FilteredMessageHandler
{
	public :

		IE_CORE_DECLAREMEMBERPTR( LevelFilteredMessageHandler );

		/// Creates a message handler that filter messages based on the message level,
		///  and outputs to another message handler.
		LevelFilteredMessageHandler( MessageHandlerPtr handler, MessageHandler::Level level );
		virtual ~LevelFilteredMessageHandler();

		virtual void handle( Level level, const std::string &context, const std::string &message );

		MessageHandler::Level getLevel() const;
		void setLevel( MessageHandler::Level level );

		/// Returns a message level based on the value of the IECORE_LOG_LEVEL
		/// environment variable, defaulting to Warning level if it isn't set.
		static MessageHandler::Level defaultLevel();

	protected :

		MessageHandler::Level m_level;
};

}; // namespace IECore

#endif // IE_CORE_LEVELFILTEREDMESSAGEHANDLER_H
