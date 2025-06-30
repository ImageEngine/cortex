//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2023, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DataAlgo.h"
#include "IECore/TypeTraits.h"

#include "IECoreScene/MeshAlgo.h"

#include <unordered_map>

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;



namespace
{

// Custom comparison functions which include support for imath types
template< typename T >
inline bool imathCompare( const T &a, const T &b )
{
	return a < b;
}

template< typename T >
inline bool imathCompare( const Imath::Vec2<T> &a, const Imath::Vec2<T> &b )
{
	return ( a.x < b.x ) || ( a.x == b.x && a.y < b.y );
}

template< typename T >
inline bool imathCompare( const Imath::Vec3<T> &a, const Imath::Vec3<T> &b )
{
	return ( a.x < b.x ) || ( a.x == b.x && a.y < b.y ) || ( a.x == b.x && a.y == b.y && a.z < b.z );
}

template< typename T >
inline bool imathCompare( const Imath::Color3<T> &a, const Imath::Color3<T> &b )
{
	return ( a.x < b.x ) || ( a.x == b.x && a.y < b.y ) || ( a.x == b.x && a.y == b.y && a.z < b.z );
}

template< typename T >
inline bool imathCompare( const Imath::Color4<T> &a, const Imath::Color4<T> &b )
{
	return ( a.r < b.r ) || ( a.r == b.r && a.g < b.g ) || ( a.r == b.r && a.g == b.g && a.b < b.b ) ||
		( a.r == b.r && a.g == b.g && a.b == b.b && a.a < b.a ) ;
}

template< typename T >
inline bool imathCompare( const Imath::Quat<T> &a, const Imath::Quat<T> &b )
{
	return ( a.r < b.r ) || ( a.r == b.r && imathCompare( a.v, b.v ) );
}

template< typename T >
inline bool imathCompare( const Imath::Matrix33<T> &a, const Imath::Matrix33<T> &b )
{
	for( int i = 0; i < 3; i++ )
	{
		for( int j = 0; j < 3; j++ )
		{
			if( a[i][j] < b[i][j] )
			{
				return true;
			}
			else if( a[i][j] > b[i][j] )
			{
				return false;
			}
		}
	}
	return false;
}

template< typename T >
inline bool imathCompare( const Imath::Matrix44<T> &a, const Imath::Matrix44<T> &b )
{
	for( int i = 0; i < 4; i++ )
	{
		for( int j = 0; j < 4; j++ )
		{
			if( a[i][j] < b[i][j] )
			{
				return true;
			}
			else if( a[i][j] > b[i][j] )
			{
				return false;
			}
		}
	}
	return false;
}

template< typename T >
inline bool imathCompare( const Imath::Box<T> &a, const Imath::Box<T> &b )
{
	return imathCompare( a.min, b.min ) || ( a.min == b.min && imathCompare( a.max, b.max ) );
}


template< typename T >
void initializeFaceToSegments(
	const IECore::TypedData< std::vector< T > > *primVarData,
	const IntVectorData *indicesData,
	int &numSegments,
	ConstIntVectorDataPtr &faceToSegmentIndexData,
	std::vector<int> &remapSegmentIndices,
	int &remapSegmentIndexMin,
	const IECore::Canceller *canceller
)
{
	const std::vector<T> &data = primVarData->readable();
	numSegments = 0;
	remapSegmentIndexMin = 0;

	if( !indicesData )
	{
		if constexpr( std::is_same< T, int >::value )
		{
			// Special case for integer primVar. An integer primvar is common, because it is produced by
			// sources like the Gaffer MeshSegments node, and in this case, we can go much faster using
			// a vector than a hash map. This requires us to check the range of the data first though.

			Canceller::check( canceller );

			int dataMin = data[0];
			int dataMax = data[0];
			for( int i : data )
			{
				dataMin = std::min( i, dataMin );
				dataMax = std::max( i, dataMax );
			}

			// This is purely a heuristic - the vector is so much more efficient that we could even go
			// larger than data.size() * 4 and still win with the vector, but it seems like a reasonable
			// cutoff - once you get up to data.size() * 1000, the hash map will definitely win. The
			// important thing is that if the data is already contiguous unique integers ( ie. from
			// MeshSegments ) we always hit the fast path.
			if( (size_t)( dataMax - dataMin ) < data.size() * 4 )
			{
				// Instead of using a uniqueSegmentMap, we can just use the remapSegmentIndices vector for looking up ids
				remapSegmentIndexMin = dataMin;
				remapSegmentIndices.resize( dataMax + 1 - dataMin, -1 );

				Canceller::check( canceller );

				// We initially use the remapSegmentIndices vector just to store a flag for whether each
				// index is used, -1 == not used, 0 == used.
				//
				// This could be an indepedent data structure, but because of how we use it to build
				// remapSegmentIndices, it's better for memory use and locality to just use the same memory
				for( int d : data )
				{
					int &ins = remapSegmentIndices[ d - remapSegmentIndexMin ];
					if( ins == -1 )
					{
						ins = 0;
					}
				}

				Canceller::check( canceller );

				// Rather than needing to sort, we can just scan through the remap vector in order -
				// the first non-negative value is the first segment
				for( int &r : remapSegmentIndices )
				{
					if( r != -1 )
					{
						r = numSegments;
						numSegments++;
					}
				}

				Canceller::check( canceller );

				faceToSegmentIndexData = primVarData;

				return;
			}
		}
	}

	// Since we haven't taken the fast path, we need to treat the data values generically,
	// which means we need a map to identify the number of unique values
	std::unordered_map< T, int > uniqueSegmentMap;

	// After this if/else for indices, we will have one way or another have set up
	// faceToSegmentIndexData, uniqueSegmentMap, and optionally remapSegmentIndices,
	// such that: iterating through faceToSegmentIndexData for each face will yield
	// a segment id for each face, with the segment ids being contiguous integers,
	// and uniqueSegmentMap mapping from the original prim var value for a segment
	// to each segment id. Afterwards, we just need to fix the order.
	if( !indicesData )
	{
		IntVectorDataPtr buildFaceToSegmentIndexData = new IntVectorData;
		std::vector<int> &buildFaceToSegmentIndex = buildFaceToSegmentIndexData->writable();

		// We don't have any indices to start with ... take the simple but slow path - just check the
		// value in uniqueSegmentMap for every face, to populate faceToSegmentIndexData.
		buildFaceToSegmentIndex.reserve( data.size() );
		for( const auto &d : data )
		{
			// Due to the weirdness that is the implementation of std::vector<bool>, which is incompatible
			// with the hack we're using for deciding when to check the Canceller, we just don't cancel
			// for bools. Hopefully it won't take too long to hash all those bools
			if constexpr( !std::is_same< T, bool >::value )
			{
				if( ( &d - &(data[0]) ) % 10000 == 0 )
				{
					Canceller::check( canceller );
				}
			}
			unsigned int segmentId = uniqueSegmentMap.try_emplace( d, uniqueSegmentMap.size() ).first->second;
			buildFaceToSegmentIndex.push_back( segmentId );
		}

		faceToSegmentIndexData = buildFaceToSegmentIndexData;
	}
	else
	{
		// We have indices, so we'll use the existing indices, and only look at the data to build
		// remapSegmentIndices. It's possible that remapSegmentIndices is unnecessary here, and we
		// could just use the indices directly, but we need to remap if there are elements of the
		// data that are never used, or the same value appears multiple times in the data. The
		// easiest way to detect these situations is just to build the remapSegmentIndices.
		remapSegmentIndices.resize( data.size(), -1 );

		const std::vector<int> &indices = indicesData->readable();
		faceToSegmentIndexData = indicesData;

		for( int index : indices )
		{
			if( ( &index - &(indices[0]) ) % 10000 == 0 )
			{
				Canceller::check( canceller );
			}

			int &curSegment = remapSegmentIndices[ index ];
			if( curSegment != -1 )
			{
				continue;
			}

			// Note that we check the map when we first encounter this index in the indices,
			// but if we encounter it multiple times, the value will already be set in remapSegmentIndices,
			// and we'll take the continue above.
			// This is an important optimization, since it means if we have an indexed primvar with
			// 1000000 faces referencing 10 strings, we perform 10 string hashes, not 1000000
			unsigned int segmentId = uniqueSegmentMap.try_emplace( data[ index ], uniqueSegmentMap.size() ).first->second;
			curSegment = segmentId;

		}
	}

	numSegments = uniqueSegmentMap.size();

	// OK, now we just need to sort the segments. First read all the value/index pairs from uniqueSegmentMap
	// into a dense vector that we can sort.
	std::vector< std::pair< T, int > > sortList;
	sortList.reserve( numSegments );
	for( const auto &s : uniqueSegmentMap )
	{
		sortList.push_back( s );
	}

	// Now sort, using our custom comparison that support imath types
	std::sort( sortList.begin(), sortList.end(),
		[](const auto & a, const auto & b) -> bool
		{
			return imathCompare( a.first, b.first );
		}
	);

	// Now that it's been sorted, reading the indices from the second component of the sortList elements
	// gives us a mapping from sorted order to original order. We need a mapping the other way around,
	// so applySort is a vector which holds an inverted version of this mapping
	std::vector< int > applySort( sortList.size() );
	for( unsigned int i = 0; i < sortList.size(); i++ )
	{
		applySort[ sortList[i].second ] = i;
	}

	if( remapSegmentIndices.size() )
	{
		// If we are already using the remapping, we need to apply the sort to the existing remapping
		for( int &r : remapSegmentIndices )
		{
			r = applySort[ r ];
		}
	}
	else
	{
		// No existing remapping, we can just use the sort as the remapping
		remapSegmentIndices.swap( applySort );
	}

}

} // namespace

