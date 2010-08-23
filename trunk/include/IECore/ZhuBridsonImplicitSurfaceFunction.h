//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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


#ifndef IE_CORE_ZHUBRIDSONIMPLICITSURFACEFUNCTION_H
#define IE_CORE_ZHUBRIDSONIMPLICITSURFACEFUNCTION_H

#include "boost/static_assert.hpp"
#include "boost/type_traits.hpp"

#include "IECore/ImplicitSurfaceFunction.h"
#include "IECore/KDTree.h"
#include "IECore/VectorTypedData.h"
#include "IECore/VectorOps.h"

namespace IECore
{

/// An implicit function describing a "blobby" from a collection of points, and radii, as
/// described in "Animating Sand as a Fluid", Zhu & Bridson, Siggraph 2005. 
/// NB. The paper stipulates that the particle radii should be a close estimate to the distance to the surface,
/// and that a post-step may sometimes be necessary to remove any artefacts around concave areas.
template<typename P, typename V>
class ZhuBridsonImplicitSurfaceFunction : public ImplicitSurfaceFunction<P, V>
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

		IE_CORE_DECLAREMEMBERPTR2( ZhuBridsonImplicitSurfaceFunction<P, V> );

		/// Construct an implicit surface function from parallel arrays of positions, and radii
		ZhuBridsonImplicitSurfaceFunction( typename PointVectorData::ConstPtr p, ConstDoubleVectorDataPtr r, V smoothingRadius );

		virtual ~ZhuBridsonImplicitSurfaceFunction();

		/// Evaluate the function at the specified point
		inline Value operator()( const Point &p );

		/// Evaluate the function at the specified point
		virtual Value getValue( const Point &p );

	protected:

		typedef typename PointVector::const_iterator PointIterator;
		typedef KDTree< PointIterator > Tree;

		typename PointVectorData::ConstPtr m_p;
		ConstDoubleVectorDataPtr m_radius;
		V m_smoothingRadius;

		Tree *m_tree;

		inline Value k( const Value &s ) const;
};

typedef ZhuBridsonImplicitSurfaceFunction<Imath::V3f, float>  ZhuBridsonImplicitSurfaceFunctionV3ff;
typedef ZhuBridsonImplicitSurfaceFunction<Imath::V3f, double> ZhuBridsonImplicitSurfaceFunctionV3fd;
typedef ZhuBridsonImplicitSurfaceFunction<Imath::V3d, float>  ZhuBridsonImplicitSurfaceFunctionV3df;
typedef ZhuBridsonImplicitSurfaceFunction<Imath::V3d, double> ZhuBridsonImplicitSurfaceFunctionV3dd;


} // namespace IECore

#include "IECore/ZhuBridsonImplicitSurfaceFunction.inl"

#endif // IE_CORE_ZHUBRIDSONIMPLICITSURFACEFUNCTION_H
