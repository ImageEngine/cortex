//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/MeshAlgo.h"

#include "IECore/DataAlgo.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TriangleAlgo.h"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

namespace
{

// Compute all the new indices needed to triangulate the mesh.
// \todo : Multithread this.
void triangulateMeshIndices(
	const MeshPrimitive *mesh,
	std::vector<int> &newVertexIds,
	std::vector<int> &newFaceVertexIds,
	std::vector<int> &newUniformIds,
	const IECore::Canceller *canceller
)
{
	const std::vector<int> &verticesPerFace = mesh->verticesPerFace()->readable();
	const std::vector<int> &vertexIds = mesh->vertexIds()->readable();

	newVertexIds.clear();
	newFaceVertexIds.clear();
	newUniformIds.clear();

	int numTris = 0;
	for( auto n : verticesPerFace )
	{
		numTris += n - 2;
	}

	newVertexIds.reserve( numTris * 3 );
	newFaceVertexIds.reserve( numTris * 3 );
	newUniformIds.reserve( numTris );

	int faceVertexIdStart = 0;
	for( int faceIdx = 0; faceIdx < (int)verticesPerFace.size(); faceIdx++ )
	{
		if( ( faceIdx % 100 ) == 0 )
		{
			Canceller::check( canceller );
		}

		int numFaceVerts = verticesPerFace[ faceIdx ];

		if( numFaceVerts > 3 )
		{
			/// For the time being, just do a simple triangle fan.

			const int i0 = faceVertexIdStart + 0;
			const int v0 = vertexIds[i0];

			int i1 = faceVertexIdStart + 1;
			int i2 = faceVertexIdStart + 2;
			int v1 = vertexIds[i1];
			int v2 = vertexIds[i2];

			for( int i = 1; i < numFaceVerts - 1; i++ )
			{
				i1 = faceVertexIdStart + ( ( i + 0 ) % numFaceVerts );
				i2 = faceVertexIdStart + ( ( i + 1 ) % numFaceVerts );
				v1 = vertexIds[i1];
				v2 = vertexIds[i2];

				/// Triangulate the vertices
				newVertexIds.push_back( v0 );
				newVertexIds.push_back( v1 );
				newVertexIds.push_back( v2 );

				/// Store the indices required to rebuild the facevarying primvars
				newFaceVertexIds.push_back( i0 );
				newFaceVertexIds.push_back( i1 );
				newFaceVertexIds.push_back( i2 );

				newUniformIds.push_back( faceIdx );
			}
		}
		else
		{
			assert( numFaceVerts == 3 );

			int i0 = faceVertexIdStart + 0;
			int i1 = faceVertexIdStart + 1;
			int i2 = faceVertexIdStart + 2;

			/// Copy across the vertexId data
			newVertexIds.push_back( vertexIds[i0] );
			newVertexIds.push_back( vertexIds[i1] );
			newVertexIds.push_back( vertexIds[i2] );

			/// Store the indices required to rebuild the facevarying primvars
			newFaceVertexIds.push_back( i0 );
			newFaceVertexIds.push_back( i1 );
			newFaceVertexIds.push_back( i2 );

			newUniformIds.push_back( faceIdx );
		}

		faceVertexIdStart += numFaceVerts;
	}
}

DataPtr newMatchingData( const Data *source )
{
	return dispatch( source,
		[]( const auto *typedSource ) -> DataPtr
		{
			using DataType = typename std::remove_const_t< std::remove_pointer_t< decltype( typedSource ) > >;
			typename DataType::Ptr result = new DataType();

			if constexpr( TypeTraits::IsGeometricTypedData< DataType >::value )
			{
				result->setInterpretation( typedSource->getInterpretation() );
			}

			return result;
		}
	);
}


/// A functor for use with despatchTypedData, which copies elements from another vector, as specified by an array of indices into that data
struct TriangleDataRemap
{
	typedef void ReturnType;

	TriangleDataRemap( const std::vector<int> &indices, const Canceller *canceller ) : m_other( nullptr ), m_indices( indices ), m_canceller( canceller )
	{
	}

	const Data *m_other;
	const std::vector<int> &m_indices;
	const Canceller *m_canceller;

	template<typename T>
	void operator()( T *data )
	{
		assert( data );
		typename T::ValueType &dataWritable = data->writable();

		const T *otherData = runTimeCast<const T, const Data>( m_other );
		assert( otherData );
		const typename T::ValueType &otherDataReadable = otherData->readable();

		dataWritable.clear();
		dataWritable.resize( m_indices.size() );

		auto f = [&dataWritable, &otherDataReadable, this]( const tbb::blocked_range<size_t> &r )
		{
			Canceller::check( m_canceller );
			for( size_t i = r.begin(); i != r.end(); ++i )
			{
				dataWritable[i] = otherDataReadable[m_indices[i]];
			}
		};

		const bool useTBB = false;
		if ( useTBB )
		{
			tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
			tbb::parallel_for( tbb::blocked_range<size_t>( 0, m_indices.size() ), f, taskGroupContext );
		}
		else
		{
			f( tbb::blocked_range<size_t>( 0, m_indices.size() ) );
		}

		assert( dataWritable.size() == m_indices.size() );
	}
};

/// A simple class to allow MeshAlgo::triangulate to operate on either V3fVectorData or V3dVectorData using
/// despatchTypedData
struct TriangulateFn
{
	typedef void ReturnType;

