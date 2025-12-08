//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2025, Image Engine Design Inc. All rights reserved.
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

#include "IECore/RampData.h"

#include "IECore/TypedData.inl"

#include <iostream>

using namespace std;
using namespace IECore;

namespace IECore
{

static IndexedIO::EntryID g_interpolationEntry("interpolation");
static IndexedIO::EntryID g_xEntry("x");
static IndexedIO::EntryID g_yEntry("y");

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( RampffData, RampffDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( RampfColor3fData, RampfColor3fDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( RampfColor4fData, RampfColor4fDataTypeId )

#define SPECIALISE( TNAME, YBASETYPE, YBASESIZE )												\
																								\
	template<>																					\
	void TNAME::save( SaveContext *context ) const												\
	{																							\
		Data::save( context );																	\
		IndexedIOPtr container = context->container( staticTypeName(), 0 );						\
		const ValueType &s = readable();														\
																								\
		container->write( g_interpolationEntry, (int)s.interpolation);							\
																								\
		vector<ValueType::XType> x; 															\
		vector<ValueType::YType> y; 															\
		ValueType::PointContainer::const_iterator it;											\
		for( it=s.points.begin(); it!=s.points.end(); it++ )									\
		{																						\
			x.push_back( it->first );															\
			y.push_back( it->second );															\
		}																						\
		container->write( g_xEntry, &(x[0]), x.size() );										\
		container->write( g_yEntry, (const YBASETYPE*)&(y[0]), y.size() * YBASESIZE );			\
	}																							\
																								\
	template<>																					\
	void TNAME::load( LoadContextPtr context )													\
	{																							\
		Data::load( context );																	\
		unsigned int v = 0;																		\
		ConstIndexedIOPtr container = context->container( staticTypeName(), v );				\
		ValueType &s = writable();																\
																								\
		int interpolationInt;																	\
		container->read( g_interpolationEntry, interpolationInt );								\
		s.interpolation = (RampInterpolation)interpolationInt;						\
																								\
		vector<ValueType::XType> x; 															\
		vector<ValueType::YType> y; 															\
		IndexedIO::Entry e = container->entry( "x" );											\
		x.resize( e.arrayLength() );															\
		y.resize( e.arrayLength() );															\
		ValueType::XType *xp = &(x[0]);															\
		container->read( g_xEntry, xp, e.arrayLength() );										\
		YBASETYPE *yp = (YBASETYPE *)&(y[0]);													\
		container->read( g_yEntry, yp, e.arrayLength() * YBASESIZE );							\
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

SPECIALISE( RampffData, float, 1 )
SPECIALISE( RampfColor3fData, float, 3 )
SPECIALISE( RampfColor4fData, float, 4 )

// Instantiations
template class TypedData<Rampff>;
template class TypedData<RampfColor3f>;
template class TypedData<RampfColor4f>;

}
