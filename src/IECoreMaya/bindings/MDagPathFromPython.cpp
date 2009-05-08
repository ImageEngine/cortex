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

#include <boost/python.hpp>

#include "IECoreMaya/StatusException.h"

#include "maya/MSelectionList.h"
#include "maya/MString.h"
#include "maya/MDagPath.h"

using namespace boost::python;

namespace IECoreMaya
{

struct MDagPathFromPython
{
	static void *convertible( PyObject *obj )
	{
		if( PyUnicode_Check( obj ) || PyString_Check( obj ) )
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
		void *storage = ((converter::rvalue_from_python_storage<MDagPath>*)data)->storage.bytes;

		MString name;

		if( PyUnicode_Check( obj ) )
		{
			PyObject *ascii = PyUnicode_AsASCIIString( obj );
			name = PyString_AsString( ascii );
			Py_DECREF( ascii );
		}
		else
		{
			name = PyString_AsString( obj );
		}

		MSelectionList s;
		StatusException::throwIfError( s.add( name ) );

		MDagPath path;
		StatusException::throwIfError( s.getDagPath( 0, path ) );

		new (storage) MDagPath( path );
		data->convertible = storage;
	}
};

void bindMDagPathFromPython()
{
	converter::registry::push_back(
		&MDagPathFromPython::convertible,
		&MDagPathFromPython::construct,
		type_id<MDagPath>()
	);
}

} // namespace IECore