	MeshPrimitive *m_mesh;
	const Canceller * m_canceller;

	TriangulateFn( MeshPrimitive *mesh, const Canceller *canceller ) : m_mesh( mesh ), m_canceller( canceller )
	{
	}

	template<typename T>
	ReturnType operator()( T *p )
	{
		IntVectorDataPtr newVertexIds = new IntVectorData();
		std::vector<int> &newVertexIdsWritable = newVertexIds->writable();
		std::vector<int> faceVaryingIndices;
		std::vector<int> uniformIndices;
		triangulateMeshIndices( m_mesh, newVertexIdsWritable, faceVaryingIndices, uniformIndices, m_canceller );

		IntVectorDataPtr newVerticesPerFace = new IntVectorData();
		std::vector<int> &newVerticesPerFaceWritable = newVerticesPerFace->writable();
		newVerticesPerFaceWritable.resize( newVertexIdsWritable.size() / 3, 3 );

		const typename T::ValueType &pReadable = p->readable();
		m_mesh->setTopologyUnchecked( newVerticesPerFace, newVertexIds, pReadable.size(), m_mesh->interpolation() );

		/// Rebuild all the facevarying primvars, using the list of indices into the old data we created above.
		assert( faceVaryingIndices.size() == newVertexIds->readable().size() );
		TriangleDataRemap varyingRemap( faceVaryingIndices, m_canceller );
		TriangleDataRemap uniformRemap( uniformIndices, m_canceller );
		for( PrimitiveVariableMap::iterator it = m_mesh->variables.begin(); it != m_mesh->variables.end(); ++it )
		{
			TriangleDataRemap *remap = nullptr;
			if( it->second.interpolation == PrimitiveVariable::FaceVarying )
			{
				remap = &varyingRemap;
			}
			else if( it->second.interpolation == PrimitiveVariable::Uniform )
			{
				remap = &uniformRemap;
			}
			else
			{
				continue;
			}

			const Data *inputData = it->second.indices ? it->second.indices.get() : it->second.data.get();
			DataPtr result = newMatchingData( inputData );
			remap->m_other = inputData;

			// \todo - using this to reindex data is a waste of time and memory.  If there are no indices,
			// we could simply set the indices of the primvar to the needed indexes.  This would be simpler,
			// almost free, and likely results in more efficient computations downstream as well ( since
			// the data will be smaller to operate on ).  The only non-trivial part of this change is
			// evaluating whether anyone is relying on the previous behaviour, or exposing a parameter to
			// control it - the next person to touch this code should definitely do this.
			despatchTypedData<TriangleDataRemap, TypeTraits::IsVectorTypedData>( result.get(), *remap );

			if( it->second.indices )
			{
				it->second.indices = runTimeCast<IntVectorData>( result );
			}
			else
			{
				it->second.data = result;
			}
		}

		assert( m_mesh->arePrimitiveVariablesValid() );
	}

	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( const T *data, const F &functor )
		{
			assert( data );

			throw InvalidArgumentException(
				(
					boost::format( "MeshAlgo::triangulate: Invalid data type \"%s\" for primitive variable \"P\"." ) %
						Object::typeNameFromTypeId( data->typeId() )
				).str()
			);
		}
	};
};

} // namespace

MeshPrimitivePtr MeshAlgo::triangulate(
	const MeshPrimitive *mesh, const Canceller *canceller
)
{
	MeshPrimitivePtr meshCopy = mesh->copy();

	if ( !mesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "MeshAlgo::triangulate : Mesh with invalid primitive variables" );
	}

	// already triangulated
	if ( mesh->maxVerticesPerFace() == 3 )
	{
		return meshCopy;
	}

	PrimitiveVariableMap::const_iterator pvIt = meshCopy->variables.find( "P" );
	if( pvIt == meshCopy->variables.end() )
	{
		throw InvalidArgumentException( "MeshAlgo::triangulate : MeshPrimitive has no \"P\" data" );
	}

	const DataPtr &verticesData = pvIt->second.data;

	TriangulateFn fn( meshCopy.get(), canceller );

	despatchTypedData<TriangulateFn, TypeTraits::IsFloatVec3VectorTypedData, TriangulateFn::ErrorHandler>( verticesData.get(), fn );

	return meshCopy;
}
