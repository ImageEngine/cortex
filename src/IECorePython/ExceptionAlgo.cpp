//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "boost/python.hpp"

#include "IECorePython/ExceptionAlgo.h"

#include "IECore/Exception.h"

using namespace boost::python;

namespace
{

std::string formatInternal(
	PyObject *exceptionPyObject, PyObject *valuePyObject, PyObject *tracebackPyObject,
	bool withStacktrace, int *lineNumber = nullptr
)
{
	if( !exceptionPyObject )
	{
		throw IECore::Exception( "No Python exception set" );
	}

	PyErr_NormalizeException( &exceptionPyObject, &valuePyObject, &tracebackPyObject );

	object exception( ( handle<>( exceptionPyObject ) ) );

	// valuePyObject and tracebackPyObject may be null.
	object value;
	if( valuePyObject )
	{
		value = object( handle<>( valuePyObject ) );
	}

	object traceback;
	if( tracebackPyObject )
	{
		traceback = object( handle<>( tracebackPyObject ) );
	}

	if( lineNumber )
	{
		if( PyErr_GivenExceptionMatches( value.ptr(), PyExc_SyntaxError ) )
		{
			*lineNumber = extract<int>( value.attr( "lineno" ) );
		}
		else if( traceback )
		{
			*lineNumber = extract<int>( traceback.attr( "tb_lineno" ) );
		}
	}

	object tracebackModule( import( "traceback" ) );

	object formattedList;
	if( withStacktrace )
	{
		formattedList = tracebackModule.attr( "format_exception" )( exception, value, traceback );
	}
	else
	{
		formattedList = tracebackModule.attr( "format_exception_only" )( exception, value );
	}

	object formatted = str( "" ).join( formattedList );
	std::string s = extract<std::string>( formatted );

	return s;
}

} // namespace

namespace IECorePython
{

namespace ExceptionAlgo
{

std::string formatPythonException( bool withStacktrace, int *lineNumber )
{
	PyObject *exceptionPyObject, *valuePyObject, *tracebackPyObject;
	PyErr_Fetch( &exceptionPyObject, &valuePyObject, &tracebackPyObject );
	return formatInternal( exceptionPyObject, valuePyObject, tracebackPyObject, withStacktrace, lineNumber );
}

void translatePythonException( bool withStacktrace )
{
	PyObject *exceptionPyObject, *valuePyObject, *tracebackPyObject;
	PyErr_Fetch( &exceptionPyObject, &valuePyObject, &tracebackPyObject );

	// If the python exception is one bound via IECorePython::ExceptionClass,
	// then we can extract and throw the C++ exception held internally.
	if( PyObject_HasAttrString( valuePyObject, "__exceptionPointer" ) )
	{
		object exceptionPointerMethod( handle<>( PyObject_GetAttrString( valuePyObject, "__exceptionPointer" ) ) );
		object exceptionPointerObject = exceptionPointerMethod();
		std::exception_ptr exceptionPointer = boost::python::extract<std::exception_ptr>( exceptionPointerObject )();
		std::rethrow_exception( exceptionPointer );
	}

	// Otherwise, we just throw a generic exception describing the python error.
	throw IECore::Exception( formatInternal( exceptionPyObject, valuePyObject, tracebackPyObject, withStacktrace ) );
}

} // namespace ExceptionAlgo

} // namespace IECorePython
