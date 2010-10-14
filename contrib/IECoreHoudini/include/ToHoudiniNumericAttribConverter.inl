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

#ifndef IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_INL
#define IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_INL

#include "boost/format.hpp"

#include "GEO/GEO_Vertex.h"

#include "IECore/VectorTraits.h"

#include "ToHoudiniNumericAttribConverter.h"
#include "TypeTraits.h"

namespace IECoreHoudini
{

template<typename T>
ToHoudiniAttribConverter::Description<ToHoudiniNumericVectorAttribConverter<T> > ToHoudiniNumericVectorAttribConverter<T>::m_description( T::staticTypeId() );

template<typename T>
ToHoudiniNumericVectorAttribConverter<T>::ToHoudiniNumericVectorAttribConverter( const IECore::Data *data ) :
	ToHoudiniAttribConverter( data, "Converts numeric IECore VectorTypedData to a GB_Attribute on the provided GU_Detail." )
{
}

template<typename T>
ToHoudiniNumericVectorAttribConverter<T>::~ToHoudiniNumericVectorAttribConverter()
{
}

template<typename T>
GB_AttributeRef ToHoudiniNumericVectorAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
{
	throw IECore::Exception( "ToHoudiniNumericVectorAttribConverter does not support Detail attributes." );
}

template<typename T>
GB_AttributeRef ToHoudiniNumericVectorAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, GEO_PointList *points ) const
{
	return doVectorConversion<GEO_PointList>( data, name, geo, points, GEO_POINT_DICT );
}

template<typename T>
GB_AttributeRef ToHoudiniNumericVectorAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, GEO_PrimList *primitives ) const
{
	return doVectorConversion<GEO_PrimList>( data, name, geo, primitives, GEO_PRIMITIVE_DICT );
}

template<typename T>
GB_AttributeRef ToHoudiniNumericVectorAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, VertexList *vertices ) const
{
	return doVectorConversion<VertexList>( data, name, geo, vertices, GEO_VERTEX_DICT );
}

template<typename T>
template<typename Container>
GB_AttributeRef ToHoudiniNumericVectorAttribConverter<T>::doVectorConversion( const IECore::Data *data, std::string name, GU_Detail *geo, Container *container, GEO_AttributeOwner owner ) const
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
		throw IECore::Exception( ( boost::format( "ToHoudiniNumericVectorAttribConverter::doConversion: PrimitiveVariable \"%s\" is not of a supported data type." ) % name ).str() );
	}

	GB_AttributeRef attrRef = geo->addAttribute( name.c_str(), size, attribType, defaultValue, owner );
	if ( GBisAttributeRefInvalid( attrRef ) )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniNumericVectorAttribConverter::doConversion: Invalid GB_AttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}

	typename T::ConstPtr dataPtr = IECore::runTimeCast<const T>( data );
	const BaseType *src = dataPtr->baseReadable();
	
	size_t entries = container ? container->entries() : 0;
	for ( size_t i=0; i < entries; i++ )
	{
		/// \todo: castAttribData() is deprecated in Houdini 11. replace this with setValue()
		/// when we drop support for Houdini 10.
		BaseType *dest =  (*container)[i]->template castAttribData<BaseType>( attrRef );
		for ( size_t j=0; j < dimensions; j++ )
		{
			dest[j] = src[ i*dimensions + j ];
		}
	}
	
	return attrRef;
}

template<typename T>
ToHoudiniAttribConverter::Description<ToHoudiniNumericDetailAttribConverter<T> > ToHoudiniNumericDetailAttribConverter<T>::m_description( T::staticTypeId() );

template<typename T>
ToHoudiniNumericDetailAttribConverter<T>::ToHoudiniNumericDetailAttribConverter( const IECore::Data *data ) :
	ToHoudiniAttribConverter( data, "Converts numeric IECore SimpleTypedData to a GB_Attribute on the provided GU_Detail." )
{
}

template<typename T>
ToHoudiniNumericDetailAttribConverter<T>::~ToHoudiniNumericDetailAttribConverter()
{
}

template<typename T>
GB_AttributeRef ToHoudiniNumericDetailAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
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
		throw IECore::Exception( ( boost::format( "ToHoudiniNumericDetailAttribConverter::doConversion: PrimitiveVariable \"%s\" is not of a supported data type." ) % name ).str() );
	}

	GB_AttributeRef attrRef = geo->addAttribute( name.c_str(), size, attribType, defaultValue, GEO_DETAIL_DICT );
	if ( GBisAttributeRefInvalid( attrRef ) )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniNumericDetailAttribConverter::doConversion: Invalid GB_AttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}

	typename T::ConstPtr dataPtr = IECore::runTimeCast<const T>( data );
	const BaseType *src = dataPtr->baseReadable();

	/// \todo: castAttribData() is deprecated in Houdini 11. replace this with setValue()
	/// when we drop support for Houdini 10.
	BaseType *dest = geo->attribs().castAttribData<BaseType>( attrRef );
	for ( size_t j=0; j < dimensions; j++ )
	{
		dest[j] = src[j];
	}
	
	return attrRef;
}

template<typename T>
GB_AttributeRef ToHoudiniNumericDetailAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, GEO_PointList *points ) const
{
	throw IECore::Exception( "ToHoudiniNumericDetailAttribConverter does not support Point attributes." );
}

template<typename T>
GB_AttributeRef ToHoudiniNumericDetailAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, GEO_PrimList *primitives ) const
{
	throw IECore::Exception( "ToHoudiniNumericDetailAttribConverter does not support Primitive attributes." );
}

template<typename T>
GB_AttributeRef ToHoudiniNumericDetailAttribConverter<T>::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, VertexList *vertices ) const
{
	throw IECore::Exception( "ToHoudiniNumericDetailAttribConverter does not support Vertex attributes." );
}

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_INL
