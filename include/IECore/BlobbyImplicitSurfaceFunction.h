//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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


#ifndef IE_CORE_BLOBBYIMPLICITSURFACEFUNCTION_H
#define IE_CORE_BLOBBYIMPLICITSURFACEFUNCTION_H

#include "IECore/ImplicitSurfaceFunction.h"
#include "IECore/BoundedKDTree.h"
#include "IECore/VectorTypedData.h"
#include "IECore/VectorOps.h"

namespace IECore
{

/// An implicit function describing a "blobby" from a collection of points, radii, and strengths
template<typename P, typename V>
class BlobbyImplicitSurfaceFunction : public ImplicitSurfaceFunction<P, V>
{
	public:
		typedef P Point;
		typedef VectorTraits<P> PointTraits;
		typedef typename VectorTraits<P>::BaseType PointBaseType;
		typedef V Value;
		typedef VectorTraits<V> ValueTraits;
		typedef typename VectorTraits<V>::BaseType ValueBaseType;
		
		typedef std::vector<P> PointVector;
		typedef TypedData<PointVector> PointVectorData;
		
		typedef Imath::Box<P> Bound;
		
		IE_CORE_DECLAREMEMBERPTR2( BlobbyImplicitSurfaceFunction<P, V> );
		
		/// Construct an implicit surface function from parallel arrays of positions, radii, and strengths
		BlobbyImplicitSurfaceFunction( typename PointVectorData::ConstPtr p, ConstDoubleVectorDataPtr r, ConstDoubleVectorDataPtr s );
		
		virtual ~BlobbyImplicitSurfaceFunction();

		/// Evaluate the function at the specified point
		inline Value operator()( const Point &p );

		/// Evaluate the function at the specified point		
		virtual Value getValue( const Point &p );
		
	protected:

		typedef std::vector< Bound > BoundVector;
		typedef typename BoundVector::const_iterator BoundVectorConstIterator;
		typedef BoundedKDTree< BoundVectorConstIterator > Tree;
		
		typename PointVectorData::ConstPtr m_p;		
		ConstDoubleVectorDataPtr m_radius;		
		ConstDoubleVectorDataPtr m_strength;

		BoundVector m_bounds;
		Tree *m_tree;
};

typedef BlobbyImplicitSurfaceFunction<Imath::V3f, float>  BlobbyImplicitSurfaceFunctionV3ff;
typedef BlobbyImplicitSurfaceFunction<Imath::V3f, double> BlobbyImplicitSurfaceFunctionV3fd;
typedef BlobbyImplicitSurfaceFunction<Imath::V3d, float>  BlobbyImplicitSurfaceFunctionV3df;
typedef BlobbyImplicitSurfaceFunction<Imath::V3d, double> BlobbyImplicitSurfaceFunctionV3dd;

#include "IECore/BlobbyImplicitSurfaceFunction.inl"

} // namespace IECore

#endif // IE_CORE_BLOBBYIMPLICITSURFACEFUNCTION_H
