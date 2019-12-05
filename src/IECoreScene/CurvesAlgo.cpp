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
#include "IECoreScene/private/PrimitiveVariableAlgos.h"

#include "IECore/DataAlgo.h"
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

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data.get(), result.get() );

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

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

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

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data.get(), result.get() );

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

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

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

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

		return result;
	}

	const CurvesPrimitive *m_curves;
};

struct CurvesVaryingToVertex
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

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

		return result;
	}

	const CurvesPrimitive *m_curves;
};


template<typename T>
CurvesPrimitivePtr deleteCurves(
	const CurvesPrimitive *curvesPrimitive,
	PrimitiveVariable::IndexedView<T>& deleteFlagView,
	bool invert
)
{
	IECoreScene::PrimitiveVariableAlgos::DeleteFlaggedUniformFunctor<T> deleteUniformFn( deleteFlagView, invert );
	IECoreScene::PrimitiveVariableAlgos::DeleteFlaggedVaryingFunctor<T> deleteVaryingFn ( deleteFlagView, curvesPrimitive, invert );
	IECoreScene::PrimitiveVariableAlgos::DeleteFlaggedVertexFunctor<T> deleteVertexFn(
		curvesPrimitive->variableSize( PrimitiveVariable::Vertex ),
		/* vertexIds */ nullptr,
		curvesPrimitive->verticesPerCurve(),
		deleteFlagView,
		invert
	);

	const IECore::Data *inputVertsPerCurve = IECore::runTimeCast<const IECore::Data>( curvesPrimitive->verticesPerCurve() );

	IECoreScene::PrimitiveVariableAlgos::IndexedData outputVertsPerCurve = dispatch( inputVertsPerCurve, deleteUniformFn );

	IntVectorDataPtr verticesPerCurve = IECore::runTimeCast<IECore::IntVectorData>( outputVertsPerCurve.data );

	CurvesPrimitivePtr outCurvesPrimitive = new CurvesPrimitive( verticesPerCurve, curvesPrimitive->basis(), curvesPrimitive->periodic() );

	for (PrimitiveVariableMap::const_iterator it = curvesPrimitive->variables.begin(), e = curvesPrimitive->variables.end(); it != e; ++it)
	{
		if( !curvesPrimitive->isPrimitiveVariableValid( it->second ) )
		{
			throw InvalidArgumentException(
				boost::str ( boost::format( "CurvesAlgo::deleteCurves cannot process invalid primitive variable \"%s\"" ) % it->first ) );
		}

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
				const IECore::Data *inputData = it->second.data.get();
				deleteUniformFn.setIndices( it->second.indices.get() );
				IECoreScene::PrimitiveVariableAlgos::IndexedData outputData = dispatch( inputData, deleteUniformFn );
				outCurvesPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, outputData.data, outputData.indices );

				break;
			}
			case PrimitiveVariable::Varying:
			case PrimitiveVariable::FaceVarying:
			{
				const IECore::Data *inputData = it->second.data.get();
				deleteVaryingFn.setIndices( it->second.indices.get() );
				IECoreScene::PrimitiveVariableAlgos::IndexedData outputData = dispatch( inputData, deleteVaryingFn );
				outCurvesPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, outputData.data, outputData.indices );

				break;
			}
			case PrimitiveVariable::Vertex:
			{
				const IECore::Data *inputData = it->second.data.get();
				deleteVertexFn.setIndices( it->second.indices.get() );
				IECoreScene::PrimitiveVariableAlgos::IndexedData outputData = dispatch( inputData, deleteVertexFn );
				outCurvesPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, outputData.data, outputData.indices );
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
		dstData = dispatch( srcData.get(), fn );
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
		PrimitiveVariable::IndexedView<int> deleteFlagView( curvesToDelete );
		return ::deleteCurves( curvesPrimitive, deleteFlagView, invert );
	}

	const BoolVectorData *boolDeleteFlagData = runTimeCast<const BoolVectorData>( curvesToDelete.data.get() );

	if( boolDeleteFlagData )
	{
		PrimitiveVariable::IndexedView<bool> deleteFlagView( curvesToDelete );
		return ::deleteCurves( curvesPrimitive, deleteFlagView, invert );
	}

	const FloatVectorData *floatFlagData = runTimeCast<const FloatVectorData>( curvesToDelete.data.get() );

	if( floatFlagData )
	{
		PrimitiveVariable::IndexedView<float> deleteFlagView( curvesToDelete );
		return ::deleteCurves( curvesPrimitive, deleteFlagView, invert );
	}

	throw InvalidArgumentException( "CurvesAlgo::deleteCurves requires an Uniform [Int | Bool | Float]VectorData primitiveVariable " );

}

} //namespace CurveAlgo
} //namespace IECoreScene
