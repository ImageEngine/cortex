//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/ImfTimeCodeBinding.h"

#include "IECorePython/IECoreBinding.h"

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImfTimeCode.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/format.hpp"

#include <sstream>
#include <string>

using namespace boost::python;

namespace IECorePython
{

template<>
std::string repr<Imf::TimeCode>( Imf::TimeCode &x )
{
	std::stringstream s;
	s << "IECore.TimeCode( ";
	s << x.hours() << ", ";
	s << x.minutes() << ", ";
	s << x.seconds() << ", ";
	s << x.frame() << ", ";
	s << x.dropFrame() << ", ";
	s << x.colorFrame() << ", ";
	s << x.fieldPhase() << ", ";
	s << x.bgf0() << ", ";
	s << x.bgf1() << ", ";
	s << x.bgf2() << ", ";
	s << x.binaryGroup( 1 ) << ", ";
	s << x.binaryGroup( 2 ) << ", ";
	s << x.binaryGroup( 3 ) << ", ";
	s << x.binaryGroup( 4 ) << ", ";
	s << x.binaryGroup( 5 ) << ", ";
	s << x.binaryGroup( 6 ) << ", ";
	s << x.binaryGroup( 7 ) << ", ";
	s << x.binaryGroup( 8 );
	s << " )";
	return s.str();
}

template<>
std::string str<Imf::TimeCode>( Imf::TimeCode &x )
{

	return ( boost::format( "%02d:%02d:%02d:%02d" ) % x.hours() % x.minutes() % x.seconds() % x.frame() ).str();
}

bool equal( Imf::TimeCode &x, Imf::TimeCode &y )
{
	return ( x.timeAndFlags() == y.timeAndFlags() && x.userData() == y.userData() );
}

bool notEqual( Imf::TimeCode &x, Imf::TimeCode &y )
{
	return !equal( x, y );
}

void bindImfTimeCode()
{
	scope timeCodeScope = class_<Imf::TimeCode>( "TimeCode" )

		.def( init<>() )
		.def( init<const Imf::TimeCode &>() )

		.def(
			init< int, optional< int, Imf::TimeCode::Packing > >
			(
				(
					arg( "timeAndFlags" ),
					arg( "userData" ) = 0,
					arg( "packing" )
				)
			)
		)

		.def(
			init< int, int, int, int, optional< bool, bool, bool, bool, bool, bool, int, int, int, int, int, int, int, int > >
			(
				(
					arg( "hours" ),
					arg( "minutes" ),
					arg( "seconds" ),
					arg( "frame" ),
					arg( "dropFrame" ) = false,
					arg( "colorFrame" ) = false,
					arg( "fieldPhase" ) = false,
					arg( "bgf0" ) = false,
					arg( "bgf1" ) = false,
					arg( "bgf2" ) = false,
					arg( "binaryGroup1" ) = 0,
					arg( "binaryGroup2" ) = 0,
					arg( "binaryGroup3" ) = 0,
					arg( "binaryGroup4" ) = 0,
					arg( "binaryGroup5" ) = 0,
					arg( "binaryGroup6" ) = 0,
					arg( "binaryGroup7" ) = 0,
					arg( "binaryGroup8" ) = 0
				)
			)
		)

		.def( "hours",  &Imf::TimeCode::hours )
		.def( "minutes",  &Imf::TimeCode::minutes )
		.def( "seconds",  &Imf::TimeCode::seconds )
		.def( "frame",  &Imf::TimeCode::frame )
		.def( "dropFrame",  &Imf::TimeCode::dropFrame )
		.def( "colorFrame",  &Imf::TimeCode::colorFrame )
		.def( "fieldPhase",  &Imf::TimeCode::fieldPhase )
		.def( "bgf0",  &Imf::TimeCode::bgf0 )
		.def( "bgf1",  &Imf::TimeCode::bgf1 )
		.def( "bgf2",  &Imf::TimeCode::bgf2 )
		.def( "binaryGroup",  &Imf::TimeCode::binaryGroup )
		.def( "userData",  &Imf::TimeCode::userData )

		.def( "setHours",  &Imf::TimeCode::setHours )
		.def( "setMinutes",  &Imf::TimeCode::setMinutes )
		.def( "setSeconds",  &Imf::TimeCode::setSeconds )
		.def( "setFrame",  &Imf::TimeCode::setFrame )
		.def( "setDropFrame",  &Imf::TimeCode::setDropFrame )
		.def( "setColorFrame",  &Imf::TimeCode::setColorFrame )
		.def( "setFieldPhase",  &Imf::TimeCode::setFieldPhase )
		.def( "setBgf0",  &Imf::TimeCode::setBgf0 )
		.def( "setBgf1",  &Imf::TimeCode::setBgf1 )
		.def( "setBgf2",  &Imf::TimeCode::setBgf2 )
		.def( "setBinaryGroup",  &Imf::TimeCode::setBinaryGroup )
		.def( "setUserData",  &Imf::TimeCode::setUserData )

		.def( "__str__", &str<Imf::TimeCode> )
		.def( "__repr__", &repr<Imf::TimeCode> )
		.def( "__eq__", &equal )
		.def( "__ne__", &notEqual )
	;

	enum_< Imf::TimeCode::Packing >( "Packing" )
		.value( "TV60", Imf::TimeCode::TV60_PACKING )
		.value( "TV50", Imf::TimeCode::TV50_PACKING )
		.value( "FILM24", Imf::TimeCode::FILM24_PACKING )
	;

	// these need to be defined after the Packing enum so the default values can
	// be converted to python. They are still member functions thanks to the scope.
	def( "timeAndFlags",  &Imf::TimeCode::timeAndFlags, ( arg_( "self" ), arg_( "packing" )=Imf::TimeCode::TV60_PACKING ) );
	def( "setTimeAndFlags",  &Imf::TimeCode::setTimeAndFlags, ( arg_( "self" ), arg_( "value" ), arg_( "packing" )=Imf::TimeCode::TV60_PACKING ) );

}

}