IECoreScene::MeshAlgo::MeshSplitter::MeshSplitter( ConstMeshPrimitivePtr mesh, const PrimitiveVariable &segmentPrimitiveVariable, const IECore::Canceller *canceller ) : m_mesh( mesh ), m_segmentPrimitiveVariable( segmentPrimitiveVariable )
{
	if( segmentPrimitiveVariable.interpolation != IECoreScene::PrimitiveVariable::Interpolation::Uniform )
	{
		throw IECore::Exception( "Primitive variable passed to MeshSplitter must be uniform." );
	}

	if( !mesh->isPrimitiveVariableValid( segmentPrimitiveVariable ) )
	{
		throw IECore::Exception( "Primitive variable passed to MeshSplitter must be valid." );
	}

	const size_t numFaces = mesh->numFaces();
	if( numFaces == 0 )
	{
		// If we don't initialize anything, numMeshes() will return 0, meaning there is no valid context to
		// call mesh() in, which is correct for an empty mesh
		return;
	}

	int numSegments = 0;
	ConstIntVectorDataPtr faceToSegmentIndexData;
	std::vector<int> remapSegmentIndices;
	// remapSegmentIndexMin specifies the lowest value in the faceToSegmentIndexBuffer that we need to remap:
	// it shifts all accesses to the remapSegmentIndices, allowing remapSegmentIndices to be used when the
	// lowest element is not 0
	int remapSegmentIndexMin = 0;


	IECore::dispatch( segmentPrimitiveVariable.data.get(), [ segmentPrimitiveVariable, &numSegments, &faceToSegmentIndexData, &remapSegmentIndices, &remapSegmentIndexMin, canceller]( const auto *primVarData )
		{
			using DataType = typename std::remove_pointer_t< decltype( primVarData ) >;
			if constexpr ( !TypeTraits::IsVectorTypedData<DataType>::value )
			{
				throw IECore::Exception( "Invalid PrimitiveVariable, data is not a vector." );
			}
			else
			{
				initializeFaceToSegments< typename DataType::ValueType::value_type >(
					primVarData,
					segmentPrimitiveVariable.indices.get(),
					numSegments,
					faceToSegmentIndexData,
					remapSegmentIndices,
					remapSegmentIndexMin,
					canceller
				);
			}
		}
	);

	const std::vector<int> &faceToSegmentIndex = faceToSegmentIndexData->readable();

	// Now that we have our faceToSegmentIndex and remapSegmentIndices vector, we can count the number of faces
	// for each output mesh
	std::vector<int> faceCounts;
	faceCounts.resize( numSegments, 0 );

	Canceller::check( canceller );

	for( int i : faceToSegmentIndex )
	{
		faceCounts[ remapSegmentIndices[ i - remapSegmentIndexMin ] ]++;
	}

	// We need store the faces so that it's easy to access all the faces for one output mesh at a time.
	// To keep things nice and contiguous, and avoid small allocations for small meshes, we will allocate
	// some vectors with the original size of the verticesPerFace vector, but sorted by output mesh index

	Canceller::check( canceller );

	// meshIndices stores the offset in m_faceRemap where each mesh starts
	m_meshIndices.reserve( faceCounts.size() );
	int meshStartIndex = 0;
	for( int c : faceCounts )
	{
		m_meshIndices.push_back( meshStartIndex );
		meshStartIndex += c;
	}

	// Now output the faceRemap vector, which tells us for each output face, the index of the source face
	const std::vector<int> &verticesPerFace = mesh->verticesPerFace()->readable();

	// We do this by keeping track of the current position for each output mesh, and scanning through
	// all the input faces, incrementing the correct output mesh position when we find a face for that
	// mesh.
	std::vector<int> curMeshIndices( m_meshIndices );

	Canceller::check( canceller );

	m_faceRemap.resize( numFaces );
	for( unsigned int faceIndex = 0; faceIndex < numFaces; faceIndex++ )
	{
		int meshId = remapSegmentIndices[ faceToSegmentIndex[ faceIndex ] - remapSegmentIndexMin ];
		m_faceRemap[ curMeshIndices[ meshId ] ] = faceIndex;
		curMeshIndices[ meshId ]++;
	}

	Canceller::check( canceller );

	// When accessing faces through m_faceRemap, we need to independently access a face based on its index.
	// We don't want to scan from the start summing all the verticesPerFace each time, so this requires
	// us to pre-accumulate a running sum of verticesPerFace, that we can index directly into
	int faceVertexIndex = 0;
	m_faceIndices.reserve( numFaces );
	for( int f : verticesPerFace )
	{
		m_faceIndices.push_back( faceVertexIndex );
		faceVertexIndex += f;
	}

}

