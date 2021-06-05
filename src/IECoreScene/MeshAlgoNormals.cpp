//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2019, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PolygonAlgo.h"

#include "boost/format.hpp"
#include "boost/iterator/transform_iterator.hpp"
#include "boost/iterator/zip_iterator.hpp"
#include "boost/tuple/tuple.hpp"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

PrimitiveVariable MeshAlgo::calculateNormals( const MeshPrimitive *mesh, PrimitiveVariable::Interpolation interpolation, const std::string &position, const Canceller *canceller )
{
	const V3fVectorData *pData = mesh->variableData<V3fVectorData>( position, PrimitiveVariable::Vertex );
	if( !pData )
	{
		throw InvalidArgumentException( boost::str( boost::format( "MeshAlgo::calculateNormals : MeshPrimitive has no \"%s\" primitive variable." ) % position ) );
	}
	const std::vector<V3f> &points = pData->readable();

	if( interpolation != PrimitiveVariable::Vertex && interpolation != PrimitiveVariable::Uniform )
	{
		throw InvalidArgumentException( "MeshAlgo::calculateNormals : \"interpolation\" must be Vertex or Uniform" );
	}

	V3fVectorDataPtr normalsData = new V3fVectorData;
	normalsData->setInterpretation( GeometricData::Normal );
	auto &normals = normalsData->writable();

	const auto &verticesPerFace = mesh->verticesPerFace()->readable();
	if( interpolation == PrimitiveVariable::Uniform )
	{
		normals.reserve( verticesPerFace.size() );
	}
	else
	{
		normals.resize( points.size(), Imath::V3f( 0 ) );
	}

	const auto &vertIds = mesh->vertexIds()->readable();
	const int *vertId = &(vertIds[0]);
	for( auto numVerts : verticesPerFace )
	{
		Canceller::check( canceller );
		// calculate the face normal. note that this method is very naive, and doesn't
		// cope with colinear vertices or concave faces - we could use polygonNormal() from
		// PolygonAlgo.h to deal with that, but currently we'd prefer to avoid the overhead.
		const V3f &p0 = points[*vertId];
		const V3f &p1 = points[*(vertId+1)];
		const V3f &p2 = points[*(vertId+2)];

		V3f normal = ( p2 - p1 ).cross( p0 - p1 );
		normal.normalize();

		if( interpolation == PrimitiveVariable::Uniform )
		{
			normals.push_back( normal );
			vertId += numVerts;
		}
		else
		{
			// accumulate the face normal onto each of the vertices for this face.
			for( int i=0; i < numVerts; ++i )
			{
				normals[*vertId] += normal;
				++vertId;
			}
		}
	}

	// normalize each of the vertex normals
	if( interpolation == PrimitiveVariable::Vertex )
	{
		for( size_t i = 0; i < normals.size(); i++ )
		{
			if( i % 1000 )
			{
				Canceller::check( canceller );
			}

			normals[i].normalize();
		}
	}

	return PrimitiveVariable( interpolation, normalsData );
}
