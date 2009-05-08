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
//	     other contributors to this software may be used to endorse or
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

#include <cassert>
#include <math.h>

#include "IECore/OversamplesCalculator.h"

using namespace IECore;

OversamplesCalculator::OversamplesCalculator( float frameRate, unsigned samplesPerFrame, unsigned ticksPerSecond )
		: m_frameRate( frameRate ), m_samplesPerFrame( samplesPerFrame ), m_ticksPerSecond( ticksPerSecond )
{
}

void OversamplesCalculator::setFrameRate( float frameRate )
{
	m_frameRate = frameRate;
}

float OversamplesCalculator::getFrameRate() const
{
	return m_frameRate;
}

void OversamplesCalculator::setSamplesPerFrame( unsigned samplesPerFrame )
{
	m_samplesPerFrame = samplesPerFrame;
}

unsigned OversamplesCalculator::getSamplesPerFrame() const
{
	return m_samplesPerFrame;
}

void OversamplesCalculator::setTicksPerSecond( unsigned ticksPerSecond )
{
	m_ticksPerSecond = ticksPerSecond;
}

unsigned OversamplesCalculator::getTicksPerSecond() const
{
	return m_ticksPerSecond;
}

int OversamplesCalculator::framesToTicks( float f ) const
{
	return ( int )( f * m_ticksPerSecond /  m_frameRate );
}

float OversamplesCalculator::ticksToFrames( int i ) const
{
	return ( float )i / m_ticksPerSecond * m_frameRate ;
}

int OversamplesCalculator::nearestTick( int tick ) const
{
	int low, high;
	float frame = ticksToFrames( tick );
	tickInterval( frame, low, high );
	if (( tick - low )*( tick-low ) < ( tick-high )*( tick-high ) )
	{
		return low;
	}
	else
	{
		return high;
	}
}

float OversamplesCalculator::tickInterval( float frame, int &tickLow, int &tickHigh ) const
{
	float step = ( float )m_ticksPerSecond / ( m_frameRate * m_samplesPerFrame );

	/// Maya seems to suffer from rounding issues so that, for example, tick numbers at 24fps with 3
	/// samples per frame end up as:
	/// 250, 333, 416, 499, 583, 555, 749
	/// Note that the 499 and 749 should ideally be 500 and 750 respectively.
	/// Here we deliberately discard some of the precision in an attempt to match this behaviour.
	step = int( step*100000 )/100000.0;
	float tickF = frame * m_ticksPerSecond /  m_frameRate ;
	int tick = ( int )( tickF );

	float tickLowF = ( float )tick - fmod(( float )tick,step );
	float tickHighF =  tickLowF + step;

	/// Due to rounding errors we might find that our low/high bound does not actually bracket the tick.  Correct
	/// for that here.
	if ( tick > ( int )tickHighF )
	{
		tickLowF += step;
		tickHighF += step;
	}
	if ( tick < ( int )tickLowF )
	{
		tickLowF -= step;
		tickHighF -= step;
	}
	tickLow = int( tickLowF );
	tickHigh = int( tickHighF );
	assert( tick >= tickLow );
	assert( tick <= tickHigh );

	float x = ( float )( tickF - tickLowF ) / ( float )( tickHigh - tickLowF );
	return x;
}