namespace {

// Reindexer allows taking a list of indices that reference some subset of an id range, and
// compress the id range into a shorter range of only the id's that are used by the indices.
// You can then output a new list of indices into the compressed range, and call remapData()
// to reorder data stored with the ids into the compressed range, or getDataRemapping() to
// return a vector that describes the required reordering.
//
// This is the performance critical part of both splitting vertices on a mesh, and splitting
// primitive variables.
//
// It is implemented as a vector of fixed size blocks of memory spanning the entire range of
// original ids. This is wasteful of memory when the number of indices is very low relative
// to the range of original ids ( ie. you are splitting an extremely large mesh into extremely
// small pieces ), but it much more efficient to just index into a location than it is to
// hash an integer to use it as a hashmap key.
//
// It would be possible to implement a much more compact version storing only 1 bit per id,
// with separate counts every 64 or 128 ids, which would perform better on very large
// meshes being split into very small pieces, but that doesn't help much in average cases,
// and is a fair bit more complicated. If we encounter issues with performance when
// splitting into tiny meshes, the simplest solution is probably to switch to an unordered_map
// when numIndices is much smaller than numOriginalIds - the break even point for performance
// seems to be when numOriginalIds is about 10 000 times greater than numIndices - or actually
// much higher when the indices are fairly coherent ( ie. the ids which are selected fall
// mainly in the same range, so many are in the same block ), which is common for most ways of
// producing meshes.

class Reindexer
{
public:

