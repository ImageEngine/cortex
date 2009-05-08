//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_WRAPPER_H
#define IECORE_WRAPPER_H

#include <boost/python.hpp>
#include <cassert>

#include <IECore/bindings/WrapperGarbageCollector.h>

namespace IECore
{

/// Use this class to wrap c++ objects so they can be inherited from in python,
/// and even override c++ virtual functions. We can't use boost::python::wrapper
/// for this alone because it has massive issues with memory management - ie it
/// makes no attempt to do any, whereas our WrapperGarbageCollector base class
/// handles that in ok fashion.
/// \todo I suspect we can completely drop the inheritance
/// of boost::python::wrapper as we only use it for the get_override method, and
/// we could copy it out of the boost sources. Doing so would make the rather
/// complicated class hierarchy here a lot simpler. If we were to create a libIECorePython
/// we could also collapse WrapperGarbageCollector and WrapperGarbageCollectorBase
/// into one class which would also make things a little clearer.
template<typename T>
class Wrapper : public boost::python::wrapper<T>, public WrapperGarbageCollector
{
	public:
		Wrapper(PyObject *self, RefCounted* r) : WrapperGarbageCollector( self, r )
		{
			assert(m_pyObject);
			boost::python::detail::initialize_wrapper(m_pyObject, this);
			Py_INCREF(m_pyObject);
		}

		virtual ~Wrapper()
		{
			assert(m_pyObject);
			if( m_pyObject->ob_refcnt > 0 )
			{
				// i don't know that there are any circumstances under which we can get here. it's certainly
				// important that we don't call Py_DECREF if the reference count is already 0 though, as this yields
				// a negative reference count and screws up the clearing of weak references, resulting in
				// crashes.
				Py_DECREF(m_pyObject);
			}
		}

		boost::python::override get_override(const char *name) const
		{
			assert(m_pyObject);
			assert(m_pyObject == boost::python::detail::wrapper_base_::get_owner(*this));

			boost::python::override func = this->boost::python::wrapper<T>::get_override(name);
			return func;
		}

};

}

#endif // IECORE_WRAPPER_H
