//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_INL
#define IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_INL

#include "GEO/GEO_Vertex.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/VectorTraits.h"

#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

namespace IECoreHoudini
{

template<class T>
FromHoudiniGeometryConverter::Description<T>::Description( IECore::TypeId resultType )
{
	FromHoudiniGeometryConverter::registerConverter( resultType, creator, canConvert );
}

template<class T>
FromHoudiniGeometryConverterPtr FromHoudiniGeometryConverter::Description<T>::creator( const GU_DetailHandle &handle )
{
	return new T( handle );
}

template<class T>
FromHoudiniGeometryConverter::Convertability FromHoudiniGeometryConverter::Description<T>::canConvert( const GU_DetailHandle &handle )
{
	GU_DetailHandleAutoReadLock readHandle( handle );
	
	const GU_Detail *geo = readHandle.getGdp();
	if ( !geo )
	{
		return Inapplicable;
	}
	
	return T::canConvert( geo );
}

struct SetInterpretation
{
	typedef void ReturnType;
	
	GA_TypeInfo m_type;
	
	template<typename T>
	void operator() ( T *data )
	{
		assert( data );
		
		if ( m_type == GA_TYPE_POINT )
		{
			data->setInterpretation( IECore::GeometricData::Point );
		}
		else if ( m_type == GA_TYPE_NORMAL )
		{
			data->setInterpretation( IECore::GeometricData::Normal );
		}
		else if ( m_type == GA_TYPE_VECTOR )
		{
			data->setInterpretation( IECore::GeometricData::Vector );
		}
		else if ( m_type == GA_TYPE_COLOR )
		{
			data->setInterpretation( IECore::GeometricData::Color );
		}
	}
};

template <typename T>
typename T::Ptr FromHoudiniGeometryConverter::extractData( const GA_Attribute *attr, const GA_Range &range, int elementIndex ) const
{
	typedef typename T::BaseType BaseType;
	
	typename T::Ptr data = new T();
	data->writable().resize( range.getEntries() );
	BaseType *dest = data->baseWritable();
	
	if ( elementIndex == -1 )
	{
		attr->getAIFTuple()->getRange( attr, range, dest );
	}
	else
	{
		attr->getAIFTuple()->getRange( attr, range, dest, elementIndex, 1 );
	}
	
	// set the geometric interpretation if it exists
	SetInterpretation func = { attr->getTypeInfo() };
	IECore::despatchTypedData< SetInterpretation, IECore::TypeTraits::IsGeometricTypedData, IECore::DespatchTypedDataIgnoreError >( data.get(), func );
	
	return data;
}

template <>
IECore::QuatfVectorDataPtr FromHoudiniGeometryConverter::extractData<IECore::QuatfVectorData>( const GA_Attribute *attr, const GA_Range &range, int elementIndex ) const;

template <typename T>
typename T::Ptr FromHoudiniGeometryConverter::extractData( const GA_Attribute *attr ) const
{
	typedef typename T::BaseType BaseType;
	typedef typename T::ValueType ValueType;
	
	typename T::Ptr data = new T();
	BaseType *dest = data->baseWritable();

	unsigned dimensions = IECore::VectorTraits<ValueType>::dimensions();
	attr->getAIFTuple()->get( attr, 0, dest, dimensions );

	// set the geometric interpretation if it exists
	SetInterpretation func = { attr->getTypeInfo() };
	IECore::despatchTypedData< SetInterpretation, IECore::TypeTraits::IsGeometricTypedData, IECore::DespatchTypedDataIgnoreError >( data.get(), func );
	
	return data;
}

}

#endif // IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_INL
