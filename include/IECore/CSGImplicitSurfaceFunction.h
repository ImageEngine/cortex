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


#ifndef IE_CORE_CSGIMPLICITSURFACEFUNCTION_H
#define IE_CORE_CSGIMPLICITSURFACEFUNCTION_H

#include "IECore/ImplicitSurfaceFunction.h"

namespace IECore
{

/// An implicit surface function for performing CSG operations on other implicit surfaces: union, intersection, and difference.
template<typename P, typename V>
class CSGImplicitSurfaceFunction : public ImplicitSurfaceFunction<P, V>
{
	public:
		
		typedef P Point;
		typedef VectorTraits<P> PointTraits;
		typedef typename VectorTraits<P>::BaseType PointBaseType;
		typedef V Value;
		typedef VectorTraits<V> ValueTraits;
		typedef typename VectorTraits<V>::BaseType ValueBaseType;
		
		typedef ImplicitSurfaceFunction<P, V> Fn;
		
		IE_CORE_DECLAREMEMBERPTR2( CSGImplicitSurfaceFunction<P, V> );
		
		typedef enum
		{
			Union,
			Intersection,
			Difference
		} Mode;
	
		/// Construct a new implicit surface from the two given functions and the specified CSG operation
		CSGImplicitSurfaceFunction( typename Fn::Ptr fn1, typename Fn::Ptr fn2, Mode mode );
		
		Value operator()( const Point &p );
		
		virtual Value getValue( const Point &p );
		
	protected:

		typename Fn::Ptr m_fn1, m_fn2;
		Mode m_mode;
		
};

typedef CSGImplicitSurfaceFunction<Imath::V3f, float>  CSGImplicitSurfaceFunctionV3ff;
typedef CSGImplicitSurfaceFunction<Imath::V3f, double> CSGImplicitSurfaceFunctionV3fd;
typedef CSGImplicitSurfaceFunction<Imath::V3d, float>  CSGImplicitSurfaceFunctionV3df;
typedef CSGImplicitSurfaceFunction<Imath::V3d, double> CSGImplicitSurfaceFunctionV3dd;

} // namespace IECore

#include "IECore/CSGImplicitSurfaceFunction.inl"

#endif // IE_CORE_CSGIMPLICITSURFACEFUNCTION_H
