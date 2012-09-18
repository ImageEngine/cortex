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


#include <math.h>
#include "IECore/Exception.h"

namespace IECore
{

template<int U>
OversamplesCalculator<U>::OversamplesCalculator( double frameRate, int desiredOversamples ) :
	m_frameRate( frameRate ), m_oversamples( desiredOversamples )
{
	double step = U / (double)(frameRate * desiredOversamples);
	while ( fabs(step - floor(step)) > 0.0001 && (m_frameRate * m_oversamples <= (double)U) )
	{
		m_oversamples ++;
		step = (double)U / (m_frameRate * m_oversamples);
	}

	if (fabs(step - floor(step)) > 0.0001)
	{
		throw IECore::Exception( "Unsupported oversamples/frame rate combination." );
	}
	m_step = (int)step;
}

template<int U>
int OversamplesCalculator<U>::frameToTime( double frame ) const
{
	return int(frame * (double)U / m_frameRate);
}

template<int U>
int OversamplesCalculator<U>::timeUnit() const
{
	return U;
}

template<int U>
int OversamplesCalculator<U>::actualOversamples() const
{
	return m_oversamples;
}

template<int U>
int OversamplesCalculator<U>::stepSize() const
{
	return m_step;
}

template<int U>
int OversamplesCalculator<U>::stepRound( int time ) const
{
	return (int)( time - (time % m_step) );
}

template<int U>
double OversamplesCalculator<U>::relativeStepOffset( int time ) const
{
	return (double)(time % m_step) / (double)m_step;
}

} // namespace IECore
