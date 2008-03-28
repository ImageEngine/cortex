//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include "IECore/MatrixMultiplyOp.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/NullObject.h"
#include "IECore/TypedDataDespatch.h"

#include <cassert>

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;
using namespace boost::mpl;

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
		staticTypeName(),
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
	
	IntParameter::PresetsMap modePresets;
	modePresets["point"] = Point;
	modePresets["vector"] = Vector;
	modePresets["normal"] = Normal;
	m_modeParameter = new IntParameter(
		"mode",
		"The interpretation of the vectors, which modifies the way "
		"in which they are transformed.",
		Point,
		Point,
		Normal,
		modePresets,
		true
	);
	parameters()->addParameter( m_matrixParameter );
	parameters()->addParameter( m_modeParameter );
}

MatrixMultiplyOp::~MatrixMultiplyOp()
{

}

ObjectParameterPtr MatrixMultiplyOp::matrixParameter()
{
	return m_matrixParameter;
}

ConstObjectParameterPtr MatrixMultiplyOp::matrixParameter() const
{
	return m_matrixParameter;
}

IntParameterPtr MatrixMultiplyOp::modeParameter()
{
	return m_modeParameter;
}

ConstIntParameterPtr MatrixMultiplyOp::modeParameter() const
{
	return m_modeParameter;
}

struct MultiplyArgs
{
	DataPtr data;
	ConstObjectPtr matrix;
	MatrixMultiplyOp::Mode mode;
};

template< typename U, typename Enable = void >
struct MultiplyFunctor
{
	void operator() ( typename U::Ptr data, MultiplyArgs args )
	{
		// empty functor.
	}
};

template< typename U >
struct MultiplyFunctor< U, typename enable_if< or_< is_same< U, V3fVectorData >, is_same< U, V3dVectorData > > >::type >
{

	template< typename T >
	void multiply33( typename U::Ptr data, const T &matrix, MatrixMultiplyOp::Mode mode )
	{
		typename U::ValueType::iterator beginIt = data->writable().begin();
		typename U::ValueType::iterator endIt = data->writable().end();
		if( mode==MatrixMultiplyOp::Point || mode==MatrixMultiplyOp::Vector )
		{
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{
				*it *= matrix;
			}
		}
		else
		{
			// normal
			T m = matrix.inverse();
			m.transpose();
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{	
				*it *= matrix;
			}
		}
	}
	
	template< typename T >
	void multiply( typename U::Ptr data, const T &matrix, MatrixMultiplyOp::Mode mode )
	{
		typename U::ValueType::iterator beginIt = data->writable().begin();
		typename U::ValueType::iterator endIt = data->writable().end();
		if( mode==MatrixMultiplyOp::Point )
		{
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{
				*it *= matrix;
			}
		}
		else if( mode==MatrixMultiplyOp::Vector )
		{
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{	
				matrix.multDirMatrix( *it, *it );
			}
		}
		else
		{
			// normal
			T m = matrix.inverse();
			m.transpose();
			for ( typename U::ValueType::iterator it = beginIt; it != endIt; it++ )
			{	
				m.multDirMatrix( *it, *it );
			}
		}
	}

	void operator() ( typename U::Ptr data, MultiplyArgs args )
	{
		switch ( args.matrix->typeId() )
		{
		case M33fDataTypeId:
			multiply33( data, static_pointer_cast< const M33fData >( args.matrix )->readable(), args.mode );
			break;
		case M33dDataTypeId:
			multiply33( data, static_pointer_cast< const M33dData >( args.matrix )->readable(), args.mode );
			break;
		case M44fDataTypeId:
			multiply( data, static_pointer_cast< const M44fData >( args.matrix )->readable(), args.mode );
			break;
		case M44dDataTypeId:
			multiply( data, static_pointer_cast< const M44dData >( args.matrix )->readable(), args.mode );
			break;
		case TransformationMatrixfDataTypeId:
			multiply( data, static_pointer_cast< const TransformationMatrixfData >( args.matrix )->readable().transform(), args.mode );
			break;
		case TransformationMatrixdDataTypeId:
			multiply( data, static_pointer_cast< const TransformationMatrixdData >( args.matrix )->readable().transform(), args.mode );
			break;
		default:
			throw InvalidArgumentException( "Data supplied is not a known matrix type." );
		}
	}
};

void MatrixMultiplyOp::modify( ObjectPtr toModify, ConstCompoundObjectPtr operands )
{
	DataPtr data = static_pointer_cast< Data >( toModify );
	MultiplyArgs args = { data, m_matrixParameter->getValue(), (Mode)m_modeParameter->getNumericValue() };
	despatchVectorTypedDataFn< void, MultiplyFunctor, MultiplyArgs>( data, args );
}
