//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/VectorTypedData.h"
#include "IECore/TypedData.inl"

using namespace Imath;
using namespace std;
using namespace IECore;

#define IE_CORE_DEFINEVECTORTYPEDDATACOMMONSPECIALISATION( T, TID, TNAME )		\
	template<>																	\
	TypeId TypedData< std::vector< T > >::typeId() const						\
	{																			\
		return TID;																\
	}																			\
	template<>																	\
	TypeId TypedData< std::vector< T > >::staticTypeId()						\
	{																			\
		return TID;																\
	}																			\
	template<>																	\
	std::string TypedData< std::vector< T > >::typeName() const					\
	{																			\
		return #TNAME;															\
	}																			\
	template<>																	\
	std::string TypedData< std::vector< T > >::staticTypeName()					\
	{																			\
		return #TNAME;															\
	}																			\

#define IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( T )										\
	template<>																							\
	void TypedData< std::vector< T > >::memoryUsage( Object::MemoryAccumulator &accumulator ) const		\
	{																									\
		Data::memoryUsage( accumulator );																\
		accumulator.accumulate( &readable(), sizeof(std::vector<T>) + readable().capacity() * sizeof(T) );	\
	}																									\

#define IE_CORE_DEFINESIMPLEVECTORTYPEDDATAIOSPECIALISATION( T )									\
	template<>																						\
	void TypedData<std::vector<T> >::save( Object::SaveContext *context ) const						\
	{																								\
		Data::save( context );																		\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );				\
		container->write( "value", &(readable()[0]), readable().size() );							\
	}																								\
	template<>																						\
	void TypedData<std::vector<T> >::load( LoadContextPtr context )									\
	{																								\
		Data::load( context );																		\
		unsigned int v = 0;																			\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), v );				\
		IndexedIO::Entry e = container->ls( "value" );												\
		writable().resize( e.arrayLength() );														\
		T *p = &(writable()[0]);																	\
		container->read( "value", p, e.arrayLength() );												\
	}																								\
	
#define IE_CORE_DEFINEIMATHVECTORTYPEDDATAIOSPECIALISATION( T, BT, N )								\
	template<>																						\
	void TypedData<std::vector<T> >::save( SaveContext *context ) const								\
	{																								\
		Data::save( context );																		\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );				\
		container->write( "value", (const BT *)&(readable()[0]), readable().size() * N );			\
	}																								\
	template<>																						\
	void TypedData<std::vector<T> >::load( LoadContextPtr context )									\
	{																								\
		Data::load( context );																		\
		unsigned int v = 0;																			\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), v );				\
		IndexedIO::Entry e = container->ls( "value" );												\
		writable().resize( e.arrayLength() / N );													\
		BT *p = (BT *)&(writable()[0]);																\
		container->read( "value", p, e.arrayLength() );												\
	}	
	
#define IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( T, TID, TNAME )		\
	IE_CORE_DEFINEVECTORTYPEDDATACOMMONSPECIALISATION( T, TID, TNAME )			\
	IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( T )					\
	IE_CORE_DEFINESIMPLEVECTORTYPEDDATAIOSPECIALISATION( T)						\

#define IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( T, TID, TNAME, BT, N )	\
	IE_CORE_DEFINEVECTORTYPEDDATACOMMONSPECIALISATION( T, TID, TNAME )				\
	IE_CORE_DEFINEVECTORTYPEDDATAMEMUSAGESPECIALISATION( T )						\
	IE_CORE_DEFINEIMATHVECTORTYPEDDATAIOSPECIALISATION( T, BT, N )					\
	
namespace IECore
{
// specialisation definitions for vector types...
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( half, HalfVectorDataTypeId, HalfVectorData )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( float, FloatVectorDataTypeId, FloatVectorData )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( double, DoubleVectorDataTypeId, DoubleVectorData )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( int, IntVectorDataTypeId, IntVectorData )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( unsigned int, UIntVectorDataTypeId, UIntVectorData )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( long, LongVectorDataTypeId, LongVectorData )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( char, CharVectorDataTypeId, CharVectorData )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATASPECIALISATION( unsigned char, UCharVectorDataTypeId, UCharVectorData )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V2f, V2fVectorDataTypeId, V2fVectorData, float, 2 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V2d, V2dVectorDataTypeId, V2dVectorData, double, 2 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V3f, V3fVectorDataTypeId, V3fVectorData, float, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( V3d, V3dVectorDataTypeId, V3dVectorData, double, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box2f, Box2fVectorDataTypeId, Box2fVectorData, float, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box2d, Box2dVectorDataTypeId, Box2dVectorData, double, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box3f, Box3fVectorDataTypeId, Box3fVectorData, float, 6 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Box3d, Box3dVectorDataTypeId, Box3dVectorData, double, 6 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( M33f, M33fVectorDataTypeId, M33fVectorData, float, 9 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( M33d, M33dVectorDataTypeId, M33dVectorData, double, 9 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( M44f, M44fVectorDataTypeId, M44fVectorData, float, 16 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( M44d, M44dVectorDataTypeId, M44dVectorData, double, 16 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Quatf, QuatfVectorDataTypeId, QuatfVectorData, float, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Quatd, QuatdVectorDataTypeId, QuatdVectorData, double, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Color3f, Color3fVectorDataTypeId, Color3fVectorData, float, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Color4f, Color4fVectorDataTypeId, Color4fVectorData, float, 4 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Color3<double>, Color3dVectorDataTypeId, Color3dVectorData, double, 3 )
IE_CORE_DEFINEIMATHVECTORTYPEDDATASPECIALISATION( Color4<double>, Color4dVectorDataTypeId, Color4dVectorData, double, 4 )

// the string type needs it's own memoryUsage so we don't use the whole macro for it's specialisations

IE_CORE_DEFINEVECTORTYPEDDATACOMMONSPECIALISATION( std::string, StringVectorDataTypeId, StringVectorData )
IE_CORE_DEFINESIMPLEVECTORTYPEDDATAIOSPECIALISATION( string )

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

IE_CORE_DEFINEVECTORTYPEDDATACOMMONSPECIALISATION( bool, BoolVectorDataTypeId, BoolVectorData )

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
	
// explicitly instantiate each of the types we defined in the public ui.
template class TypedData<vector<bool> >;
template class TypedData<vector<half> >;
template class TypedData<vector<float> >;
template class TypedData<vector<double> >;
template class TypedData<vector<int> >;
template class TypedData<vector<unsigned int> >;
template class TypedData<vector<long> >;
template class TypedData<vector<char> >;
template class TypedData<vector<unsigned char> >;
template class TypedData<vector<V2f> >;
template class TypedData<vector<V2d> >;
template class TypedData<vector<V3f> >;
template class TypedData<vector<V3d> >;
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