	// Construct a Reindexer
	//
	// numOriginalIds : determines the highest integer that may appear in the indices
	// numIndices : how many indices will be added. You must call addIndex() this many times
	// blockSize : a performance tuning value determining how large the blocks that are
	//     allocated to hold ids are. Should be left at default.
	//
	Reindexer( int numOriginalIds, int numIndices, int blockSize = 1024 ) :
		m_newIndicesData( new IntVectorData() ),
		m_newIndices( m_newIndicesData->writable() ),
		m_blockSize( blockSize ),
		m_fromOldIds( ( numOriginalIds - 1 ) / blockSize + 1 ),
		m_numIdsUsed( 0 ),
		m_indicesComputed( false )
	{
		m_newIndices.reserve( numIndices );
	}

	// Add an index - if the indexed id is not yet part of the output ids, it will be included
	void addIndex( int id )
	{
		// Determine which block to use, and the index within that block
		int blockId = id / m_blockSize;
		int subIndex = id % m_blockSize;

		auto &block = m_fromOldIds[ blockId ];

		if( !block )
		{
			// Need to allocate the block for this index
			block = std::make_unique< std::vector<int> >( m_blockSize, -1 );
		}

		// We initially record that this index is used just by marking it with a 0, against the background of -1.
		// Once computeIndices is called, the 0 will be replaced with a new index, only counting indices that are
		// used.
		(*block)[ subIndex ] = 0;

		m_newIndices.push_back( id );

		m_indicesComputed = false;
	}

	// Don't add the index, but just test if it is a part of the reindex. If it is an
	// id which has already been added, return the new id, otherwise return -1
	inline int testIndex( int id )
	{
		computeIndices();
		int blockId = id / m_blockSize;
		int subIndex = id % m_blockSize;
		auto &block = m_fromOldIds[ blockId ];
		if( block )
		{
			return (*block)[ subIndex ];
		}
		else
		{
			return -1;
		}
	}

