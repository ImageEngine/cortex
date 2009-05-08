//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

using namespace boost::python;

namespace IECore
{

struct ExtractStringFromUnicode
{
	static void *convertible( PyObject *obj )
	{
		if( PyUnicode_Check( obj ) )
		{
			return obj;
		}
		else
		{
			return 0;
		}
	}

	static void construct( PyObject *obj, converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = ((converter::rvalue_from_python_storage<std::string>*)data)->storage.bytes;

		PyObject *ascii = PyUnicode_AsASCIIString( obj );
		if( ascii )
		{
			new (storage) std::string( PyString_AsString( ascii ) );
			// record successful construction
			data->convertible = storage;
			Py_DECREF( ascii );
		}
		else
		{
			data->convertible = 0;
		}
	}
};

void bindUnicodeToString()
{
	// we don't use unicode in the cortex api, but we often end up passing unicode strings
	// to it and having to do the encoding to ascii each time. this just automates that process.
	// this is mainly needed when using IECoreMaya as the maya apis love to return unicode, but
	// we're enabling the conversion centrally in IECore so behaviour doesn't change depending on
	// what is imported.
	converter::registry::push_back(
		&ExtractStringFromUnicode::convertible,
		&ExtractStringFromUnicode::construct,
		type_id<const std::string &>()
	);

}

} // namespace IECore
