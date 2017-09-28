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

#include "IECore/Export.h"
#include "IECore/VectorTypedData.h"
#include "IECore/TypedData.inl"

using namespace Imath;
using namespace std;
using namespace IECore;

static IndexedIO::EntryID g_valueEntry("value");
static IndexedIO::EntryID g_sizeEntry("size");

#define IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( TNAME )										\
	template<>																								\
	void TNAME::memoryUsage( Object::MemoryAccumulator &accumulator ) const			\
	{																										\
		Data::memoryUsage( accumulator );																	\
		accumulator.accumulate( &readable(), sizeof( TNAME::ValueType ) + readable().capacity() * sizeof( TNAME::ValueType::value_type ) );	\
	}																										\

#define IE_CORE_DEFINEVECTORTYPEDDATATRAITSSPECIALIZATION( TNAME )											\
	template <>																									\
	size_t TNAME::baseSize() const																		\
	{																											\
		if ( !TNAME::hasBase() )																				\
		{																										\
			throw Exception( std::string( TNAME::staticTypeName() ) + " has no base type." );									\
		}																										\
		return ( sizeof( TNAME::ValueType::value_type ) / sizeof( TNAME::BaseType ) ) * this->readable().size();	\
	}																											\
	template <>																									\
	const TNAME::BaseType * TNAME::baseReadable() const															\
	{																											\
		if ( !TNAME::hasBase() )																				\
		{																										\
			throw Exception( std::string( TNAME::staticTypeName() ) + " has no base type." );					\
		}																										\
		return reinterpret_cast< const TNAME::BaseType * >( &(this->readable()[0]) );							\
	}																											\
	template <>																									\
	TNAME::BaseType * TNAME::baseWritable()																		\
	{																											\
		if ( !TNAME::hasBase() )																				\
		{																										\
			throw Exception( std::string( TNAME::staticTypeName() ) + " has no base type." );					\
		}																										\
		return reinterpret_cast< TNAME::BaseType * >( &(this->writable()[0]) );									\
	}																											\

#define IE_CORE_DEFINENOBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME )								\
	template<>																						\
	void TNAME::save( Object::SaveContext *context ) const											\
	{																								\
		Data::save( context );																		\
		IndexedIO *container = context->rawContainer();												\
		container->write( g_valueEntry, &(readable()[0]), readable().size() );						\
	}																								\
	template<>																						\
	void TNAME::load( LoadContextPtr context )														\
	{																								\
		Data::load( context );																		\
		try																							\
		{																							\
			const IndexedIO *container = context->rawContainer();									\
			IndexedIO::Entry e = container->entry( g_valueEntry );									\
			writable().resize( e.arrayLength() );													\
			if ( e.arrayLength() ) 																	\
			{ 																						\
				TNAME::ValueType::value_type *p = &(writable()[0]); 								\
				assert( p ); 																		\
				container->read( g_valueEntry, p, e.arrayLength() ); 								\
			} 																						\
		}																							\
		catch( ... )																				\
		{																							\
			unsigned int v = 0;																		\
			ConstIndexedIOPtr container = context->container( staticTypeName(), v );				\
			IndexedIO::Entry e = container->entry( g_valueEntry );									\
			writable().resize( e.arrayLength() );													\
			if ( e.arrayLength() ) 																	\
			{ 																						\
				TNAME::ValueType::value_type *p = &(writable()[0]); 								\
				assert( p ); 																		\
				container->read( g_valueEntry, p, e.arrayLength() ); 								\
			} 																						\
		}																							\
	}																								\

#define IE_CORE_DEFINEBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME, N, FALLBACKNAME )								\
	template<>																						\
	void TNAME::save( SaveContext *context ) const													\
	{																								\
		Data::save( context );																		\
		IndexedIO *container = context->rawContainer();												\
		assert( ( sizeof( TNAME::ValueType::value_type ) / sizeof( TNAME::BaseType ) ) == N );		\
		container->write( g_valueEntry, baseReadable(), baseSize() );								\
	}																								\
	template<>																						\
	void TNAME::load( LoadContextPtr context )														\
	{																								\
		Data::load( context );																		\
		try																							\
		{																							\
			const IndexedIO *container = context->rawContainer();									\
			IndexedIO::Entry e = container->entry( g_valueEntry );									\
			writable().resize( e.arrayLength() / N );												\
			if ( e.arrayLength() ) 																	\
			{ 																						\
				TNAME::BaseType *p = baseWritable(); 												\
				assert( p ) ; 																		\
				container->read( g_valueEntry, p, e.arrayLength() ); 								\
			} 																						\
		}																							\
		catch( ... )																				\
		{																							\
			unsigned int v = 0;																		\
			ConstIndexedIOPtr container = context->container( FALLBACKNAME::staticTypeName(), v );				\
			IndexedIO::Entry e = container->entry( g_valueEntry );									\
			writable().resize( e.arrayLength() / N );												\
			if ( e.arrayLength() ) 																	\
			{ 																						\
				TNAME::BaseType *p = baseWritable(); 												\
				assert( p ) ; 																		\
				container->read( g_valueEntry, p, e.arrayLength() ); 								\
			} 																						\
		}																							\
	}

