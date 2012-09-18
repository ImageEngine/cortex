//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "IECore/private/ScopedTIFFErrorHandler.h"
#include "IECore/Exception.h"

using namespace IECore::Detail;

tbb::mutex ScopedTIFFErrorHandler::g_handlerMutex;
TIFFErrorHandler ScopedTIFFErrorHandler::g_previousHandler = 0;
size_t ScopedTIFFErrorHandler::g_handlerCount = 0;

tbb::enumerable_thread_specific<std::stack<ScopedTIFFErrorHandler *> > ScopedTIFFErrorHandler::g_handlerStack;

ScopedTIFFErrorHandler::ScopedTIFFErrorHandler()
{
	{
		tbb::mutex::scoped_lock lock( g_handlerMutex );
		if( !g_handlerCount )
		{
			g_previousHandler = TIFFSetErrorHandler( handler );
		}
		g_handlerCount++;
	}
	
	std::stack<ScopedTIFFErrorHandler *> &handlers = g_handlerStack.local();
	handlers.push( this );
}

ScopedTIFFErrorHandler::~ScopedTIFFErrorHandler()
{
	std::stack<ScopedTIFFErrorHandler *> &handlers = g_handlerStack.local();
	assert( handlers.top() == this );
	handlers.pop();
	
	{
		tbb::mutex::scoped_lock lock( g_handlerMutex );
		
		assert( g_handlerCount );
		g_handlerCount--;
		
		if( !g_handlerCount )
		{
			TIFFSetErrorHandler( g_previousHandler );
		}
	}
}

void ScopedTIFFErrorHandler::handler( const char *module, const char *fmt, va_list ap )
{
	const std::stack<ScopedTIFFErrorHandler *> &handlers = g_handlerStack.local();
	if( !handlers.size() )
	{
		// an unknown thread is using libtiff without using a TIFFErrorHandler.
		// forward the error to the previous handler.
		if( g_previousHandler )
		{
			g_previousHandler( module, fmt, ap );
		}
		return;
	}
	
	ScopedTIFFErrorHandler *handler = handlers.top();
	
	const unsigned int bufSize = 1024;
	char buf[bufSize];
	vsnprintf( buf, bufSize-1, fmt, ap );

	/// Make sure string is null-terminated
	buf[bufSize-1] = '\0';

	std::string context = module ? module : "libtiff";

	if( handler->m_errorMessage.size() )
	{
		handler->m_errorMessage += "\n";
	}
	handler->m_errorMessage += context + " : " + buf;
}

bool ScopedTIFFErrorHandler::hasError() const
{
	return m_errorMessage.size();
}

void ScopedTIFFErrorHandler::throwIfError() const
{
	if( m_errorMessage.size() )
	{
		throw IOException( m_errorMessage );
	}
}

const std::string &ScopedTIFFErrorHandler::errorMessage() const
{
	return m_errorMessage;
}

void ScopedTIFFErrorHandler::clear()
{
	m_errorMessage.clear();
}
