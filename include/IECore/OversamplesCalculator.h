//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

/// Performs conversions between floating-point time and a discretized "tick"-based system, such as
/// the one Maya uses (there are 6000 Maya ticks per second). Also provides a utility for computing the
/// interpolation factor between two successive ticks for a given frame under different oversampling conditions.
/// \todo Rename to something more descriptive of its function
class OversamplesCalculator
{
	public:
		OversamplesCalculator(
			float frameRate=24.0,
			unsigned samplesPerFrame=1,
			unsigned ticksPerSecond=6000
		);

		void setFrameRate( float frameRate );
		float getFrameRate() const;
		void setSamplesPerFrame( unsigned samplesPerFrame );
		unsigned getSamplesPerFrame() const;
		void setTicksPerSecond( unsigned ticksPerSecond );
		unsigned getTicksPerSecond() const;

		/// Convert the given fractional frame into ticks
		int framesToTicks( float f ) const;

		/// Convert the specfied tick to frames
		float ticksToFrames( int i ) const;

		/// Returns the tick nearest to the argument
		int nearestTick( int tick ) const ;

		/// Returns lerp factor, and the times of the adjacent ticks
		float tickInterval( float frame, int &tickLow, int &tickHigh ) const;

	private:

		float m_frameRate ;
		int m_samplesPerFrame ;
		int m_ticksPerSecond ;
};

}; // namespace IECore


#endif // IE_CORE_OVERSAMPLESCALCULATOR_H