#define IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( TNAME, TID )			\
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TID )					\
	IE_CORE_DEFINEVECTORTYPEDDATATRAITSSPECIALIZATION( TNAME )					\
	IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( TNAME )				\
	IE_CORE_DEFINEBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME, 1, TNAME )				\

#define IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( TNAME, TID, N )		\
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TID )					\
	IE_CORE_DEFINEVECTORTYPEDDATATRAITSSPECIALIZATION( TNAME )					\
	IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( TNAME )				\
	IE_CORE_DEFINEBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME, N, TNAME )				\

#define IE_CORE_DEFINEIMATHGEOMETRICVECTORTYPEDDATASPECIALISATION( TNAME, TID, BTID, N )		\
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME ## Base, BTID )					\
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TID )						\
	IE_CORE_DEFINEVECTORTYPEDDATATRAITSSPECIALIZATION( TNAME ## Base )						\
	IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( TNAME ## Base )						\
	IE_CORE_DEFINEBASEVECTORTYPEDDATAIOSPECIALISATION( TNAME ## Base, N, TNAME )				\

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
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( Int64VectorData, Int64VectorDataTypeId )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( UInt64VectorData, UInt64VectorDataTypeId )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( InternedStringVectorData, InternedStringVectorDataTypeId )

IE_CORE_DEFINEIMATHGEOMETRICVECTORTYPEDDATASPECIALISATION( V2fVectorData, V2fVectorDataTypeId, V2fVectorDataBaseTypeId, 2 )
IE_CORE_DEFINEIMATHGEOMETRICVECTORTYPEDDATASPECIALISATION( V2dVectorData, V2dVectorDataTypeId, V2dVectorDataBaseTypeId, 2 )
IE_CORE_DEFINEIMATHGEOMETRICVECTORTYPEDDATASPECIALISATION( V2iVectorData, V2iVectorDataTypeId, V2iVectorDataBaseTypeId, 2 )
IE_CORE_DEFINEIMATHGEOMETRICVECTORTYPEDDATASPECIALISATION( V3fVectorData, V3fVectorDataTypeId, V3fVectorDataBaseTypeId, 3 )
IE_CORE_DEFINEIMATHGEOMETRICVECTORTYPEDDATASPECIALISATION( V3dVectorData, V3dVectorDataTypeId, V3dVectorDataBaseTypeId, 3 )
IE_CORE_DEFINEIMATHGEOMETRICVECTORTYPEDDATASPECIALISATION( V3iVectorData, V3iVectorDataTypeId, V3iVectorDataBaseTypeId, 3 )

IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box2iVectorData, Box2iVectorDataTypeId, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box2fVectorData, Box2fVectorDataTypeId, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box2dVectorData, Box2dVectorDataTypeId, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box3iVectorData, Box3iVectorDataTypeId, 6 )
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

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( StringVectorData, StringVectorDataTypeId )
IE_CORE_DEFINENOBASEVECTORTYPEDDATAIOSPECIALISATION( StringVectorData )

template<>
void StringVectorData::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );

	size_t count = 0;
	const std::vector< std::string > &vector = readable();
	std::vector< std::string >::const_iterator iterV = vector.begin();
	while (iterV != vector.end())
	{
		count += iterV->capacity();
		iterV++;
	}
	accumulator.accumulate( &readable(), sizeof(std::vector<string>) + count );
}

// the boolean type need it's own io, hash and memoryUsage so we don't use the whole macro for it's specialisations either
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( BoolVectorData, BoolVectorDataTypeId )

// short and unsigned short data types save/load themelves as int and unsigned int arrays, respectively.
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ShortVectorData, ShortVectorDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( UShortVectorData, UShortVectorDataTypeId )


template<>
void BoolVectorData::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );
	accumulator.accumulate( &readable(), sizeof(std::vector<bool>) + readable().capacity() / 8 );
}

template<>
MurmurHash SharedDataHolder<std::vector<bool> >::hash() const
{
	// we can't hash the raw data from inside the vector 'cos it's specialised
	// to optimise for space, and that means the only access to the data is through
	// a funny proxy class. so we repack the data into something we can deal with
	// and hash that instead.
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
	MurmurHash result;
	result.append( &(p[0]), p.size() );
	return result;
}

template<>
void BoolVectorData::save( Object::SaveContext *context ) const
{
	Data::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), 0 );
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

	container->write( g_sizeEntry, s );
	container->write( g_valueEntry, &(p[0]), p.size() );
}

