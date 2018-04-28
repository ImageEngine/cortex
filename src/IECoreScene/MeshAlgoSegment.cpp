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

#include "boost/format.hpp"

#include "IECore/DespatchTypedData.h"
#include "IECore/DataAlgo.h"

#include "IECoreScene/MeshAlgo.h"

#include "boost/format.hpp"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

#include <unordered_set>

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

/// template to dispatch only primvars which are supported by the SplitTask
/// Numeric & stiring like arrays, which contain elements which can be added to a std::set
template<typename T> struct IsDeletablePrimVar : boost::mpl::or_< IECore::TypeTraits::IsStringVectorTypedData<T>, IECore::TypeTraits::IsNumericVectorTypedData<T> > {};

namespace
{

template<typename T>
class SplitTask : public tbb::task
{
	public:
		SplitTask(const std::vector<T> &segments, MeshPrimitive *mesh, const std::string &primvarName, std::vector<MeshPrimitivePtr> &outputMeshes,  size_t offset, size_t depth = 0)
		: m_segments(segments), m_mesh(mesh), m_primvarName(primvarName), m_outputMeshes(outputMeshes), m_offset(offset), m_depth(depth)
		{
		}

		task *execute() override
		{

			if ( m_mesh->numFaces() == 0 && !m_segments.empty() )
			{
				m_outputMeshes[m_offset] = m_mesh;
				return nullptr;
			}

			if ( m_segments.size () == 0 )
			{
				return nullptr;
			}

			size_t offset = m_segments.size() / 2;
			typename std::vector<T>::iterator mid = m_segments.begin() + offset;

			IECoreScene::PrimitiveVariable segmentPrimVar = m_mesh->variables.find( m_primvarName )->second;

			std::vector<T> lowerSegments (m_segments.begin(), mid);
			std::vector<T> upperSegments (mid, m_segments.end());

			std::set<T> lowerSegmentsSet ( m_segments.begin(), mid );
			std::set<T> upperSegmentsSet (mid, m_segments.end());

			const auto &readable = runTimeCast<IECore::TypedData<std::vector<T> > >( segmentPrimVar.data )->readable();

			BoolVectorDataPtr deletionArrayLower = new BoolVectorData();
			auto &writableLower = deletionArrayLower->writable();

			BoolVectorDataPtr deletionArrayUpper = new BoolVectorData();
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
				m_outputMeshes[m_offset] = m_mesh;
				return nullptr;
			}

			IECoreScene::PrimitiveVariable delPrimVarLower( IECoreScene::PrimitiveVariable::Uniform, deletionArrayLower );
			IECoreScene::MeshPrimitivePtr a = MeshAlgo::deleteFaces( m_mesh, delPrimVarLower, false ) ;

			IECoreScene::PrimitiveVariable delPrimVarUpper( IECoreScene::PrimitiveVariable::Uniform, deletionArrayUpper);
			IECoreScene::MeshPrimitivePtr b = MeshAlgo::deleteFaces( m_mesh, delPrimVarUpper, false ) ;

			size_t numSplits = 2;

			set_ref_count( 1 + numSplits);

			SplitTask *tA = new( allocate_child() ) SplitTask( lowerSegments, a.get(), m_primvarName, m_outputMeshes, m_offset, m_depth + 1);
			spawn( *tA );

			SplitTask *tB = new( allocate_child() ) SplitTask( upperSegments, b.get(), m_primvarName, m_outputMeshes, m_offset + offset, m_depth + 1 );
			spawn( *tB );

			wait_for_all();

			return nullptr;
		}

	private:
		std::vector<T> m_segments;
		MeshPrimitive *m_mesh;
		std::string m_primvarName;
		std::vector<MeshPrimitivePtr> &m_outputMeshes;
		size_t m_offset;
		size_t m_depth;
};

class TaskSegmenter
{
	public:
		TaskSegmenter( ConstMeshPrimitivePtr mesh, Data *data, const std::string &primVarName ) : m_mesh( mesh ), m_data( data ), m_primVarName( primVarName )
		{
		}

		typedef std::vector<MeshPrimitivePtr> ReturnType;

		template<typename T>
		ReturnType operator()( T *array )
		{
			T *segments = IECore::runTimeCast<T>( m_data );

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

			SplitTask<typename T::ValueType::value_type> *task = new( tbb::task::allocate_root() ) SplitTask<typename T::ValueType::value_type> ( segmentsReadable, const_cast<MeshPrimitive*>(m_mesh.get()), m_primVarName, results, 0 );
			tbb::task::spawn_root_and_wait( *task );

			return results;

		}

	private:
		ConstMeshPrimitivePtr m_mesh;
		Data *m_data;
		std::string m_primVarName;

};

class Segmenter
{
	public:
		Segmenter( const MeshPrimitive &mesh, Data *data, const IntVectorData *indices ) : m_mesh( mesh ), m_data( data ), m_indices( indices )
		{
		}

		typedef std::vector<MeshPrimitivePtr> ReturnType;

		template<typename T>
		ReturnType operator()( T *array )
		{
			T *segments = IECore::runTimeCast<T>( m_data );

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

			ReturnType results;
			results.resize( segmentsReadable.size() );
			const auto &readable = array->readable();

			tbb::parallel_for(
				tbb::blocked_range<size_t>( 0, segmentsReadable.size() ), [this, &readable, &segmentsReadable, &results]( const tbb::blocked_range<size_t> &r )
				{
					BoolVectorDataPtr deletionArray = new BoolVectorData();
					auto &writable = deletionArray->writable();

					for (size_t j = r.begin(); j < r.end(); ++j)
					{
						if( m_indices )
						{
							auto &readableIndices = m_indices->readable();
							writable.resize( readableIndices.size() );

							for( size_t i = 0; i < readableIndices.size(); ++i )
							{
								size_t index = readableIndices[i];
								writable[i] = segmentsReadable[j] != readable[index];
							}
						}
						else
						{
							writable.resize( readable.size() );
							for( size_t i = 0; i < readable.size(); ++i )
							{
								writable[i] = segmentsReadable[j] != readable[i];
							}
						}

						IECoreScene::PrimitiveVariable delPrimVar( IECoreScene::PrimitiveVariable::Uniform, deletionArray );
						results[j] = MeshAlgo::deleteFaces( &m_mesh, delPrimVar, false ) ;
					}
				}
			);

			return results;
		}

	private:
		const MeshPrimitive &m_mesh;
		Data *m_data;
		const IntVectorData *m_indices;

};

} // namespace



std::vector<MeshPrimitivePtr> IECoreScene::MeshAlgo::segment( const MeshPrimitive *mesh, const PrimitiveVariable &primitiveVariable, const IECore::Data *segmentValues )
{
	MeshPrimitivePtr meshCopy = mesh->copy();

	for (const auto &pv : meshCopy->variables )
	{
		meshCopy->variables[pv.first] = PrimitiveVariable( pv.second.interpolation, pv.second.expandedData() );
	}

	DataPtr data;
	if( !segmentValues )
	{
		data = IECore::uniqueValues( primitiveVariable.data.get() );
		segmentValues = data.get();
	}

	std::string primitiveVariableName;
	for (const auto &pv : mesh->variables )
	{
		if ( pv.second == primitiveVariable )
		{
			primitiveVariableName = pv.first;
		}
	}

	TaskSegmenter taskSegmenter( meshCopy.get(), const_cast<IECore::Data*> (segmentValues), primitiveVariableName );

	return despatchTypedData<TaskSegmenter, IsDeletablePrimVar>(  primitiveVariable.data.get(), taskSegmenter );
}
