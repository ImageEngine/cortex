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

#ifndef IECOREPYTHON_RUNTIMETYPEDBINDING_INL
#define IECOREPYTHON_RUNTIMETYPEDBINDING_INL

#include "IECorePython/ExceptionAlgo.h"
#include "IECorePython/Export.h"
#include "IECorePython/RefCountedBinding.h"

#include "IECore/RunTimeTyped.h"

namespace IECorePython
{

namespace Detail
{

IECOREPYTHON_API const char *nameWithoutNamespace( const char *name );

template<class T>
static IECore::TypeId typeId( T &t )
{
	return t.T::typeId();
}

template<class T>
static const char *typeName( T &t )
{
	return t.T::typeName();
}

template<class T>
static bool isInstanceOf( T &t, IECore::TypeId i )
{
	return t.T::isInstanceOf( i );
}

template<class T>
static bool isInstanceOf2( T &t, const char *n )
{
	return t.T::isInstanceOf( n );
}

} // namespace Detail

//////////////////////////////////////////////////////////////////////////
// RunTimeTypedWrapper
//////////////////////////////////////////////////////////////////////////

template<typename T>
template<typename... Args>
RunTimeTypedWrapper<T>::RunTimeTypedWrapper( PyObject *self, Args&&... args )
	:	RefCountedWrapper<T>( self, std::forward<Args>( args )... )
{
}

template<typename T>
IECore::TypeId RunTimeTypedWrapper<T>::typeId() const
{
	if( this->isSubclassed() )
	{
		IECorePython::ScopedGILLock gilLock;
		try
		{
			if( boost::python::object f = this->methodOverride( "typeId" ) )
			{
				boost::python::object res = f();
				return boost::python::extract<IECore::TypeId>( res );
			}
		}
		catch( const boost::python::error_already_set &e )
		{
			ExceptionAlgo::translatePythonException();
		}
	}
	return T::typeId();
}

template<typename T>
const char *RunTimeTypedWrapper<T>::typeName() const
{
	if( this->isSubclassed() )
	{
		IECorePython::ScopedGILLock gilLock;
		try
		{
			if( boost::python::object f = this->methodOverride( "typeName" ) )
			{
				boost::python::object res = f();
				return boost::python::extract<const char *>( res );
			}
		}
		catch( const boost::python::error_already_set &e )
		{
			ExceptionAlgo::translatePythonException();
		}
	}
	return T::typeName();
}

template<typename T>
bool RunTimeTypedWrapper<T>::isInstanceOf( IECore::TypeId typeId ) const
{
	if( T::isInstanceOf( typeId ) )
	{
		return true;
	}

	if( this->isSubclassed() )
	{
		IECorePython::ScopedGILLock gilLock;
		try
		{
			if( boost::python::object f = this->methodOverride( "isInstanceOf" ) )
			{
				boost::python::object res = f( typeId );
				return boost::python::extract<bool>( res );
			}
		}
		catch( const boost::python::error_already_set &e )
		{
			ExceptionAlgo::translatePythonException();
		}
	}
	return false;
}

template<typename T>
bool RunTimeTypedWrapper<T>::isInstanceOf( const char *typeName ) const
{
	if( T::isInstanceOf( typeName ) )
	{
		return true;
	}

	if( this->isSubclassed() )
	{
		IECorePython::ScopedGILLock gilLock;
		try
		{
			if( boost::python::object f = this->methodOverride( "isInstanceOf" ) )
			{
				boost::python::object res = f( typeName );
				return boost::python::extract<bool>( res );
			}
		}
		catch( const boost::python::error_already_set &e )
		{
			ExceptionAlgo::translatePythonException();
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// RunTimeTypedClass
//////////////////////////////////////////////////////////////////////////

template<typename T, typename TWrapper>
RunTimeTypedClass<T, TWrapper>::RunTimeTypedClass( const char *docString )
	:	BaseClass( Detail::nameWithoutNamespace( T::staticTypeName() ), docString )
{

	BaseClass::BaseClass::def( "typeId", &Detail::typeId<T> );
	BaseClass::BaseClass::def( "typeName", &Detail::typeName<T> );
	BaseClass::BaseClass::def( "isInstanceOf", &Detail::isInstanceOf<T> );
	BaseClass::BaseClass::def( "isInstanceOf", &Detail::isInstanceOf2<T> );

	BaseClass::BaseClass::def( "staticTypeName", &T::staticTypeName );
	BaseClass::BaseClass::staticmethod( "staticTypeName" );

	BaseClass::BaseClass::def( "staticTypeId", &T::staticTypeId );
	BaseClass::BaseClass::staticmethod( "staticTypeId" );

	BaseClass::BaseClass::def( "baseTypeId", ( IECore::TypeId (*)( IECore::TypeId ) )( &IECore::RunTimeTyped::baseTypeId ) );
	BaseClass::BaseClass::def( "baseTypeId", ( IECore::TypeId (*)() )&T::baseTypeId );
	BaseClass::BaseClass::staticmethod( "baseTypeId" );

	BaseClass::BaseClass::def( "baseTypeName", &T::baseTypeName );
	BaseClass::BaseClass::staticmethod( "baseTypeName" );

	BaseClass::BaseClass::def( "inheritsFrom", (bool (*)( const char * ) )&T::inheritsFrom );
	BaseClass::BaseClass::def( "inheritsFrom", (bool (*)( IECore::TypeId ) )&T::inheritsFrom );
	BaseClass::BaseClass::def( "inheritsFrom", (bool (*)( const char *, const char * ) )&IECore::RunTimeTyped::inheritsFrom );
	BaseClass::BaseClass::def( "inheritsFrom", (bool (*)( IECore::TypeId, IECore::TypeId ) )&IECore::RunTimeTyped::inheritsFrom );
	BaseClass::BaseClass::staticmethod( "inheritsFrom" );

}

} // namespace IECorePython

#endif // IECOREPYTHON_RUNTIMETYPEDBINDING_INL
