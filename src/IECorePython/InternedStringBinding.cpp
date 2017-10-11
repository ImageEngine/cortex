//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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
#include "boost/functional/hash.hpp"

#include "IECore/InternedString.h"
#include "IECorePython/InternedStringBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

struct InternedStringFromPython
{
	InternedStringFromPython()
	{
		converter::registry::push_back(
			&convertible,
			&construct,
			type_id<InternedString> ()
		);
	}

	static void *convertible( PyObject *obj_ptr )
	{
		if ( !PyString_Check( obj_ptr ) )
		{
			return nullptr;
		}
		return obj_ptr;
	}

	static void construct(
	        PyObject *obj_ptr,
	        converter::rvalue_from_python_stage1_data *data )
	{
		assert( obj_ptr );
		assert( PyString_Check( obj_ptr ) );

		void* storage = (( converter::rvalue_from_python_storage<InternedString>* ) data )->storage.bytes;
		new( storage ) InternedString( PyString_AsString( obj_ptr ) );
		data->convertible = storage;
	}
};

static string repr( const InternedString &str )
{
	stringstream s;
	s << "IECore.InternedString(\"" << str.value() << "\")";
	return s.str();
}

static size_t hash( const InternedString &str )
{
	return boost::hash<const char *>()( str.c_str() );
}

void bindInternedString()
{

	class_<InternedString>( "InternedString" )
		.def( init<const char *>() )
		.def( init<InternedString>() )
		.def( init<int64_t>() )
		.def( "__str__", &InternedString::value, return_value_policy<copy_const_reference>() )
		.def( "value", &InternedString::value, return_value_policy<copy_const_reference>() )
		.def( self == self )
		.def( self != self )
		.def( "numUniqueStrings", &InternedString::numUniqueStrings ).staticmethod( "numUniqueStrings" )
		.def( "__repr__", &repr )
		.def( "__hash__", &hash )
	;
	implicitly_convertible<InternedString, string>();

	InternedStringFromPython();

}

} // namespace IECorePython
