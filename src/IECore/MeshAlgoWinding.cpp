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

#include "IECore/DespatchTypedData.h"
#include "IECore/MeshAlgo.h"
#include "IECore/PolygonIterator.h"

using namespace Imath;
using namespace IECore;

//////////////////////////////////////////////////////////////////////////
// Reverse winding
//////////////////////////////////////////////////////////////////////////

namespace
{

template<typename T>
void reverseWinding( MeshPrimitive *mesh, T &values )
{
	for( PolygonIterator it = mesh->faceBegin(), eIt = mesh->faceEnd(); it != eIt; ++it )
	{
		std::reverse( it.faceVaryingBegin( values.begin() ), it.faceVaryingEnd( values.begin() ) );
	}
}

struct ReverseWindingFunctor
{

	typedef void ReturnType;

	ReverseWindingFunctor( MeshPrimitive *mesh ) : m_mesh( mesh )
	{
	}

	template<typename T>
	void operator()( T *data )
	{
		reverseWinding( m_mesh, data->writable() );
	}

	private :

		MeshPrimitive *m_mesh;

};

} // namespace

void IECore::MeshAlgo::reverseWinding( MeshPrimitive *mesh )
{
	IntVectorDataPtr vertexIds = mesh->vertexIds()->copy();
	::reverseWinding( mesh, vertexIds->writable() );
	mesh->setTopologyUnchecked(
		mesh->verticesPerFace(),
		vertexIds,
		mesh->variableSize( PrimitiveVariable::Vertex ),
		mesh->interpolation()
	);

	ReverseWindingFunctor reverseWindingFunctor( mesh );
	for( auto &it : mesh->variables )
	{
		if( it.second.interpolation == PrimitiveVariable::FaceVarying )
		{
			if( it.second.indices )
			{
				::reverseWinding<IntVectorData::ValueType>( mesh, it.second.indices->writable() );
			}
			else
			{
				despatchTypedData<ReverseWindingFunctor, TypeTraits::IsVectorTypedData>( it.second.data.get(), reverseWindingFunctor );
			}
		}
	}
}

