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

// This include needs to be the very first to prevent problems with warnings 
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "OpenEXR/ImathRandom.h"
#include "OpenEXR/ImathVec.h"

#include "IECore/bindings/ImathRandomBinding.h"

using namespace boost::python;
using namespace Imath;

namespace IECore 
{

template<typename T, typename F>
void bindRand( const char *className )
{	

	class_<T>( className )
		.def( init<unsigned long int>() )
		.def( "init", &T::init )
		.def( "nextb", &T::nextb )
		.def( "nexti", &T::nexti )
		.def( "nextf", (F (T::*)())&T::nextf )
		.def( "nextf", (F (T::*)( F, F ))&T::nextf )
		.def( "gauss", &gaussRand<T> )
		.def( "solidCirclef", &solidSphereRand<V2f,T> )
		.def( "solidCircled", &solidSphereRand<V2d,T> )
		.def( "solidSpheref", &solidSphereRand<V3f,T> )
		.def( "solidSphered", &solidSphereRand<V3d,T> )
		.def( "hollowCirclef", &hollowSphereRand<V2f,T> )
		.def( "hollowCircled", &hollowSphereRand<V2d,T> )
		.def( "hollowSpheref", &hollowSphereRand<V3f,T> )
		.def( "hollowSphered", &hollowSphereRand<V3d,T> )
		.def( "gaussCirclef", &gaussSphereRand<V2f,T> )
		.def( "gaussCircled", &gaussSphereRand<V2d,T> )
		.def( "gaussSpheref", &gaussSphereRand<V3f,T> )
		.def( "gaussSphered", &gaussSphereRand<V3d,T> )
	;
	
}

void bindImathRandom()
{
	bindRand<Rand32, float>( "Rand32" );
	bindRand<Rand48, double>( "Rand48" );

}

} // namespace IECore
