//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2019, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREPYTHON_EXCEPTIONBINDING_INL
#define IECOREPYTHON_EXCEPTIONBINDING_INL

#include "boost/make_unique.hpp"

namespace IECorePython
{

namespace Detail
{

inline boost::python::object exceptionInit( boost::python::tuple args, boost::python::dict kw )
{
	// Initialise our internal implementation

	boost::python::object self = args[0];
	boost::python::object implementationClass = self.attr( "__Implementation" );

	boost::python::object implementation;
	if( boost::python::len( args ) == 2 )
	{
		boost::python::object a1 = args[1];
		if( PyObject_IsInstance( a1.ptr(), implementationClass.ptr() ) )
		{
			// We're being constructed from our exception translator, with
			// the instance of our implementation being provided directly.
			implementation = a1;
		}
	}

	if( implementation == boost::python::object() )
	{
		// We're being constructed from Python. We need to construct an implementation
		// ourselves.
		boost::python::tuple implementationArgs( args.slice( 1, boost::python::_ ) );
		implementation = implementationClass( *implementationArgs, **kw );
	}

	self.attr( "__implementation" ) = implementation;

	// Call our base class initialiser, passing a string describing the
	// contents of our implementation.

	boost::python::object base = self.attr( "__class__" ).attr( "__bases__" )[0];
	base.attr( "__init__" )( self, boost::python::str( implementation ) );

	return boost::python::object();
}

inline boost::python::object exceptionGetAttr( boost::python::object exception, const char *name )
{
	boost::python::object implementation = exception.attr( "__implementation" );
	return implementation.attr( name );
}

template<typename T>
inline std::exception_ptr implementationExceptionPointer( const T &t )
{
	try
	{
		throw t;
	}
	catch( ... )
	{
		return std::current_exception();
	}
}

template<typename T, typename std::enable_if<std::is_base_of<std::exception, T>::value, T>::type* = nullptr>
inline const char *implementationStr( const T &t )
{
	return t.what();
}

template<typename T, typename std::enable_if<boost::mpl::not_<std::is_base_of<std::exception, T>>::value, T>::type* = nullptr>
inline const char *implementationStr( const T &t )
{
	return "";
}

} // namespace Detail

template<typename T>
ExceptionClass<T>::ExceptionClass( const char *className, PyObject *base )
{
	// Python exception types must derive from PyExc_Exception or one of its base classes,
	// so we can't bind exceptions via `boost::python::class_` as usual. Instead we must use
	// PyErr_NewException. See https://mail.python.org/pipermail/cplusplus-sig/2012-June/016649.html.
	//
	// PyErr_NewException crashes if the class name isn't qualified by the module name.
	// The binary component of our modules is always of the ugly form "Foo._Foo", so we use
	// just the "Foo" part.
	boost::python::scope scope;
	std::string scopeName = boost::python::extract<std::string>( scope.attr( "__name__" ) );
	scopeName = scopeName.substr( 0, scopeName.find( '.' ) );
	const std::string qualifiedClassName = scopeName + "." + className;

	PyObject *exceptionClass = PyErr_NewException( (char *)qualifiedClassName.c_str(), base, nullptr );
	boost::python::object exceptionClassObject( boost::python::borrowed( exceptionClass ) );
	scope.attr( className ) = exceptionClassObject;

	// We then bind `T` using `boost::python::class_` as usual, as a private implementation class
	// inside our PyExc_Exception-derived type. Constructors are added later by client code calling
	// `ExceptionClass.def()`.
	{
		boost::python::scope privateScope( exceptionClassObject );
		m_implementationClass = boost::make_unique<ImplementationClass>( "__Implementation", boost::python::no_init );
		m_implementationClass->def( "__exceptionPointer", &Detail::implementationExceptionPointer<T> );
		m_implementationClass->def( "__str__", &Detail::implementationStr<T> );
	}

	// We can now give the outer class a custom init that constructs an instance of the internal class.
	exceptionClassObject.attr( "__init__" ) = boost::python::raw_function( &Detail::exceptionInit );
	// And a getattr that forwards to the implementation.
	exceptionClassObject.attr( "__getattr__") = boost::python::make_function( &Detail::exceptionGetAttr );

	// We can now register an exception translator to convert from C++ `T` instances,
	// to Python instances of our new `exceptionClass`.

	boost::python::register_exception_translator<T>(
		[exceptionClass]( const T &e ) {
			boost::python::object exceptionClassObject( boost::python::borrowed( exceptionClass ) );
			boost::python::object implementation( e );
			boost::python::object exception = exceptionClassObject( implementation );
			Py_INCREF( exception.ptr() );
			PyErr_SetObject( exceptionClass, exception.ptr() );
		}
	);
}

template<typename T>
template<typename... Args>
ExceptionClass<T> &ExceptionClass<T>::def( Args&&... args )
{
	m_implementationClass->def( std::forward<Args>( args )... );
	return *this;
}

} // namespace IECorePython

#endif // IECOREPYTHON_EXCEPTIONBINDING_INL
