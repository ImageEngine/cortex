//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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


#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CurvesPrimitiveEvaluator.h"
#include "IECore/CurvesAlgo.h"

using namespace IECore;
using namespace Imath;

namespace
{

// we can't perform the numeric value averaging for Box and Quat data,
// so we make our own TypeTrait to explicitly mark them as unsupported
template< typename T > struct IsArithmeticVectorTypedData : boost::mpl::and_<
	TypeTraits::IsNumericBasedVectorTypedData<T>,
	boost::mpl::not_< TypeTraits::IsBox<typename TypeTraits::VectorValueType<T>::type > >,
boost::mpl::not_< TypeTraits::IsQuat<typename TypeTraits::VectorValueType<T>::type > >
> {};

// PrimitiveEvaluator only supports certain types
template< typename T > struct IsPrimitiveEvaluatableTypedData : boost::mpl::and_<
	TypeTraits::IsNumericBasedVectorTypedData<T>,
	boost::mpl::or_<
		TypeTraits::IsVec3<typename TypeTraits::VectorValueType<T>::type >,
	boost::is_same< typename TypeTraits::VectorValueType<T>::type, float >,
	boost::is_same< typename TypeTraits::VectorValueType<T>::type, int >,
	TypeTraits::IsColor3<typename TypeTraits::VectorValueType<T>::type >
>
> {};


template<typename T>
static T evalPrimVar( PrimitiveEvaluator::Result *result, const PrimitiveVariable &primVar )
{
	throw Exception( "PrimvarResamplerCache : This should never be called because of IsPrimitiveEvaluatableTypedData" );
	return T();
}


template<>
Imath::V3f evalPrimVar<Imath::V3f>( PrimitiveEvaluator::Result *result, const PrimitiveVariable &primVar )
{
	return result->vectorPrimVar( primVar );
}

template<>
float evalPrimVar<float>( PrimitiveEvaluator::Result *result, const PrimitiveVariable &primVar )
{
	return result->floatPrimVar( primVar );
}

template<>
int evalPrimVar<int>( PrimitiveEvaluator::Result *result, const PrimitiveVariable &primVar )
{
	return result->intPrimVar( primVar );
}

template<>
Imath::Color3f evalPrimVar<Imath::Color3f>( PrimitiveEvaluator::Result *result, const PrimitiveVariable &primVar )
{
	return result->colorPrimVar( primVar );
}


struct  CurvesUniformToVertex
{
	typedef DataPtr ReturnType;

	CurvesUniformToVertex( const std::vector<int> &offsets )	:	m_offsets( offsets )
	{
	}

	template<typename From> ReturnType operator()( typename From::ConstPtr data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		typename From::ValueType::const_iterator srcIt = data->readable().begin();

		trg.reserve( m_offsets.size() );

		for ( std::vector<int>::const_iterator oIt = m_offsets.begin(); oIt != m_offsets.end(); oIt++, srcIt++ )
		{
			for ( int i = 0; i < *oIt; i++ )
			{
				trg.push_back( *srcIt );
			}
		}
		return result;
	}

	const std::vector<int> &m_offsets;
};

struct  CurvesVertexToUniform
{
	typedef DataPtr ReturnType;

	CurvesVertexToUniform( const CurvesPrimitive *curves ) : m_curves( curves )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		typename From::ValueType::const_iterator srcIt = data->readable().begin();

		trg.reserve( m_curves->variableSize( PrimitiveVariable::Uniform ) );

		const std::vector<int> &offsets = m_curves->verticesPerCurve()->readable();
		for( std::vector<int>::const_iterator oIt = offsets.begin(); oIt != offsets.end(); ++oIt )
		{
			// initialize with the first value to avoid
			// ambiguitity during default construction
			typename From::ValueType::value_type total = *srcIt;
			++srcIt;

			for( int i = 1; i < *oIt; ++i, ++srcIt )
			{
				total += *srcIt;
			}

			trg.push_back( total / *oIt );
		}

		return result;
	}

	const CurvesPrimitive *m_curves;
};

struct  CurvesUniformToVarying
{
	typedef DataPtr ReturnType;

	CurvesUniformToVarying( const CurvesPrimitive *curves ) : m_curves( curves )
	{
	}

	template<typename From> ReturnType operator()( typename From::ConstPtr data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		typename From::ValueType::const_iterator srcIt = data->readable().begin();

		trg.reserve( m_curves->variableSize( PrimitiveVariable::Varying ) );

		size_t numCurves = m_curves->numCurves();
		for( size_t i = 0; i < numCurves; ++i, ++srcIt )
		{
			for( size_t j = 0; j < m_curves->numSegments( i ) + 1; ++j )
			{
				trg.push_back( *srcIt );
			}
		}

		return result;
	}

