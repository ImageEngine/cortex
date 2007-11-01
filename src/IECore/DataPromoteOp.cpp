//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <boost/format.hpp>

#include "IECore/DataPromoteOp.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/NullObject.h"

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

template<typename T, typename F>
DataPtr promote2( typename F::ConstPtr d )
{
	typename T::Ptr result = new T;
	typename T::ValueType &vt = result->writable();
	const typename F::ValueType &vf = d->readable();
	vt.resize( vf.size() );
	typename T::ValueType::iterator tIt = vt.begin();
	for( typename F::ValueType::const_iterator it = vf.begin(); it!=vf.end(); it++ )
	{
		*tIt++ = typename T::ValueType::value_type( *it );
	}
	return result;
}

template<typename F>
DataPtr promote1( typename F::ConstPtr d, TypeId targetType )
{
	switch( targetType )
	{
		case V2fVectorDataTypeId :
			return promote2<V3dVectorData, F>( d );	
		case V2dVectorDataTypeId :
			return promote2<V3dVectorData, F>( d );
		case V3fVectorDataTypeId :
			return promote2<V3fVectorData, F>( d );
		case V3dVectorDataTypeId :
			return promote2<V3dVectorData, F>( d );
		case Color3fVectorDataTypeId :
			return promote2<Color3fVectorData, F>( d );
		default :
			throw Exception( "Unsupported target data type \"" + Object::typeNameFromTypeId( targetType ) + "\"." );
	}
}

/// \todo Promotions for the SimpleTypedData classes too.
/// \todo I'm sure there's something we can do in TypedDataDespatch that would
/// allow us to despatch to specific subgroups of data (in this case scalar data)
/// based on some compile-time predicate thingummy. That would be really useful
/// in a whole bunch of places.
ObjectPtr DataPromoteOp::doOperation( ConstCompoundObjectPtr operands )
{
	const TypeId targetType = (TypeId)m_targetTypeParameter->getNumericValue();
	ConstDataPtr srcData = static_pointer_cast<const Data>( m_objectParameter->getValue() );
	
	switch( srcData->typeId() )
	{
		case UIntVectorDataTypeId :
			return promote1<UIntVectorData>( static_pointer_cast<const UIntVectorData>( srcData ), targetType );
		case IntVectorDataTypeId :
			return promote1<IntVectorData>( static_pointer_cast<const IntVectorData>( srcData ), targetType );
		case FloatVectorDataTypeId :
			return promote1<FloatVectorData>( static_pointer_cast<const FloatVectorData>( srcData ), targetType );
		case DoubleVectorDataTypeId :
			return promote1<DoubleVectorData>( static_pointer_cast<const DoubleVectorData>( srcData ), targetType );	
		default :
			throw Exception( "Unsupported source data type \"" + srcData->typeName() + "\"." );
	}
}
