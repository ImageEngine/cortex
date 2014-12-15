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

#include "IECore/SimpleTypedData.h"
#include "IECore/Export.h"
#include "IECore/TypedData.inl"

#include <iostream>

namespace IECore
{

static IndexedIO::EntryID g_valueEntry("value");

LongDataAlias::TypeDescription<IntData> LongDataAlias::m_typeDescription( LongDataTypeId, "LongData" );

#define IE_CORE_DEFINEBASETYPEDDATAIOSPECIALISATION( TNAME, N, FALLBACKNAME )	\
	template<> \
	void TNAME::save( SaveContext *context ) const \
	{ \
		Data::save( context ); \
		assert( baseSize() == N ); \
		IndexedIO *container = context->rawContainer(); \
		container->write( g_valueEntry, TNAME::baseReadable(), TNAME::baseSize() ); \
	} \
	\
	template<> \
	void TNAME::load( LoadContextPtr context ) \
	{ \
		Data::load( context ); \
		assert( ( sizeof( TNAME::ValueType ) / sizeof( TNAME::BaseType ) ) == N ); \
		TNAME::BaseType *p = TNAME::baseWritable(); \
		try \
		{ \
			const IndexedIO *container = context->rawContainer(); \
			container->read( g_valueEntry, p, N ); \
		} \
		catch( ... ) \
		{ \
			unsigned int v = 0;	\
			ConstIndexedIOPtr container = context->container( FALLBACKNAME::staticTypeName(), v ); \
			container->read( g_valueEntry, p, N ); \
		} \
	}

#define IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( TNAME ) \
	template<> \
	TNAME::TypedData() \
		:	m_data( ValueType( 0 ) ) \
	{ \
	}

#define IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( TNAME, TID, N ) \
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TID ) \
	IE_CORE_DEFINEBASETYPEDDATAIOSPECIALISATION( TNAME, N, TNAME ) \