	// Get the new indices. Call after calling addIndex for every original index
	IntVectorDataPtr getNewIndices()
	{
		computeIndices();
		return m_newIndicesData;
	}

	// Given data for range 0 .. numOriginalIndices - 1, set the output
	// to a size based on the number of unique ids used by the indices,
	// and set the values to the corresponding input data.
	template <typename T >
	void remapData( const std::vector<T> &in, std::vector<T> &out )
	{
		computeIndices();
		out.resize( m_numIdsUsed );
		for( unsigned int i = 0; i < m_fromOldIds.size(); i++ )
		{
			auto &blockPointer = m_fromOldIds[ i ];
			if( blockPointer )
			{
				std::vector<int> &block = *blockPointer;
				for( int j = 0; j < m_blockSize; j++ )
				{
					int newId = block[j];
					if( newId != -1 )
					{
						int oldId = i * m_blockSize + j;
						out[ newId ] = in[oldId];
					}
				}
			}
		}
	}

	// Like remapData, but instead of returning remapped data, return the
	// original id corresponding to each id of the output
	void getDataRemapping( std::vector<int> &dataRemap )
	{
		computeIndices();
		dataRemap.resize( m_numIdsUsed );
		for( unsigned int i = 0; i < m_fromOldIds.size(); i++ )
		{
			auto &blockPointer = m_fromOldIds[ i ];
			if( blockPointer )
			{
				std::vector<int> &block = *blockPointer;
				for( int j = 0; j < m_blockSize; j++ )
				{
					int newId = block[j];
					if( newId != -1 )
					{
						int oldId = i * m_blockSize + j;
						dataRemap[ newId ] = oldId;
					}
				}
			}
		}
	}

private:

	void computeIndices()
	{
		// Once indices have been added, and before using them, this function is called to
		// compute the new indices.
		if( m_indicesComputed )
		{
			return;
		}

		m_indicesComputed = true;

		for( unsigned int blockId = 0; blockId < m_fromOldIds.size(); blockId++ )
		{
			auto &block = m_fromOldIds[ blockId ];
			if( !block )
			{
				continue;
			}

			for( int i = 0; i < m_blockSize; i++ )
			{
				if( (*block)[i] != -1 )
				{
					(*block)[i] = m_numIdsUsed;
					m_numIdsUsed++;
				}
			}
		}

		for( int &id : m_newIndices )
		{
			int blockId = id / m_blockSize;
			int subIndex = id % m_blockSize;

			id = (*m_fromOldIds[ blockId ])[subIndex];
		}
	}

	// IntVectorData to hold the new indices
	IntVectorDataPtr m_newIndicesData;
	std::vector< int > &m_newIndices;

	// A performance tuning value determining how large the blocks that are allocated to hold ids are.
	const int m_blockSize;

	// Store the mapping from old ids to new ids. The outer vector holds a unique_ptr for each
	// block of m_blockSize ids in the original id range. These pointers are null if no ids from
	// that block have been used. Once a block is used, it is allocated with a vector that is set
	// to -1 for ids which have not been used, and zeros for ids which have been used.  When computeIndices()
	// is called, all used elements get a new id assigned, relative to just the used ids.
	std::vector< std::unique_ptr< std::vector< int > > > m_fromOldIds;

	// How many unique ids have appeared in the indices added so far
	int m_numIdsUsed;

	// Whether we have yet computed the new indices for each used index
	bool m_indicesComputed;

};

struct ResamplePrimitiveVariableFunctor
{

	// Resample a primitive variable, given all the necessary indices from the mesh.
	//
	// The implementation is all pretty straightforward, since we already have the
	// indices we need for each case prepared, and we can use Reindexer to deal with
	// indexed primvars.
	//
	// All the bogus [[maybe_unused]] statements are required due to this bug:
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81676
	// which I think was fixed in GCC 10, but I haven't tested

