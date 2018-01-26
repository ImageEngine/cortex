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


#include "IECoreScene/CurvesAlgo.h"

#include "IECoreScene/CurvesPrimitiveEvaluator.h"
#include "IECoreScene/private/PrimitiveAlgoUtils.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;

namespace
{


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

		const PrimitiveVariable *primVar = nullptr;
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
			return nullptr;
		}

		ConstCurvesPrimitiveEvaluatorPtr evaluator = new CurvesPrimitiveEvaluator( m_curves );
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


		const PrimitiveVariable *primVar = nullptr;
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
			return nullptr;
		}

		ConstCurvesPrimitiveEvaluatorPtr evaluator = new CurvesPrimitiveEvaluator( m_curves );
		PrimitiveEvaluator::ResultPtr evaluatorResult = evaluator->createResult();

		size_t numCurves = m_curves->numCurves();
		const std::vector<int> &verticesPerCurve = evaluator->verticesPerCurve();
		for( size_t i = 0; i < numCurves; ++i )
		{
			float step = 1.0f / verticesPerCurve[i];
			for( int j = 0; j < verticesPerCurve[i]; ++j )
			{
				evaluator->pointAtV( i, j * step, evaluatorResult.get() );
				trg.push_back( evalPrimVar<typename From::ValueType::value_type>( evaluatorResult.get(), *primVar ) );
			}
		}

		return result;
	}

	const CurvesPrimitive *m_curves;
};

// todo: this was lifted from MeshAlgo and the duplicate class should be refactored into PrimitiveAlgoUtils
template<typename U>
class DeleteFlaggedUniformFunctor
{
	public:
		typedef DataPtr ReturnType;

		DeleteFlaggedUniformFunctor( typename IECore::TypedData<std::vector<U> >::ConstPtr flagData, bool invert ) : m_flagData( flagData ), m_invert ( invert )
		{
		}

		template<typename T>
		ReturnType operator()( const T *data )
		{
			const typename T::ValueType &inputs = data->readable();
			const std::vector<U> &flags = m_flagData->readable();

			T *filteredResultData = new T();
			typename T::ValueType &filteredResult = filteredResultData->writable();

			filteredResult.reserve( inputs.size() );

			for( size_t i = 0; i < inputs.size(); ++i )
			{
				if( (m_invert && flags[i]) || (!m_invert && !flags[i]) )
				{
					filteredResult.push_back( inputs[i] );
				}
			}

			return filteredResultData;
		}

	private:
		typename IECore::TypedData<std::vector<U> >::ConstPtr m_flagData;
		bool m_invert;
};

// todo: this was lifted from MeshAlgo and the duplicate class should be refactored into PrimitiveAlgoUtils
// todo: I've renamed a few things but it's the same code as MeshAlgo::deleteFaces
// todo: note it's a duplicate of DeleteFlaggedFaceVaryingFunctor in MeshAlgo.cpp
template<typename U>
class DeleteFlaggedVertexFunctor
{
	public:
		typedef DataPtr ReturnType;

		DeleteFlaggedVertexFunctor(  typename IECore::TypedData<std::vector<U> >::ConstPtr flagData, ConstIntVectorDataPtr verticesPerCurve, bool invert ) : m_flagData( flagData ), m_verticesPerCurve( verticesPerCurve ), m_invert ( invert )
		{
		}

		template<typename T>
		ReturnType operator()( const T *data )
		{
			const typename T::ValueType &inputs = data->readable();
			const std::vector<int> &verticesPerCurve= m_verticesPerCurve->readable();
			const std::vector<U> &flags = m_flagData->readable();

			T *filteredResultData = new T();
			typename T::ValueType &filteredResult = filteredResultData->writable();

			filteredResult.reserve( inputs.size() );

			size_t offset = 0;
			for( size_t c = 0; c < verticesPerCurve.size(); ++c )
			{
				int numVerts = verticesPerCurve[c];
				if( (m_invert && flags[c]) || (!m_invert && !flags[c]) )
				{
					for( int v = 0; v < numVerts; ++v )
					{
						filteredResult.push_back( inputs[offset + v] );
					}
				}
				offset += numVerts;
			}

			return filteredResultData;
		}
	private:
		typename IECore::TypedData<std::vector<U> >::ConstPtr m_flagData;
		ConstIntVectorDataPtr m_verticesPerCurve;
		bool m_invert;
};

template<typename U>
class DeleteFlaggedVaryingFunctor
{
	public:
		typedef DataPtr ReturnType;

		DeleteFlaggedVaryingFunctor( typename IECore::TypedData<std::vector<U> >::ConstPtr flagData, const CurvesPrimitive* curvesPrimitive, bool invert ) : m_flagData( flagData ), m_curvesPrimitive (curvesPrimitive), m_invert ( invert )
		{
		}

