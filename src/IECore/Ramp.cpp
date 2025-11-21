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


#include "IECore/Ramp.h"
#include "IECore/CubicBasis.h"

using namespace IECore;

namespace
{

template<typename X, typename Y>
inline Y monotoneSlopeCompute( const Y& deltaY1, const Y& deltaY2, const X& deltaX1, const X& deltaX2 )
{
    // Using this weighted harmonic mean to compute slopes ensures a monotone curve.
    // This is apparently a result by Fritsch and Carlson, from here:
    //
    // F. N. Fritsch and R. E. Carlson
    // SIAM Journal on Numerical Analysis
    // Vol. 17, No. 2 (Apr., 1980), pp. 238-246
    //
    // Haven't actually gotten ahold of this paper, but stackexchange says that it says this,
    // and it seems to work quite well.
    if( deltaY1 * deltaY2 > 0.0f )
    {
        return 3.0f * ( deltaX1 + deltaX2 ) / (
            ( 2.0f * deltaX2 + deltaX1 ) / deltaY1 +
            ( deltaX2 + 2.0f * deltaX1 ) / deltaY2 );
    }
    else
    {
        return 0;
    }
}

template<>
inline Imath::Color3f monotoneSlopeCompute<>(
    const Imath::Color3f& deltaY1, const Imath::Color3f& deltaY2,
    const float& deltaX1, const float& deltaX2 )
{
    return Imath::Color3f(
        monotoneSlopeCompute( deltaY1[0], deltaY2[0], deltaX1, deltaX2 ),
        monotoneSlopeCompute( deltaY1[1], deltaY2[1], deltaX1, deltaX2 ),
        monotoneSlopeCompute( deltaY1[2], deltaY2[2], deltaX1, deltaX2 )
    );
}

template<>
inline Imath::Color4f monotoneSlopeCompute<>(
    const Imath::Color4f& deltaY1, const Imath::Color4f& deltaY2,
    const float& deltaX1, const float& deltaX2 )
{
    return Imath::Color4f(
        monotoneSlopeCompute( deltaY1[0], deltaY2[0], deltaX1, deltaX2 ),
        monotoneSlopeCompute( deltaY1[1], deltaY2[1], deltaX1, deltaX2 ),
        monotoneSlopeCompute( deltaY1[2], deltaY2[2], deltaX1, deltaX2 ),
        monotoneSlopeCompute( deltaY1[3], deltaY2[3], deltaX1, deltaX2 )
    );
}

// This function translates a set of control points for a MonotoneCubic curve into a set of
// bezier control points.  The X values are set up to make each segment linear in X, which
// makes the control point behaviour a bit more predictable when used as a color ramp.
// The Y tangents are adjusted to compensate for the discontinuity in the slope of the
// parameterization across control points, which mean that the X/Y tangent is continuous across
// control points.  This curve type seems to work quite well in practice for color ramps.
//
// Note that the way we are evaluating this type of curve by using a spline solver on the X
// axis is ridiculously inefficient - the bezier control points are arranged so it's actually
// always linear, and could be quickly solved analyticly.  But because we need to store these
// curves in IECore::Spline when evaluating, we don't have any way to specify different
// interpolations for X and Y. So we just use bezier curve with appropriately set handles to
// store our linear X curve.
template<typename T>
void monotoneCubicCVsToBezierCurve( const typename T::PointContainer &cvs, typename T::PointContainer &result )
{
	if( cvs.size() < 2 )
	{
		result = cvs;
		return;
	}

	result.clear();

	// NOTE : It would seem more reasonable to use the slope of the first and last segment for the
	// endpoints, instead of clamping to 0.  The current argument for clamping to zero is consistency
	// with the Htoa ramp
	typename T::YType prevSlope(0);

	typename T::PointContainer::const_iterator i = cvs.begin();
	const typename T::Point *p1 = &*i;
	i++;
	const typename T::Point *p2 = &*i;
	i++;

	for(;;)
	{
		typename T::YType nextSlope;


		const typename T::Point *pNext = nullptr;
		if( i == cvs.end() )
		{
			nextSlope = typename T::YType( 0 );
		}
		else
		{
			pNext = &*i;

			typename T::XType xDelta1 = p2->first - p1->first;
			typename T::XType xDelta2 = pNext->first - p2->first;
			typename T::YType yDelta1 = p2->second - p1->second;
			typename T::YType yDelta2 = pNext->second - p2->second;

			nextSlope = monotoneSlopeCompute( yDelta1 / xDelta1, yDelta2 / xDelta2, xDelta1, xDelta2);

			// NOTE : If we copied everything else about this function, but instead just used:
			//     0.5 * ( yDelta1 / xDelta1 + yDelta2 / xDelta2 )
			// for the slope here, this would produce a CatmullRom sort of spline, but with the simpler linear
			// behaviour of the knot values.  This is what Htoa uses for a CatmullRom ramp, and might be a pretty
			// useful curve type ( quite possibly more useful than our current CatmullRom, though it would no
			// longer correspond to a CatmullRom basis in OSL ).
		}

		typename T::XType xDelta = p2->first - p1->first;

		result.insert( *p1 );
		result.insert( typename T::Point(
			p1->first + ( 1.0f/3.0f ) * xDelta,
			p1->second + (1.0f/3.0f) * prevSlope * xDelta ) );
		result.insert( typename T::Point(
			p1->first + ( 2.0f/3.0f ) * xDelta,
			p2->second - (1.0f/3.0f) * nextSlope * xDelta ) );

		if( i == cvs.end() )
		{
			break;
		}
		else
		{
			p1 = p2;
			p2 = pNext;
			prevSlope = nextSlope;
			i++;
		}
	}

	result.insert( *p2 );
}

template<typename T>
const typename T::PointContainer *convertMonotoneCubic( const T &source, typename T::PointContainer &storage)
{
	if( source.interpolation == RampInterpolation::MonotoneCubic )
	{
		if( source.points.size() > 1 )
		{
		}
		else
		{
			storage = source.points;
		}
		return &storage;
	}
	else
	{
		return &source.points;
	}
}

int endPointMultiplicity( IECore::RampInterpolation interpolation )
{
	int multiplicity = 1;
	if( interpolation == IECore::RampInterpolation::CatmullRom )
	{
		multiplicity = 2;
	}
	else if( interpolation == IECore::RampInterpolation::BSpline )
	{
		multiplicity = 3;
	}
	return multiplicity;
}

std::pair< size_t, size_t > getOSLEndPointDuplication( IECore::RampInterpolation interpolation )
{
	if( interpolation == IECore::RampInterpolation::CatmullRom )
	{
        return std::make_pair( 1, 1 );
	}
	else if( interpolation == IECore::RampInterpolation::BSpline )
	{
        return std::make_pair( 2, 2 );
	}
    else if( interpolation == IECore::RampInterpolation::Linear )
    {
        // OSL discards the first and last segment of linear curves
        // "To maintain consistency with the other spline types"
        // so we need to duplicate the end points to preserve all provided segments
        return std::make_pair( 1, 1 );
    }
    else if( interpolation == IECore::RampInterpolation::Constant )
    {
        // Also, "To maintain consistency", "constant splines ignore the first and the two last
        // data values."
        return std::make_pair( 1, 2 );
    }


    return std::make_pair( 0, 0 );
}

// Removes start and end points with duplicated X values.
//
// Sources of spline data may or may not contain duplicated end points for a variety of reasons
// ( such as IECore::Splineff having duplicated end points so that spline evaluation will reach
// the final value, or OSL have duplicated end points even for constant and linear splines ).
//
// The spline UI's that Gaffer uses to interact with splines don't support duplicated end
// points well, so regardless of why they are there, this function will remove them.
template<typename PointContainer>
void trimEndPoints( PointContainer &points )
{
	if( points.empty() )
	{
		return;
	}

	// Count how many initial points have an X value matching the first point
	typename PointContainer::const_iterator startDuplicates = ++points.begin();
	while( startDuplicates != points.end() && startDuplicates->first == points.begin()->first )
	{
		startDuplicates++;
	}
	// We need to keep one of these points, the rest are duplicates and should be removed
	startDuplicates--;

	points.erase( points.begin(), startDuplicates );

	// Count how many points have an X value matching the last point
	typename PointContainer::const_reverse_iterator endDuplicates = ++points.rbegin();
	while( endDuplicates != points.rend() && endDuplicates->first == points.rbegin()->first )
	{
		endDuplicates++;
	}

	// We need to keep one of these points, the rest are duplicates and should be removed
	endDuplicates--;

	points.erase( endDuplicates.base(), points.rbegin().base() );

	// This originally indicated whether the end point duplication matched the expected multiplicity
	// for a certain interpolation. We no longer make assumptions about how the input data handles end
	// point multiplicity, and instead just remove any duplicates, so we no longer consider any inputs
	// to be invalid.
	return;
}

}