	template<typename T>
	PrimitiveVariable operator()( [[maybe_unused]] const T *data, const PrimitiveVariable &primVar, [[maybe_unused]] int startIndex, [[maybe_unused]] int numFaces, [[maybe_unused]] int totalFaceVerts, [[maybe_unused]] const std::vector<int> &faceRemap, [[maybe_unused]] const std::vector<int> &verticesPerFace, [[maybe_unused]] const std::vector<int> &faceIndices, [[maybe_unused]] const std::vector<int> &vertRemapBackwards, [[maybe_unused]] const Canceller *canceller )
	{
		if(
			primVar.interpolation != PrimitiveVariable::Interpolation::Uniform &&
			primVar.interpolation != PrimitiveVariable::Interpolation::Vertex &&
			primVar.interpolation != PrimitiveVariable::Interpolation::Varying &&
			primVar.interpolation != PrimitiveVariable::Interpolation::FaceVarying
		)
		{
			// Just copying works for constants
			return primVar;
		}

		if constexpr ( ! IECore::TypeTraits::IsVectorTypedData< T >::value )
		{
			throw IECore::Exception( "Invalid PrimitiveVariable, data is not a vector." );
		}
		else
		{
			const typename T::ValueType &in = data->readable();
			typename T::Ptr outData = new T;
			if constexpr ( IECore::TypeTraits::IsGeometricTypedData< T >::value )
			{
				outData->setInterpretation( data->getInterpretation() );
			}

			typename T::ValueType &out = outData->writable();

			if( !primVar.indices )
			{
				switch( primVar.interpolation )
				{
				case PrimitiveVariable::Interpolation::Uniform:
					out.reserve( numFaces );
					for( int i = 0; i < numFaces; i++ )
					{
						out.push_back( in[ faceRemap[ startIndex + i ] ] );
					}
					break;
				case PrimitiveVariable::Interpolation::Vertex:
				case PrimitiveVariable::Interpolation::Varying:
					out.reserve( vertRemapBackwards.size() );
					for( int remap : vertRemapBackwards )
					{
						out.push_back( in[ remap ] );
					}
					break;
				case PrimitiveVariable::Interpolation::FaceVarying:
					out.reserve( totalFaceVerts );
					for( int i = 0; i < numFaces; i++ )
					{
						int numVerts = verticesPerFace[ faceRemap[ startIndex + i ] ];
						int faceStart = faceIndices[ faceRemap[ startIndex + i ] ];
						for( int j = 0; j < numVerts; j++ )
						{
							out.push_back( in[ faceStart + j ] );
						}
					}
					break;
				default:
					// Impossible because of early return if not one of 4 case values
					assert( false );
				}

				return PrimitiveVariable( primVar.interpolation, outData );
			}

			const std::vector<int> &inIndices = primVar.indices->readable();

			IntVectorDataPtr outIndicesData;

			switch( primVar.interpolation )
			{
			case PrimitiveVariable::Interpolation::Uniform:
				{
					Reindexer r( in.size(), numFaces );
					for( int i = 0; i < numFaces; i++ )
					{
						if( i % 1000 == 0 )
						{
							Canceller::check( canceller );
						}

						r.addIndex( inIndices[ faceRemap[ startIndex + i ] ] );
					}
					outIndicesData = r.getNewIndices();
					r.remapData( in, out );
				}
				break;
			case PrimitiveVariable::Interpolation::Vertex:
			case PrimitiveVariable::Interpolation::Varying:
				{
					Reindexer r( in.size(), vertRemapBackwards.size() );
					for( unsigned int i = 0; i < vertRemapBackwards.size(); i++ )
					{
						if( i % 1000 == 0 )
						{
							Canceller::check( canceller );
						}
						r.addIndex( inIndices[ vertRemapBackwards[i] ] );
					}
					outIndicesData = r.getNewIndices();
					r.remapData( in, out );
				}
				break;
			case PrimitiveVariable::Interpolation::FaceVarying:
				{
					Reindexer r( in.size(), totalFaceVerts );
					for( int i = 0; i < numFaces; i++ )
					{
						if( i % 1000 == 0 )
						{
							Canceller::check( canceller );
						}

						int numVerts = verticesPerFace[ faceRemap[ startIndex + i ] ];
						int faceStart = faceIndices[ faceRemap[ startIndex + i ] ];
						for( int j = 0; j < numVerts; j++ )
						{
							r.addIndex( inIndices[ faceStart + j ] );
						}
					}
					outIndicesData = r.getNewIndices();
					r.remapData( in, out );
				}
				break;
			default:
				// Impossible because of early return if not one of 4 case values
				assert( false );
			}

			return PrimitiveVariable( primVar.interpolation, outData, outIndicesData );
		}
	}
};

} // namespace


