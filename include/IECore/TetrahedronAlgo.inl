//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_TETRAHEDRONALGO_INL
#define IECORE_TETRAHEDRONALGO_INL

#include <cassert>

#include "OpenEXR/ImathLimits.h"

#include "IECore/VectorOps.h"
#include "IECore/TriangleAlgo.h"

namespace IECore
{

template<typename Vec>
typename VectorTraits<Vec>::BaseType tetrahedronVolume( 
	const Vec &v0, 
	const Vec &v1, 	
	const Vec &v2, 
	const Vec &v3
)
{
	/// From http://en.wikipedia.org/wiki/Tetrahedron
	return fabs( 
		vecDot( 
			vecSub( v0, v3 ),

			vecCross(
				vecSub( v1, v3 ),
				vecSub( v2, v3 )
			)
		)
	) / 6.0;
}

template<typename Vec>
Vec tetrahedronPoint( 
	const Vec &v0, 
	const Vec &v1, 	
	const Vec &v2, 
	const Vec &v3,
	typename VectorTraits<Vec>::BaseType barycentric[4]
)
{
	return barycentric[0] * v0 + barycentric[1] * v1 + barycentric[2] * v2 + barycentric[3] * v3;
}

template<typename Vec>
void tetrahedronBarycentric( 
	const Vec &v0, 
	const Vec &v1, 	
	const Vec &v2, 
	const Vec &v3,
	const Vec &p,
	typename VectorTraits<Vec>::BaseType barycentric[4]
)
{
	typename VectorTraits<Vec>::BaseType totalVolume = tetrahedronVolume( v0, v1, v2, v3 );
	
	barycentric[0] = tetrahedronVolume( p,  v1, v2, v3 ) / totalVolume ;
	barycentric[1] = tetrahedronVolume( v0, p,  v2, v3 ) / totalVolume ;
	barycentric[2] = tetrahedronVolume( v0, v1, p,  v3 ) / totalVolume ;
	barycentric[3] = 1.0 - barycentric[0] - barycentric[1] - barycentric[2] ;
}

template<typename Vec>
typename VectorTraits<Vec>::BaseType tetrahedronClosestBarycentric( 
	const Vec &v0, 
	const Vec &v1, 	
	const Vec &v2, 
	const Vec &v3,
	const Vec &p,
	typename VectorTraits<Vec>::BaseType barycentric[4]
)
{
	Vec centroid = ( v0 + v1 + v2 + v3 ) / 4.0;
	Vec closestPoint = p;
	typename VectorTraits<Vec>::BaseType closestDistSqrd = Imath::limits< typename VectorTraits<Vec>::BaseType >::max();
	
	Vec v[4] = { v0, v1, v2, v3 };
		
	bool inside = true;
	for ( unsigned i = 0; i < 4; i++)
	{
		/// Only need to consider facse which the point is "outside"
		Imath::V3i faceIndices = tetrahedronFaceIndices( i );
		
		/// Compute a face normal which points away from the tetrahedron's centroid
		Vec faceNormal = triangleNormal( v[faceIndices[0]], v[faceIndices[1]], v[faceIndices[2]] );		
		if ( ( v[faceIndices[0]] - centroid ).dot(faceNormal) < 0.0 )
		{
			faceNormal = -faceNormal;
		}
		
		typename VectorTraits<Vec>::BaseType planeConstant = faceNormal.dot( v[i] );
		typename VectorTraits<Vec>::BaseType planeDistance = faceNormal.dot( p ) - planeConstant;
		
		if ( planeDistance >= 0.0 )
		{
			inside = false;
			Imath::Vec3<typename VectorTraits<Vec>::BaseType> triBarycentric;
			typename VectorTraits<Vec>::BaseType triDistSqrd = triangleClosestBarycentric( v[faceIndices[0]], v[faceIndices[1]], v[faceIndices[2]], p, triBarycentric );
		
			if ( triDistSqrd < closestDistSqrd )
			{
				closestDistSqrd = triDistSqrd;
				closestPoint = trianglePoint( v[faceIndices[0]], v[faceIndices[1]], v[faceIndices[2]], triBarycentric );
			}
		}
	}
		
	tetrahedronBarycentric( v0, v1, v2, v3, closestPoint, barycentric );
	
	if ( inside )
	{
		return 0.0;
	}
	
	return closestDistSqrd;
}

inline Imath::V3i tetrahedronFaceIndices( int face )
{
	switch (face)
	{
		case 0:
			return Imath::V3i( 0, 2, 1 );
		case 1:
			return Imath::V3i( 0, 1, 3 );
		case 2:
			return Imath::V3i( 0, 3, 2 );
		case 3:
			return Imath::V3i( 1, 2, 3 );									
		default:
			assert(false);
			return Imath::V3i( 0, 0, 0 );
	}
}

} // namespace IECore

#endif // IECORE_TETRAHEDRONALGO_INL


