//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DataInterleaveOp.h"

#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/Exception.h"
#include "IECore/NullObject.h"
#include "IECore/ObjectVector.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( DataInterleaveOp );

InternedString DataInterleaveOp::g_dataName( "data" );
InternedString DataInterleaveOp::g_targetTypeName( "targetType" );

DataInterleaveOp::DataInterleaveOp()
	:	Op(
			"Interleaves several VectorData instances into a single one.",
			new ObjectParameter(
				"result",
				"The interleaved Data object.",
				new NullObject(),
				DataTypeId
			)
		)
{
	parameters()->addParameter(

		new ObjectVectorParameter(
			g_dataName,
			"The data to be interleaved. This should consist of a series of VectorData instances "
			"of the same type and length.",
			new ObjectVector()
		)

	);

	parameters()->addParameter(

		new IntParameter(
			g_targetTypeName,
			"The type of data created.",
			InvalidTypeId,
			0,
			Imath::limits<int>::max()
		)

	);
}

DataInterleaveOp::~DataInterleaveOp()
{
}

ObjectVectorParameter *DataInterleaveOp::dataParameter()
{
	return parameters()->parameter<ObjectVectorParameter>( g_dataName );
}

const ObjectVectorParameter *DataInterleaveOp::dataParameter() const
{
	return parameters()->parameter<ObjectVectorParameter>( g_dataName );
}

IntParameter *DataInterleaveOp::targetTypeParameter()
{
	return parameters()->parameter<IntParameter>( g_targetTypeName );
}

const IntParameter *DataInterleaveOp::targetTypeParameter() const
{
	return parameters()->parameter<IntParameter>( g_targetTypeName );
}

struct DataInterleaveOp::InterleaveFnStage1
{

	typedef DataPtr ReturnType;

	InterleaveFnStage1( const std::vector<ObjectPtr> &dataArrays, TypeId resultType )
		:	m_dataArrays( dataArrays ), m_resultType( resultType )
	{
	}

	template<typename From>
	ReturnType operator()( typename From::ConstPtr data )
	{
		typedef typename From::BaseType FromBaseType;
		std::vector<const FromBaseType *> rawDataArrays;

		size_t arrayLength = 0;
		for( size_t i=0; i<m_dataArrays.size(); ++i )
		{
			const From *f = runTimeCast<const From>( m_dataArrays[i].get() );
			if( !f )
			{
				throw InvalidArgumentException( "Inputs must all be of the same type" );
			}
			if( i==0 )
			{
				arrayLength = f->baseSize();
			}
			else
			{
				if( f->baseSize() != arrayLength )
				{
					throw InvalidArgumentException( "Inputs must all be of the same length" );
				}
			}

			rawDataArrays.push_back( f->baseReadable() );
		}

		InterleaveFnStage2<FromBaseType> fn( rawDataArrays, arrayLength );

		DataPtr result = boost::static_pointer_cast<Data>( Object::create( m_resultType ) );

		return despatchTypedData<InterleaveFnStage2<FromBaseType>, TypeTraits::IsNumericBasedVectorTypedData>( result.get(), fn );
	}

	const std::vector<ObjectPtr> &m_dataArrays;
	TypeId m_resultType;

};

template<class FromBaseType>
struct DataInterleaveOp::InterleaveFnStage2
{

	typedef DataPtr ReturnType;

	InterleaveFnStage2( const std::vector<const FromBaseType *> &rawDataArrays, size_t arrayLength )
		:	m_rawDataArrays( rawDataArrays ), m_arrayLength( arrayLength )
	{
	}

	template<typename To>
	ReturnType operator()( typename To::Ptr data )
	{
		typedef typename To::BaseType BaseType;
		typedef typename To::ValueType::value_type ValueType;

		size_t numRawDataArrays = m_rawDataArrays.size();

		const size_t dimensions = sizeof( ValueType ) / sizeof( BaseType );
		if( dimensions != 1 )
		{
			if( dimensions != numRawDataArrays )
			{
				throw InvalidArgumentException( "Number of inputs must match dimension of output type" );
			}
		}

		typename To::ValueType &writable = data->writable();
		writable.resize( m_rawDataArrays.size() * m_arrayLength / dimensions );

		BaseType *baseWritable = data->baseWritable();

		ScaledDataConversion<FromBaseType, BaseType> converter;

		for( size_t i=0; i<m_arrayLength; ++i )
		{
			for( size_t j=0; j<numRawDataArrays; ++j )
			{
				*baseWritable++ = converter( m_rawDataArrays[j][i] );
			}
		}

		return data;
	}

	const std::vector<const FromBaseType *> &m_rawDataArrays;
	const size_t m_arrayLength;

};

ObjectPtr DataInterleaveOp::doOperation( const CompoundObject *operands )
{
	TypeId targetType = (TypeId)operands->member<IntData>( g_targetTypeName )->readable();
	if( !RunTimeTyped::inheritsFrom( targetType, Data::staticTypeId() ) )
	{
		throw InvalidArgumentException( "Target type must derive from data" );
	}

	const ObjectVector *inputData = operands->member<ObjectVector>( g_dataName );

	if( !inputData->members().size() )
	{
		return Object::create( targetType );
	}

	ConstDataPtr firstData = runTimeCast<const Data>( inputData->members()[0] );
	if( !firstData )
	{
		throw InvalidArgumentException( "Expected a Data object" );
	}

	InterleaveFnStage1 fn( inputData->members(), targetType );

	DataPtr result = despatchTypedData<InterleaveFnStage1, TypeTraits::IsNumericVectorTypedData>( const_cast<Data *>( firstData.get() ), fn );

	return result;
}
