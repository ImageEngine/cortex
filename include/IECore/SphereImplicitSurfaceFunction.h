//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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


#ifndef IE_CORE_SPHEREIMPLICITSURFACEFUNCTION_H
#define IE_CORE_SPHEREIMPLICITSURFACEFUNCTION_H

#include "IECore/ImplicitSurfaceFunction.h"
#include "IECore/VectorOps.h"

namespace IECore
{

/// An implicit surface describing a sphere
template<typename P, typename V>
class SphereImplicitSurfaceFunction : public ImplicitSurfaceFunction<P, V>
{
	public:
		
		typedef P Point;
		typedef VectorTraits<P> PointTraits;
		typedef typename VectorTraits<P>::BaseType PointBaseType;
		typedef V Value;
		typedef VectorTraits<V> ValueTraits;
		typedef typename VectorTraits<V>::BaseType ValueBaseType;
		
		typedef boost::intrusive_ptr<SphereImplicitSurfaceFunction<P,V> > Ptr;
		typedef boost::intrusive_ptr<const SphereImplicitSurfaceFunction<P,V> > ConstPtr;
	
		SphereImplicitSurfaceFunction( const Point &center, PointBaseType radius ) : m_center( center ), m_radius( radius )
		{
		}
		
		Value operator()( const Point &p )
		{
			Point separation;
			vecSub( p, m_center, separation );
			
			return -(1.0 - ( vecDistance( p, m_center ) / m_radius ));
		}
		
		virtual Value getValue( const Point &p )
		{									
			return this->operator()(p);
		}
		
	protected:

		Point m_center;
		PointBaseType m_radius;
		
};

typedef SphereImplicitSurfaceFunction<Imath::V3f, float>  SphereImplicitSurfaceFunctionV3ff;
typedef SphereImplicitSurfaceFunction<Imath::V3f, double> SphereImplicitSurfaceFunctionV3fd;
typedef SphereImplicitSurfaceFunction<Imath::V3d, float>  SphereImplicitSurfaceFunctionV3df;
typedef SphereImplicitSurfaceFunction<Imath::V3d, double> SphereImplicitSurfaceFunctionV3dd;

} // namespace IECore

#endif // IE_CORE_SPHEREIMPLICITSURFACEFUNCTION_H