template<>
void BoolVectorData::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = 0;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	unsigned int s = 0;
	container->read( g_sizeEntry, s );
	std::vector<unsigned char> p;
	p.resize( s / 8 + 1 );
	unsigned char *value = &(p[0]);
	container->read( g_valueEntry, value, p.size() );
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
	IndexedIOPtr container = context->container( staticTypeName(), 1 );
	container->write( g_valueEntry, baseReadable(), baseSize() );
}

template<>
void ShortVectorData::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = 0;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	std::vector<short> &b = writable();

	if ( v == 0 )
	{
		/// Version 0 stored the array of shorts as ints
		unsigned int s = 0;
		container->read( g_sizeEntry, s );
		std::vector<int> p;
		p.resize( s );
		int *value = &(p[0]);
		container->read( g_valueEntry, value, p.size() );
		b.resize( s, false );
		for( unsigned int i=0; i<s; i++ )
		{
			b[i] = static_cast< short > ( p[i] );
		}
	}
	else
	{
		/// Version 1 stores the shorts natively
		IndexedIO::Entry e = container->entry( g_valueEntry );
		writable().resize( e.arrayLength() );
		short *p = baseWritable();
		container->read( g_valueEntry, p, e.arrayLength() );
	}
}

template<>
void UShortVectorData::save( Object::SaveContext *context ) const
{
	Data::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), 1 );
	container->write( g_valueEntry, baseReadable(), baseSize() );
}

template<>
void UShortVectorData::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = 0;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	std::vector<unsigned short> &b = writable();

	if ( v == 0 )
	{
		/// Version 0 stored the array unsigned shorts as unsigned ints
		unsigned int s = 0;
		container->read( g_sizeEntry, s );
		std::vector<unsigned int> p;
		p.resize( s );
		unsigned int *value = &(p[0]);
		container->read( g_valueEntry, value, p.size() );
		b.resize( s, false );
		for( unsigned int i=0; i<s; i++ )
		{
			b[i] = static_cast< unsigned short > ( p[i] );
		}
	}
	else
	{
		/// Version 1 stores the unsigned shorts natively
		IndexedIO::Entry e = container->entry( g_valueEntry );
		writable().resize( e.arrayLength() );
		unsigned short *p = baseWritable();
		container->read( g_valueEntry, p, e.arrayLength() );
	}
}

// explicitly instantiate each of the types we defined in the public ui.
template class IECORE_API TypedData<vector<float> >;
template class IECORE_API TypedData<vector<bool> >;
template class IECORE_API TypedData<vector<half> >;
template class IECORE_API TypedData<vector<double> >;
template class IECORE_API TypedData<vector<int> >;
template class IECORE_API TypedData<vector<unsigned int> >;
template class IECORE_API TypedData<vector<char> >;
template class IECORE_API TypedData<vector<unsigned char> >;
template class IECORE_API TypedData<vector<short> >;
template class IECORE_API TypedData<vector<unsigned short> >;
template class IECORE_API TypedData<vector<int64_t> >;
template class IECORE_API TypedData<vector<uint64_t> >;

template class IECORE_API TypedData<vector<V2f> >;
template class IECORE_API TypedData<vector<V2d> >;
template class IECORE_API TypedData<vector<V2i> >;
template class IECORE_API TypedData<vector<V3f> >;
template class IECORE_API TypedData<vector<V3d> >;
template class IECORE_API TypedData<vector<V3i> >;

template class IECORE_API GeometricTypedData<vector<V2f> >;
template class IECORE_API GeometricTypedData<vector<V2d> >;
template class IECORE_API GeometricTypedData<vector<V2i> >;
template class IECORE_API GeometricTypedData<vector<V3f> >;
template class IECORE_API GeometricTypedData<vector<V3d> >;
template class IECORE_API GeometricTypedData<vector<V3i> >;

template class IECORE_API TypedData<vector<Color3f> >;
template class IECORE_API TypedData<vector<Color4f> >;
template class IECORE_API TypedData<vector<Color3<double> > >;
template class IECORE_API TypedData<vector<Color4<double> > >;
template class IECORE_API TypedData<vector<Box2i> >;
template class IECORE_API TypedData<vector<Box2f> >;
template class IECORE_API TypedData<vector<Box2d> >;
template class IECORE_API TypedData<vector<Box3i> >;
template class IECORE_API TypedData<vector<Box3f> >;
template class IECORE_API TypedData<vector<Box3d> >;
template class IECORE_API TypedData<vector<M33f> >;
template class IECORE_API TypedData<vector<M33d> >;
template class IECORE_API TypedData<vector<M44f> >;
template class IECORE_API TypedData<vector<M44d> >;
template class IECORE_API TypedData<vector<Quatf> >;
template class IECORE_API TypedData<vector<Quatd> >;
template class IECORE_API TypedData<vector<string> >;
template class IECORE_API TypedData<vector<InternedString> >;

} // namespace IECore
