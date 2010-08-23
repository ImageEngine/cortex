//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/SplineData.h"
#include "IECore/TypedData.inl"

#include <iostream>

using namespace std;
using namespace IECore;

namespace IECore
{

IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( SplineffData, SplineffDataTypeId )
IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( SplineddData, SplineddDataTypeId )
IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( SplinefColor3fData, SplinefColor3fDataTypeId )
IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( SplinefColor4fData, SplinefColor4fDataTypeId )

IE_CORE_DEFINETYPEDDATANOBASESIZE( SplineffData )
IE_CORE_DEFINETYPEDDATANOBASESIZE( SplineddData )
IE_CORE_DEFINETYPEDDATANOBASESIZE( SplinefColor3fData )
IE_CORE_DEFINETYPEDDATANOBASESIZE( SplinefColor4fData )

#define SPECIALISE( TNAME, YBASETYPE, YBASESIZE )												\
																								\
	template<>																					\
	void TNAME::save( SaveContext *context ) const												\
	{																							\
		Data::save( context );																	\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );			\
		const ValueType &s = readable();														\
																								\
		container->write( "basis", s.basis.matrix.getValue(), 16 );								\
		container->write( "step", s.basis.step );												\
																								\
		vector<ValueType::XType> x; 															\
		vector<ValueType::YType> y; 															\
		ValueType::PointContainer::const_iterator it;											\
		for( it=s.points.begin(); it!=s.points.end(); it++ )									\
		{																						\
			x.push_back( it->first );															\
			y.push_back( it->second );															\
		}																						\
		container->write( "x", &(x[0]), x.size() );												\
		container->write( "y", (const YBASETYPE*)&(y[0]), y.size() * YBASESIZE );				\
	}																							\
																								\
	template<>																					\
	void TNAME::load( LoadContextPtr context )													\
	{																							\
		Data::load( context );																	\
		unsigned int v = 0;																		\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), v );			\
		ValueType &s = writable();																\
																								\
		ValueType::XType *bp = s.basis.matrix.getValue();										\
		container->read( "basis", bp, 16 );														\
		container->read( "step", s.basis.step );												\
																								\
		vector<ValueType::XType> x; 															\
		vector<ValueType::YType> y; 															\
		IndexedIO::Entry e = container->ls( "x" );												\
		x.resize( e.arrayLength() );															\
		y.resize( e.arrayLength() );															\
		ValueType::XType *xp = &(x[0]);															\
		container->read( "x", xp, e.arrayLength() );											\
		YBASETYPE *yp = (YBASETYPE *)&(y[0]);													\
		container->read( "y", yp, e.arrayLength() * YBASESIZE );								\
																								\
		s.points.clear();																		\
		for( unsigned i=0; i<x.size(); i++ )													\
		{																						\
			s.points.insert( ValueType::PointContainer::value_type( x[i], y[i] ) );				\
		}																						\
	}																							\
																								\
	template<>																					\
	void TNAME::memoryUsage( Object::MemoryAccumulator &accumulator ) const						\
	{																							\
		Data::memoryUsage( accumulator );														\
		const ValueType &s = readable();														\
		ValueType::PointContainer::const_iterator it;											\
		size_t m = s.points.size() * ( sizeof( ValueType::XType ) + sizeof( ValueType::YType ) ); 	\
		m += sizeof( ValueType );																\
		accumulator.accumulate( m );															\
	}																							\

SPECIALISE( SplineffData, float, 1 )
SPECIALISE( SplineddData, double, 1 )
SPECIALISE( SplinefColor3fData, float, 3 )
SPECIALISE( SplinefColor4fData, float, 4 )

}

// Instantiations
template class TypedData<Splineff>;
template class TypedData<Splinedd>;
template class TypedData<SplinefColor3f>;
template class TypedData<SplinefColor4f>;
