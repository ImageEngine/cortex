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

#include <tbb/version.h>
#if TBB_INTERFACE_VERSION >= 12040
#include "oneapi/tbb/blocked_range.h"
#include "oneapi/tbb/parallel_for.h"
#include <oneapi/tbb/task_group.h>
#else	
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#endif

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
	IECore::DataPtr operator()( const IECore::GeometricTypedData<std::vector<T> > *data )
	{
		const auto &src = data->readable();
		if( !src.empty() )
		{
			return new IECore::GeometricTypedData<T>( std::accumulate( src.begin() + 1, src.end(), *src.begin() ) / src.size(), data->getInterpretation() );
		}
		return nullptr;
	}

	template<typename T>
	IECore::DataPtr operator()( const IECore::TypedData<std::vector<T> > *data, typename std::enable_if<IsArithmeticVectorTypedData<IECore::TypedData<std::vector<T> > >::value>::type *enabler = nullptr )
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

struct FillVectorFromValue
{
	template<typename T>
	using IsVectorTypedDataDefined = std::negation< std::is_void< typename IECore::TypedDataTraits< std::vector< T > >::DataHolder > >;

	explicit FillVectorFromValue( const size_t len )
	: m_len( len )
	{}

	template< typename T >
	IECore::DataPtr operator()( const IECore::GeometricTypedData< T > *data, typename std::enable_if< IsVectorTypedDataDefined< T >::value >::type *enabler = nullptr ) const
	{
		using VectorT = IECore::GeometricTypedData< std::vector< T > >;
		typename VectorT::Ptr newData = new VectorT();
		newData->writable().resize( m_len, static_cast< const IECore::GeometricTypedData< T > * >( data )->readable() );
		return newData;
	}

	template< typename T >
	IECore::DataPtr operator()( const IECore::TypedData< T > *data, typename std::enable_if< IsVectorTypedDataDefined< T >::value >::type *enabler = nullptr ) const
	{
		using VectorT = IECore::TypedData< std::vector< T > >;
		typename VectorT::Ptr newData = new VectorT();
		newData->writable().resize( m_len, static_cast< const IECore::TypedData< T > * >( data )->readable() );
		return newData;
	}

	IECore::DataPtr operator()( const IECore::Data *data ) const
	{
		return nullptr;
	}

private:

	size_t m_len;
};

/// template to dispatch only primvars which are supported by the SplitTask
/// Numeric & string like arrays, which contain elements which can be added to a std::set
template<typename T> struct IsDeletablePrimVar : boost::mpl::or_< IECore::TypeTraits::IsStringVectorTypedData<T>, IECore::TypeTraits::IsNumericVectorTypedData<T> > {};

#if TBB_INTERFACE_VERSION >= 12040
template<typename T, typename S, typename P>
class SplitTask
{
	private:
		typedef typename P::Ptr Ptr;
	public:
		SplitTask(const std::vector<T> &segments, typename P::Ptr primitive, const S& splitter, const std::string &primvarName, std::vector<Ptr> &outputPrimitives, size_t offset, size_t depth, const IECore::Canceller *canceller )
			: m_segments(segments), m_primitive(primitive), m_splitter(splitter), m_primvarName(primvarName), m_outputPrimitives( outputPrimitives ), m_offset(offset), m_depth(depth), m_canceller( canceller )
		{
		}

