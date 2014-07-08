//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/mpl/eval_if.hpp"
#include "boost/mpl/or.hpp"
#include "boost/type_traits/is_same.hpp"
#include "boost/utility/enable_if.hpp"

#include "IECore/MatrixMultiplyOp.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/NullObject.h"
#include "IECore/DespatchTypedData.h"

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;
using namespace boost::mpl;

IE_CORE_DEFINERUNTIMETYPED( MatrixMultiplyOp );

static TypeId inputTypes[] =
{
	V3fVectorDataTypeId,
	V3dVectorDataTypeId,
	InvalidTypeId
};

static TypeId matrixTypes[] =
{
	M33fDataTypeId,
	M33dDataTypeId,
	M44fDataTypeId,
	M44dDataTypeId,
	TransformationMatrixfDataTypeId,
	TransformationMatrixdDataTypeId,
	InvalidTypeId
};

MatrixMultiplyOp::MatrixMultiplyOp()
	:	ModifyOp(
		"Performs inplace matrix multiplication on 3D vector Data types.",
		new ObjectParameter(
			"result",
			"Converted Data object.",
			new NullObject(),
			DataTypeId
		),
		new ObjectParameter(
			"object",
			"The vector object that will be transformed by the matrix.",
			new NullObject(),
			&inputTypes[0]
		)
	)
{
	m_matrixParameter = new ObjectParameter(
		"matrix",
		"The matrix or transformation used on the multiplication.",
		new M44fData(),
		&matrixTypes[0]
	);

	parameters()->addParameter( m_matrixParameter );
}

MatrixMultiplyOp::~MatrixMultiplyOp()
{

}

ObjectParameter * MatrixMultiplyOp::matrixParameter()
{
	return m_matrixParameter.get();
}

const ObjectParameter * MatrixMultiplyOp::matrixParameter() const
{
	return m_matrixParameter.get();
}

struct MultiplyFunctor
{
	typedef void ReturnType;

	DataPtr data;
	ConstObjectPtr matrix;

	template< typename T, typename U >
	void multiply33( U * data, const T &matrix )
	{
		assert( data );
		
		GeometricData::Interpretation mode = data->getInterpretation();
		typename U::ValueType::iterator beginIt = data->writable().begin();
		typename U::ValueType::iterator endIt = data->writable().end();
		if ( mode==GeometricData::Point || mode==GeometricData::Vector )
		{
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{
				*it *= matrix;
			}
		}
		else if ( mode == GeometricData::Normal )
		{
			T m = matrix.inverse();
			m.transpose();
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{
				*it *= matrix;
			}
		}
	}

	template< typename T, typename U >
	void multiply( U * data, const T &matrix )
	{
		assert( data );
		
		GeometricData::Interpretation mode = data->getInterpretation();
		typename U::ValueType::iterator beginIt = data->writable().begin();
		typename U::ValueType::iterator endIt = data->writable().end();
		if ( mode == GeometricData::Point )
		{
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{
				*it *= matrix;
			}
		}
		else if ( mode == GeometricData::Vector )
		{
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{
				matrix.multDirMatrix( *it, *it );
			}
		}
		else if ( mode == GeometricData::Normal )
		{
			T m = matrix.inverse();
			m.transpose();
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{
				m.multDirMatrix( *it, *it );
			}
		}
	}

	template<typename U>
	void operator() ( U * data )
	{
		assert( data );

		switch ( matrix->typeId() )
		{
		case M33fDataTypeId:
			multiply33<M33f, U>( data, boost::static_pointer_cast< const M33fData >( matrix )->readable() );
			break;
		case M33dDataTypeId:
			multiply33<M33d, U>( data, boost::static_pointer_cast< const M33dData >( matrix )->readable() );
			break;
		case M44fDataTypeId:
			multiply<M44f, U>( data, boost::static_pointer_cast< const M44fData >( matrix )->readable() );
			break;
		case M44dDataTypeId:
			multiply<M44d, U>( data, boost::static_pointer_cast< const M44dData >( matrix )->readable() );
			break;
		case TransformationMatrixfDataTypeId:
			multiply<M44f, U>( data, boost::static_pointer_cast< const TransformationMatrixfData >( matrix )->readable().transform() );
			break;
		case TransformationMatrixdDataTypeId:
			multiply<M44d, U>( data, boost::static_pointer_cast< const TransformationMatrixdData >( matrix )->readable().transform() );
			break;
		default:
			throw InvalidArgumentException( "Data supplied is not a known matrix type." );
		}
	}

};

void MatrixMultiplyOp::modify( Object * toModify, const CompoundObject * operands )
{
	Data *data = static_cast< Data * >( toModify );
	MultiplyFunctor func = { data, m_matrixParameter->getValue() };
	despatchTypedData< MultiplyFunctor, TypeTraits::IsFloatVec3VectorTypedData >( data, func );
}
