//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_MOTIONALGO_H
#define IECOREAPPLESEED_MOTIONALGO_H

#include "IECoreAppleseed/Export.h"

#include "IECore/Object.h"

#include "foundation/math/scalar.h"

namespace IECoreAppleseed
{

namespace MotionAlgo
{

// appleseed requires a power of two number of primitive deformation samples,
// equally spaced between shutter open / close times.
// check the time samples and return true if they satisfy the conditions.
template< typename TimeContainer >
bool checkTimeSamples( const TimeContainer &times, float shutterOpenTime, float shutterCloseTime )
{
	// check that the number of samples is a power of 2.
	if( !foundation::is_pow2( times.size() ) )
	{
		return false;
	}

	const float eps = 0.01f;

	// check that the first and last samples match the shutter times.
	if( !foundation::feq( shutterOpenTime, *times.begin(), eps ) )
	{
		return false;
	}

	if( !foundation::feq( shutterCloseTime, *times.rbegin(), eps ) )
	{
		return false;
	}

	// check that the samples are equally spaced.
	typename TimeContainer::const_iterator next( times.begin() );
	typename TimeContainer::const_iterator it( next++ );

	float sampleInterval = *next - *it;

	for( typename TimeContainer::const_iterator e( times.end() ) ; next != e; ++next )
	{
		if( !foundation::feq( sampleInterval, *next - *it, eps ) )
		{
			return false;
		}

		it = next;
	}

	return true;
}

// Resample a set of primitive keys so that number of samples
// is a power of 2, equally spaced between the shutter open and close times.
IECOREAPPLESEED_API void resamplePrimitiveKeys( const std::vector<const IECore::Object *> &samples, const std::vector<float> &times, float shutterOpenTime, float shutterCloseTime, std::vector<IECore::ObjectPtr> &resampledKeys );

} // namespace MotionAlgo

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_MOTIONALGO_H
