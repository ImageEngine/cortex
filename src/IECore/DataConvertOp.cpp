//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DataConvertOp.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/NullObject.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/Exception.h"
#include "IECore/ScaledDataConversion.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( DataConvertOp );

InternedString DataConvertOp::g_dataName( "data" );
InternedString DataConvertOp::g_targetTypeName( "targetType" );

DataConvertOp::DataConvertOp()
	:	Op(
			"Converts between different Data types, using a ScaledDataConversion.",
			new ObjectParameter(
				"result",
				"The converted Data object.",
				new NullObject(),
				DataTypeId
			)
		)
{
	parameters()->addParameter(
	
		new ObjectParameter(
			g_dataName,
			"The data to be converted.",
			new NullObject(),
			Data::staticTypeId()
		)
	
	);
	
	parameters()->addParameter(
	
		new IntParameter(
			g_targetTypeName,
			"The data type to convert to.",
			InvalidTypeId,
			0,
			Imath::limits<int>::max()
		)
		
	);
}

DataConvertOp::~DataConvertOp()
{
}

ObjectParameter *DataConvertOp::dataParameter()
{
	return parameters()->parameter<ObjectParameter>( g_dataName );
}

const ObjectParameter *DataConvertOp::dataParameter() const
{
	return parameters()->parameter<ObjectParameter>( g_dataName );
}

IntParameter *DataConvertOp::targetTypeParameter()
{
	return parameters()->parameter<IntParameter>( g_targetTypeName );
}

const IntParameter *DataConvertOp::targetTypeParameter() const
{
	return parameters()->parameter<IntParameter>( g_targetTypeName );
}

struct DataConvertOp::ConvertFnStage1
{
	
	typedef DataPtr ReturnType;

	ConvertFnStage1( TypeId resultType )
		:	m_resultType( resultType )
	{
	}

	template<typename From>
	ReturnType operator()( typename From::ConstPtr data )
	{
		typedef typename From::BaseType FromBaseType;
		
		ConvertFnStage2<FromBaseType> fn( data->baseReadable(), data->baseSize() );
		
		DataPtr result = staticPointerCast<Data>( Object::create( m_resultType ) );
	
		return despatchTypedData<ConvertFnStage2<FromBaseType>, TypeTraits::IsNumericBasedVectorTypedData>( result.get(), fn );
	}
	
	TypeId m_resultType;

};

template<class FromBaseType>
struct DataConvertOp::ConvertFnStage2
{

	typedef DataPtr ReturnType;
	
	ConvertFnStage2( const FromBaseType *rawData, size_t arrayLength )
		:	m_rawData( rawData ), m_arrayLength( arrayLength )
	{
	}

	template<typename To>
	ReturnType operator()( typename To::Ptr data )
	{
		typedef typename To::BaseType BaseType;
		typedef typename To::ValueType::value_type ValueType;
			
		const size_t dimensions = sizeof( ValueType ) / sizeof( BaseType );
		if( m_arrayLength % dimensions )
		{
			throw InvalidArgumentException( "Input data must have length equal to a multiple of the dimension of the output data" );
		}
		
		typename To::ValueType &writable = data->writable();
		writable.resize( m_arrayLength / dimensions );
		
		BaseType *baseWritable = data->baseWritable();
		
		ScaledDataConversion<FromBaseType, BaseType> converter;
		
		for( size_t i=0; i<m_arrayLength; ++i )
		{
			baseWritable[i] = converter( m_rawData[i] );
		}
	
		return data;
	}
	
	const FromBaseType *m_rawData;
	const size_t m_arrayLength;
	
};

ObjectPtr DataConvertOp::doOperation( const CompoundObject *operands )
{
	TypeId targetType = (TypeId)operands->member<IntData>( g_targetTypeName )->readable();
	if( !RunTimeTyped::inheritsFrom( targetType, Data::staticTypeId() ) )
	{
		throw InvalidArgumentException( "Target type must derive from data" );
	}
	
	ConstDataPtr data = operands->member<Data>( g_dataName );
	
	ConvertFnStage1 fn( targetType );
	
	DataPtr result = despatchTypedData<ConvertFnStage1, TypeTraits::IsNumericBasedVectorTypedData>( const_cast<Data *>( data.get() ), fn );

	return result;
}
