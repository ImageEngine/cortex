//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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
#include "IECore/Export.h"
#include "IECore/TypedData.inl"

#include <iostream>

using namespace std;
using namespace IECore;

namespace IECore
{

static IndexedIO::EntryID g_interpolationEntry("interpolation");
static IndexedIO::EntryID g_dimensionEntry("dimension");
static IndexedIO::EntryID g_domainMinEntry("domainMin");
static IndexedIO::EntryID g_domainMaxEntry("domainMax");
static IndexedIO::EntryID g_dataSizeEntry("dataSize");
static IndexedIO::EntryID g_dataEntry("data");

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( CubeColorLookupfData, CubeColorLookupfDataTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( CubeColorLookupdData, CubeColorLookupdDataTypeId )

#define SPECIALISE( TNAME )\
\
	template<>\
	void TNAME::save( SaveContext *context ) const\
	{\
		Data::save( context );\
		IndexedIOPtr container = context->container( staticTypeName(), 0 );\
		const ValueType &s = readable();\
\
		container->write( g_interpolationEntry, (short)s.m_interpolation ); \
		container->write( g_dimensionEntry, s.m_dimension.getValue(), 3 ); \
		container->write( g_domainMinEntry, s.m_domain.min.getValue(), 3 ); \
		container->write( g_domainMaxEntry, s.m_domain.max.getValue(), 3 ); \
		int dataSize = s.data().size() ; \
		container->write( g_dataSizeEntry, dataSize ); \
		if ( s.data().size() ) \
		{ \
			ValueType::ColorType::BaseType *c = ( ValueType::ColorType::BaseType * )( s.data()[0].getValue() ); \
			container->write( g_dataEntry, c, dataSize * 3 ); \
		} \
	}\
\
	template<>\
	void TNAME::load( LoadContextPtr context )\
	{\
		Data::load( context );\
		unsigned int v = 0;\
		ConstIndexedIOPtr container = context->container( staticTypeName(), v );\
		ValueType &s = writable();\
		\
		short interp; \
		container->read( g_interpolationEntry, interp ); \
		s.m_interpolation = ( ValueType::Interpolation ) interp; \
		Imath::V3i dimension; \
		int *dim = dimension.getValue(); \
		container->read( g_dimensionEntry, dim, 3 ); \
		ValueType::BoxType domain; \
		ValueType::VecType::BaseType *f = domain.min.getValue(); \
		container->read( g_domainMinEntry, f, 3 ); \
		f = domain.max.getValue();\
		container->read( g_domainMaxEntry, f, 3 ); \
		int dataSize; \
		container->read( g_dataSizeEntry, dataSize ); \
		ValueType::DataType data; \
		data.resize( dataSize ); \
		if ( dataSize > 0 ) \
		{ \
			ValueType::ColorType::BaseType *c = ( ValueType::ColorType::BaseType * )( data[0].getValue() ); \
			container->read( g_dataEntry, c, dataSize * 3 ); \
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
\
	template<>\
	MurmurHash SharedDataHolder<TNAME::ValueType>::hash() const\
	{\
		const TNAME::ValueType &s = readable();\
		MurmurHash result;\
		result.append( s.dimension() );\
		result.append( s.domain() );\
		result.append( (int)s.getInterpolation() );\
		result.append( &(s.data()[0]), s.data().size() );\
		return result;\
	}\

SPECIALISE( CubeColorLookupfData )
SPECIALISE( CubeColorLookupdData )

// Instantiations
template class IECORE_API TypedData<CubeColorLookupf>;
template class IECORE_API TypedData<CubeColorLookupd>;

}
