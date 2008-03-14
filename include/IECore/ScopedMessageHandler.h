//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_SCOPEDMESSAGEHANDLER_H
#define IECORE_SCOPEDMESSAGEHANDLER_H

#include "IECore/MessageHandler.h"

#include "boost/noncopyable.hpp"

namespace IECore
{

IE_CORE_DECLAREPTR( MessageHandler );

/// The ScopedMessageHandler does not actually implement the MessageHandler
/// interface. Instead it provides a simple way of managing the duration for
/// which another MessageHandler is current.
class ScopedMessageHandler : public boost::noncopyable
{

	public :
	
		/// Pushes the handler as the current MessageHandler.
		ScopedMessageHandler( MessageHandlerPtr handler );
		/// Pops the previously pushed MessageHandler, throwing
		/// an Exception if it is not the current one.
		~ScopedMessageHandler();
		
	private :
	
		MessageHandlerPtr m_handler;
		
};

}; // namespace IECore

#endif // IECORE_SCOPEDMESSAGEHANDLER_H
