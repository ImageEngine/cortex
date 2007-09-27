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

#include "IECore/SimpleTypedData.h"
#include "IECore/TypedData.inl"

#include <iostream>

namespace IECore {

#define IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( T, TID, TNAME )		\
																			\
	template<>																\
	TypeId TypedData<T>::typeId() const										\
	{																		\
		return TID;															\
	}																		\
	template<>																\
	TypeId TypedData<T>::staticTypeId()										\
	{																		\
		return TID;															\
	}																		\
	template<>																\
	std::string TypedData<T>::typeName() const								\
	{																		\
		return #TNAME;														\
	}																		\
	template<>																\
	std::string TypedData<T>::staticTypeName()								\
	{																		\
		return #TNAME;														\
	}																		\

#define IE_CORE_DEFINEIMATHTYPEDDATAIOSPECIALISATION( T, BT, N )									\
																									\
	template<>																						\
	void TypedData<T>::save( SaveContext *context ) const											\
	{																								\
		Data::save( context );																		\
		IndexedIOInterfacePtr container = context->rawContainer();									\
		container->write( "value", (const BT *)&(readable()), N );									\
	}																								\
																									\
	template<>																						\
	void TypedData<T>::load( LoadContextPtr context )												\
	{																								\
		Data::load( context );																		\
		IndexedIOInterfacePtr container;															\
		BT *p = (BT *)&(writable());																\
		try																							\
		{																							\
			container = context->rawContainer();													\
			container->read( "value", p, N );														\
		}																							\
		catch( ... )																				\
		{																							\
			unsigned int v = 0;																		\
			container = context->container( staticTypeName(), v );									\
			container->read( "value", p, N );														\
		}																							\
	}																			

#define IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( T, TID, TNAME, BT, N )							\
	IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( T, TID, TNAME )									\
	IE_CORE_DEFINEIMATHTYPEDDATAIOSPECIALISATION( T, BT, N )										\
			
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( bool, BoolDataTypeId, BoolData )
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( float, FloatDataTypeId, FloatData )
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( double, DoubleDataTypeId, DoubleData )
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( int, IntDataTypeId, IntData )
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( long, LongDataTypeId, LongData )
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( unsigned int, UIntDataTypeId, UIntData )
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( char, CharDataTypeId, CharData )
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( unsigned char, UCharDataTypeId, UCharData )
IE_CORE_DEFINESIMPLETYPEDDATASPECIALISATION( std::string, StringDataTypeId, StringData )

IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::V2i, V2iDataTypeId, V2iData, int, 2 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::V3i, V3iDataTypeId, V3iData, int, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::V2f, V2fDataTypeId, V2fData, float, 2 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::V3f, V3fDataTypeId, V3fData, float, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::V2d, V2dDataTypeId, V2dData, double, 2 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::V3d, V3dDataTypeId, V3dData, double, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Color3f, Color3fDataTypeId, Color3fData, float, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Color4f, Color4fDataTypeId, Color4fData, float, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Color3<double>, Color3dDataTypeId, Color3dData, double, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Color4<double>, Color4dDataTypeId, Color4dData, double, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Box2i, Box2iDataTypeId, Box2iData, int, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Box3i, Box3iDataTypeId, Box3iData, int, 6 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Box2f, Box2fDataTypeId, Box2fData, float, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Box3f, Box3fDataTypeId, Box3fData, float, 6 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Box2d, Box2dDataTypeId, Box2dData, double, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Box3d, Box3dDataTypeId, Box3dData, double, 6 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::M33f, M33fDataTypeId, M33fData, float, 9 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::M33d, M33dDataTypeId, M33dData, double, 9 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::M44f, M44fDataTypeId, M44fData, float, 16 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::M44d, M44dDataTypeId, M44dData, double, 16 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Quatf, QuatfDataTypeId, QuatfData, float, 4 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( Imath::Quatd, QuatdDataTypeId, QuatdData, double, 4 )

template<>
void StringData::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );
	accumulator.accumulate( &readable(), readable().capacity() );
}

template<> 
void TypedData<bool>::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIOInterfacePtr container = context->rawContainer();
	unsigned char c = readable();
	container->write( "value", c );
}

template<> 
void TypedData<bool>::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned char c;
	try
	{
		// optimised format for new files
		IndexedIOInterfacePtr container = context->rawContainer();
		container->read( "value", c );
	}
	catch( ... )
	{
		// backwards compatibility with old files
		unsigned int v = 0;
		IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
		container->read( "value", c );
	}
	
	writable() = c;
}

template class TypedData<bool>;
template class TypedData<float>;
template class TypedData<double>;
template class TypedData<int>;
template class TypedData<long>;
template class TypedData<unsigned int>;
template class TypedData<char>;
template class TypedData<unsigned char>;
template class TypedData<std::string>;
template class TypedData<Imath::V2i>;
template class TypedData<Imath::V3i>;
template class TypedData<Imath::V2f>;
template class TypedData<Imath::V3f>;
template class TypedData<Imath::V2d>;
template class TypedData<Imath::V3d>;
template class TypedData<Imath::Color3f>;
template class TypedData<Imath::Color4f>;
template class TypedData<Imath::Color3<double> >;
template class TypedData<Imath::Color4<double> >;
template class TypedData<Imath::Box2i>;
template class TypedData<Imath::Box3i>;
template class TypedData<Imath::Box2f>;
template class TypedData<Imath::Box3f>;
template class TypedData<Imath::Box2d>;
template class TypedData<Imath::Box3d>;
template class TypedData<Imath::M33f>;
template class TypedData<Imath::M33d>;
template class TypedData<Imath::M44f>;
template class TypedData<Imath::M44d>;
template class TypedData<Imath::Quatf>;
template class TypedData<Imath::Quatd>;

} // namespace IECore
