//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECorePython/CancellerBinding.h"
#include "IECorePython/ExceptionBinding.h"
#include "IECorePython/RefCountedBinding.h"

#include "IECore/Canceller.h"

#include <optional>

using namespace boost::python;
using namespace IECore;

namespace
{

double elapsedTimeWrapper( const Canceller &canceller )
{
	return std::chrono::duration<double>( canceller.elapsedTime() ).count();
}

class ScopedChildWrapper : public boost::noncopyable
{

	public :

		ScopedChildWrapper( Canceller &parent, Canceller &child )
			:	m_parent( &parent ), m_child( &child )
		{
		}

		void enter()
		{
			m_scope.emplace( m_parent.get(), m_child );
		}

		bool exit( boost::python::object excType, boost::python::object excValue, boost::python::object excTraceBack )
		{
			m_scope.reset();
			return false; // Don't suppress exceptions
		}

	private :

		CancellerPtr m_parent;
		CancellerPtr m_child;
		std::optional<Canceller::ScopedChild> m_scope;

};

} // namespace

namespace IECorePython
{

void bindCanceller()
{

	{
		scope s = RefCountedClass<Canceller, RefCounted>( "Canceller" )
			.def( init<>() )
			.def( "cancel", &Canceller::cancel )
			.def( "cancelled", &Canceller::cancelled )
			.def( "check", &Canceller::check )
			.staticmethod( "check" )
			.def( "elapsedTime", &elapsedTimeWrapper )
			.def( "addChild", &Canceller::addChild )
			.def( "removeChild", &Canceller::removeChild )
		;

		class_<ScopedChildWrapper, boost::noncopyable>( "ScopedChild", no_init )
			.def( init<Canceller &, Canceller &>() )
			.def( "__enter__", &ScopedChildWrapper::enter, return_self<>() )
			.def( "__exit__", &ScopedChildWrapper::exit )
		;
	}

	ExceptionClass<Cancelled>( "Cancelled", PyExc_RuntimeError )
		.def( init<>() )
	;

}

} // namespace IECorePython