		template<typename T>
		ReturnType operator()( const T *data )
		{
			const typename T::ValueType &inputs = data->readable();
			const std::vector<U> &flags = m_flagData->readable();

			T *filteredResultData = new T();
			typename T::ValueType &filteredResult = filteredResultData->writable();

			filteredResult.reserve( inputs.size() );

			size_t offset = 0;
			for( size_t c = 0; c < m_curvesPrimitive->numCurves(); ++c )
			{
				int numVarying = m_curvesPrimitive->numSegments( c ) + 1;

				if( (m_invert && flags[c]) || (!m_invert && !flags[c]) )
				{
					for( int v = 0; v < numVarying; ++v )
					{
						filteredResult.push_back( inputs[offset + v] );
					}
				}
				offset += numVarying;
			}

			return filteredResultData;
		}
	private:
		typename IECore::TypedData<std::vector<U> >::ConstPtr m_flagData;
		const CurvesPrimitive *m_curvesPrimitive;
		bool m_invert;
};

template<typename T>
CurvesPrimitivePtr deleteCurves( const CurvesPrimitive *curvesPrimitive, const typename IECore::TypedData<std::vector<T> > *deleteFlagData, bool invert )
{
	DeleteFlaggedUniformFunctor<T> deleteUniformFn( deleteFlagData, invert );
	DeleteFlaggedVertexFunctor<T> deleteVertexFn ( deleteFlagData, curvesPrimitive->verticesPerCurve(), invert );
	DeleteFlaggedVaryingFunctor<T> deleteVaryingFn ( deleteFlagData, curvesPrimitive, invert );

	IECore::Data *inputVertsPerCurve = const_cast< IECore::Data * >( IECore::runTimeCast<const IECore::Data>( curvesPrimitive->verticesPerCurve() ) );
	IECore::DataPtr outputVertsPerCurve = despatchTypedData<DeleteFlaggedUniformFunctor<T>, TypeTraits::IsVectorTypedData>( inputVertsPerCurve, deleteUniformFn );

	IntVectorDataPtr verticesPerCurve = IECore::runTimeCast<IECore::IntVectorData>(outputVertsPerCurve);

	CurvesPrimitivePtr outCurvesPrimitive = new CurvesPrimitive( verticesPerCurve, curvesPrimitive->basis(), curvesPrimitive->periodic() );

	for (PrimitiveVariableMap::const_iterator it = curvesPrimitive->variables.begin(), e = curvesPrimitive->variables.end(); it != e; ++it)
	{
		switch( it->second.interpolation )
		{
			case PrimitiveVariable::Constant:
			case PrimitiveVariable::Invalid:
			{
				outCurvesPrimitive->variables[it->first] = it->second;
				break;
			}
			case PrimitiveVariable::Uniform:
			{
				IECore::Data *inputData = const_cast< IECore::Data * >( it->second.data.get() );
				IECore::DataPtr ouptputData = despatchTypedData<DeleteFlaggedUniformFunctor<T> , TypeTraits::IsVectorTypedData>( inputData, deleteUniformFn );
				outCurvesPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, ouptputData );

				break;
			}
			case PrimitiveVariable::Varying:
			case PrimitiveVariable::FaceVarying:
			{
				IECore::Data *inputData = const_cast< IECore::Data * >( it->second.data.get() );
				IECore::DataPtr ouptputData = despatchTypedData<DeleteFlaggedVaryingFunctor<T>, TypeTraits::IsVectorTypedData>( inputData, deleteVaryingFn );
				outCurvesPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, ouptputData );

				break;
			}
			case PrimitiveVariable::Vertex:
			{
				IECore::Data *inputData = const_cast< IECore::Data * >( it->second.data.get() );
				IECore::DataPtr ouptputData = despatchTypedData<DeleteFlaggedVertexFunctor<T>, TypeTraits::IsVectorTypedData>( inputData, deleteVertexFn );
				outCurvesPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, ouptputData );
				break;
			}
		}
	}

	return outCurvesPrimitive;
}
} //anonymous namespace