	const CurvesPrimitive *m_curves;
};

struct  CurvesVaryingToUniform
{
	typedef DataPtr ReturnType;

	CurvesVaryingToUniform( const CurvesPrimitive *curves ) : m_curves( curves )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		typename From::ValueType::const_iterator srcIt = data->readable().begin();

		trg.reserve( m_curves->variableSize( PrimitiveVariable::Uniform ) );

		size_t numCurves = m_curves->numCurves();
		for( size_t i = 0; i < numCurves; ++i )
		{
			// initialize with the first value to avoid
			// ambiguitity during default construction
			typename From::ValueType::value_type total = *srcIt;
			++srcIt;

			size_t varyingSize = m_curves->numSegments( i ) + 1;
			for( size_t j = 1; j < varyingSize; ++j, ++srcIt )
			{
				total += *srcIt;
			}

			trg.push_back( total / varyingSize );
		}

		return result;
	}

	const CurvesPrimitive *m_curves;
};

struct  CurvesVertexToVarying
{
	typedef DataPtr ReturnType;

	CurvesVertexToVarying( const CurvesPrimitive *curves ) : m_curves( curves )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();

		trg.reserve( m_curves->variableSize( PrimitiveVariable::Varying ) );

		const PrimitiveVariable *primVar = NULL;
		for( PrimitiveVariableMap::const_iterator it = m_curves->variables.begin(); it != m_curves->variables.end(); ++it )
		{
			if( it->second.data->isEqualTo( data ) )
			{
				primVar = &it->second;
				break;
			}
		}

		if( !primVar )
		{
			return NULL;
		}

		IECore::ConstCurvesPrimitiveEvaluatorPtr evaluator =  runTimeCast<CurvesPrimitiveEvaluator>(IECore::PrimitiveEvaluator::create( m_curves )) ;

//		ConstCurvesPrimitiveEvaluatorPtr evaluator = SharedPrimitiveEvaluators::getTyped< CurvesPrimitiveEvaluator >( m_curves );
		if( !evaluator )
		{
			return NULL;
		}

		PrimitiveEvaluator::ResultPtr evaluatorResult = evaluator->createResult();

		size_t numCurves = m_curves->numCurves();
		for( size_t i = 0; i < numCurves; ++i )
		{
			size_t numSegments = m_curves->numSegments( i );
			float step = 1.0f / numSegments;
			for( size_t j = 0; j < numSegments + 1; ++j )
			{
				evaluator->pointAtV( i, j * step, evaluatorResult.get() );
				trg.push_back( evalPrimVar<typename From::ValueType::value_type>( evaluatorResult.get(), *primVar ) );
			}
		}

		return result;
	}

	const CurvesPrimitive *m_curves;
};

struct  CurvesVaryingToVertex
{
	typedef DataPtr ReturnType;

	CurvesVaryingToVertex( const CurvesPrimitive *curves ) : m_curves( curves )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();

		trg.reserve( m_curves->variableSize( PrimitiveVariable::Vertex ) );


		const PrimitiveVariable *primVar = NULL;
		for( PrimitiveVariableMap::const_iterator it = m_curves->variables.begin(); it != m_curves->variables.end(); ++it )
		{
			if( it->second.data->isEqualTo( data ) )
			{
				primVar = &it->second;
				break;
			}
		}

		if( !primVar )
		{
			return NULL;
		}

		IECore::ConstCurvesPrimitiveEvaluatorPtr evaluator =  runTimeCast<CurvesPrimitiveEvaluator>(IECore::PrimitiveEvaluator::create( m_curves )) ;
		if( !evaluator )
		{
			return NULL;
		}

		PrimitiveEvaluator::ResultPtr evaluatorResult = evaluator->createResult();

		size_t numCurves = m_curves->numCurves();
		const std::vector<int> &verticesPerCurve = evaluator->verticesPerCurve();
		for( size_t i = 0; i < numCurves; ++i )
		{
			float step = 1.0f / verticesPerCurve[i];
			for( size_t j = 0; j < verticesPerCurve[i]; ++j )
			{
				evaluator->pointAtV( i, j * step, evaluatorResult.get() );
				trg.push_back( evalPrimVar<typename From::ValueType::value_type>( evaluatorResult.get(), *primVar ) );
			}
		}

