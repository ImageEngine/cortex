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

#include "IECore/TimeDurationData.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IECoreBinding.h"

#include <sstream>

using namespace boost;
using namespace boost::python;

namespace IECore
{

/// \todo Share this with the one in DataTimeDataBinding
static int getMicroseconds( const posix_time::time_duration &dur )
{
	static long ticksPerSecond = posix_time::time_duration::ticks_per_second();
	long fractionalSeconds = dur.fractional_seconds();
	
	static const int oneMillion = 1000000;
	
	/// Prevent over/underflow
	if ( ticksPerSecond > oneMillion )
	{
		return fractionalSeconds / ( ticksPerSecond / oneMillion );
	}
	else
	{
		return fractionalSeconds * ( oneMillion / ticksPerSecond );
	}
}

struct TimeDurationFromPythonDelta
{
	TimeDurationFromPythonDelta()
	{
		converter::registry::push_back(
			&convertible,
			&construct,
			type_id<posix_time::time_duration> ()
		);
	}

	static void *convertible( PyObject *obj_ptr )
	{
		if ( !PyDelta_Check( obj_ptr ) )
		{
			return 0;
		}
		return obj_ptr;
	}

	static void construct(
	        PyObject *obj_ptr,
	        converter::rvalue_from_python_stage1_data *data )
	{
		assert( obj_ptr );
		assert( PyDelta_Check( obj_ptr ) );

		PyDateTime_Delta const* pyDelta = reinterpret_cast<PyDateTime_Delta*>( obj_ptr );

		long days = pyDelta->days;
		bool isNegative = days < 0;
		if ( isNegative )
		{
			days = -days;
		}
				
		posix_time::time_duration td = posix_time::hours(24)*days
			+ posix_time::seconds( pyDelta->seconds )
			+ posix_time::microseconds( pyDelta->microseconds );

		if ( isNegative )
		{
			td = td.invert_sign();
		}		
		

		void* storage = (( converter::rvalue_from_python_storage<posix_time::time_duration>* ) data )->storage.bytes;
		new( storage ) posix_time::time_duration( td );
		data->convertible = storage;
	}
};

struct TimeDurationToPythonDelta
{
	static PyObject *convert( const posix_time::time_duration &td )
	{
		long days = td.hours() / 24;
		if ( days < 0 )
			days --;
		long seconds = td.total_seconds() - days * ( 24 * 60 * 60 );
		long microSeconds = getMicroseconds( td );
		
		if ( days < 0 )
		{
			microSeconds = 1000000 - 1 - microSeconds;
		}
		
		return PyDelta_FromDSU( days, seconds, microSeconds );
	}
};

template<>
std::string repr<TimeDurationData>( TimeDurationData &x )
{
	object item( x.readable() );

	assert( item.attr( "__repr__" ) != object() );

	std::stringstream s;

	s << "IECore.TimeDurationData( ";
	s << call_method< std::string >( item.ptr(), "__repr__" );
	s << " )";

	return s.str();
}

template<>
std::string str<TimeDurationData>( TimeDurationData &x )
{
	return posix_time::to_simple_string( x.readable() );
}

static void setValue( TimeDurationDataPtr data, const posix_time::time_duration &v )
{
	assert( data );
	data->writable() = v;
}

static const posix_time::time_duration &getValue( TimeDurationDataPtr data )
{
	assert( data );
	return data->readable();
}

void bindTimeDurationData()
{
	PyDateTime_IMPORT;

	TimeDurationFromPythonDelta();
	to_python_converter<posix_time::time_duration, TimeDurationToPythonDelta > ();

	RunTimeTypedClass<TimeDurationData>()
		.def( init<>() )
		.def( init<const TimeDurationData::ValueType &>() )
		.add_property( "value", make_function( &getValue, return_value_policy<copy_const_reference>() ), &setValue )
		.def( "__repr__", &repr<TimeDurationData> )
		.def( "__str__", &str<TimeDurationData> )
	;

}

} // namespace IECore
