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

#include "datetime.h"

#include "IECore/TimePeriodData.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IECoreBinding.h"

#include <sstream>

using namespace boost;
using namespace boost::python;

namespace IECore
{

template<class T>
static int cmp( T &x, T &y )
{
	const typename T::ValueType &xData = x.readable();
	const typename T::ValueType &yData = y.readable();
	if( xData < yData )
	{
		return -1;
	}
	if( xData > yData )
	{
		return 1;
	}
	return 0;
}

template<>
std::string repr<TimePeriodData>( TimePeriodData &x )
{
	object item( x.readable() );

	assert( item.attr( "__repr__" ) != object() );

	std::stringstream s;

	s << "IECore.TimePeriodData( ";
	s << call_method< std::string >( item.ptr(), "__repr__" );
	s << " )";

	return s.str();
}

template<>
std::string str<TimePeriodData>( TimePeriodData &x )
{
	return posix_time::to_simple_string( x.readable() );
}

static void setValue( TimePeriodDataPtr data, const TimePeriod &v )
{
	assert( data );
	data->writable() = v;
}

static const TimePeriod &getValue( TimePeriodDataPtr data )
{
	assert( data );
	return data->readable();
}

void bindTimePeriodData()
{
	PyDateTime_IMPORT;

	RunTimeTypedClass<TimePeriodData>()
		.def( init<>() )
		.def( init<const TimePeriodData::ValueType &>() )
		.add_property( "value", make_function( &getValue, return_value_policy<copy_const_reference>() ), &setValue )
		.def( "__repr__", &repr<TimePeriodData> )
		.def( "__str__", &str<TimePeriodData> )
		.def( "__cmp__", &str<TimePeriodData> )
	;
}

} // namespace IECore
