//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_SCOPEDTIFFERRORHANDLER_H
#define IE_CORE_SCOPEDTIFFERRORHANDLER_H

#include <stdarg.h>
#include <setjmp.h>
#include <vector>
#include "tbb/mutex.h"

#include "tiffio.h"

namespace IECore
{

/// A class that hides the TIFF error handling mechanism by raising exceptions instead.
/// The object will catch any TIFF errors until it gets out of scope.
class ScopedTIFFErrorHandler
{
	public:
		ScopedTIFFErrorHandler( );
		virtual ~ScopedTIFFErrorHandler();

	protected:

		jmp_buf m_jmpBuffer;
		std::string m_errorMessage;

		static tbb::mutex m_setupMutex;
		static void output(const char* module, const char* fmt, va_list ap);
		static TIFFErrorHandler m_previousHandler;
		static unsigned int m_handlerCount;
};

} // namespace IECore

#endif // IE_CORE_SCOPEDTIFFERRORHANDLER_H
