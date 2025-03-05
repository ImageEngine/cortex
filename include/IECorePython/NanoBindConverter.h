
//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2025, Alex Fuller. All rights reserved.
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

#ifndef IECOREPYTHON_NANOBINDCONVERTER_H
#define IECOREPYTHON_NANOBINDCONVERTER_H

#include "boost/python.hpp"

#include "nanobind/nanobind.h"

namespace IECorePython
{

// Registers `boost::python` converters for types
// wrapped using nanobind.
template<typename T>
struct NanoBindConverter
{

	static void registerConverters()
	{
		boost::python::to_python_converter<T, ToNanoBind>();
		boost::python::converter::registry::push_back(
			&FromNanoBind::convertible,
			&FromNanoBind::construct,
			boost::python::type_id<T>()
		);
	}

	private :

		struct ToNanoBind
		{
			static PyObject *convert( const T &t )
			{
				nanobind::object o = nanobind::cast( t );
				Py_INCREF( o.ptr() );
				return o.ptr();
			}
		};

		struct FromNanoBind
		{

			static void *convertible( PyObject *object )
			{
				nanobind::handle handle( object );
				return nanobind::cast<T>( handle ) ? object : nullptr;
			}

			static void construct( PyObject *object, boost::python::converter::rvalue_from_python_stage1_data *data )
			{
				void *storage = ( ( boost::python::converter::rvalue_from_python_storage<T> * ) data )->storage.bytes;
				T *t = new( storage ) T;
				data->convertible = storage;

				nanobind::handle handle( object );
				*t = nanobind::cast<T>( handle );
			}

		};

};

} // namespace IECorePython

#endif // IECOREPYTHON_NANOBINDCONVERTER_H

