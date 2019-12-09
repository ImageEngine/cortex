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


#include "IECoreScene/PointsAlgo.h"

#include "IECoreScene/private/PrimitiveAlgoUtils.h"
#include "IECoreScene/private/PrimitiveVariableAlgos.h"

#include "IECore/DataAlgo.h"
#include "IECore/DataCastOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

#include "boost/format.hpp"

#include <numeric>

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;

namespace
{

struct PointsVertexToUniform
{
	typedef DataPtr ReturnType;

	PointsVertexToUniform( const PointsPrimitive *points ) : m_points( points )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		if ( !src.empty() )
		{
			trg.push_back( std::accumulate( src.begin() + 1, src.end(), *src.begin() ) / src.size() );
		}

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

		return result;
	}

	const PointsPrimitive *m_points;
};

struct PointsUniformToVertex
{
	typedef DataPtr ReturnType;

	PointsUniformToVertex( const PointsPrimitive *points ) : m_points( points )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType::const_iterator srcIt = data->readable().begin();

		trg.resize( m_points->variableSize( PrimitiveVariable::Vertex ), *srcIt );

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<From> copier;
		copier( data, result.get() );

		return result;
	}

	const PointsPrimitive *m_points;
};

template<typename T>
PointsPrimitivePtr deletePoints( const PointsPrimitive *pointsPrimitive, IECoreScene::PrimitiveVariable::IndexedView<T>& deleteFlagView, bool invert )
{
	PointsPrimitivePtr outPointsPrimitive = new PointsPrimitive( 0 );

	IECoreScene::PrimitiveVariableAlgos::DeleteFlaggedUniformFunctor<T> vertexFunctor( deleteFlagView, invert );

	for( PrimitiveVariableMap::const_iterator it = pointsPrimitive->variables.begin(), e = pointsPrimitive->variables.end(); it != e; ++it )
	{
		switch( it->second.interpolation )
		{
			case PrimitiveVariable::Vertex:
			case PrimitiveVariable::Varying:
			case PrimitiveVariable::FaceVarying:
			{
				if( !pointsPrimitive->isPrimitiveVariableValid( it->second ) )
				{
					throw InvalidArgumentException(
						boost::str ( boost::format( "PointsAlgo::deletePoints cannot process invalid primitive variable \"%s\"" ) % it->first ) );
				}
				const IECore::Data *inputData = it->second.data.get();
				vertexFunctor.setIndices( it->second.indices.get() );
				IECoreScene::PrimitiveVariableAlgos::IndexedData indexedData = dispatch( inputData, vertexFunctor );
				outPointsPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, indexedData.data, indexedData.indices );
				break;
			}
			case PrimitiveVariable::Uniform:
			case PrimitiveVariable::Constant:
			case PrimitiveVariable::Invalid:
			{
				outPointsPrimitive->variables[it->first] = it->second;
				break;
			}
		}
	}

	V3fVectorDataPtr positionData = outPointsPrimitive->variableData<V3fVectorData>( "P" );
	if( positionData )
	{
		outPointsPrimitive->setNumPoints( positionData->readable().size() );
	}

	return outPointsPrimitive;
}

struct CollectDataFn
{
	CollectDataFn( size_t size ) : offset( 0 ), m_size( size )
	{
	}

	typedef void ReturnType;

	template<typename T>
	ReturnType operator()( const T *data )
	{
		T *container = nullptr;
		if( !outputData )
		{
			container = new T();
			outputData = container;

			container->writable().resize( m_size );
		}
		else
		{
			container = runTimeCast<T>( outputData.get() );
		}

		std::copy( data->readable().begin(), data->readable().end(), container->writable().begin() + offset );
	}

	DataPtr outputData;
	size_t offset;
	size_t m_size;
};

