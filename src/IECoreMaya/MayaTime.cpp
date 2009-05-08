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

#include <cassert>

#include "maya/MTime.h"

#include "IECore/Exception.h"
#include "IECoreMaya/MayaTime.h"

using namespace IECore;
using namespace IECoreMaya;

double MayaTime::fps( MTime::Unit unit )
{
	unsigned i = 0;
	while( m_table[ i ].unit != MTime::kInvalid )
	{
		if ( m_table[ i ].unit == unit )
		{
			return m_table[ i ].fps;
		}
		i++;
	}
	throw Exception( "Invalid time unit!" );
	return 0;
}

MTime::Unit MayaTime::unit( double fps )
{
	unsigned i = 0;
	while( m_table[ i ].unit != MTime::kInvalid )
	{
		if ( m_table[ i ].fps == fps )
		{
			return m_table[ i ].unit;
		}
		i++;
	}
	throw Exception( "Invalid fps!" );
	return MTime::kInvalid;
}

MayaTime::TimeTable MayaTime::m_table[] =
{
	// MTime, , Unit  ->  FPS
	{ MTime::kHours, 1. / (60 * 60) },
	{ MTime::kMinutes, 1. / 60 },
	{ MTime::kSeconds, 1.},
	{ MTime::kMilliseconds, 1000.},
	{ MTime::kGames, 15},
	{ MTime::kFilm, 24},
	{ MTime::kPALFrame, 25},
	{ MTime::kNTSCFrame, 30},
	{ MTime::kShowScan, 48},
	{ MTime::kPALField, 50},
	{ MTime::kNTSCField, 60},
	{ MTime::k2FPS, 2},
	{ MTime::k3FPS, 3},
	{ MTime::k4FPS, 4},
	{ MTime::k5FPS, 5},
	{ MTime::k6FPS, 6},
	{ MTime::k8FPS, 8},
	{ MTime::k10FPS, 10},
	{ MTime::k12FPS, 12},
	{ MTime::k16FPS, 16},
	{ MTime::k20FPS, 20},
	{ MTime::k40FPS, 40},
	{ MTime::k75FPS, 75},
	{ MTime::k80FPS, 80},
	{ MTime::k100FPS, 100},
	{ MTime::k120FPS, 120},
	{ MTime::k125FPS, 125},
	{ MTime::k150FPS, 150},
	{ MTime::k200FPS, 200},
	{ MTime::k240FPS, 240},
	{ MTime::k250FPS, 250},
	{ MTime::k300FPS, 300},
	{ MTime::k375FPS, 375},
	{ MTime::k400FPS, 400},
	{ MTime::k500FPS, 500},
	{ MTime::k600FPS, 600},
	{ MTime::k750FPS, 750},
	{ MTime::k1200FPS, 1200},
	{ MTime::k1500FPS, 1500},
	{ MTime::k2000FPS, 2000},
	{ MTime::k3000FPS, 3000},
	{ MTime::k6000FPS, 6000},
	{ MTime::kInvalid, 0 }
};
