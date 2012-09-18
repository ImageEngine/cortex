//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECore/OversamplesCalculator.h"
#include "IECorePython/OversamplesCalculatorBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

tuple tickInterval( const OversamplesCalculator &o, float f )
{
	int tickLow = 0;
	int tickHigh = 0;

	float x = o.tickInterval( f, tickLow, tickHigh );
	return make_tuple( x, tickLow, tickHigh );
}

void bindOversamplesCalculator()
{
	class_< OversamplesCalculator >( "OversamplesCalculator", no_init )
		.def(
			init< optional< float, unsigned, unsigned > >
			(
				(
					arg( "frameRate" ) = float(24.0),
					arg( "samplesPerFrame" ) = unsigned(1),
					arg( "ticksPerSecond" ) = unsigned(6000)
				)
			)
		)

		.def( "setFrameRate", &OversamplesCalculator::setFrameRate )
		.def( "getFrameRate", &OversamplesCalculator::getFrameRate )
		.def( "setSamplesPerFrame", &OversamplesCalculator::setSamplesPerFrame )
		.def( "getSamplesPerFrame", &OversamplesCalculator::getSamplesPerFrame )
		.def( "setTicksPerSecond", &OversamplesCalculator::setTicksPerSecond )
		.def( "getTicksPerSecond", &OversamplesCalculator::getTicksPerSecond )
		.def( "framesToTicks", &OversamplesCalculator::framesToTicks )
		.def( "ticksToFrames", &OversamplesCalculator::ticksToFrames )
		.def( "nearestTick", &OversamplesCalculator::nearestTick )
		.def( "tickInterval", &tickInterval )
	;
}

} // namespace IECorePython