DataPtr mergePrimVars( const std::vector<PointsPrimitivePtr> &pointsPrimitives, const std::string &primVarName, size_t totalCount )
{
	CollectDataFn fn( totalCount );

	for( size_t i = 0; i < pointsPrimitives.size(); ++i )
	{
		PrimitiveVariableMap::const_iterator it = pointsPrimitives[i]->variables.find( primVarName );

		if( it != pointsPrimitives[i]->variables.end() )
		{
			PrimitiveVariable primVar = it->second;
			despatchTypedData<CollectDataFn, TypeTraits::IsVectorTypedData>( const_cast< Data * >( primVar.data.get() ), fn );
		}

		fn.offset += pointsPrimitives[i]->getNumPoints();
	}

	return fn.outputData;
}

} // anonymous namespace

namespace IECoreScene
{

namespace PointsAlgo
{

void resamplePrimitiveVariable( const PointsPrimitive *points, PrimitiveVariable &primitiveVariable, PrimitiveVariable::Interpolation interpolation )
{
	if ( primitiveVariable.interpolation == interpolation )
	{
		return;
	}

	DataPtr dstData = nullptr;
	DataPtr srcData = nullptr;

	if( primitiveVariable.indices )
	{
		if( primitiveVariable.interpolation < interpolation )
		{
			// upsampling can be a resampling of indices
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
		primitiveVariable = PrimitiveVariable(PrimitiveVariable::Constant, dstData );
		return;
	}

	if ( primitiveVariable.interpolation == PrimitiveVariable::Constant )
	{

		DataPtr arrayData = Detail::createArrayData(primitiveVariable, points, interpolation);
		if (arrayData)
		{
			primitiveVariable = PrimitiveVariable(interpolation, arrayData);
		}
		return;
	}
	else if( interpolation == PrimitiveVariable::Uniform )
	{
		if( primitiveVariable.interpolation == PrimitiveVariable::Vertex || primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
		{
			PointsVertexToUniform fn( points );
			dstData = despatchTypedData<PointsVertexToUniform, IECoreScene::Detail::IsArithmeticVectorTypedData>( const_cast< Data * >( srcData.get() ), fn );
		}
	}
	else if( interpolation == PrimitiveVariable::Vertex || interpolation == PrimitiveVariable::Varying || interpolation == PrimitiveVariable::FaceVarying )
	{
		if( primitiveVariable.interpolation == PrimitiveVariable::Uniform )
		{
			PointsUniformToVertex fn( points );
			dstData = despatchTypedData<PointsUniformToVertex, TypeTraits::IsNumericBasedVectorTypedData>( const_cast< Data * >( srcData.get() ), fn );
		}
		else if( primitiveVariable.interpolation == PrimitiveVariable::Vertex || primitiveVariable.interpolation == PrimitiveVariable::Varying || primitiveVariable.interpolation == PrimitiveVariable::FaceVarying )
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

PointsPrimitivePtr deletePoints( const PointsPrimitive *pointsPrimitive, const PrimitiveVariable &pointsToDelete, bool invert /* = false */ )
{

	if( pointsToDelete.interpolation != PrimitiveVariable::Vertex )
	{
		throw InvalidArgumentException(
			boost::str ( boost::format( "PointsAlgo::deletePoints requires a Vertex [Int|Bool|Float]VectorData primitiveVariable. %1% interpolation found " ) % pointsToDelete.interpolation )
		);
	}

	if( runTimeCast<const IntVectorData>( pointsToDelete.data.get() ) )
	{
		PrimitiveVariable::IndexedView<int> deleteFlagView( pointsToDelete );
		return ::deletePoints( pointsPrimitive, deleteFlagView, invert );
	}

	if( runTimeCast<const BoolVectorData>( pointsToDelete.data.get() ) )
	{
		PrimitiveVariable::IndexedView<bool> deleteFlagView( pointsToDelete );
		return ::deletePoints( pointsPrimitive, deleteFlagView, invert );
	}


	if( runTimeCast<const FloatVectorData>( pointsToDelete.data.get() ) )
	{
		PrimitiveVariable::IndexedView<float> deleteFlagView( pointsToDelete );
		return ::deletePoints( pointsPrimitive, deleteFlagView, invert );
	}

	throw InvalidArgumentException( "PointsAlgo::deletePoints requires an Vertex [Int|Bool|Float]VectorData primitiveVariable " );

}

PointsPrimitivePtr mergePoints( const std::vector<const PointsPrimitive *> &pointsPrimitives )
{
	size_t totalPointCount = 0;
	typedef std::map<std::string, IECore::TypeId> FoundPrimvars;
	FoundPrimvars foundPrimvars;

	PrimitiveVariableMap constantPrimVars;

	std::vector<PointsPrimitivePtr> validatedPointsPrimitives( pointsPrimitives.size() );

	// find out which primvars can be merged
	for( size_t i = 0; i < pointsPrimitives.size(); ++i )
	{
		PointsPrimitivePtr pointsPrimitive = validatedPointsPrimitives[i] = pointsPrimitives[i]->copy();

		totalPointCount += pointsPrimitive->getNumPoints();
		PrimitiveVariableMap &variables = pointsPrimitive->variables;
		for( PrimitiveVariableMap::iterator it = variables.begin(); it != variables.end(); ++it )
		{
			DataPtr data = it->second.data;
			const IECore::TypeId typeId = data->typeId();
			PrimitiveVariable::Interpolation interpolation = it->second.interpolation;
			const std::string &name = it->first;

			bool bExistingConstant = constantPrimVars.find( name ) != constantPrimVars.end();
			FoundPrimvars::const_iterator fIt = foundPrimvars.find( name );
			bool bExistingVertex = fIt != foundPrimvars.end();

			if( interpolation == PrimitiveVariable::Constant )
			{
				if( bExistingVertex )
				{
					std::string msg = boost::str( boost::format( "PointsAlgo::mergePoints mismatching primvar %s" ) % name );
					throw InvalidArgumentException( msg );
				}

				if( !bExistingConstant )
				{
					constantPrimVars[name] = it->second;
				}
				continue;
			}

			if( interpolation == PrimitiveVariable::Vertex )
			{

				PrimitiveVariableMap::const_iterator constantPrimVarIt = constantPrimVars.find( name );
				if( constantPrimVarIt != constantPrimVars.end() )
				{
					std::string msg = boost::str( boost::format( "PointsAlgo::mergePoints mismatching primvar %s" ) % name );
					throw InvalidArgumentException( msg );
				}

				if( !bExistingVertex )
				{
					foundPrimvars[name] = typeId;
				}
				else
				{
					if( fIt->second != typeId )
					{
						DataCastOpPtr castOp = new DataCastOp();

						castOp->objectParameter()->setValue( data );
						castOp->targetTypeParameter()->setNumericValue( fIt->second );

						try
						{
							it->second.data = runTimeCast<Data>( castOp->operate() );
						}
						catch( const IECore::Exception &e )
						{
							std::string msg = boost::str( boost::format( "PointsAlgo::mergePoints unable to cast primvar %s (%s) " ) % name % e.what() );
							throw InvalidArgumentException( msg );
						}
					}
				}
			}
		}
	}

	// allocate the new points primitive and copy the primvars
	PointsPrimitivePtr newPoints = new PointsPrimitive( totalPointCount );

	// copy constant primvars
	for( PrimitiveVariableMap::const_iterator it = constantPrimVars.begin(); it != constantPrimVars.end(); ++it )
	{
		newPoints->variables[it->first] = it->second;
	}

	// merge vertex primvars
	for( FoundPrimvars::const_iterator it = foundPrimvars.begin(); it != foundPrimvars.end(); ++it )
	{
		DataPtr mergedData = mergePrimVars( validatedPointsPrimitives, it->first, totalPointCount );
		newPoints->variables[it->first] = PrimitiveVariable( PrimitiveVariable::Vertex, mergedData );
	}

	return newPoints;
}

} // namespace PointsAlgo
} // namespace IECoreScene
