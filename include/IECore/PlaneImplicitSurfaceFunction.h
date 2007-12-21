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


#ifndef IE_CORE_PLANEIMPLICITSURFACEFUNCTION_H
#define IE_CORE_PLANEIMPLICITSURFACEFUNCTION_H

#include "IECore/ImplicitSurfaceFunction.h"
#include "IECore/VectorOps.h"

namespace IECore
{

/// An implicit surface describing a plane
template<typename P, typename V>
class PlaneImplicitSurfaceFunction : public ImplicitSurfaceFunction<P, V>
{
	public:
		
		typedef P Point;
		typedef VectorTraits<P> PointTraits;
		typedef typename VectorTraits<P>::BaseType PointBaseType;
		typedef V Value;
		typedef VectorTraits<V> ValueTraits;
		typedef typename VectorTraits<V>::BaseType ValueBaseType;
		
		typedef boost::intrusive_ptr<PlaneImplicitSurfaceFunction<P,V> > Ptr;
		typedef boost::intrusive_ptr<const PlaneImplicitSurfaceFunction<P,V> > ConstPtr;
	
		/// Construct an implicit plane from a normal and a distance from the origin
		PlaneImplicitSurfaceFunction( const Point &normal, PointBaseType distance ) : m_normal( normal ), m_distance( distance )
		{
			vecNormalize( m_normal );
		}
		
		/// Construct an implicit plane from a normal and a point on the plane
		PlaneImplicitSurfaceFunction( const Point &normal, const Point &origin ) : m_normal( normal )
		{
			vecNormalize( m_normal );
			
			m_distance = -vecDot( m_normal, origin );
		}				
		
		Value operator()( const Point &p )
		{
			return vecDot( m_normal, p ) + m_distance;
		}
		
		virtual Value getValue( const Point &p )
		{									
			return this->operator()(p);
		}
		
	protected:

		Point m_normal;
		PointBaseType m_distance;

		
};

typedef PlaneImplicitSurfaceFunction<Imath::V3f, float>  PlaneImplicitSurfaceFunctionV3ff;
typedef PlaneImplicitSurfaceFunction<Imath::V3f, double> PlaneImplicitSurfaceFunctionV3fd;
typedef PlaneImplicitSurfaceFunction<Imath::V3d, float>  PlaneImplicitSurfaceFunctionV3df;
typedef PlaneImplicitSurfaceFunction<Imath::V3d, double> PlaneImplicitSurfaceFunctionV3dd;

} // namespace IECore

#endif // IE_CORE_PLANEIMPLICITSURFACEFUNCTION_H
