//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "IECore/VectorTypedData.h"
#include "IECore/TypedData.inl"

using namespace Imath;
using namespace std;
using namespace IECore;

LongVectorDataAlias::TypeDescription<IntVectorData> LongVectorDataAlias::m_typeDescription( LongVectorDataTypeId, "LongVectorData" );

#define IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( TNAME )										\
	template<>																								\
	void TNAME::memoryUsage( Object::MemoryAccumulator &accumulator ) const			\
	{																										\
		Data::memoryUsage( accumulator );																	\
		accumulator.accumulate( &readable(), sizeof( TNAME::ValueType ) + readable().capacity() * sizeof( TNAME::ValueType::value_type ) );	\
	}																										\

#define IE_CORE_DEFINEVECTORTYPEDDATATRAITSSPECIALIZATION( TNAME )											\
	template <>																									\
	unsigned long TNAME::baseSize()	 const																		\
	{																											\
		if ( !TNAME::hasBase() )																				\
		{																										\
			throw Exception( TNAME::staticTypeName() + " has no base type." );									\
		}																										\
		return ( sizeof( TNAME::ValueType::value_type ) / sizeof( TNAME::BaseType ) ) * this->readable().size();	\
	}																											\
	template <>																									\
	const TNAME::BaseType * TNAME::baseReadable() const															\
	{																											\
		if ( !TNAME::hasBase() )																				\
		{																										\
			throw Exception( TNAME::staticTypeName() + " has no base type." );									\
		}																										\
		return reinterpret_cast< const TNAME::BaseType * >( &(this->readable()[0]) );							\
	}																											\
	template <>																									\
	TNAME::BaseType * TNAME::baseWritable()																		\
	{																											\
		if ( !TNAME::hasBase() )																				\
		{																										\
			throw Exception( TNAME::staticTypeName() + " has no base type." );									\
		}																										\
		return reinterpret_cast< TNAME::BaseType * >( &(this->writable()[0]) );									\
	}																											\

#define IE_CORE_DEFINENOBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME )								\
	template<>																						\
	void TNAME::save( Object::SaveContext *context ) const											\
	{																								\
		Data::save( context );																		\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );				\
		container->write( "value", &(readable()[0]), readable().size() );							\
	}																								\
	template<>																						\
	void TNAME::load( LoadContextPtr context )														\
	{																								\
		Data::load( context );																		\
		unsigned int v = 0;																			\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), v );				\
		IndexedIO::Entry e = container->ls( "value" );												\
		writable().resize( e.arrayLength() );														\
		TNAME::ValueType::value_type *p = &(writable()[0]);											\
		container->read( "value", p, e.arrayLength() );												\
	}																								\
	
#define IE_CORE_DEFINEBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME, N )								\
	template<>																						\
	void TNAME::save( SaveContext *context ) const													\
	{																								\
		Data::save( context );																		\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );				\
		assert( ( sizeof( TNAME::ValueType::value_type ) / sizeof( TNAME::BaseType ) ) == N );		\
		container->write( "value", baseReadable(), baseSize() );									\
	}																								\
	template<>																						\
	void TNAME::load( LoadContextPtr context )														\
	{																								\
		Data::load( context );																		\
		unsigned int v = 0;																			\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), v );				\
		IndexedIO::Entry e = container->ls( "value" );												\
		writable().resize( e.arrayLength() / N );													\
		TNAME::BaseType *p = baseWritable();														\
		container->read( "value", p, e.arrayLength() );												\
	}	
	
#define IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( TNAME, TID )			\
	IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( TNAME, TID )					\
	IE_CORE_DEFINEVECTORTYPEDDATATRAITSSPECIALIZATION( TNAME )					\
	IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( TNAME )				\
	IE_CORE_DEFINEBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME, 1)				\

#define IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( TNAME, TID, N )		\
	IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( TNAME, TID )					\
	IE_CORE_DEFINEVECTORTYPEDDATATRAITSSPECIALIZATION( TNAME )					\
	IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( TNAME )				\
	IE_CORE_DEFINEBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME, N )				\
	
