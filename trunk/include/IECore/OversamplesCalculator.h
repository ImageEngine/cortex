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

#ifndef IE_CORE_OVERSAMPLESCALCULATOR_H
#define IE_CORE_OVERSAMPLESCALCULATOR_H

namespace IECore
{

/// A templated class that computes oversampling rate based on the time unit U (measured in FPS) and related parameters.
template<int U>
class OversamplesCalculator
{
	public:
	OversamplesCalculator( double frameRate, int desiredOversamples );
	/// converts your continuous frame number ( dependent on the frameRate used ) to a time unit this object understands.
	int frameToTime( double frame ) const;
	/// finds the nearest supported oversampling rate based on the given frame rate.
	int actualOversamples() const;
	/// returns the time unit used for the oversampling computation.
	int timeUnit() const;
	/// returns the time step size that should be used in order to get the oversampling
	int stepSize() const;
	/// rounds the given time to the biggest step-aligned frame smaller than the given time.
	int stepRound( int time ) const;
	/// returns a number in the interval [0,1) that defines the relative offset of the given time to the
	// nearest smaller step-aligned frame.
	double relativeStepOffset( int time ) const;
	
	private:
	
	double m_frameRate;
	int m_oversamples;
	int m_step;
};

typedef OversamplesCalculator< 6000 > OversamplesCalculator6kFPS;

}; // namespace IECore

#include "OversamplesCalculator.inl"

#endif // IE_CORE_OVERSAMPLESCALCULATOR_H