		return result;
	}

	const CurvesPrimitive *m_curves;
};

struct  AverageValueFromVector
{
	typedef DataPtr ReturnType;

	template<typename From> ReturnType operator()( typename From::ConstPtr data )
	{
		const typename From::ValueType &src = data->readable();
		if ( src.size() )
		{
			return new TypedData< typename From::ValueType::value_type >( std::accumulate( src.begin() + 1, src.end(), *src.begin() ) / src.size() );
		}
		return NULL;
	}
};

} //anonymous namespace

namespace IECore
{
namespace CurvesAlgo
{

PrimitiveVariable resamplePrimitiveVariable( const CurvesPrimitive &curves, const PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation )
{
	DataPtr result;


	if ( interpolation == PrimitiveVariable::Constant )
	{
		AverageValueFromVector fn;
		result = despatchTypedData<AverageValueFromVector, IsArithmeticVectorTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
	}

	if ( primitiveVariable.interpolation == PrimitiveVariable::Constant )
	{
		size_t len = curves.variableSize( interpolation );
		switch( primitiveVariable.data->typeId() )
		{
			case IntDataTypeId:
			{
				IntVectorDataPtr newData = new IntVectorData();
				newData->writable().resize( len, static_cast< const IntData * >( primitiveVariable.data.get() )->readable() );
				result = newData;
			}
				break;
			case FloatDataTypeId:
			{
				FloatVectorDataPtr newData = new FloatVectorData();
				newData->writable().resize( len, static_cast< const FloatData * >( primitiveVariable.data.get() )->readable() );
				result = newData;
			}
				break;
			case V2fDataTypeId:
			{
				V2fVectorDataPtr newData = new V2fVectorData();
				newData->writable().resize( len, static_cast< const V2fData * >( primitiveVariable.data.get() )->readable() );
				result = newData;
			}
				break;
			case V3fDataTypeId:
			{
				V3fVectorDataPtr newData = new V3fVectorData();
				newData->writable().resize( len, static_cast< const V3fData * >( primitiveVariable.data.get() )->readable() );
				result = newData;
			}
				break;
			case Color3fDataTypeId:
			{
				Color3fVectorDataPtr newData = new Color3fVectorData();
				newData->writable().resize( len, static_cast< const Color3fData * >( primitiveVariable.data.get() )->readable() );
				result = newData;
			}
				break;
			default:
				// return 0 if it was constant and for all the other interpolation, like uniform it may still work on following primitive specific code.
				break;
		}
	}

	if ( interpolation == PrimitiveVariable::Uniform )
	{
		if ( primitiveVariable.interpolation == PrimitiveVariable::Vertex )
		{
			CurvesVertexToUniform fn( &curves );
			result = despatchTypedData<CurvesVertexToUniform, IsArithmeticVectorTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
		}
		else if ( primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			CurvesVaryingToUniform fn( &curves );
			result = despatchTypedData<CurvesVaryingToUniform, IsArithmeticVectorTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
		}
	}
	else if ( interpolation == PrimitiveVariable::Vertex )
	{
		if ( primitiveVariable.interpolation == PrimitiveVariable::Uniform )
		{
			CurvesUniformToVertex fn( curves.verticesPerCurve()->readable() );
			result = despatchTypedData<CurvesUniformToVertex, TypeTraits::IsNumericBasedVectorTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
		}
		else if ( primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			CurvesVaryingToVertex fn( &curves );
			result = despatchTypedData<CurvesVaryingToVertex, IsPrimitiveEvaluatableTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
		}
	}
	else if ( interpolation == PrimitiveVariable::Varying || interpolation == PrimitiveVariable::FaceVarying )
	{
		if ( primitiveVariable.interpolation == PrimitiveVariable::Uniform )
		{
			CurvesUniformToVarying fn( &curves );
			result = despatchTypedData<CurvesUniformToVarying, TypeTraits::IsNumericBasedVectorTypedData>( const_cast< Data * >( primitiveVariable.data.get()), fn );
		}
		else if ( primitiveVariable.interpolation == PrimitiveVariable::Vertex )
		{
			CurvesVertexToVarying fn( &curves );
			result = despatchTypedData<CurvesVertexToVarying, IsPrimitiveEvaluatableTypedData>( const_cast< Data * >( primitiveVariable.data.get() ), fn );
		}
		else if ( primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			result = primitiveVariable.data;
		}
	}
	return PrimitiveVariable(interpolation, result);
}

} //namespace CurveAlgo
} //namespace IECore
