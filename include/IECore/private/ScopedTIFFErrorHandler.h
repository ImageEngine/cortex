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

#ifndef IECORE_SCOPEDTIFFERRORHANDLER_H
#define IECORE_SCOPEDTIFFERRORHANDLER_H

#include <stack>

#include "tiffio.h"

#include "tbb/mutex.h"
#include "tbb/enumerable_thread_specific.h"

namespace IECore
{
namespace Detail
{

/// Installs an error handler for the time it is in scope, and captures
/// any errors coming it's way. These can then be converted to exceptions
/// by calling the throwIfError() member function.
class ScopedTIFFErrorHandler
{
	public :
	
		ScopedTIFFErrorHandler();
		virtual ~ScopedTIFFErrorHandler();

		/// Returns true if any errors have been captured.
		bool hasError() const;
		/// Throws a descriptive IOException if any errors have been captured.
		void throwIfError() const;
		/// The error messages captured to far.
		const std::string &errorMessage() const;
		/// Clears any errors captured so far.
		void clear();
		
	private :

		static void handler( const char *module, const char *fmt, va_list ap );

		std::string m_errorMessage;
		
		static tbb::mutex g_handlerMutex;
		static TIFFErrorHandler g_previousHandler;
		static size_t g_handlerCount;
		
		static tbb::enumerable_thread_specific<std::stack<ScopedTIFFErrorHandler *> > g_handlerStack;

};

} // namespace Detail
} // namespace IECore

#endif // IECORE_SCOPEDTIFFERRORHANDLER_H
