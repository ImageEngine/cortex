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

#ifndef IECORE_TETRAHEDRONALGO_H
#define IECORE_TETRAHEDRONALGO_H

#include "IECore/VectorTraits.h"

namespace IECore
{

/// Returns the volume of the tetrahedron defined by the 4 specified vertices
template<typename Vec>
typename VectorTraits<Vec>::BaseType tetrahedronVolume( 
	const Vec &v0, 
	const Vec &v1, 	
	const Vec &v2, 
	const Vec &v3
);

/// Returns the point of the tetrahedron which has the given barycentric coordinates
template<typename Vec>
Vec tetrahedronPoint( 
	const Vec &v0, 
	const Vec &v1, 	
	const Vec &v2, 
	const Vec &v3,
	typename VectorTraits<Vec>::BaseType barycentric[4]
);

/// Returns the barycentric coordinates of the given point relative to the tetrahedron. The point is
/// assumed to be inside the tetrahedron.
template<typename Vec>
void tetrahedronBarycentric( 
	const Vec &v0, 
	const Vec &v1, 	
	const Vec &v2, 
	const Vec &v3,
	const Vec &p,
	typename VectorTraits<Vec>::BaseType barycentric[4]
);

/// Returns the squared-distance to the closest point on the tetrahedron, and computes that point's barycentric coordinates
template<typename Vec>
typename VectorTraits<Vec>::BaseType tetrahedronClosestBarycentric( 
	const Vec &v0, 
	const Vec &v1, 	
	const Vec &v2, 
	const Vec &v3,
	const Vec &p,
	typename VectorTraits<Vec>::BaseType barycentric[4]
);

/// A tetrahedron has 4 faces, each of which is triangle. This function returns the vertex indices which make up
/// the triangle on the specified face.
inline Imath::V3i tetrahedronFaceIndices( int face );

} // namespace IECore

#include "IECore/TetrahedronAlgo.inl"

#endif // IECORE_TETRAHEDRONALGO_H
