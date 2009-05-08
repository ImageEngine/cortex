//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
#include "boost/python/make_constructor.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/time_parsers.hpp"

#include "IECore/TimePeriodData.h"
#include "IECore/bindings/IECoreBinding.h"

#include <sstream>

using namespace boost;
using namespace boost::python;

namespace IECore
{

template<>
std::string repr<TimePeriod>( TimePeriod &x )
{
	object beginObject( x.begin() );
	assert( beginObject.attr( "__repr__" ) != object() );

	object endObject( x.end() );
	assert( endObject.attr( "__repr__" ) != object() );

	std::stringstream s;

	s << "IECore.TimePeriod( ";
	s << call_method< std::string >( beginObject.ptr(), "__repr__" );
	s << ", ";
	s << call_method< std::string >( endObject.ptr(), "__repr__" );
	s << " )";

	return s.str();
}

template<>
std::string str<TimePeriod>( TimePeriod &x )
{
	std::stringstream s;

	s << posix_time::to_simple_string( x.begin() );
	s << " ";
	s << posix_time::to_simple_string( x.end() );

	return s.str();
}

struct TimePeriodHelper
{
	static TimePeriod intersection( TimePeriod &x, const TimePeriod &d )
	{
		return x.intersection( d );
	}

	static TimePeriod merge( TimePeriod &x, const TimePeriod &d )
	{
		return x.merge( d );
	}

	static TimePeriod span( TimePeriod &x, const TimePeriod &d )
	{
		return x.span( d );
	}

	static bool contains( TimePeriod &x, const TimePeriod &d )
	{
		return x.contains( d );
	}

};

void bindTimePeriod()
{
	bool (TimePeriod::*containsTime)( const posix_time::ptime & ) const = &TimePeriod::contains;

	class_< TimePeriod >( "TimePeriod", no_init )
		.def( init< posix_time::ptime, posix_time::ptime >() )
		.def( init< posix_time::ptime, posix_time::time_duration >() )
		.def( init< TimePeriod >() )
		.def( "shift", &TimePeriod::shift )

		.def( "begin", &TimePeriod::begin )
		.def( "end", &TimePeriod::end )
		.def( "last", &TimePeriod::last )
		.def( "length", &TimePeriod::length )
		.def( "isNull", &TimePeriod::is_null )
		.def( "contains", containsTime )
		.def( "containsTimePeriod", &TimePeriodHelper::contains )
		.def( "intersects", &TimePeriod::intersects )
		.def( "intersection", &TimePeriodHelper::intersection )
		.def( "merge", &TimePeriodHelper::merge )
		.def( "span", &TimePeriodHelper::span )
		.def( "__repr__", &repr<TimePeriod> )
		.def( "__str__", &str<TimePeriod> )

		.def( self == self )
		.def( self != self )
		.def( self > self )
		.def( self < self )
		.def( self >= self )
		.def( self <= self )
	;

	implicitly_convertible<TimePeriod, boost::posix_time::time_period>();
}

} // namespace IECore
