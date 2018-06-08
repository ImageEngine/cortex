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

#ifndef IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_INL
#define IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_INL

#include "boost/format.hpp"

#include "IECore/DespatchTypedData.h"
#include "IECore/VectorTraits.h"

#include "IECoreHoudini/ToHoudiniNumericAttribConverter.h"
#include "IECoreHoudini/TypeTraits.h"

namespace IECoreHoudini
{

template<typename T>
ToHoudiniAttribConverter::Description<ToHoudiniNumericVectorAttribConverter<T> > ToHoudiniNumericVectorAttribConverter<T>::m_description( T::staticTypeId() );

template<typename T>
ToHoudiniNumericVectorAttribConverter<T>::ToHoudiniNumericVectorAttribConverter( const IECore::Data *data ) :
	ToHoudiniAttribConverter( data, "Converts numeric IECore VectorTypedData to a GA_Attribute on the provided GU_Detail." )
{
}

template<typename T>
ToHoudiniNumericVectorAttribConverter<T>::~ToHoudiniNumericVectorAttribConverter()
{
}

template<typename T>
GA_RWAttributeRef ToHoudiniNumericVectorAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
{
	throw IECore::Exception( "ToHoudiniNumericVectorAttribConverter does not support Detail attributes." );
}

struct GetInterpretation
{
	typedef void ReturnType;

	GA_RWAttributeRef m_attrRef;

	template<typename T>
	ReturnType operator() ( T *data )
	{
		assert( data );

		IECore::GeometricData::Interpretation interp = data->getInterpretation();

		if ( interp == IECore::GeometricData::Point )
		{
			m_attrRef.setTypeInfo( GA_TYPE_POINT );
		}
		else if ( interp == IECore::GeometricData::Normal )
		{
			m_attrRef.setTypeInfo( GA_TYPE_NORMAL );
		}
		else if ( interp == IECore::GeometricData::Vector )
		{
			m_attrRef.setTypeInfo( GA_TYPE_VECTOR );
		}
		else if ( interp == IECore::GeometricData::Color )
		{
			m_attrRef.setTypeInfo( GA_TYPE_COLOR );
		}
		else if ( interp == IECore::GeometricData::UV )
		{
			m_attrRef.setTypeInfo( GA_TYPE_TEXTURE_COORD );
		}
	}
};

template<typename T>
GA_RWAttributeRef ToHoudiniNumericVectorAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const
{
	assert( data );

	typedef typename T::BaseType BaseType;
	typedef typename T::ValueType::value_type ValueType;

 	unsigned dimensions = sizeof( ValueType ) / sizeof( BaseType );

	GA_RWAttributeRef attrRef;

	if ( IECoreHoudini::TypeTraits::IsVectorAttribFloatTypedData<T>::value )
	{
		attrRef = geo->addFloatTuple( range.getOwner(), name.c_str(), dimensions );
	}
	else if ( IECoreHoudini::TypeTraits::IsVectorAttribIntTypedData<T>::value )
	{
		attrRef = geo->addIntTuple( range.getOwner(), name.c_str(), dimensions );
	}
	else
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniNumericVectorAttribConverter::doConversion: PrimitiveVariable \"%s\" is not of a supported data type." ) % name ).str() );
	}

	if ( attrRef.isInvalid() )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniNumericVectorAttribConverter::doConversion: Invalid GA_RWAttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}

	if ( IECoreHoudini::TypeTraits::IsAttribColorTypedData<T>::value )
	{
		attrRef.setTypeInfo( GA_TYPE_COLOR );
	}

	typename T::ConstPtr dataPtr = IECore::runTimeCast<const T>( data );
	const BaseType *src = dataPtr->baseReadable();

	GA_Attribute *attr = attrRef.getAttribute();
	attr->getAIFTuple()->setRange( attr, range, src );

	// set the geometric interpretation if it exists
	GetInterpretation func = { attrRef };
	IECore::despatchTypedData< GetInterpretation, IECore::TypeTraits::IsGeometricTypedData, IECore::DespatchTypedDataIgnoreError >( const_cast<IECore::Data *>( data ), func );

	return attrRef;
}

template<typename T>
ToHoudiniAttribConverter::Description<ToHoudiniNumericDetailAttribConverter<T> > ToHoudiniNumericDetailAttribConverter<T>::m_description( T::staticTypeId() );

template<typename T>
ToHoudiniNumericDetailAttribConverter<T>::ToHoudiniNumericDetailAttribConverter( const IECore::Data *data ) :
	ToHoudiniAttribConverter( data, "Converts numeric IECore SimpleTypedData to a GA_Attribute on the provided GU_Detail." )
{
}

template<typename T>
ToHoudiniNumericDetailAttribConverter<T>::~ToHoudiniNumericDetailAttribConverter()
{
}

template<typename T>
GA_RWAttributeRef ToHoudiniNumericDetailAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
{
	assert( data );

	typedef typename T::BaseType BaseType;
	typedef typename T::ValueType ValueType;

 	unsigned dimensions = sizeof( ValueType ) / sizeof( BaseType );

	GA_RWAttributeRef attrRef;

	if ( IECoreHoudini::TypeTraits::IsDetailAttribFloatTypedData<T>::value )
	{
		attrRef = geo->addFloatTuple( GA_ATTRIB_DETAIL, name.c_str(), dimensions );
	}
	else if ( IECoreHoudini::TypeTraits::IsDetailAttribIntTypedData<T>::value )
	{
		attrRef = geo->addIntTuple( GA_ATTRIB_DETAIL, name.c_str(), dimensions );
	}
	else
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniNumericDetailAttribConverter::doConversion: PrimitiveVariable \"%s\" is not of a supported data type." ) % name ).str() );
	}

	if ( attrRef.isInvalid() )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniNumericDetailAttribConverter::doConversion: Invalid GA_RWAttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}

	if ( IECoreHoudini::TypeTraits::IsAttribColorTypedData<T>::value )
	{
		attrRef.setTypeInfo( GA_TYPE_COLOR );
	}

	typename T::ConstPtr dataPtr = IECore::runTimeCast<const T>( data );
	const BaseType *src = dataPtr->baseReadable();

	GA_Attribute *attr = attrRef.getAttribute();
	attr->getAIFTuple()->setRange( attr, geo->getGlobalRange(), src );

	// set the geometric interpretation if it exists
	GetInterpretation func = { attrRef };
	IECore::despatchTypedData< GetInterpretation, IECore::TypeTraits::IsGeometricTypedData, IECore::DespatchTypedDataIgnoreError >( const_cast<IECore::Data *>( data ), func );

	return attrRef;
}

template<typename T>
GA_RWAttributeRef ToHoudiniNumericDetailAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const
{
	throw IECore::Exception( "ToHoudiniNumericDetailAttribConverter does not support Element attributes." );
}

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_INL
