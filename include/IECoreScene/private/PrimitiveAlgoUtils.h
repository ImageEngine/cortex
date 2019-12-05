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

#ifndef IECORESCENE_PRIMITIVEALGOUTILS_H
#define IECORESCENE_PRIMITIVEALGOUTILS_H

#include "IECoreScene/CurvesPrimitive.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"

#include "IECore/DataAlgo.h"
#include "IECore/TypeTraits.h"

#include "boost/mpl/and.hpp"

#include <numeric>

#include "boost/format.hpp"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

#include <unordered_set>
#include <type_traits>


namespace IECoreScene
{
namespace Detail
{

inline int numPrimitives(const IECoreScene::MeshPrimitive *mesh)
{
	return mesh->numFaces();
}

inline int numPrimitives(const IECoreScene::CurvesPrimitive *curves)
{
	return curves->numCurves();
}

inline int numPrimitives(const IECoreScene::PointsPrimitive *points)
{
	return points->getNumPoints();
}

inline IECoreScene::PrimitiveVariable::Interpolation splitPrimvarInterpolation(const IECoreScene::MeshPrimitive *mesh)
{
	return IECoreScene::PrimitiveVariable::Interpolation::Uniform;
}

inline IECoreScene::PrimitiveVariable::Interpolation splitPrimvarInterpolation(const IECoreScene::CurvesPrimitive *curves)
{
	return IECoreScene::PrimitiveVariable::Interpolation::Uniform;
}

inline IECoreScene::PrimitiveVariable::Interpolation splitPrimvarInterpolation(const IECoreScene::PointsPrimitive *points)
{
	return IECoreScene::PrimitiveVariable::Interpolation::Vertex;
}

template< typename T > struct IsArithmeticVectorTypedData
	: boost::mpl::and_
	<
		IECore::TypeTraits::IsNumericBasedVectorTypedData<T>,
		boost::mpl::not_< IECore::TypeTraits::IsBox<typename IECore::TypeTraits::VectorValueType<T>::type > >,
		boost::mpl::not_< IECore::TypeTraits::IsQuat<typename IECore::TypeTraits::VectorValueType<T>::type > >
	>
{};

struct AverageValueFromVector
{
	AverageValueFromVector()
	{
	}

	template<typename T>
	IECore::DataPtr operator()( const IECore::TypedData<std::vector<T> > *data, typename std::enable_if<IsArithmeticVectorTypedData<IECore::TypedData<std::vector<T> > >::value>::type *enabler = nullptr  )
	{
		const auto &src = data->readable();
		if( !src.empty() )
		{
			return new IECore::TypedData<T>( std::accumulate( src.begin() + 1, src.end(), *src.begin() ) / src.size() );
		}
		return nullptr;
	}

