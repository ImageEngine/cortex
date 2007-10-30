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

namespace IECore 
{

#define IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( TNAME, TID )			\
	IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( TNAME, TID )			\

#define IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( TNAME, TID, N )		\
	IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( TNAME, TID )			\
	IE_CORE_DEFINEBASETYPEDDATAIOSPECIALISATION( TNAME, N )				\
			

IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( BoolData, BoolDataTypeId )
IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( FloatData, FloatDataTypeId )
IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( DoubleData, DoubleDataTypeId )
IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( IntData, IntDataTypeId )
IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( LongData, LongDataTypeId )
IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( UIntData, UIntDataTypeId )
IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( CharData, CharDataTypeId )
IE_CORE_DEFINEBASETYPEDDATASPECIALISATION( UCharData, UCharDataTypeId )

IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( StringData, StringDataTypeId )
IE_CORE_DEFINETYPEDDATANOBASESIZE( StringData )

IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( V2iData, V2iDataTypeId, 2 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( V3iData, V3iDataTypeId, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( V2fData, V2fDataTypeId, 2 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( V3fData, V3fDataTypeId, 3 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( V2dData, V2dDataTypeId, 2 )
IE_CORE_DEFINEIMATHTYPEDDATASPECIALISATION( V3dData, V3dDataTypeId, 3 )
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
