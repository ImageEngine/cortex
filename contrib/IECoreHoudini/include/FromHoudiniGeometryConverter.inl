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

#ifndef IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_INL
#define IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_INL

#include "GEO/GEO_Vertex.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/VectorTraits.h"

#include "FromHoudiniGeometryConverter.h"

namespace IECoreHoudini
{

template<class T>
FromHoudiniGeometryConverter::Description<T>::Description( IECore::TypeId resultType, bool isDefault )
{
	FromHoudiniGeometryConverter::registerConverter( resultType, isDefault, creator );
}

template<class T>
FromHoudiniGeometryConverterPtr FromHoudiniGeometryConverter::Description<T>::creator( const GU_DetailHandle &handle )
{
	return new T( handle );
}

template <typename Container>
void FromHoudiniGeometryConverter::transferAttribData(
	const Container &container, IECore::Primitive *result,
	IECore::PrimitiveVariable::Interpolation interpolation,
	const GB_Attribute *attr, const GB_AttributeRef &attrRef
) const
{
	IECore::DataPtr dataPtr = 0;

	switch ( attr->getType() )
	{
		case GB_ATTRIB_FLOAT :
		{
			unsigned dimensions = attr->getSize() / sizeof( float );
			switch ( dimensions )
			{
				case 1:
					dataPtr = extractData<IECore::FloatVectorData>( container, attrRef );
					break;
				case 2:
					dataPtr = extractData<IECore::V2fVectorData>( container, attrRef );
					break;
				case 3:
					dataPtr = extractData<IECore::V3fVectorData>( container, attrRef );
					break;
				default:
					break;
			}
			break;
		}
		case GB_ATTRIB_INT :
 		{
			unsigned dimensions = attr->getSize() / sizeof( float );
			switch ( dimensions )
			{
				case 1:
					dataPtr = extractData<IECore::IntVectorData>( container, attrRef );
					break;
				case 2:
					dataPtr = extractData<IECore::V2iVectorData>( container, attrRef );
					break;
				case 3:
					dataPtr = extractData<IECore::V3iVectorData>( container, attrRef );
					break;
				default:
					break;
			}
			break;
 		}
 		case GB_ATTRIB_VECTOR :
 		{
			unsigned dimensions = attr->getSize() / (sizeof( float ) * 3);
			if ( dimensions == 1 ) // only support single element vectors
			{
				dataPtr = extractData<IECore::V3fVectorData>( container, attrRef );
			}
 			break;
 		}
		default :
		{
			break;
		}
	}

	if ( dataPtr )
	{
		result->variables[ std::string( attr->getName() ) ] = IECore::PrimitiveVariable( interpolation, dataPtr );
	}
}

template <typename T, typename Container>
IECore::DataPtr FromHoudiniGeometryConverter::extractData( const Container &container, const GB_AttributeRef &attrRef ) const
{
	typedef typename T::BaseType BaseType;
	typedef typename T::ValueType::value_type ValueType;
	
	size_t size = container.entries();
	
	typename T::Ptr data = new T();
	data->writable().resize( size );
	BaseType *dest = data->baseWritable();
	
	unsigned dimensions = IECore::VectorTraits<ValueType>::dimensions();
	for ( size_t i=0; i < size; i++ )
	{
		/// \todo: castAttribData() is deprecated in Houdini 11. replace this with getValue()
		/// when we drop support for Houdini 10.
		const BaseType *src = container[i]->template castAttribData<BaseType>( attrRef );
		for ( size_t j=0; j < dimensions; j++ )
		{
			dest[ i*dimensions + j ] = src[j];
		}
	}

	return data;
}

template <typename T>
IECore::DataPtr FromHoudiniGeometryConverter::extractData( const GB_AttributeTable &attribs, const GB_AttributeRef &attrRef ) const
{
	typedef typename T::BaseType BaseType;
	typedef typename T::ValueType ValueType;
	
	typename T::Ptr data = new T();
	BaseType *dest = data->baseWritable();

	/// \todo: castAttribData() is deprecated in Houdini 11. replace this with getValue()
	/// when we drop support for Houdini 10.
	const BaseType *src = attribs.castAttribData<BaseType>( attrRef );

	unsigned dimensions = IECore::VectorTraits<ValueType>::dimensions();
	for ( size_t j=0; j < dimensions; j++ )
	{
		dest[j] = src[j];
	}

	return data;
}

}

#endif // IECOREHOUDINI_FROMHOUDINIGEOMETRYCONVERTER_INL
