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

#ifndef IE_CORE_FILTEREDMESSAGEHANDLER_H
#define IE_CORE_FILTEREDMESSAGEHANDLER_H

#include "IECore/MessageHandler.h"

namespace IECore
{

class FilteredMessageHandler;
IE_CORE_DECLAREPTR( FilteredMessageHandler );

/// This abstract base class that implements filtering MessageHandler of any kind.
class FilteredMessageHandler : public MessageHandler
{
	public :

		IE_CORE_DECLAREMEMBERPTR( FilteredMessageHandler );

		/// Creates a message handler that filter messages and outputs
		/// to another message handler. The handler will hold an intrusive
		/// pointer to the given message handler so that the object
		/// will not be destroyed before the destruction of this one.
		FilteredMessageHandler( MessageHandlerPtr handler );

	protected :

		virtual ~FilteredMessageHandler();


	protected :

		MessageHandlerPtr m_handler;
};

}; // namespace IECore

#endif // IE_CORE_FILTEREDMESSAGEHANDLER_H