		void execute()
		{

			if ( numPrimitives ( m_primitive.get() ) == 0 && !m_segments.empty() )
			{
				m_outputPrimitives[m_offset] = m_primitive;
			}

			if ( m_segments.size () == 0 )
			{
				return;
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
			}

			IECoreScene::PrimitiveVariable::Interpolation i = splitPrimvarInterpolation( m_primitive.get() );

			IECoreScene::PrimitiveVariable delPrimVarLower( i, deletionArrayLower );
			Ptr a = m_splitter( m_primitive.get(), delPrimVarLower, false, m_canceller ) ;

			IECoreScene::PrimitiveVariable delPrimVarUpper( i, deletionArrayUpper);
			Ptr b = m_splitter( m_primitive.get(), delPrimVarUpper, false, m_canceller ) ;

			size_t numSplits = 2;
			
 			oneapi::tbb::task_group tg;
	  		tg.run([=] {
	  			SplitTask lowerTask(lowerSegments, a, m_splitter, m_primvarName, m_outputPrimitives, m_offset, m_depth + 1, m_canceller);
            			lowerTask.execute();
       			});

        		tg.run([=] {
            			SplitTask upperTask(upperSegments, b, m_splitter, m_primvarName, m_outputPrimitives, m_offset + offset, m_depth + 1, m_canceller);
            			upperTask.execute();
        		});

        		tg.wait();
		}

	private:

		std::vector<T> m_segments;
		typename P::Ptr m_primitive;
		const S &m_splitter;
		std::string m_primvarName;
		std::vector<Ptr> &m_outputPrimitives;
		size_t m_offset;
		size_t m_depth;
		const IECore::Canceller *m_canceller;
};
#else
template<typename T, typename S, typename P>
class SplitTask : public tbb::task
{
	private:
		typedef typename P::Ptr Ptr;
	public:
		SplitTask(const std::vector<T> &segments, typename P::Ptr primitive, const S& splitter, const std::string &primvarName, std::vector<Ptr> &outputPrimitives, size_t offset, size_t depth, const IECore::Canceller *canceller )
			: m_segments(segments), m_primitive(primitive), m_splitter(splitter), m_primvarName(primvarName), m_outputPrimitives( outputPrimitives ), m_offset(offset), m_depth(depth), m_canceller( canceller )
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
			Ptr a = m_splitter( m_primitive.get(), delPrimVarLower, false, m_canceller ) ;

			IECoreScene::PrimitiveVariable delPrimVarUpper( i, deletionArrayUpper);
			Ptr b = m_splitter( m_primitive.get(), delPrimVarUpper, false, m_canceller ) ;

			size_t numSplits = 2;

			set_ref_count( 1 + numSplits);

			SplitTask *tA = new( allocate_child() ) SplitTask( lowerSegments, a, m_splitter,  m_primvarName, m_outputPrimitives, m_offset, m_depth + 1, m_canceller);
			spawn( *tA );

			SplitTask *tB = new( allocate_child() ) SplitTask( upperSegments, b, m_splitter, m_primvarName, m_outputPrimitives, m_offset + offset, m_depth + 1, m_canceller );
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
		const IECore::Canceller *m_canceller;
};
#endif

template<typename P, typename S>
class TaskSegmenter
{
	public:
		TaskSegmenter( const P *primitive, IECore::Data *data, const std::string &primVarName, S &splitter, const IECore::Canceller *canceller ) : m_primitive( primitive ), m_data( data ), m_primVarName( primVarName ), m_splitter(splitter), m_canceller( canceller )
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

#if TBB_INTERFACE_VERSION >= 12040
			oneapi::tbb::task_group_context taskGroupContext(oneapi::tbb::task_group_context::isolated);
            oneapi::tbb::task_group tg(taskGroupContext);

            tg.run([&] {
                        SplitTask<T, S, P> task(
                        segmentsReadable,
                        const_cast<P*>(m_primitive),
                        m_splitter,
                        m_primVarName,
                        results,
                        0,
                        0,
                        m_canceller
                        );
                        task.execute();
                });

            tg.wait();

#else
			tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
			SplitTask<T, S, P> *task = new( tbb::task::allocate_root( taskGroupContext ) ) SplitTask<T, S, P>(
				segmentsReadable,
				const_cast<P *>(m_primitive),
				m_splitter,
				m_primVarName,
				results,
				0,
				0,
				m_canceller
			);
			tbb::task::spawn_root_and_wait( *task );
#endif
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
		const IECore::Canceller *m_canceller;
};


} // namespace Detail
} // namespace IECoreScene

#endif // IECORESCENE_PRIMITIVEALGOUTILS_H