namespace IECore
{
// specialisation definitions for vector types...
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( FloatVectorData, FloatVectorDataTypeId )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( HalfVectorData, HalfVectorDataTypeId )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( DoubleVectorData, DoubleVectorDataTypeId )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( IntVectorData, IntVectorDataTypeId )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( UIntVectorData, UIntVectorDataTypeId )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( CharVectorData, CharVectorDataTypeId )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( UCharVectorData, UCharVectorDataTypeId )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V2fVectorData, V2fVectorDataTypeId, 2 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V2dVectorData, V2dVectorDataTypeId, 2 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V2iVectorData, V2iVectorDataTypeId, 2 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V3fVectorData, V3fVectorDataTypeId, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V3dVectorData, V3dVectorDataTypeId, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V3iVectorData, V3iVectorDataTypeId, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box2fVectorData, Box2fVectorDataTypeId, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box2dVectorData, Box2dVectorDataTypeId, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box3fVectorData, Box3fVectorDataTypeId, 6 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box3dVectorData, Box3dVectorDataTypeId, 6 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( M33fVectorData, M33fVectorDataTypeId, 9 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( M33dVectorData, M33dVectorDataTypeId, 9 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( M44fVectorData, M44fVectorDataTypeId, 16 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( M44dVectorData, M44dVectorDataTypeId, 16 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( QuatfVectorData, QuatfVectorDataTypeId, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( QuatdVectorData, QuatdVectorDataTypeId, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Color3fVectorData, Color3fVectorDataTypeId, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Color4fVectorData, Color4fVectorDataTypeId, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Color3dVectorData, Color3dVectorDataTypeId, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Color4dVectorData, Color4dVectorDataTypeId, 4 )

// the string type needs it's own memoryUsage so we don't use the whole macro for it's specialisations

IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( StringVectorData, StringVectorDataTypeId )
IE_CORE_DEFINETYPEDDATANOBASESIZE( StringVectorData )
IE_CORE_DEFINENOBASEVECTORTYPEDDATAIOSPECIALISATION( StringVectorData )

template<>
void StringVectorData::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );
	
	size_t count = 0;
	const std::vector< std::string > &vector = m_data->data;
	std::vector< std::string >::const_iterator iterV = vector.begin();
	while (iterV != vector.end())
	{
		count += iterV->capacity();
		iterV++;
	}
	accumulator.accumulate( &readable(), sizeof(std::vector<string>) + count );
}

// the boolean type need it's own io and memoryUsage so we don't use the whole macro for it's specialisations either
IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( BoolVectorData, BoolVectorDataTypeId )
IE_CORE_DEFINETYPEDDATANOBASESIZE( BoolVectorData )

// short and unsigned short data types save/load themelves as int and unsigned int arrays, respectively.
IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( ShortVectorData, ShortVectorDataTypeId )
IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( UShortVectorData, UShortVectorDataTypeId )


template<>
void BoolVectorData::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );
	accumulator.accumulate( &readable(), sizeof(std::vector<bool>) + readable().capacity() / 8 );
}

template<>
void BoolVectorData::save( Object::SaveContext *context ) const
{
	Data::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );
	// we can't write out the raw data from inside the vector 'cos it's specialised
	// to optimise for space, and that means the only access to the data is through
	// a funny proxy class. so we repack the data into something we can deal with
	// and write that out instead. essentially i think we're making exactly the same
	// raw data. rubbish.
	const std::vector<bool> &b = readable();
	std::vector<unsigned char> p;
	unsigned int s = b.size();
	p.resize( s/8 + 1, 0 );

	for( unsigned int i=0; i<b.size(); i++ )
	{
		if( b[i] )
		{
			p[i/8] |= 1 << (i % 8);
		}
	}
	
	container->write( "size", s );
	container->write( "value", &(p[0]), p.size() );
}

template<>
void BoolVectorData::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = 0;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	
	unsigned int s = 0;
	container->read( "size", s );
	std::vector<unsigned char> p;
	p.resize( s / 8 + 1 );
	unsigned char *value = &(p[0]);
	container->read( "value", value, p.size() );
	std::vector<bool> &b = writable();
	b.resize( s, false );
	for( unsigned int i=0; i<s; i++ )
	{
		b[i] = ( p[i/8] >> (i % 8) ) & 1;
	}
}

