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

namespace IECorePython
{

namespace ExceptionAlgo
{

std::string formatPythonException( bool withStacktrace, int *lineNumber )
{
	PyObject *exceptionPyObject, *valuePyObject, *tracebackPyObject;
	PyErr_Fetch( &exceptionPyObject, &valuePyObject, &tracebackPyObject );

	if( !exceptionPyObject )
	{
		throw IECore::Exception( "No Python exception set" );
	}

	std::string simpleFormat = "Could not get exception text\n";
	if( valuePyObject )
	{
		simpleFormat = PyString_AsString(valuePyObject) + std::string( "\n" );
	}

	PyErr_NormalizeException( &exceptionPyObject, &valuePyObject, &tracebackPyObject );


	try
	{
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

			simpleFormat += "Simple Traceback:\n";
			object curTraceback = traceback;
			while( curTraceback )
			{
				std::string filename = extract<std::string>( curTraceback.attr( "tb_frame" ).attr( "f_code" ).attr( "co_filename" ) );
				int lineNo = extract<int>( curTraceback.attr( "tb_frame" ).attr("f_lineno") );
				simpleFormat += filename + " : Line " + std::to_string( lineNo ) +  "\n";
				curTraceback = curTraceback.attr( "tb_next" );
			}
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
	catch( boost::python::error_already_set &e )
	{
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		return std::string( "Python exception can't be formatted nicely due to Python failure: " ) + PyString_AsString( pvalue ) + "\nOriginal exception is: " + simpleFormat;
	}
}

void translatePythonException( bool withStacktrace )
{
	throw IECore::Exception( formatPythonException( withStacktrace ) );
}

} // namespace ExceptionAlgo

} // namespace IECorePython
