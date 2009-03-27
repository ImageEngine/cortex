//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/DataPromoteOp.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/NullObject.h"
#include "IECore/DespatchTypedData.h"

#include <cassert>

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

DataPromoteOp::DataPromoteOp()
		:	Op(
		        staticTypeName(),
		        "Promotes scalar data types to compound data types.",
		        new ObjectParameter(
		                "result",
		                "Promoted Data object.",
		                new NullObject(),
		                DataTypeId
		        )
		)
{
	m_objectParameter = new ObjectParameter(
	        "object",
	        "The Data object that will be promoted.",
	        new NullObject(),
	        DataTypeId
	);
	m_targetTypeParameter = new IntParameter(
	        "targetType",
	        "The target Data typeId.",
	        InvalidTypeId,
	        0,
	        Imath::limits<int>::max()
	);
	parameters()->addParameter( m_objectParameter );
	parameters()->addParameter( m_targetTypeParameter );
}

DataPromoteOp::~DataPromoteOp()
{
}

namespace IECore
{

template<typename T>
struct DataPromoteOp::Promote2Fn<T, typename boost::enable_if< TypeTraits::IsVectorTypedData<T> >::type >
{
	typedef DataPtr ReturnType;

	template<typename F>
	ReturnType operator()( typename F::ConstPtr d ) const
	{
		assert( d );
		typename T::Ptr result = new T;
		typename T::ValueType &vt = result->writable();
		const typename F::ValueType &vf = d->readable();
		vt.resize( vf.size() );
		typename T::ValueType::iterator tIt = vt.begin();
		for ( typename F::ValueType::const_iterator it = vf.begin(); it!=vf.end(); it++ )
		{
			*tIt++ = typename T::ValueType::value_type( *it );
		}
		return result;
	}
};

template<typename T>
struct DataPromoteOp::Promote2Fn<T, typename boost::enable_if< TypeTraits::IsSimpleTypedData<T> >::type >
{
	typedef DataPtr ReturnType;

	template<typename F>
	ReturnType operator()( typename F::ConstPtr d ) const
	{
		assert( d );	
		typename T::Ptr result = new T;

		result->writable() = typename T::ValueType( d->readable() );

		return result;
	}
};

struct DataPromoteOp::Promote1Fn
{
	typedef DataPtr ReturnType;

	TypeId m_targetType;

	Promote1Fn( TypeId targetType ) : m_targetType( targetType )
	{
	}

	template<typename T, typename Enable = void >
	struct Func
	{
		ReturnType operator()( typename T::ConstPtr d, TypeId ) const
		{
			assert( d );
			throw Exception( "DataPromoteOp: Unsupported source data type \"" + d->typeName() + "\"." );
		}
	};

	template<typename F>
	ReturnType operator()( typename F::ConstPtr d ) const
	{
		assert( d );
		Func<F> f;
		return f(d, m_targetType);
	}

};

template<typename F >
struct DataPromoteOp::Promote1Fn::Func< F, typename boost::enable_if< TypeTraits::IsNumericVectorTypedData<F> >::type >
{
	ReturnType operator()( typename F::ConstPtr d, TypeId targetType ) const
	{
		assert( d );	
		switch ( targetType )
		{
		case V2fVectorDataTypeId :
		{
			Promote2Fn<V2fVectorData> fn;
			return fn.template operator()<F>( d );
		}
		case V2dVectorDataTypeId :
		{
			Promote2Fn<V2dVectorData> fn;
			return fn.template operator()<F>( d );
		}
		case V3fVectorDataTypeId :
		{
			Promote2Fn<V3fVectorData> fn;
			return fn.template operator()<F>( d );
		}
		case V3dVectorDataTypeId :
		{
			Promote2Fn<V3dVectorData> fn;
			return fn.template operator()<F>( d );
		}
		case Color3fVectorDataTypeId :
		{
			Promote2Fn<Color3fVectorData> fn;
			return fn.template operator()<F>( d );
		}
		default :
			throw Exception( "DataPromoteOp: Unsupported target data type \"" + Object::typeNameFromTypeId( targetType ) + "\"." );
		}
	}
};

template<typename F >
struct DataPromoteOp::Promote1Fn::Func< F, typename boost::enable_if< TypeTraits::IsNumericSimpleTypedData<F> >::type >
{
	ReturnType operator()( typename F::ConstPtr d, TypeId targetType ) const
	{
		assert( d );	
		switch ( targetType )
		{
		case V2fDataTypeId :
		{
			Promote2Fn<V2fData> fn;
			return fn.template operator()<F>( d );
		}
		case V2dDataTypeId :
		{
			Promote2Fn<V2dData> fn;
			return fn.template operator()<F>( d );
		}
		case V3fDataTypeId :
		{
			Promote2Fn<V3fData> fn;
			return fn.template operator()<F>( d );
		}
		case V3dDataTypeId :
		{
			Promote2Fn<V3dData> fn;
			return fn.template operator()<F>( d );
		}
		case Color3fDataTypeId :
		{
			Promote2Fn<Color3fData> fn;
			return fn.template operator()<F>( d );
		}
		default :
			throw Exception( "DataPromoteOp: Unsupported target data type \"" + Object::typeNameFromTypeId( targetType ) + "\"." );
		}
	}
};

} // namespace IECore

ObjectPtr DataPromoteOp::doOperation( ConstCompoundObjectPtr operands )
{
	assert( operands );

	const TypeId targetType = (TypeId)m_targetTypeParameter->getNumericValue();
	DataPtr srcData = static_pointer_cast<Data>( m_objectParameter->getValue() );
	assert( srcData );

	Promote1Fn fn( targetType );

	DataPtr targetData = despatchTypedData< Promote1Fn, TypeTraits::IsNumericTypedData >( srcData, fn );
	assert( targetData );
	assert( targetData->typeId() == targetType );

#ifndef NDEBUG
	size_t srcSize = despatchTypedData< TypedDataSize >( srcData ) ;
	size_t targetSize = despatchTypedData< TypedDataSize >( targetData ) ;

	/// This post-condition is stated in the class documentation
	assert( srcSize == targetSize );
#endif

	return targetData;
}
