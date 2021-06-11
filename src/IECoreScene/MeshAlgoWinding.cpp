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

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/PolygonIterator.h"

#include "IECore/DataAlgo.h"

#include <unordered_set>

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Reverse winding
//////////////////////////////////////////////////////////////////////////

namespace
{

template<typename T>
void reverseWinding( MeshPrimitive *mesh, T &values, const Canceller *canceller )
{
	for( PolygonIterator it = mesh->faceBegin(), eIt = mesh->faceEnd(); it != eIt; ++it )
	{
		Canceller::check( canceller );
		std::reverse( it.faceVaryingBegin( values.begin() ), it.faceVaryingEnd( values.begin() ) );
	}
}

struct ReverseWindingFunctor
{

	ReverseWindingFunctor( MeshPrimitive *mesh, const Canceller *canceller ) : m_mesh( mesh ), m_canceller( canceller )
	{
	}

	template<typename T>
	void operator()( TypedData<std::vector<T>> *data )
	{
		reverseWinding( m_mesh, data->writable(), m_canceller );
	}

	void operator()( Data *data )
	{
		throw IECore::Exception( "Expected VectorTypedData" );
	}

	private :

		MeshPrimitive *m_mesh;
		const Canceller *m_canceller;

};

} // namespace

void IECoreScene::MeshAlgo::reverseWinding( MeshPrimitive *mesh, const Canceller *canceller )
{
	IntVectorDataPtr vertexIds = mesh->vertexIds()->copy();
	::reverseWinding( mesh, vertexIds->writable(), canceller );
	mesh->setTopologyUnchecked(
		mesh->verticesPerFace(),
		vertexIds,
		mesh->variableSize( PrimitiveVariable::Vertex ),
		mesh->interpolation()
	);

	std::unordered_set<const Data *> visited;
	ReverseWindingFunctor reverseWindingFunctor( mesh, canceller );
	for( auto &it : mesh->variables )
	{
		if( it.second.interpolation == PrimitiveVariable::FaceVarying )
		{
			if( it.second.indices )
			{
				if( visited.insert( it.second.indices.get() ).second )
				{
					::reverseWinding<IntVectorData::ValueType>( mesh, it.second.indices->writable(), canceller );
				}
			}
			else if( visited.insert( it.second.data.get() ).second )
			{
				dispatch( it.second.data.get(), reverseWindingFunctor );
			}
		}
	}
}

