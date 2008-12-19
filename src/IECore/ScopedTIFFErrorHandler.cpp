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



#include "boost/format.hpp"

#include "IECore/ScopedTIFFErrorHandler.h"
#include "IECore/Exception.h"

using namespace IECore;

std::vector< ScopedTIFFErrorHandler * > &ScopedTIFFErrorHandler::handlers()
{
	static std::vector< ScopedTIFFErrorHandler * > g_handlers;
	return g_handlers;
}

ScopedTIFFErrorHandler::ScopedTIFFErrorHandler()
{
	m_previousHandler = TIFFSetErrorHandler( &output );
	handlers().push_back( this );
}

ScopedTIFFErrorHandler::~ScopedTIFFErrorHandler()
{
	assert( handlers().back() == this );
	TIFFSetErrorHandler( m_previousHandler );
	handlers().pop_back();
}

void ScopedTIFFErrorHandler::output(const char* module, const char* fmt, va_list ap)
{	
	ScopedTIFFErrorHandler *handler = handlers().back();
	// Ensure that any variables we allocate here don't get lost due to the longjmp call
	{	
		/// Reconstruct the actual error in a buffer of (arbitrary) maximum length.
		const unsigned int bufSize = 1024;
		char buf[bufSize];
		vsnprintf( &buf[0], bufSize-1, fmt, ap );

		/// Make sure string is null-terminated
		buf[bufSize-1] = '\0';

		std::string context = "libtiff";
		if (module)
		{
			context = std::string( module );
		} 

		handler->m_errorMessage = ( boost::format( "%s : %s" ) % context % buf ).str() ;
	}
	longjmp( handler->m_jmpBuffer, 1 );
}