namespace IECore
{

template<typename X, typename Y>
IECore::Spline<X,Y> Ramp<X,Y>::evaluator() const
{
	using ResultType = IECore::Spline<X,Y>;
	ResultType result;

	result.points = points;

	if( interpolation == RampInterpolation::Linear )
	{
		result.basis = ResultType::Basis::linear();
	}
	else if( interpolation == RampInterpolation::CatmullRom )
	{
		result.basis = ResultType::Basis::catmullRom();
	}
	else if( interpolation == RampInterpolation::BSpline )
	{
		result.basis = ResultType::Basis::bSpline();
	}
	else if( interpolation == RampInterpolation::MonotoneCubic )
	{
		monotoneCubicCVsToBezierCurve< Ramp<X,Y> >( points, result.points );
		result.basis = ResultType::Basis::bezier();
	}
	else if( interpolation == RampInterpolation::Constant )
	{
		result.basis = ResultType::Basis::constant();
	}

	int multiplicity = endPointMultiplicity( interpolation );

	if( multiplicity && result.points.size() )
	{
		for( int i = 0; i < multiplicity - 1; ++i )
		{
			result.points.insert( *result.points.begin() );
			result.points.insert( *result.points.rbegin() );
		}
	}

	return result;
}

template<typename X, typename Y>
void Ramp<X,Y>::fromOSL( const std::string &basis, const std::vector<X> &positions, const std::vector<Y> &values, const std::string &identifier )
{
	points.clear();
	size_t n = std::min( positions.size(), values.size() );

	if( basis == "bezier" )
	{
		for( size_t i = 0; i < n; i += 3 )
		{
			points.insert( Point( positions[i], values[i] ) );
		}

        interpolation = RampInterpolation::MonotoneCubic;

		std::vector<X> testPositions;
		std::vector<Y> testValues;
		std::string testBasis;
		toOSL( testBasis, testPositions, testValues );
		if( !( testPositions == positions && testValues == values ) )
		{
			IECore::msg( IECore::MessageHandler::Warning, "Ramp", "While loading shader parameter " + identifier + " found bezier curve that cannot be represented in Gaffer. Using most similar MonotoneCubic curve instead." );
		}
		return;
	}
	else
	{
		for( size_t i = 0; i < n; ++i )
		{
			points.insert( Point( positions[i], values[i] ) );
		}
	}

    if( basis == "bspline" )
    {
        interpolation = RampInterpolation::BSpline;
    }
    else if( basis == "linear" )
    {
        interpolation = RampInterpolation::Linear;
    }
    else if( basis == "constant" )
    {
        interpolation = RampInterpolation::Constant;
    }
	else if( basis == "monotonecubic" )
	{
		// 3delight actually supports monotonecubic, so it's possible we could see an input
		// directly using this interpolation.
		interpolation = RampInterpolation::MonotoneCubic;
	}
    else
    {
        interpolation = RampInterpolation::CatmullRom;
    }

	trimEndPoints( points );

}

template<typename X, typename Y>
void Ramp<X,Y>::toOSL( std::string &basis, std::vector<X> &positions, std::vector<Y> &values ) const
{
	PointContainer storage;
	const PointContainer *currentPoints = &points;

    if( interpolation == RampInterpolation::MonotoneCubic )
    {
		monotoneCubicCVsToBezierCurve< Ramp<X,Y> >( points, storage );
		currentPoints = &storage;
        basis = "bezier";
    }
    else if( interpolation == RampInterpolation::BSpline )
    {
        basis = "bspline";
    }
    else if( interpolation == RampInterpolation::Linear )
    {
        basis = "linear";
    }
    else if( interpolation == RampInterpolation::Constant )
    {
        // Also, "To maintain consistency", "constant splines ignore the first and the two last
        // data values."
        basis = "constant";
    }
	else
	{
		basis = "catmull-rom";
	}
    auto [ duplicateStartPoints, duplicateEndPoints ] = getOSLEndPointDuplication( interpolation );

    positions.reserve( currentPoints->size() );
    values.reserve( currentPoints->size() + duplicateStartPoints + duplicateEndPoints );

    if( currentPoints->size() )
    {
        for( size_t i = 0; i < duplicateStartPoints; i++ )
        {
            positions.push_back( currentPoints->begin()->first );
            values.push_back( currentPoints->begin()->second );
        }
    }
    for( auto &it : *currentPoints )
    {
        positions.push_back( it.first );
        values.push_back( it.second );
    }
    if( currentPoints->size() )
    {
        for( size_t i = 0; i < duplicateEndPoints; i++ )
        {
            positions.push_back( currentPoints->rbegin()->first );
            values.push_back( currentPoints->rbegin()->second );
        }
    }
}

template<typename X, typename Y>
int Ramp<X,Y>::oslStartPointMultiplicity() const
{
	if( interpolation == RampInterpolation::MonotoneCubic )
	{
		// I guess the only way to handle this properly would be to do the end point duplication
		// and conversion from monotone to bezier in our OSL shaders ( like the PRMan and 3delight
		// shader libraries do ). Except that the PRMan approach has a potentially significant
		// downside that if the inputs are varying, the array conversion code can't be constant
		// folded, and you would end up doing the array processing per-shading point. 3delight
		// appears to have implemented their own spline handling, which may not have this problem,
		// but we don't want to do this ourselves. For now, not handling inputs to monotoneCubic
		// ramps seems pretty reasonable.
		throw IECore::Exception( "Cannot connect adaptors to ramp when using monotoneCubic interpolation" );
	}

	// The "multiplicity" is one greater than the number of duplicates: by default, without duplication,
	// every control point has a multiplicity of 1, and after 1 duplicate, there is a multiplicity of 2.
	return getOSLEndPointDuplication( interpolation ).first + 1;
}

template<typename X, typename Y>
void Ramp<X,Y>::fromDeprecatedSpline( const IECore::Spline<X, Y> &deprecated )
{
	// This is sort of similar to fromOSL, except the source is an old Cortex Spline instead of
	// separate position and value vectors. In theory, there might be some way to share a bit more
	// code here, but it's probably not worth refactoring for the sake of this method, which it
	// should be possible to remove in the future ( once we no longer need to support old SCC files )

	if( deprecated.basis == CubicBasis<X>::bezier() )
	{
		int count = 0;
		for( const auto &i : deprecated.points )
		{
			if( ( count % 3 ) == 0 )
			{
				points.insert( i );
			}
			count++;
		}

        interpolation = RampInterpolation::MonotoneCubic;

		// Unlike fromOSL, we don't check whether monotoneCubic actually matches the original bezier
		// ... if the source data is from an scc produced by old Gaffer, then it should have come from
		// a monotoneCubic.

		return;
	}
	else
	{
		points = deprecated.points;
	}

    if( deprecated.basis == CubicBasis<X>::bSpline() )
    {
        interpolation = RampInterpolation::BSpline;
    }
    else if( deprecated.basis == CubicBasis<X>::linear() )
    {
        interpolation = RampInterpolation::Linear;
    }
    else if( deprecated.basis == CubicBasis<X>::constant() )
    {
        interpolation = RampInterpolation::Constant;
    }
    else
    {
        interpolation = RampInterpolation::CatmullRom;
    }

	trimEndPoints( points );

}

template<typename X, typename Y>
inline bool Ramp<X,Y>::operator==( const Ramp<X,Y> &rhs ) const
{
	return interpolation == rhs.interpolation && points == rhs.points;
}

template<typename X, typename Y>
inline bool Ramp<X,Y>::operator!=( const Ramp<X,Y> &rhs ) const
{
	return !operator==( rhs );
}

// explicit instantiation
template class Ramp<float, float>;
template class Ramp<float, Imath::Color3f>;
template class Ramp<float, Imath::Color4f>;

} // namespace IECore
