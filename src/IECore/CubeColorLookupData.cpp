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

#include "IECore/CubeColorLookupData.h"
#include "IECore/TypedData.inl"

#include <iostream>

using namespace std;
using namespace IECore;

namespace IECore
{

IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( CubeColorLookupfData, CubeColorLookupfDataTypeId )
IE_CORE_DEFINECOMMONTYPEDDATASPECIALISATION( CubeColorLookupdData, CubeColorLookupdDataTypeId )

IE_CORE_DEFINETYPEDDATANOBASESIZE( CubeColorLookupfData )
IE_CORE_DEFINETYPEDDATANOBASESIZE( CubeColorLookupdData )

#define SPECIALISE( TNAME )\
\
	template<>\
	void TNAME::save( SaveContext *context ) const\
	{\
		Data::save( context );\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), 0 );\
		const ValueType &s = readable();\
\
		container->write( "interpolation", (short)s.m_interpolation ); \
		container->write( "dimension", s.m_dimension.getValue(), 3 ); \
		container->write( "domainMin", s.m_domain.min.getValue(), 3 ); \
		container->write( "domainMax", s.m_domain.max.getValue(), 3 ); \
		int dataSize = s.data().size() ; \
		container->write( "dataSize", dataSize ); \
		if ( s.data().size() ) \
		{ \
			ValueType::ColorType::BaseType *c = ( ValueType::ColorType::BaseType * )( s.data()[0].getValue() ); \
			container->write( "data", c, dataSize * 3 ); \
		} \
	}\
\
	template<>\
	void TNAME::load( LoadContextPtr context )\
	{\
		Data::load( context );\
		unsigned int v = 0;\
		IndexedIOInterfacePtr container = context->container( staticTypeName(), v );\
		ValueType &s = writable();\
		\
		short interp; \
		container->read( "interpolation", interp ); \
		s.m_interpolation = ( ValueType::Interpolation ) interp; \
		Imath::V3i dimension; \
		int *dim = dimension.getValue(); \
		container->read( "dimension", dim, 3 ); \
		ValueType::BoxType domain; \
		ValueType::VecType::BaseType *f = domain.min.getValue(); \
		container->read( "domainMin", f, 3 ); \
		f = domain.max.getValue();\
		container->read( "domainMax", f, 3 ); \
		int dataSize; \
		container->read( "dataSize", dataSize ); \
		ValueType::DataType data; \
		data.resize( dataSize ); \
		if ( dataSize > 0 ) \
		{ \
			ValueType::ColorType::BaseType *c = ( ValueType::ColorType::BaseType * )( data[0].getValue() ); \
			container->read( "data", c, dataSize * 3 ); \
		} \
		s.setCube( dimension, data, domain ); \
	}\
\
	template<>\
	void TNAME::memoryUsage( Object::MemoryAccumulator &accumulator ) const\
	{\
		Data::memoryUsage( accumulator );\
		const ValueType &s = readable();\
		size_t m = s.m_data.size() * sizeof( ValueType::DataType::value_type ); \
		accumulator.accumulate( m );\
	}\

SPECIALISE( CubeColorLookupfData )
SPECIALISE( CubeColorLookupdData )

}

// Instantiations
template class TypedData<CubeColorLookupf>;
template class TypedData<CubeColorLookupd>;
