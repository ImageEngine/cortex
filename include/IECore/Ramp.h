//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2025, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_RAMP_H
#define IECORE_RAMP_H

#include "IECore/Export.h"
#include "IECore/Spline.h"
#include "IECore/MessageHandler.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "Imath/ImathColor.h"
IECORE_POP_DEFAULT_VISIBILITY

#include <map>

namespace IECore
{

// This lives outside the class because we don't want multiple incompatible templated versions of
// the same enum floating around
enum class RampInterpolation
{
	Linear = 0,
	CatmullRom = 1,
	BSpline = 2,
	MonotoneCubic = 3,
	Constant = 4,
};

/// A Ramp represents a spline-like curve as it is represented in a simple UI: with a set of independent
/// control points, and an interpolation type selected from RampInterpolation.
///
/// Rather than storing the lower level IECore::Spline*, we now store this Ramp type in shader networks,
/// and only convert to the lower level class with the evaluator() function when evaluation is needed.
///
/// This was chosen as superior to IECore::Spline* because IECoreS::spline* requires duplicating the
/// end points in order to make the curve reach the first and last control point.
template<typename X, typename Y>
class IECORE_EXPORT Ramp
{

	public :

		using XType = X;
		using YType = Y;

		using PointContainer = std::multimap<X, Y>;
		using Point = typename PointContainer::value_type;

		Ramp() : interpolation( RampInterpolation::CatmullRom )
		{
		}

		Ramp( const PointContainer &p, RampInterpolation i )
			: points( p ), interpolation( i )
		{
		}

		PointContainer points;
		RampInterpolation interpolation;


		// Convert to Cortex Spline
		// In the future, IECore::Spline may be replaced with IECore::SplineEvaluator, and this
		// function would be the only way to setup one.
		IECore::Spline<X, Y> evaluator() const;

		// Convert to and from a set of arguments that could be passed to a pair of spline() and
		// splineinverse() functions in OSL. This can be useful in converting ramps to parameters
		// for OSL shaders.
		//
		// Some shader libraries use these arguments directly as shader parameters ( i.e. Gaffer ).
		// Some shader libraries preprocess shader parameters before passing them to spline(),
		// so they don't need some aspects of this conversion ( like endpoint duplication ), but
		// the extra endpoint duplication doesn't cause problems ( i.e. PRMan ).
		// Some shader libraries are doing their own thing, implementing their own custom math,
		// but convention is still similar enough that these function can be a useful building
		// block in converting to something that mostly works ( i.e. 3delight ).
		void fromOSL( const std::string &basis, const std::vector<X> &positions, const std::vector<Y> &values, const std::string &identifier );
		void toOSL( std::string &basis, std::vector<X> &positions, std::vector<Y> &values ) const;

		/// The number of times `toOSL()` repeats the initial point.
		int oslStartPointMultiplicity() const;

		// In Cortex 10.6 and earlier, shader parameters were represented uing IECore::Spline*Data instead of
		// IECore::Ramp*Data. This is used in converting SCC files to the new standard.
		// \todo : This can probably be removed in the next major version - we're not actually aware of any
		// significant users of Cortex who both use SCC files, and cache shaders, so this compatibility shim
		// is only needed theoretically.
		void fromDeprecatedSpline( const IECore::Spline<X, Y> &deprecated );

		bool operator==( const Ramp<X,Y> &rhs ) const;
		bool operator!=( const Ramp<X,Y> &rhs ) const;

};

using Rampff = Ramp<float, float>;
using RampfColor3f = Ramp<float, Imath::Color3f>;
using RampfColor4f = Ramp<float, Imath::Color4f>;

template<typename X, typename Y>
inline void murmurHashAppend( IECore::MurmurHash &h, const Ramp<X,Y> &data )
{
	h.append( data.interpolation );
	for ( auto &p : data.points )
	{
		h.append( p.first );
		h.append( p.second );
	}
}

} // namespace IECore

#endif // IECORE_RAMP_H
