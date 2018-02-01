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

#include "IECore/PolygonAlgo.h"

#include "boost/format.hpp"
#include "boost/iterator/transform_iterator.hpp"
#include "boost/iterator/zip_iterator.hpp"
#include "boost/tuple/tuple.hpp"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Calculate Face Area
//////////////////////////////////////////////////////////////////////////

namespace
{

struct V2fToV3f
{
	typedef V3f result_type;

	V3f operator()( const V2f &vec2 ) const
	{
		return V3f( vec2[0], vec2[1], 0.0f );
	}
};

} // namespace

PrimitiveVariable MeshAlgo::calculateFaceArea( const MeshPrimitive *mesh, const std::string &position )
{
	const V3fVectorData *pData = mesh->variableData<V3fVectorData>( position, PrimitiveVariable::Vertex );
	if( !pData )
	{
		throw InvalidArgumentException( boost::str( boost::format( "MeshAlgo::calculateFaceArea : MeshPrimitive has no \"%s\" primitive variable." ) % position ) );
	}
	const std::vector<V3f> &p = pData->readable();

	FloatVectorDataPtr areasData = new FloatVectorData;
	std::vector<float> &areas = areasData->writable();
	areas.reserve( mesh->variableSize( PrimitiveVariable::Uniform ) );

	/// \todo: can MeshPrimitive::faceEnd() be const?
	PolygonIterator faceEnd = const_cast<MeshPrimitive*>( mesh )->faceEnd();
	for( PolygonIterator pIt = const_cast<MeshPrimitive*>( mesh )->faceBegin(); pIt != faceEnd; pIt++ )
	{
		areas.push_back( polygonArea( pIt.vertexBegin( p.begin() ), pIt.vertexEnd( p.begin() ) ) );
	}

	return PrimitiveVariable( PrimitiveVariable::Uniform, areasData );
}

PrimitiveVariable MeshAlgo::calculateFaceTextureArea( const MeshPrimitive *mesh, const std::string &uvSet, const std::string &position )
{
	PrimitiveVariable::Interpolation uvInterpolation = PrimitiveVariable::Vertex;
	ConstV2fVectorDataPtr uvData = mesh->expandedVariableData<V2fVectorData>( uvSet, PrimitiveVariable::Vertex );
	if( !uvData )
	{
		uvData = mesh->expandedVariableData<V2fVectorData>( uvSet, PrimitiveVariable::FaceVarying );
		if( !uvData )
		{
			throw InvalidArgumentException( boost::str( boost::format( "MeshAlgo::calculateFaceTextureArea : MeshPrimitive has no suitable \"%s\" primitive variable." ) % uvSet ) );
		}
		uvInterpolation = PrimitiveVariable::FaceVarying;
	}
	const std::vector<Imath::V2f> &uvs = uvData->readable();

	FloatVectorDataPtr textureAreasData = new FloatVectorData;
	std::vector<float> &textureAreas = textureAreasData->writable();
	textureAreas.reserve( mesh->variableSize( PrimitiveVariable::Uniform ) );

	/// \todo: can MeshPrimitive::faceEnd() be const?
	PolygonIterator faceEnd = const_cast<MeshPrimitive*>( mesh )->faceEnd();
	for( PolygonIterator pIt = const_cast<MeshPrimitive*>( mesh )->faceBegin(); pIt!=faceEnd; pIt++ )
	{
		if( uvInterpolation==PrimitiveVariable::Vertex )
		{
			typedef PolygonVertexIterator<std::vector<Imath::V2f>::const_iterator> VertexIterator;
			typedef boost::transform_iterator<V2fToV3f, VertexIterator> STIterator;

			STIterator begin( pIt.vertexBegin( uvs.begin() ) );
			STIterator end( pIt.vertexEnd( uvs.begin() ) );

			textureAreas.push_back( polygonArea( begin, end ) );
		}
		else
		{
			assert( uvInterpolation==PrimitiveVariable::FaceVarying );
			typedef boost::transform_iterator<V2fToV3f, std::vector<Imath::V2f>::const_iterator> STIterator;

			STIterator begin( pIt.faceVaryingBegin( uvs.begin() ) );
			STIterator end( pIt.faceVaryingEnd( uvs.begin() ) );

			textureAreas.push_back( polygonArea( begin, end ) );
		}
	}

	return PrimitiveVariable( PrimitiveVariable::Uniform, textureAreasData );
}