#define IE_CORE_DEFINEIMATHGEOMETRICTYPEDDATASPECIALISATION( TNAME, TID, BTID, N ) \
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME ## Base, BTID ) \
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TID ) \
	IE_CORE_DEFINEBASETYPEDDATAIOSPECIALISATION( TNAME ## Base, N, TNAME ) \

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( BoolData, BoolDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FloatData, FloatDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( DoubleData, DoubleDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( IntData, IntDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( UIntData, UIntDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( CharData, CharDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( UCharData, UCharDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( HalfData, HalfDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ShortData, ShortDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( UShortData, UShortDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( Int64Data, Int64DataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( UInt64Data, UInt64DataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( StringData, StringDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( InternedStringData, InternedStringDataTypeId )

IE_CORE_DEFINEIMATHGEOMETRICTYPEDDATASPECIALISATION( V2iData, V2iDataTypeId, V2iDataBaseTypeId, 2 )
IE_CORE_DEFINEIMATHGEOMETRICTYPEDDATASPECIALISATION( V3iData, V3iDataTypeId, V3iDataBaseTypeId, 3 )
IE_CORE_DEFINEIMATHGEOMETRICTYPEDDATASPECIALISATION( V2fData, V2fDataTypeId, V2fDataBaseTypeId, 2 )
IE_CORE_DEFINEIMATHGEOMETRICTYPEDDATASPECIALISATION( V3fData, V3fDataTypeId, V3fDataBaseTypeId, 3 )
IE_CORE_DEFINEIMATHGEOMETRICTYPEDDATASPECIALISATION( V2dData, V2dDataTypeId, V2dDataBaseTypeId, 2 )
IE_CORE_DEFINEIMATHGEOMETRICTYPEDDATASPECIALISATION( V3dData, V3dDataTypeId, V3dDataBaseTypeId, 3 )

IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Color3fData, Color3fDataTypeId, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Color4fData, Color4fDataTypeId, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Color3dData, Color3dDataTypeId, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Color4dData, Color4dDataTypeId, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Box2iData, Box2iDataTypeId, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Box3iData, Box3iDataTypeId, 6 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Box2fData, Box2fDataTypeId, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Box3fData, Box3fDataTypeId, 6 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Box2dData, Box2dDataTypeId, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Box3dData, Box3dDataTypeId, 6 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( M33fData, M33fDataTypeId, 9 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( M33dData, M33dDataTypeId, 9 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( M44fData, M44fDataTypeId, 16 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( M44dData, M44dDataTypeId, 16 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( QuatfData, QuatfDataTypeId, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( QuatdData, QuatdDataTypeId, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( LineSegment3fData, LineSegment3fDataTypeId, 6 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( LineSegment3dData, LineSegment3dDataTypeId, 6 )

IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( V2iDataBase )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( V3iDataBase )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( V2fDataBase )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( V3fDataBase )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( V2dDataBase )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( V3dDataBase )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( Color3fData )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( Color4fData )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( Color3dData )
IECORE_DEFINE_ZERO_INITIALISED_CONSTRUCTOR( Color4dData )

template<>
void StringData::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );
	accumulator.accumulate( readable().capacity() );
}

template<>
void TypedData<InternedString>::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIO *container = context->rawContainer();
	container->write( g_valueEntry, readable().value() );
}

template<>
void TypedData<InternedString>::load( LoadContextPtr context )
{
	Data::load( context );
	std::string v;
	const IndexedIO *container = context->rawContainer();
	container->read( g_valueEntry, v );
	writable() = v;
}

template<>
void TypedData<bool>::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIO *container = context->rawContainer();
	unsigned char c = readable();
	container->write( g_valueEntry, c );
}

template<>
void TypedData<bool>::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned char c;
	try
	{
		// optimised format for new files
		const IndexedIO *container = context->rawContainer();
		container->read( g_valueEntry, c );
	}
	catch( ... )
	{
		// backwards compatibility with old files
		unsigned int v = 0;
		ConstIndexedIOPtr container = context->container( staticTypeName(), v );
		container->read( g_valueEntry, c );
	}

	writable() = c;
}

template<>
void TypedData<short>::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIO *container = context->rawContainer();
	int c = readable();
	container->write( g_valueEntry, c );
}

template<>
void TypedData<short>::load( LoadContextPtr context )
{
	Data::load( context );
	int c;
	try
	{
		// optimised format for new files
		const IndexedIO *container = context->rawContainer();
		container->read( g_valueEntry, c );
	}
	catch( ... )
	{
		// backwards compatibility with old files
		unsigned int v = 0;
		ConstIndexedIOPtr container = context->container( staticTypeName(), v );
		container->read( g_valueEntry, c );
	}

	writable() = static_cast<short>( c );
}

template<>
void TypedData<unsigned short>::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIO *container = context->rawContainer();
	unsigned int c = readable();
	container->write( g_valueEntry, c );
}

template<>
void TypedData<unsigned short>::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int c;
	try
	{
		// optimised format for new files
		const IndexedIO *container = context->rawContainer();
		container->read( g_valueEntry, c );
	}
	catch( ... )
	{
		// backwards compatibility with old files
		unsigned int v = 0;
		ConstIndexedIOPtr container = context->container( staticTypeName(), v );
		container->read( g_valueEntry, c );
	}

	writable() = static_cast<unsigned short>( c );
}

template<>
TypedData<LineSegment3f>::TypedData()
	:	m_data( LineSegment3f( Imath::V3f( 0 ), Imath::V3f( 1, 0, 0 ) ) )
{
}

template<>
TypedData<LineSegment3d>::TypedData()
	:	m_data( LineSegment3d( Imath::V3d( 0 ), Imath::V3d( 1, 0, 0 ) ) )
{
}

template<>
void SimpleDataHolder<LineSegment3f>::hash( MurmurHash &h ) const
{
	h.append( readable().p0 );
	h.append( readable().p1 );
}

template<>
void SimpleDataHolder<LineSegment3d>::hash( MurmurHash &h ) const
{
	h.append( readable().p0 );
	h.append( readable().p1 );
}

template class IECORE_API TypedData<bool>;
template class IECORE_API TypedData<float>;
template class IECORE_API TypedData<double>;
template class IECORE_API TypedData<int>;
template class IECORE_API TypedData<unsigned int>;
template class IECORE_API TypedData<char>;
template class IECORE_API TypedData<unsigned char>;
template class IECORE_API TypedData<short>;
template class IECORE_API TypedData<unsigned short>;
template class IECORE_API TypedData<int64_t>;
template class IECORE_API TypedData<uint64_t>;
template class IECORE_API TypedData<std::string>;
template class IECORE_API TypedData<InternedString>;
template class IECORE_API TypedData<half>;

template class IECORE_API TypedData<Imath::V2i>;
template class IECORE_API TypedData<Imath::V3i>;
template class IECORE_API TypedData<Imath::V2f>;
template class IECORE_API TypedData<Imath::V3f>;
template class IECORE_API TypedData<Imath::V2d>;
template class IECORE_API TypedData<Imath::V3d>;

template class IECORE_API GeometricTypedData<Imath::V2i>;
template class IECORE_API GeometricTypedData<Imath::V3i>;
template class IECORE_API GeometricTypedData<Imath::V2f>;
template class IECORE_API GeometricTypedData<Imath::V3f>;
template class IECORE_API GeometricTypedData<Imath::V2d>;
template class IECORE_API GeometricTypedData<Imath::V3d>;

template class IECORE_API TypedData<Imath::Color3f>;
template class IECORE_API TypedData<Imath::Color4f>;
template class IECORE_API TypedData<Imath::Color3<double> >;
template class IECORE_API TypedData<Imath::Color4<double> >;
template class IECORE_API TypedData<Imath::Box2i>;
template class IECORE_API TypedData<Imath::Box3i>;
template class IECORE_API TypedData<Imath::Box2f>;
template class IECORE_API TypedData<Imath::Box3f>;
template class IECORE_API TypedData<Imath::Box2d>;
template class IECORE_API TypedData<Imath::Box3d>;
template class IECORE_API TypedData<Imath::M33f>;
template class IECORE_API TypedData<Imath::M33d>;
template class IECORE_API TypedData<Imath::M44f>;
template class IECORE_API TypedData<Imath::M44d>;
template class IECORE_API TypedData<Imath::Quatf>;
template class IECORE_API TypedData<Imath::Quatd>;
template class IECORE_API TypedData<LineSegment3f>;
template class IECORE_API TypedData<LineSegment3d>;

} // namespace IECore