MeshPrimitivePtr IECoreScene::MeshAlgo::MeshSplitter::mesh( int segmentId, const IECore::Canceller *canceller ) const
{
	if( segmentId < 0 || segmentId > (int)m_meshIndices.size() )
	{
		throw IECore::Exception( "Invalid segment id " + std::to_string( segmentId ) );
	}

	// Based on our index, and the index of the next mesh in m_meshIndices, we know how many faces to output
	const int startIndex = m_meshIndices[ segmentId ];
	const int endIndex = ( segmentId + 1 < (int)m_meshIndices.size() ) ? m_meshIndices[ segmentId + 1 ] : m_faceRemap.size();
	const int numFaces = endIndex - startIndex;

	IntVectorDataPtr verticesPerFaceData = new IntVectorData();
	std::vector<int> &verticesPerFace = verticesPerFaceData->writable();
	verticesPerFace.reserve( numFaces );
	int totalFaceVerts = 0;
	const std::vector<int> &sourceVertexIds = m_mesh->vertexIds()->readable();
	const std::vector<int> &sourceVerticesPerFace = m_mesh->verticesPerFace()->readable();

	Canceller::check( canceller );
	// Outputting the verticesPerFace is straightforward - just read the source mesh's verticesPerFace
	// through m_faceRemap
	for( int i = startIndex; i < endIndex; i++ )
	{
		int originalFaceIndex = m_faceRemap[i];
		int faceVerts = sourceVerticesPerFace[ originalFaceIndex ];
		verticesPerFace.push_back( faceVerts );
		totalFaceVerts += faceVerts;
	}

	// For the vertexIds, we need to iterate through all the original faces that are referenced in m_faceRemap,
	// and we need to use them to build a Reindexer that only references the vertices we are actually using
	Canceller::check( canceller );
	Reindexer vertReindexer( m_mesh->variableSize( PrimitiveVariable::Interpolation::Vertex ), totalFaceVerts );
	for( int i = startIndex; i < endIndex; i++ )
	{
		if( i % 1000 == 0 )
		{
			Canceller::check( canceller );
		}

		int originalFaceIndex = m_faceRemap[i];
		int faceVerts = sourceVerticesPerFace[ originalFaceIndex ];
		int faceStart = m_faceIndices[ originalFaceIndex ];
		for( int j = 0; j < faceVerts; j++ )
		{
			vertReindexer.addIndex( sourceVertexIds[ faceStart + j ] );
		}
	}

	// We need to track which original vertex our vertices came from so we can pull primvar data from them.
	Canceller::check( canceller );
	std::vector<int> vertRemapBackwards;
	vertReindexer.getDataRemapping( vertRemapBackwards );

	MeshPrimitivePtr ret = new MeshPrimitive( verticesPerFaceData, vertReindexer.getNewIndices(), m_mesh->interpolation() );

	// In order to remap the corners, we test every vertex in the original corner list, and see if it is
	// one of the vertices we are using
	const std::vector<int> &originalCornerIds = m_mesh->cornerIds()->readable();
	if( originalCornerIds.size() )
	{
		Canceller::check( canceller );
		IntVectorDataPtr cornerIdsData = new IntVectorData();
		std::vector<int> &cornerIds = cornerIdsData->writable();
		FloatVectorDataPtr cornerSharpnessesData = new FloatVectorData();
		std::vector<float> &cornerSharpnesses = cornerSharpnessesData->writable();
		const std::vector<float> &originalCornerSharpnesses = m_mesh->cornerSharpnesses()->readable();
		for( unsigned int i = 0; i < originalCornerIds.size(); i++ )
		{
			int newId = vertReindexer.testIndex( originalCornerIds[i] );
			if( newId != -1 )
			{
				cornerIds.push_back( newId );
				cornerSharpnesses.push_back( originalCornerSharpnesses[i] );
			}
		}
		ret->setCorners( cornerIdsData.get(), cornerSharpnessesData.get() );
	}

	// Creases are similar - check every vertex in the original creases, and see if it's a vertex
	// we're using. It's a little bit more complicated because omitting vertices could result in
	// deleting creases ( since you can't have a crease with less than two vertices ), or turning
	// one crease into multiple creases ( if it has at least 5 vertices, and the middle vertex is
	// not part of the output mesh, splitting it in two )
	const std::vector<int> &originalCreaseLengths = m_mesh->creaseLengths()->readable();
	if( originalCreaseLengths.size() )
	{
		Canceller::check( canceller );
		IntVectorDataPtr creaseLengthsData = new IntVectorData();
		std::vector<int> &creaseLengths = creaseLengthsData->writable();
		IntVectorDataPtr creaseIdsData = new IntVectorData();
		std::vector<int> &creaseIds = creaseIdsData->writable();
		FloatVectorDataPtr creaseSharpnessesData = new FloatVectorData();
		std::vector<float> &creaseSharpnesses = creaseSharpnessesData->writable();

		const std::vector<int> &originalCreaseIds = m_mesh->creaseIds()->readable();
		const std::vector<float> &originalCreaseSharpnesses = m_mesh->creaseSharpnesses()->readable();

		int creaseIdOffset = 0;
		for( size_t i = 0; i < originalCreaseLengths.size(); i++ )
		{
			int j = 0;
			while( j < originalCreaseLengths[i] )
			{
				// Skip non included verts
				while( j < originalCreaseLengths[i] && vertReindexer.testIndex( originalCreaseIds[creaseIdOffset + j] ) == -1 )
				{
					j++;
				}

				int startCrease = j;

				// Scan until we reach the end, or a vert that isn't included.
				// If there in a non-included vert in the middle of a crease of length 5 or more,
				// we may need to output more than one crease per input crease.
				while( j < originalCreaseLengths[i] && vertReindexer.testIndex( originalCreaseIds[creaseIdOffset + j] ) != -1 )
				{
					j++;
				}

				// We've either reached the end, or a non-included vert - output a crease
				if( j - startCrease >= 2 )
				{
					for( int k = startCrease; k < j; k++ )
					{
						// \todo - a little wasteful here, should be caching these lookups
						creaseIds.push_back( vertReindexer.testIndex( originalCreaseIds[creaseIdOffset + k] ) );
					}
					creaseLengths.push_back( j - startCrease );
					creaseSharpnesses.push_back( originalCreaseSharpnesses[i] );
				}
			}
			creaseIdOffset += originalCreaseLengths[i];
		}
		ret->setCreases( creaseLengthsData.get(), creaseIdsData.get(), creaseSharpnessesData.get() );
	}

	// Now split all primvars using	ResamplePrimitiveVariableFunctor()
	for( const auto &p : m_mesh->variables )
	{
		if( !m_mesh->isPrimitiveVariableValid( p.second ) )
		{
			IECore::msg( Msg::Error, "MeshAlgoSplit", "Cannot resample " + p.first + " because it is not valid to start with." );
			continue;
		}
		Canceller::check( canceller );
		ret->variables[ p.first ] = IECore::dispatch( p.second.data.get(), ResamplePrimitiveVariableFunctor(), p.second, startIndex, numFaces, totalFaceVerts, m_faceRemap, sourceVerticesPerFace, m_faceIndices, vertRemapBackwards, canceller );
	}

	return ret;
}

