//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreHoudini/MessageHandler.h"

using namespace IECoreHoudini;

MessageHandler::MessageHandler( HandlerFn errorFn, HandlerFn warningFn, HandlerFn infoFn, HandlerFn debugFn )
	: m_errorFn( errorFn ), m_warningFn( warningFn ), m_infoFn( infoFn ),  m_debugFn( debugFn )
{
}

MessageHandler::~MessageHandler()
{
	m_errorFn = 0;
	m_warningFn = 0;
	m_infoFn = 0;
	m_debugFn = 0;
}

void MessageHandler::handle( Level level, const std::string &context, const std::string &message )
{
	std::string msg( context + " : " + message );
	const char *m = msg.c_str();
	switch( level )
	{
		case MessageHandler::Error :
			if ( m_errorFn )
			{
				m_errorFn( m );
			}
			break;
		case MessageHandler::Warning :
			if ( m_warningFn )
			{
				m_warningFn( m );
			}
			break;
		case MessageHandler::Info :
			if ( m_infoFn )
			{
				m_infoFn( m );
			}
			break;
		case MessageHandler::Debug :
			if ( m_debugFn )
			{
				m_debugFn( m );
			}
			break;
		default :
			// do nothing for invalid levels
			break;
	}
}
