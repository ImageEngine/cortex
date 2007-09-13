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

#ifndef IE_CORE_OSTREAMMESSAGEHANDLER_H
#define IE_CORE_OSTREAMMESSAGEHANDLER_H

#include "IECore/MessageHandler.h"

#include <ostream>

namespace IECore
{

class OStreamMessageHandler;
IE_CORE_DECLAREPTR( OStreamMessageHandler );

/// This class implements a simple MessageHandler
/// to write to a std::ostream object. 
class OStreamMessageHandler : public MessageHandler
{

	public :
	
		/// Creates a message handler to output to the specified
		/// stream. The handler does not own the stream and will
		/// not attempt to delete it. This form of the constructor
		/// is intended primarily for outputing to default streams
		/// such as std::cerr.
		OStreamMessageHandler( std::ostream &stream );
		/// Creates a message handler to output to the specified
		/// stream. The handler takes ownerwship of the passed
		/// stream and will delete it on destruction.
		OStreamMessageHandler( std::ostream *stream );
		
		//! @name Shared message handlers
		/// These functions return static instances of some
		/// default message handlers that can be shared by
		/// everyone.
		////////////////////////////////////////////////////////
		//@{
		static OStreamMessageHandlerPtr cErrHandler();
		static OStreamMessageHandlerPtr cOutHandler();
		//@}
		
		virtual void handle( Level level, const std::string &context, const std::string &message );
		
	protected :
	
		virtual ~OStreamMessageHandler();
	
		
	private :

		bool m_ownStream;
		std::ostream *m_stream;
		
};

}; // namespace IECore

#endif // IE_CORE_OSTREAMMESSAGEHANDLER_H