	IECore::DataPtr operator()( IECore::Data *data )
	{
		throw IECore::InvalidArgumentException( boost::str( boost::format( "PrimitiveAlgoUtils::AverageValueFromVector : Variable has unsupported data type \"%s\"." ) % data->typeName() ) );
	}
};


inline IECore::DataPtr createArrayData( PrimitiveVariable& primitiveVariable, const Primitive *primitive, PrimitiveVariable::Interpolation interpolation )
{
	if ( primitiveVariable.interpolation != PrimitiveVariable::Constant )
		return nullptr;

	size_t len = primitive->variableSize( interpolation );
	switch( primitiveVariable.data->typeId() )
	{
		case IECore::IntDataTypeId:
		{
			IECore::IntVectorDataPtr newData = new IECore::IntVectorData();
			newData->writable().resize( len, static_cast< const IECore::IntData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::FloatDataTypeId:
		{
			IECore::FloatVectorDataPtr newData = new IECore::FloatVectorData();
			newData->writable().resize( len, static_cast< const IECore::FloatData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::V2fDataTypeId:
		{
			IECore::V2fVectorDataPtr newData = new IECore::V2fVectorData();
			newData->writable().resize( len, static_cast< const IECore::V2fData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::V3fDataTypeId:
		{
			IECore::V3fVectorDataPtr newData = new IECore::V3fVectorData();
			newData->writable().resize( len, static_cast< const IECore::V3fData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::Color3fDataTypeId:
		{
			IECore::Color3fVectorDataPtr newData = new IECore::Color3fVectorData();
			newData->writable().resize( len, static_cast< const IECore::Color3fData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::StringDataTypeId:
		{
			IECore::StringVectorDataPtr newData = new IECore::StringVectorData();
			newData->writable().resize( len, static_cast< const IECore::StringData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		default:
			return nullptr;
	}

	return nullptr;
}

/// template to dispatch only primvars which are supported by the SplitTask
/// Numeric & string like arrays, which contain elements which can be added to a std::set
template<typename T> struct IsDeletablePrimVar : boost::mpl::or_< IECore::TypeTraits::IsStringVectorTypedData<T>, IECore::TypeTraits::IsNumericVectorTypedData<T> > {};


template<typename T, typename S, typename P>
class SplitTask : public tbb::task
{
	private:
		typedef typename P::Ptr Ptr;
	public:
		SplitTask(const std::vector<T> &segments, typename P::Ptr primitive, const S& splitter, const std::string &primvarName, std::vector<Ptr> &outputPrimitives, size_t offset, size_t depth = 0)
			: m_segments(segments), m_primitive(primitive), m_splitter(splitter), m_primvarName(primvarName), m_outputPrimitives( outputPrimitives ), m_offset(offset), m_depth(depth)
		{
		}

		task *execute() override
		{

			if ( numPrimitives ( m_primitive.get() ) == 0 && !m_segments.empty() )
			{
				m_outputPrimitives[m_offset] = m_primitive;
				return nullptr;
			}

			if ( m_segments.size () == 0 )
			{
				return nullptr;
			}

			size_t offset = m_segments.size() / 2;
			typename std::vector<T>::iterator mid = m_segments.begin() + offset;

			IECoreScene::PrimitiveVariable segmentPrimVar = m_primitive->variables.find( m_primvarName )->second;

			std::vector<T> lowerSegments (m_segments.begin(), mid);
			std::vector<T> upperSegments (mid, m_segments.end());

			std::set<T> lowerSegmentsSet ( m_segments.begin(), mid );
			std::set<T> upperSegmentsSet (mid, m_segments.end());

			const auto &readable = IECore::runTimeCast<IECore::TypedData<std::vector<T> > >( segmentPrimVar.data )->readable();

			IECore::BoolVectorDataPtr deletionArrayLower = new IECore::BoolVectorData();
			auto &writableLower = deletionArrayLower->writable();

			IECore::BoolVectorDataPtr deletionArrayUpper = new IECore::BoolVectorData();
			auto &writableUpper = deletionArrayUpper->writable();

			size_t deleteCount = 0;
			if( segmentPrimVar.indices )
			{
				auto &readableIndices = segmentPrimVar.indices->readable();
				writableLower.resize( readableIndices.size() );
				writableUpper.resize( readableIndices.size() );

				for( size_t i = 0; i < readableIndices.size(); ++i )
				{
					size_t index = readableIndices[i];
					writableLower[i] = lowerSegmentsSet.find( readable[index] ) == lowerSegmentsSet.end();
					writableUpper[i] = upperSegmentsSet.find( readable[index] ) == upperSegmentsSet.end();

					deleteCount += ( writableLower[i] && !lowerSegments.empty() ) || ( writableUpper[i] && !upperSegments.empty() ) ? 1 : 0;
				}
			}
			else
			{
				writableLower.resize( readable.size() );
				writableUpper.resize( readable.size() );

				for( size_t i = 0; i < readable.size(); ++i )
				{
					writableLower[i] = lowerSegmentsSet.find( readable[i] ) == lowerSegmentsSet.end();
					writableUpper[i] = upperSegmentsSet.find( readable[i] ) == upperSegmentsSet.end();
					deleteCount += ( writableLower[i] && !lowerSegments.empty() ) || ( writableUpper[i] && !upperSegments.empty() ) ? 1 : 0;
				}
			}

			if ( m_segments.size() == 1 && deleteCount == 0)
			{
				m_outputPrimitives[m_offset] = m_primitive;
				return nullptr;
			}

			IECoreScene::PrimitiveVariable::Interpolation i = splitPrimvarInterpolation( m_primitive.get() );

			IECoreScene::PrimitiveVariable delPrimVarLower( i, deletionArrayLower );
			Ptr a = m_splitter( m_primitive.get(), delPrimVarLower, false ) ;

			IECoreScene::PrimitiveVariable delPrimVarUpper( i, deletionArrayUpper);
			Ptr b = m_splitter( m_primitive.get(), delPrimVarUpper, false ) ;

			size_t numSplits = 2;

			set_ref_count( 1 + numSplits);

			SplitTask *tA = new( allocate_child() ) SplitTask( lowerSegments, a, m_splitter,  m_primvarName, m_outputPrimitives, m_offset, m_depth + 1);
			spawn( *tA );

			SplitTask *tB = new( allocate_child() ) SplitTask( upperSegments, b, m_splitter, m_primvarName, m_outputPrimitives, m_offset + offset, m_depth + 1 );
			spawn( *tB );

			wait_for_all();

			return nullptr;
		}

	private:

		std::vector<T> m_segments;
		typename P::Ptr m_primitive;
		const S &m_splitter;
		std::string m_primvarName;
		std::vector<Ptr> &m_outputPrimitives;
		size_t m_offset;
		size_t m_depth;
};

template<typename P, typename S>
class TaskSegmenter
{
	public:
		TaskSegmenter( const P *primitive, IECore::Data *data, const std::string &primVarName, S &splitter ) : m_primitive( primitive ), m_data( data ), m_primVarName( primVarName ), m_splitter(splitter)
		{
		}

		typedef std::vector<typename P::Ptr> ReturnType;

		template<typename T>
		ReturnType operator()(
			const IECore::TypedData<std::vector<T>> *array,
			typename std::enable_if<IsDeletablePrimVar<IECore::TypedData<std::vector<T>>>::value>::type *enabler = nullptr
		)
		{
			IECore::TypedData<std::vector<T> > *segments = IECore::runTimeCast<IECore::TypedData<std::vector<T> > >( m_data );

			if ( !segments )
			{
				throw IECore::InvalidArgumentException(
					(
						boost::format( "Segment keys type '%s' doesn't match primitive variable type '%s'" ) %
							m_data->typeName() %
							array->typeName()
					).str()
				);
			}

			const auto &segmentsReadable = segments->readable();

			ReturnType results( segmentsReadable.size() );

			tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
			SplitTask<T, S, P> *task = new( tbb::task::allocate_root( taskGroupContext ) ) SplitTask<T, S, P>(
				segmentsReadable,
				const_cast<P *>(m_primitive),
				m_splitter,
				m_primVarName,
				results,
				0
			);
			tbb::task::spawn_root_and_wait( *task );

			return results;

		}

		ReturnType operator()( const IECore::Data *data )
		{
			throw IECore::Exception(
				boost::str( boost::format( "Unexpected Data: %1%" ) % ( data ? data->typeName() : std::string( "nullptr" ) ) )
			);
		}

	private:
		const P *m_primitive;
		IECore::Data *m_data;
		std::string m_primVarName;
		const S &m_splitter;
};


} // namespace Detail
} // namespace IECoreScene

#endif // IECORESCENE_PRIMITIVEALGOUTILS_H
