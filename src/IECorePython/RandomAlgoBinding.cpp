//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "IECorePython/RandomAlgoBinding.h"

#include "IECore/RandomAlgo.h"

using namespace boost::python;
using namespace Imath;
using namespace IECore;
using namespace IECorePython;

namespace
{

template<typename T>
void bind()
{

	def( "barycentricRandf", &RandomAlgo::barycentricRand<V3f, T> );
	def( "barycentricRandd", &RandomAlgo::barycentricRand<V3d, T> );

	def( "triangleRandf", &RandomAlgo::barycentricRand<V3f, T> );
	def( "triangleRandd", &RandomAlgo::barycentricRand<V3d, T> );

	def( "cosineHemisphereRandf", &RandomAlgo::cosineHemisphereRand<V3f, T> );
	def( "cosineHemisphereRandd", &RandomAlgo::cosineHemisphereRand<V3d, T> );

}

} // namespace

void IECorePython::bindRandomAlgo()
{

	object randomAlgoModule( borrowed( PyImport_AddModule( "IECore.RandomAlgo" ) ) );
	scope().attr( "RandomAlgo" ) = randomAlgoModule;

	scope randomAlgoScope( randomAlgoModule );

	bind<Rand32>();
	bind<Rand48>();

}
