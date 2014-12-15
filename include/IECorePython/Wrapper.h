//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREPYTHON_WRAPPER_H
#define IECOREPYTHON_WRAPPER_H

#include <boost/python.hpp>
#include <cassert>

#include "IECorePython/Export.h"
#include "IECorePython/WrapperGarbageCollector.h"

namespace IECorePython
{

/// Use this class to wrap c++ objects so they can be inherited from in python,
/// and even override c++ virtual functions. We can't use boost::python::wrapper
/// for this alone because it has massive issues with memory management - ie it
/// makes no attempt to do any, whereas our WrapperGarbageCollector base class
/// handles that in ok fashion.
/// \deprecated The RefCountedWrapper and RunTimeTypedWrapper classes provide
/// a replacement for this class.
/// \todo Remove for the next major version.
template<typename T>
class Wrapper : public boost::python::wrapper<T>, public WrapperGarbageCollector
{
	public:
		Wrapper(PyObject *self, IECore::RefCounted* r) : WrapperGarbageCollector( self, r )
		{
			assert(m_pyObject);
			boost::python::detail::initialize_wrapper(m_pyObject, this);
			Py_INCREF(m_pyObject);
		}

		virtual ~Wrapper()
		{
			assert(m_pyObject);
		}

		boost::python::override get_override(const char *name) const
		{
			assert(m_pyObject);
			assert(m_pyObject == boost::python::detail::wrapper_base_::get_owner(*this));
			
			boost::python::override func = this->boost::python::wrapper<T>::get_override( name );
			
			// boost's get_override calls PyObject_GetAttrString indiscriminately
			// and doesn't clear the error status if it fails - this can cause havoc elsewhere.
			// so if there's an attribute exception after the call above we clear it now.
			// see ParameterisedProceduralTest.py for a test case that exercises the problem.
			if( PyObject *err = PyErr_Occurred() )
			{
				if( PyErr_GivenExceptionMatches( err, PyExc_AttributeError ) )
				{
					PyErr_Clear();
				}
			}
			
			return func;
		}

};

}

#endif // IECOREPYTHON_WRAPPER_H
