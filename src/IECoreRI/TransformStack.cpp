//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathFun.h"

#include "IECore/Exception.h"
#include "IECore/Interpolator.h"

#include "IECoreRI/private/TransformStack.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreRI;

TransformStack::TransformStack()
	:	m_motionIndex( -1 ) // not in motion block
{
	m_stack.push( Samples( 1, Sample( 0.0f, Imath::M44f() ) ) );
}

void TransformStack::push()
{
	m_stack.push( m_stack.top() );
}

void TransformStack::pop()
{
	if( m_stack.size() <= 1 )
	{
		throw Exception( "TransformStack::pop() called without matching push()" );
	}
	m_stack.pop();
}

size_t TransformStack::size() const
{
	return m_stack.size();
}

void TransformStack::motionBegin( const std::vector<float> &times )
{
	// resample the current stack top at the specified times
	Samples newSamples;
	for( std::vector<float>::const_iterator it = times.begin(), eIt = times.end(); it != eIt; ++it )
	{
		newSamples.push_back( Sample( *it, get( *it ) ) );
	}
	m_stack.top() = newSamples;
	
	m_motionIndex = 0;
}

void TransformStack::motionEnd()
{
	m_motionIndex = -1;
}

void TransformStack::set( const Imath::M44f &matrix )
{
	Samples &samples = m_stack.top();
	if( m_motionIndex >= 0 )
	{
		if( m_motionIndex >= (int)samples.size() )
		{
			throw Exception( "TransformStack::set() called too many times for motion block" );	
		}
		samples[m_motionIndex++].matrix = matrix;
	}
	else
	{
		for( Samples::iterator it = samples.begin(), eIt = samples.end(); it != eIt; ++it )
		{
			it->matrix = matrix;
		}
	}
}

void TransformStack::concatenate( const Imath::M44f &matrix )
{
	Samples &samples = m_stack.top();
	if( m_motionIndex >= 0 )
	{
		if( m_motionIndex >= (int)samples.size() )
		{
			throw Exception( "TransformStack::concatenate() called too many times for motion block" );	
		}
		samples[m_motionIndex].matrix = matrix * samples[m_motionIndex].matrix;
		m_motionIndex++;
	}
	else
	{
		for( Samples::iterator it = samples.begin(), eIt = samples.end(); it != eIt; ++it )
		{
			it->matrix = matrix * it->matrix;
		}
	}
}

Imath::M44f TransformStack::get() const
{
	return m_stack.top()[0].matrix;
}

Imath::M44f TransformStack::get( float time ) const
{
	const Samples &samples = m_stack.top();
	if( samples.size() == 1 )
	{
		return samples[0].matrix;
	}
	
	// interpolate. find the first sample where sample.time >= time.
	
	Samples::const_iterator s1 = lower_bound( samples.begin(), samples.end(), time );
	if( s1 == samples.begin() || s1->time == time )
	{
		return s1->matrix;
	}
	else if( s1 == samples.end() )
	{
		return samples.back().matrix;
	}
	else
	{
		Samples::const_iterator s0 = s1 - 1;
		const float l = lerpfactor( time, s0->time, s1->time );
		M44f result;
		LinearInterpolator<M44f>()( s0->matrix, s1->matrix, l, result );
		return result;
	}	
}

size_t TransformStack::numSamples() const
{
	return m_stack.top().size();
}

Imath::M44f TransformStack::sample( size_t sampleIndex )
{
	return m_stack.top()[sampleIndex].matrix;
}

float TransformStack::sampleTime( size_t sampleIndex )
{
	return m_stack.top()[sampleIndex].time;
}
