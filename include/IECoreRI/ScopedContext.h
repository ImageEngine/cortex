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

#ifndef IECORERI_SCOPEDCONTEXT_H
#define IECORERI_SCOPEDCONTEXT_H

#include "ri.h"

#include "boost/noncopyable.hpp"

namespace IECoreRI
{

/// The ScopedContext class makes it easy to manage calls
/// to RiContext() so you can manage calls to many contexts
/// relatively easily, with little fear of Exceptions or
/// multiple return paths leaving you in the wrong context.
class ScopedContext : public boost::noncopyable
{

	public :

		/// Saves the current context and instates the
		/// specified context with RiContext(). If context
		/// is RI_NULL then does nothing.
		ScopedContext( RtContextHandle context );
		/// Restores the previously saved context, unless
		/// the context specified in the constructor was RI_NULL
		/// in which case it does nothing.
		~ScopedContext();

	private :

		bool m_contextWasNull;
		RtContextHandle m_previousContext;

};

} // namespace IECoreRI

#endif // IECORERI_SCOPEDCONTEXT_H
