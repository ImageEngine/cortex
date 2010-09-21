//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TOHOUDINIGEOMETRYCONVERTER_INL
#define IECOREHOUDINI_TOHOUDINIGEOMETRYCONVERTER_INL

#include "IECore/VectorTraits.h"

#include "ToHoudiniGeometryConverter.h"
#include "TypeTraits.h"

namespace IECoreHoudini
{

template<class T>
ToHoudiniGeometryConverter::Description<T>::Description( IECore::TypeId fromType )
{
	ToHoudiniGeometryConverter::registerConverter( fromType, creator );
}

template<class T>
ToHoudiniGeometryConverterPtr ToHoudiniGeometryConverter::Description<T>::creator( const IECore::Primitive *primitive )
{
	return new T( primitive );
}

template <typename Container>
ToHoudiniGeometryConverter::TransferAttrib<Container>::TransferAttrib( GU_Detail *geo, Container *container, std::string name, GEO_AttributeOwner owner )
	: m_geo( geo ), m_container( container ), m_name( name ), m_owner( owner )
{
}

template <typename Container>
template <typename T>
ToHoudiniGeometryConverter::TransferReturnType ToHoudiniGeometryConverter::TransferAttrib<Container>::operator()( typename T::ConstPtr data ) const
{
	assert( data );

	typedef typename T::BaseType BaseType;
	typedef typename T::ValueType::value_type ValueType;

 	GB_AttribType attribType;
 	unsigned dimensions = IECore::VectorTraits<ValueType>::dimensions();
 	int size = sizeof( BaseType ) * dimensions;
 	const BaseType defaultValue[3] = { 0, 0, 0 };

	if ( IECoreHoudini::TypeTraits::IsVectorGbAttribFloatTypedData<T>::value )
	{
		attribType = GB_ATTRIB_FLOAT;
	}
	else if ( IECoreHoudini::TypeTraits::IsVectorGbAttribIntTypedData<T>::value )
	{
		attribType = GB_ATTRIB_INT;
	}
	else
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniGeometryConverter::TransferAttrib: PrimitiveVariable \"%s\" is not of a supported data type." ) % m_name ).str() );
	}

	GB_AttributeRef attrRef = m_geo->addAttribute( m_name.c_str(), size, attribType, defaultValue, m_owner );
	if ( GBisAttributeRefInvalid( attrRef ) )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniGeometryConverter::TransferAttrib: Invalid GB_AttributeRef returned for PrimitiveVariable \"%s\"." ) % m_name ).str() );
	}

	const BaseType *src = data->baseReadable();
	
	size_t entries = m_container ? m_container->entries() : 0;
	for ( size_t i=0; i < entries; i++ )
	{
		/// \todo: castAttribData() is deprecated in Houdini 11. replace this with setValue()
		/// when we drop support for Houdini 10.
		BaseType *dest =  (*m_container)[i]->template castAttribData<BaseType>( attrRef );
		for ( size_t j=0; j < dimensions; j++ )
		{
			dest[j] = src[ i*dimensions + j ];
		}
	}
}

template<typename T>
ToHoudiniGeometryConverter::TransferReturnType ToHoudiniGeometryConverter::TransferDetailAttrib::operator()( typename T::ConstPtr data ) const
{
	assert( data );

	typedef typename T::BaseType BaseType;
	typedef typename T::ValueType ValueType;

 	GB_AttribType attribType;
 	unsigned dimensions = IECore::VectorTraits<ValueType>::dimensions();
	int size = sizeof( BaseType ) * dimensions;
	const BaseType defaultValue[3] = { 0, 0, 0 };

	if ( IECoreHoudini::TypeTraits::IsDetailGbAttribFloatTypedData<T>::value )
	{
		attribType = GB_ATTRIB_FLOAT;
	}
	else if ( IECoreHoudini::TypeTraits::IsDetailGbAttribIntTypedData<T>::value )
	{
		attribType = GB_ATTRIB_INT;
	}
	else
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniGeometryConverter::TransferDetailAttrib: PrimitiveVariable \"%s\" is not of a supported data type." ) % m_name ).str() );
	}

	GB_AttributeRef attrRef = m_geo->addAttribute( m_name.c_str(), size, attribType, defaultValue, GEO_DETAIL_DICT );
	if ( GBisAttributeRefInvalid( attrRef ) )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniGeometryConverter::TransferDetailAttrib: Invalid GB_AttributeRef returned for PrimitiveVariable \"%s\"." ) % m_name ).str() );
	}

	const BaseType *src = data->baseReadable();

	/// \todo: castAttribData() is deprecated in Houdini 11. replace this with setValue()
	/// when we drop support for Houdini 10.
	BaseType *dest = m_geo->attribs().castAttribData<BaseType>( attrRef );
	for ( size_t j=0; j < dimensions; j++ )
	{
		dest[j] = src[j];
	}
}

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_TOHOUDINIGEOMETRYCONVERTER_INL
