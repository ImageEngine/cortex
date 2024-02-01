
//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2024, Cinesite VFX Ltd. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
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

#ifndef IECOREPYTHON_PYBINDCONVERTER_H
#define IECOREPYTHON_PYBINDCONVERTER_H

#include "boost/python.hpp"

#include "pybind11/pybind11.h"

namespace IECorePython
{

// Registers `boost::python` converters for types
// wrapped using PyBind11.
template<typename T>
struct PyBindConverter
{

	static void registerConverters()
	{
		boost::python::to_python_converter<T, ToPyBind>();
		boost::python::converter::registry::push_back(
			&FromPyBind::convertible,
			&FromPyBind::construct,
			boost::python::type_id<T>()
		);
	}

	private :

		struct ToPyBind
		{
			static PyObject *convert( const T &t )
			{
				pybind11::object o = pybind11::cast( t );
				Py_INCREF( o.ptr() );
				return o.ptr();
			}
		};

		struct FromPyBind
		{

			static void *convertible( PyObject *object )
			{
				pybind11::handle handle( object );
				return handle.cast<T>() ? object : nullptr;
			}

			static void construct( PyObject *object, boost::python::converter::rvalue_from_python_stage1_data *data )
			{
				void *storage = ( ( boost::python::converter::rvalue_from_python_storage<T> * ) data )->storage.bytes;
				T *t = new( storage ) T;
				data->convertible = storage;

				pybind11::handle handle( object );
				*t = handle.cast<T>();
			}

		};

};

} // namespace IECorePython

#endif // IECOREPYTHON_PYBINDCONVERTER_H