template<>
void ShortVectorData::save( Object::SaveContext *context ) const
{
	Data::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );
	// we can't write out the raw data from inside the vector 'cos it's specialised
	// to optimise for space, and that means the only access to the data is through
	// a funny proxy class. so we repack the data into something we can deal with
	// and write that out instead. essentially i think we're making exactly the same
	// raw data. rubbish.
	const std::vector<short> &b = readable();
	std::vector<int> p;
	unsigned int s = b.size();
	p.resize(s);

	for( unsigned int i=0; i<s; i++ )
	{
		p[i] = b[i];
	}
	
	container->write( "size", s );
	container->write( "value", &(p[0]), p.size() );
}

template<>
void ShortVectorData::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = 0;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	
	unsigned int s = 0;
	container->read( "size", s );
	std::vector<int> p;
	p.resize( s );
	int *value = &(p[0]);
	container->read( "value", value, p.size() );
	std::vector<short> &b = writable();
	b.resize( s, false );
	for( unsigned int i=0; i<s; i++ )
	{
		b[i] = static_cast< short > ( p[i] );
	}
}

template<>
void UShortVectorData::save( Object::SaveContext *context ) const
{
	Data::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );
	// we can't write out the raw data from inside the vector 'cos it's specialised
	// to optimise for space, and that means the only access to the data is through
	// a funny proxy class. so we repack the data into something we can deal with
	// and write that out instead. essentially i think we're making exactly the same
	// raw data. rubbish.
	const std::vector<unsigned short> &b = readable();
	std::vector<unsigned int> p;
	unsigned int s = b.size();
	p.resize(s);

	for( unsigned int i=0; i<s; i++ )
	{
		p[i] = b[i];
	}
	
	container->write( "size", s );
	container->write( "value", &(p[0]), p.size() );
}

template<>
void UShortVectorData::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = 0;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	
	unsigned int s = 0;
	container->read( "size", s );
	std::vector<unsigned int> p;
	p.resize( s );
	unsigned int *value = &(p[0]);
	container->read( "value", value, p.size() );
	std::vector<unsigned short> &b = writable();
	b.resize( s, false );
	for( unsigned int i=0; i<s; i++ )
	{
		b[i] = static_cast< unsigned short > ( p[i] );
	}
}
	
// explicitly instantiate each of the types we defined in the public ui.
template class TypedData<vector<float> >;
template class TypedData<vector<bool> >;
template class TypedData<vector<half> >;
template class TypedData<vector<double> >;
template class TypedData<vector<int> >;
template class TypedData<vector<unsigned int> >;
template class TypedData<vector<char> >;
template class TypedData<vector<unsigned char> >;
template class TypedData<vector<short> >;
template class TypedData<vector<unsigned short> >;
template class TypedData<vector<V2f> >;
template class TypedData<vector<V2d> >;
template class TypedData<vector<V2i> >;
template class TypedData<vector<V3f> >;
template class TypedData<vector<V3d> >;
template class TypedData<vector<V3i> >;
template class TypedData<vector<Color3f> >;
template class TypedData<vector<Color4f> >;
template class TypedData<vector<Color3<double> > >;
template class TypedData<vector<Color4<double> > >;
template class TypedData<vector<Box2f> >;
template class TypedData<vector<Box2d> >;
template class TypedData<vector<Box3f> >;
template class TypedData<vector<Box3d> >;
template class TypedData<vector<M33f> >;
template class TypedData<vector<M33d> >;
template class TypedData<vector<M44f> >;
template class TypedData<vector<M44d> >;
template class TypedData<vector<Quatf> >;
template class TypedData<vector<Quatd> >;
template class TypedData<vector<string> >;

} // namespace IECore
