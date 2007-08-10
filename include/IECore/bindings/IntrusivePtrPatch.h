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

// Solves the downcasting problem of C++ classes.
// If you don't use this, then objects returned by intrusive_ptr for their parent classes will not be accepted back,
// when passed to a function that received intrusive_ptr to their respective classes.
//
// Based on boost headers: boost/python/converter/shared_ptr_from_python.hpp and boost/python/objects/class_metadata.hpp.

#ifndef IECORE_INTRUSIVE_PTR_PATCH_H
#define IECORE_INTRUSIVE_PTR_PATCH_H


#include <iostream>

#include <boost/python/handle.hpp>
#include <boost/python/converter/from_python.hpp>
#include <boost/python/converter/rvalue_from_python_data.hpp>
#include <boost/python/converter/registered.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/python/object/class_metadata.hpp>

namespace IECore { 

template <class T>
struct intrusive_ptr_from_python
{
	intrusive_ptr_from_python()
	{
		boost::python::converter::registry::insert(&convertible, &construct, boost::python::type_id<boost::intrusive_ptr<T> >());
	}

	private:
	
	static void* convertible(PyObject* p)
	{
		if (p == Py_None)
			return p;
		
		return boost::python::converter::get_lvalue_from_python(p, boost::python::converter::registered<T>::converters);
	}
	
	static void construct(PyObject* source, boost::python::converter::rvalue_from_python_stage1_data* data)
	{
		void* const storage = ((boost::python::converter::rvalue_from_python_storage<boost::intrusive_ptr<T> >*)data)->storage.bytes;
		// Deal with the "None" case.
		if (data->convertible == source)
			new (storage) boost::intrusive_ptr<T>();
		else
			new (storage) boost::intrusive_ptr<T>(
				static_cast<T*>(data->convertible)
				);
		
		data->convertible = storage;
	}
};

// Based on register_shared_ptr_from_python_and_casts() from boost/python/objects/class_metadata.hpp.
//
// Preamble of register_class.  Also used for callback classes, which
// need some registration of their own.
//
template <class T, class Bases>
inline void register_intrusive_ptr_from_python_and_casts(T*, Bases)
{
	using namespace boost::python::objects;

	// Constructor performs registration
	boost::python::detail::force_instantiate(intrusive_ptr_from_python<T>());

	//
	// register all up/downcasts here.  We're using the alternate
	// interface to mpl::for_each to avoid an MSVC 6 bug.
	//
	register_dynamic_id<T>();
	boost::mpl::for_each(register_base_of<T>(), (Bases*)0, (boost::add_pointer<boost::mpl::_>*)0);
}

// how to use it: create a typedef to the boost.python class_< yourClass > and provide it as the second parameter in this macro.
#define INTRUSIVE_PTR_PATCH(class, pyClass)		register_intrusive_ptr_from_python_and_casts( (class *)0, pyClass::metadata::bases() )

} // namespace IECore

#endif // IECORE_INTRUSIVE_PTR_PATCH_H