Imath::Box3f IECoreScene::MeshAlgo::MeshSplitter::bound( int segmentId, const IECore::Canceller *canceller ) const
{
	if( segmentId < 0 || segmentId > (int)m_meshIndices.size() )
	{
		throw IECore::Exception( "Invalid segment id " + std::to_string( segmentId ) );
	}

	Box3f result;
	PrimitiveVariableMap::const_iterator it = m_mesh->variables.find( "P" );
	if( it == m_mesh->variables.end() )
	{
		return result;
	}

	ConstV3fVectorDataPtr pData = runTimeCast<const V3fVectorData>( it->second.data );
	if( !pData )
	{
		return result;
	}

	const std::vector<V3f> &p = pData->readable();

	// Based on our index, and the index of the next mesh in m_meshIndices, we know how many faces to scan
	const int startIndex = m_meshIndices[ segmentId ];
	const int endIndex = ( segmentId + 1 < (int)m_meshIndices.size() ) ? m_meshIndices[ segmentId + 1 ] : m_faceRemap.size();

	const std::vector<int> &sourceVertexIds = m_mesh->vertexIds()->readable();
	const std::vector<int> &sourceVerticesPerFace = m_mesh->verticesPerFace()->readable();

	Canceller::check( canceller );

	// Loop through every face in this output, and all the vertices in each face, and extend the result
	// by the position for each vertex index
	for( int i = startIndex; i < endIndex; i++ )
	{
		if( i % 10000 == 0 )
		{
			Canceller::check( canceller );
		}

		int originalFaceIndex = m_faceRemap[i];
		int faceVerts = sourceVerticesPerFace[ originalFaceIndex ];
		int faceStart = m_faceIndices[ originalFaceIndex ];
		for( int j = 0; j < faceVerts; j++ )
		{
			result.extendBy( p[ sourceVertexIds[ faceStart + j ] ] );
		}
	}
	return result;
}
