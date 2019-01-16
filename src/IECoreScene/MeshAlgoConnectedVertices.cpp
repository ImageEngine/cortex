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

pair<IntVectorDataPtr, IntVectorDataPtr> MeshAlgo::connectedVertices( const MeshPrimitive *mesh )
{
	size_t numVertices = mesh->variableData< V3fVectorData >( "P", PrimitiveVariable::Vertex )->readable().size();
	const vector<int> &numVerticesPerFace = mesh->verticesPerFace()->readable();
	const vector<int> &vertexIds = mesh->vertexIds()->readable();

	vector< set<int> > neighbors( numVertices );

	int currentVertOffset = 0;
	for ( auto &vertsPerFace : numVerticesPerFace )
	{
		for ( int i = 0; i < vertsPerFace; ++i)
		{
			const int &faceVert = vertexIds[ currentVertOffset + i ];
			const int &faceVertNext = vertexIds[ currentVertOffset + ( i + 1 ) % vertsPerFace ];
			neighbors[ faceVert ].insert( faceVertNext );
			neighbors[ faceVertNext ].insert( faceVert );
		}
		currentVertOffset += vertsPerFace;
	}

	int neighborCount = 0;
	for ( auto &n : neighbors )
	{
		neighborCount += n.size();
	}

	IntVectorDataPtr offsets = new IntVectorData();
	IntVectorDataPtr neighborList = new IntVectorData();
	vector<int> &offsetsW = offsets->writable();
	vector<int> &neighborListW = neighborList->writable();

	neighborListW.resize( neighborCount, -1 );
	offsetsW.resize( neighbors.size(), -1 );

	int ix = 0;
	for ( size_t i = 0; i < neighbors.size(); ++i )
	{
		for ( auto &n : neighbors[ i ] )
		{
			neighborListW[ ix++ ] = n;
		}
		offsetsW[ i ] = ix;
	}

	return pair<IntVectorDataPtr, IntVectorDataPtr>( neighborList, offsets );
}
