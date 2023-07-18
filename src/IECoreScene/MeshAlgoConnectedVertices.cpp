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

using namespace std;
using namespace IECore;
using namespace IECoreScene;

namespace {

inline void linearInsert( int *list, int val )
{
	while( *list != -1 )
	{
		if( *list == val )
		{
			return;
		}
		list++;
	}
	*list = val;
}

} // namespace

pair<IntVectorDataPtr, IntVectorDataPtr> MeshAlgo::connectedVertices( const MeshPrimitive *mesh, const Canceller *canceller )
{
	size_t numVertices = mesh->variableData< V3fVectorData >( "P", PrimitiveVariable::Vertex )->readable().size();
	const vector<int> &numVerticesPerFace = mesh->verticesPerFace()->readable();
	const vector<int> &vertexIds = mesh->vertexIds()->readable();

	IntVectorDataPtr offsetsData = new IntVectorData();
	vector<int> &offsets = offsetsData->writable();
	offsets.resize( numVertices, 0 );

	Canceller::check( canceller );

	// Start initializing the offsets vector by storing the maximum number of possible neighbours each
	// vertex could have. Every time a vertex appears in the vertex id list, that means it's part of
	// a polygon, and has two more edges connecting it to two other vertices. In the common case,
	// the number we arrive at by this method is twice as high as needed, because in a manifold mesh,
	// every edge appears in the face list twice.
	for( int i : vertexIds )
	{
		offsets[ i ] += 2;
	}

	Canceller::check( canceller );

	// Convert the neighbour counts into offsets to the start of each list of possible neighbours,
	// by storing a running total
	int totalPossibleNeighbours = 0;
	for( int &o : offsets )
	{
		int count = o;
		o = totalPossibleNeighbours;
		totalPossibleNeighbours += count;
	}

	// Allocate storage for all possible neighbours, and collect neighbours from every face.
	// On a manifold mesh, only half the storage for each vertex will be used, because every
	// vertex pair occurs in two separate faces. ( Unused element will be left at -1 )
	Canceller::check( canceller );
	IntVectorDataPtr neighbourListData = new IntVectorData();
	std::vector<int> &neighbourList = neighbourListData->writable();
	neighbourList.resize( totalPossibleNeighbours, -1 );
	int faceStart = 0;
	for ( auto &vertsPerFace : numVerticesPerFace )
	{
		Canceller::check( canceller );

		for ( int i = 0; i < vertsPerFace; ++i)
		{
			int faceVert = vertexIds[ faceStart + i ];
			int faceVertNext = vertexIds[ faceStart + ( i + 1 ) % vertsPerFace ];
			linearInsert( &neighbourList[ offsets[faceVert] ], faceVertNext );
			linearInsert( &neighbourList[ offsets[faceVertNext] ], faceVert );
		}
		faceStart += vertsPerFace;
	}

	// Compact the neighbourList to contain only used vertices by removing any -1 values,
	// and update offsets accordingly - we also convert the offsets from pointing to the
	// start of the lists to the end of the lists at the same time.
	int usedOutputIndex = 0;
	for( int i = 0; i < (int)offsets.size(); i++ )
	{
		Canceller::check( canceller );
		int end = i < (int)offsets.size() - 1 ? offsets[i + 1] : totalPossibleNeighbours;
		int neighbourIndex = offsets[i];
		while( neighbourIndex < end && neighbourList[neighbourIndex] != -1 )
		{
			neighbourList[usedOutputIndex++] = neighbourList[neighbourIndex++];
		}
		offsets[i] = usedOutputIndex;
	}
	neighbourList.resize( usedOutputIndex );

	// It would be a little simpler to just call neighbourList.shrink_to_fit() here so the output would always
	// be exactly sized right, but this reallocation costs about 10% of our performance, so instead I guess
	// we'll just document that you may want to call shrink_to_fit() if you're keeping this data around.
	
	return pair<IntVectorDataPtr, IntVectorDataPtr>( neighbourListData, offsetsData );
}

pair<IntVectorDataPtr, IntVectorDataPtr> MeshAlgo::correspondingFaceVertices( const MeshPrimitive *mesh, const Canceller *canceller )
{
	size_t numVertices = mesh->variableData< V3fVectorData >( "P", PrimitiveVariable::Vertex )->readable().size();
	const vector<int> &vertexIds = mesh->vertexIds()->readable();

	IntVectorDataPtr offsetsData = new IntVectorData();
	vector<int> &offsets = offsetsData->writable();
	Canceller::check( canceller );
	offsets.resize( numVertices, 0 );

	// Start initializing the offsets vector by storing the number of face vertices
	for( int i : vertexIds )
	{
		offsets[ i ]++;
	}

	Canceller::check( canceller );

	// Convert the counts into offsets to the start of each list of face vertices
	int countFaceVertices = 0;
	for( int &o : offsets )
	{
		int count = o;
		o = countFaceVertices;
		countFaceVertices += count;
	}

	Canceller::check( canceller );
	IntVectorDataPtr faceVerticesData = new IntVectorData();
	vector<int> &faceVertices = faceVerticesData->writable();
	Canceller::check( canceller );
	faceVertices.resize( countFaceVertices );

	// Now run through all faces, storing face vertex indices in the new list. We increment the offset
	// for each face vertex we store, meaning that the indices start out pointing to the beginning of the
	// list for each vertex, and end up pointing at the end of the list for each vertex.
	for( unsigned int i = 0; i < vertexIds.size(); i++ )
	{
		int vert = vertexIds[ i ];
		faceVertices[ offsets[ vert ] ] = i;
		offsets[ vert ] ++;
	}

	return pair<IntVectorDataPtr, IntVectorDataPtr>( faceVerticesData, offsetsData );
}
