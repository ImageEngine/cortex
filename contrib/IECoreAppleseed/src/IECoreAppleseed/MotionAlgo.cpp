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

#include "IECoreAppleseed/MotionAlgo.h"

#include "IECore/Exception.h"
#include "IECore/ObjectInterpolator.h"

#include "foundation/math/scalar.h"

#include <algorithm>
#include <vector>

using namespace IECore;

namespace asf = foundation;

namespace IECoreAppleseed
{

namespace MotionAlgo
{

void resamplePrimitiveKeys( const std::vector<const IECore::Object *> &samples, const std::vector<float> &times, float shutterOpenTime, float shutterCloseTime, std::vector<IECore::ObjectPtr> &resampledSamples )
{
	resampledSamples.clear();

	int numSamples = asf::is_pow2( times.size() ) ? times.size() : asf::next_pow2( times.size() );
	assert( std::is_sorted( times.begin(), times.end() ) );

	for( int i = 0; i < numSamples; ++i)
	{
		const float time = static_cast<float>( i ) / ( numSamples - 1 ) * ( shutterCloseTime - shutterOpenTime ) + shutterOpenTime;

		if( time <= times.front() )
		{
			IECore::ObjectPtr o = samples.front()->copy();
			resampledSamples.push_back( o );
			continue;
		}

		if( time >= times.back() )
		{
			IECore::ObjectPtr o = samples.back()->copy();
			resampledSamples.push_back( o );
			continue;
		}

		std::vector<float>::const_iterator it = std::lower_bound( times.begin(), times.end(), time );
		int index = it - times.begin() - 1;

		float t = ( time - times[index] ) / ( times[index + 1] - times[index] );
		ObjectPtr obj = linearObjectInterpolation( samples[index], samples[index + 1], t );
		resampledSamples.push_back( obj );
	}
}

} // namespace MotionAlgo

} // namespace IECoreAppleseed
