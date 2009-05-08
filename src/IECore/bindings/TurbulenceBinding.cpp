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

#include "boost/python.hpp"

#include "IECore/Turbulence.h"

using namespace boost;
using namespace boost::python;

namespace IECore {

template<typename T>
void bindTurb( const char *name )
{
	class_<T>( name )
		.def( init<unsigned int, typename T::Value, typename T::Point, bool, const typename T::Noise &>(
				(	boost::python::arg( "octaves" ) = 4,
					boost::python::arg( "gain" ) = typename T::Value( 0.5 ),
					boost::python::arg( "lacunarity" ) = typename T::Point( 2.0 ),
					boost::python::arg( "turbulent" ) = true,
					boost::python::arg( "noise"  ) = typename T::Noise()			 )
			) )
		.def( "turbulence", &T::turbulence )
		.add_property( "octaves", &T::getOctaves, &T::setOctaves )
		.add_property( "gain", make_function( &T::getGain, return_value_policy<copy_const_reference>() ), &T::setGain )
		.add_property( "lacunarity", make_function( &T::getLacunarity, return_value_policy<copy_const_reference>() ), &T::setLacunarity )
		.add_property( "turbulent", &T::getTurbulent, &T::setTurbulent )
	;
}

void bindTurbulence()
{
	bindTurb<TurbulenceV3ff>( "TurbulenceV3ff" );
	bindTurb<TurbulenceV2ff>( "TurbulenceV2ff" );
	bindTurb<Turbulenceff>( "Turbulenceff" );

	bindTurb<TurbulenceV3fV2f>( "TurbulenceV3fV2f" );
	bindTurb<TurbulenceV2fV2f>( "TurbulenceV2fV2f" );
	bindTurb<TurbulencefV2f>( "TurbulencefV2f" );

	bindTurb<TurbulenceV3fV3f>( "TurbulenceV3fV3f" );
	bindTurb<TurbulenceV2fV3f>( "TurbulenceV2fV3f" );
	bindTurb<TurbulencefV3f>( "TurbulencefV3f" );

	bindTurb<TurbulenceV3fColor3f>( "TurbulenceV3fColor3f" );
	bindTurb<TurbulenceV2fColor3f>( "TurbulenceV2fColor3f" );
	bindTurb<TurbulencefColor3f>( "TurbulencefColor3f" );
}

} // namespace IECore
