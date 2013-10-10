//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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

/// This file contains the implementation of TypedData. Rather than include it
/// in a public header it is #included in SimpleTypedData.cpp and VectorTypedData.cpp,
/// and the relevant template classes are explicitly instantiated there. This prevents
/// a host of problems to do with the definition of the same symbols in multiple object
/// files.

#include <cassert>
#include <boost/type_traits/is_void.hpp>
#include "IECore/MurmurHash.h"

namespace IECore {

template<class T>
Object::TypeDescription<TypedData<T> > TypedData<T>::m_typeDescription;

//////////////////////////////////////////////////////////////////////////////////////
// helper functions
//////////////////////////////////////////////////////////////////////////////////////

namespace Detail
{

template<typename T>
inline size_t sizeOf()
{
	return sizeof( T );
}

template<>
inline size_t sizeOf<void>()
{
	return 0;
}

} // namespace Detail

//////////////////////////////////////////////////////////////////////////////////////
// constructors/destructors
//////////////////////////////////////////////////////////////////////////////////////

template<class T>
TypedData<T>::TypedData() : m_data()
{
}

template<class T>
TypedData<T>::TypedData(const T &data) : m_data ( data )
{
}

template<class T>
TypedData<T>::~TypedData()
{
}

//////////////////////////////////////////////////////////////////////////////////////
// object interface
//////////////////////////////////////////////////////////////////////////////////////

template <class T>
typename TypedData<T>::Ptr TypedData<T>::copy() const
{
	return staticPointerCast<TypedData<T> >( Data::copy() );
}

template <class T>
void TypedData<T>::copyFrom( const Object *other, CopyContext *context )
{
	Data::copyFrom( other, context );
	const TypedData<T> *tOther = static_cast<const TypedData<T> *>( other );
	m_data = tOther->m_data;
}

template <class T>
void TypedData<T>::save( SaveContext *context ) const
{
	static InternedString valueEntry("value");
	Data::save( context );
	IndexedIO *container = context->rawContainer();
	container->write( valueEntry, readable() );
}

template <class T>
void TypedData<T>::load( LoadContextPtr context )
{
	static InternedString valueEntry("value");
	Data::load( context );
	try
	{
		// optimised format for new files
		const IndexedIO *container = context->rawContainer();
		container->read( valueEntry, writable() );
	}
	catch( ... )
	{
		// backwards compatibility with old files
		unsigned int v = 0;
		ConstIndexedIOPtr container = context->container( staticTypeName(), v );
		container->read( valueEntry, writable() );
	}
}

template <class T>
bool TypedData<T>::isEqualTo( const Object *other ) const
{
	if( !Data::isEqualTo( other ) )
	{
		return false;
	}
	const TypedData<T> *tOther = static_cast<const TypedData<T> *>( other );
	return m_data == tOther->m_data;
}

template <class T>
void TypedData<T>::hash( MurmurHash &h ) const
{
	Data::hash( h );
	m_data.hash( h );
}

//////////////////////////////////////////////////////////////////////////////////////
// data access
//////////////////////////////////////////////////////////////////////////////////////

template<class T>
void TypedData<T>::operator = (const T &data)
{
	writable() = data;
}

template<class T>
void TypedData<T>::operator = (const TypedData<T> &typedData)
{
	writable() = typedData.readable();
}

template<class T>
const T & TypedData<T>::readable() const
{
	return m_data.readable();
}

template<class T>
T & TypedData<T>::writable()
{
	return m_data.writable();
}

template<class T>
void TypedData<T>::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );
	accumulator.accumulate( &readable(), sizeof( T ) );
}

//////////////////////////////////////////////////////////////////////////////////////
// low level data access
//////////////////////////////////////////////////////////////////////////////////////

template <class T>
bool TypedData<T>::hasBase()
{
	return boost::is_void<BaseType>::value == false;
}

template <class T>
size_t TypedData<T>::baseSize() const
{
	size_t sizeOfBaseType = Detail::sizeOf<BaseType>();
	if( !sizeOfBaseType )
	{
		throw Exception( std::string( staticTypeName() ) + " has no base type." );
	}

	return sizeof( T ) / sizeOfBaseType;
}

template <class T>
const typename TypedData<T>::BaseType *TypedData<T>::baseReadable() const
{
	if ( !TypedData<T>::hasBase() )
	{
		throw Exception( std::string( TypedData<T>::staticTypeName() ) + " has no base type." );
	}
	return reinterpret_cast< const typename TypedData<T>::BaseType * >( &readable() );
}

template <class T>
typename TypedData<T>::BaseType *TypedData<T>::baseWritable()
{
	if ( !TypedData<T>::hasBase() )
	{
		throw Exception( std::string( TypedData<T>::staticTypeName() ) + " has no base type." );
	}
	return reinterpret_cast< typename TypedData<T>::BaseType * >( &writable() );
}

} // namespace IECore