namespace IECoreScene
{
namespace CurvesAlgo
{

void resamplePrimitiveVariable( const CurvesPrimitive *curves, PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation )
{

	if ( interpolation == primitiveVariable.interpolation)
	{
		return;
	}

	DataPtr dstData = nullptr;
	DataPtr srcData = nullptr;

	if( primitiveVariable.indices )
	{
		if( primitiveVariable.interpolation == PrimitiveVariable::Vertex && ( interpolation == PrimitiveVariable::Varying || interpolation == PrimitiveVariable::FaceVarying ) )
		{
			// \todo: fix CurvesVertexToVarying so it works with arbitrary PrimitiveVariables
			// rather than requiring the variables exist on the input CurvesPrimitive.
			throw InvalidArgumentException( "CurvesAlgo::resamplePrimitiveVariable : Resampling indexed Vertex variables to FaceVarying/Varying is not currently supported. Expand indices first." );
		}
		else if( ( primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying ) && interpolation == PrimitiveVariable::Vertex )
		{
			// \todo: fix CurvesVaryingToVertex so it works with arbitrary PrimitiveVariables
			// rather than requiring the variables exist on the input CurvesPrimitive.
			throw InvalidArgumentException( "CurvesAlgo::resamplePrimitiveVariable : Resampling indexed FaceVarying/Varying variables to Vertex is not currently supported. Expand indices first." );
		}
		else if( primitiveVariable.interpolation < interpolation )
		{
			// upsampling can be a resampling of indices
			srcData = primitiveVariable.indices;
		}
		else if( primitiveVariable.interpolation == PrimitiveVariable::FaceVarying && interpolation == PrimitiveVariable::Varying )
		{
			// FaceVarying and Varying are the same for CurvesPrimitives
			srcData = primitiveVariable.indices;
		}
		else
		{
			// downsampling forces index expansion to
			// simplify the algorithms.
			// \todo: allow indices to be maintained.
			srcData = primitiveVariable.expandedData();
			primitiveVariable.indices = nullptr;
		}
	}
	else
	{
		// with no indices we can just resample the data
		srcData = primitiveVariable.data;
	}

	if ( interpolation == PrimitiveVariable::Constant )
	{
		Detail::AverageValueFromVector fn;
		dstData = despatchTypedData<Detail::AverageValueFromVector, Detail::IsArithmeticVectorTypedData>( const_cast< Data * >( srcData.get() ), fn );
	}
	else if ( primitiveVariable.interpolation == PrimitiveVariable::Constant )
	{
		DataPtr arrayData = Detail::createArrayData(primitiveVariable, curves, interpolation);
		if (arrayData)
		{
			dstData = arrayData;
		}
	}
	else if ( interpolation == PrimitiveVariable::Uniform )
	{
		if ( primitiveVariable.interpolation == PrimitiveVariable::Vertex )
		{
			CurvesVertexToUniform fn( curves );
			dstData = despatchTypedData<CurvesVertexToUniform, Detail::IsArithmeticVectorTypedData>( const_cast< Data * >( srcData.get() ), fn );
		}
		else if ( primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			CurvesVaryingToUniform fn( curves );
			dstData = despatchTypedData<CurvesVaryingToUniform, Detail::IsArithmeticVectorTypedData>( const_cast< Data * >( srcData.get() ), fn );
		}
	}
	else if ( interpolation == PrimitiveVariable::Vertex )
	{
		if ( primitiveVariable.interpolation == PrimitiveVariable::Uniform )
		{
			CurvesUniformToVertex fn( curves->verticesPerCurve()->readable() );
			dstData = despatchTypedData<CurvesUniformToVertex, TypeTraits::IsNumericBasedVectorTypedData>( const_cast< Data * >( srcData.get() ), fn );
		}
		else if ( primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			CurvesVaryingToVertex fn( curves );
			dstData = despatchTypedData<CurvesVaryingToVertex, IsPrimitiveEvaluatableTypedData>( const_cast< Data * >( srcData.get() ), fn );
		}
	}
	else if ( interpolation == PrimitiveVariable::Varying || interpolation == PrimitiveVariable::FaceVarying )
	{
		if ( primitiveVariable.interpolation == PrimitiveVariable::Uniform )
		{
			CurvesUniformToVarying fn( curves );
			dstData = despatchTypedData<CurvesUniformToVarying, TypeTraits::IsNumericBasedVectorTypedData>( const_cast< Data * >( srcData.get()), fn );
		}
		else if ( primitiveVariable.interpolation == PrimitiveVariable::Vertex )
		{
			CurvesVertexToVarying fn( curves );
			dstData = despatchTypedData<CurvesVertexToVarying, IsPrimitiveEvaluatableTypedData>( const_cast< Data * >( srcData.get() ), fn );
		}
		else if ( primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			dstData = srcData;
		}
	}

	if( primitiveVariable.indices )
	{
		primitiveVariable = PrimitiveVariable( interpolation, primitiveVariable.data, runTimeCast<IntVectorData>( dstData ) );
	}
	else
	{
		primitiveVariable = PrimitiveVariable( interpolation, dstData );
	}
}

CurvesPrimitivePtr deleteCurves( const CurvesPrimitive *curvesPrimitive, const PrimitiveVariable &curvesToDelete, bool invert )
{

	if( curvesToDelete.interpolation != PrimitiveVariable::Uniform )
	{
		throw InvalidArgumentException( "CurvesAlgo::deleteCurves requires an Uniform [Int | Bool | Float]VectorData primitiveVariable " );
	}

	const IntVectorData *intDeleteFlagData = runTimeCast<const IntVectorData>( curvesToDelete.data.get() );

	if( intDeleteFlagData )
	{
		return ::deleteCurves( curvesPrimitive, intDeleteFlagData, invert );
	}

	const BoolVectorData *boolDeleteFlagData = runTimeCast<const BoolVectorData>( curvesToDelete.data.get() );

	if( boolDeleteFlagData )
	{
		return ::deleteCurves( curvesPrimitive, boolDeleteFlagData, invert );
	}

	const FloatVectorData *floatFlagData = runTimeCast<const FloatVectorData>( curvesToDelete.data.get() );

	if( floatFlagData )
	{
		return ::deleteCurves( curvesPrimitive, floatFlagData, invert );
	}

	throw InvalidArgumentException( "CurvesAlgo::deleteCurves requires an Uniform [Int | Bool | Float]VectorData primitiveVariable " );

}

} //namespace CurveAlgo
} //namespace IECoreScene
