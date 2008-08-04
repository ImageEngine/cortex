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

#ifndef IECORE_SPLINE_H
#define IECORE_SPLINE_H

#include "IECore/CubicBasis.h"

#include "OpenEXR/ImathColor.h"

#include <map>

namespace IECore
{

/// A Spline class suitable for things like creating ramps of colour through
/// a series of control points or for creating simple animation curves.
template<typename X, typename Y>
class Spline
{
	
	public :
	
		typedef X XType;
		typedef Y YType;
	
		typedef CubicBasis<XType> Basis;
		typedef std::multimap<X, Y> PointContainer;
		
		/// The Spline is defined by a basis and a mapping from
		/// X to Y values defining control points for the spline.
		/// Both are public so they may be manipulated freely.
		Basis basis;
		PointContainer points;

		Spline( const Basis &basis=Basis::catmullRom() );
		Spline( const Basis &basis, const PointContainer &points );
		
		/// Find the appropriate segment and parametric position to determine
		/// the y value for a given x value. The parametric position is returned
		/// and segment is set to point to the first point in the segment. This
		/// information can then be used along with the basis matrix to calculate
		/// the y value.
		/// \todo I'm not convinced this is terribly stable when x values get close
		/// together.
		inline X solve( X x, typename PointContainer::const_iterator &segment ) const;
		/// As above but fills the points array with the points for the segment.
		inline X solve( X x, Y segment[4] ) const;
		
		/// Uses solve() to evaluate the y value for a given x position.
		inline Y operator() ( X x ) const;
		
		inline bool operator==( const Spline &rhs ) const;
		inline bool operator!=( const Spline &rhs ) const;
		
};

typedef Spline<float, float> Splineff;
typedef Spline<double, double> Splinedd;

typedef Spline<float, Imath::Color3f> SplinefColor3f;
typedef Spline<float, Imath::Color4f> SplinefColor4f;

} // namespace IECore

#include "IECore/Spline.inl"

#endif // IECORE_SPLINE_H
