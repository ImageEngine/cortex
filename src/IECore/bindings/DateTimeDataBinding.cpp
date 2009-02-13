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

#include "IECore/DateTimeData.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IECoreBinding.h"

#include <sstream>

using namespace boost;
using namespace boost::python;

namespace IECore
{

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

struct ptime_from_python_datetime
{
	ptime_from_python_datetime()
	{
		converter::registry::push_back(
			&convertible,
			&construct,
			type_id<posix_time::ptime> ()
		);
	}

	static void *convertible( PyObject *obj_ptr )
	{
		if ( !PyDateTime_Check( obj_ptr ) )
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
		assert( PyDateTime_Check( obj_ptr ) );

		PyDateTime_DateTime const* datetime = reinterpret_cast<PyDateTime_DateTime*>( obj_ptr );

		const posix_time::ptime t(
			gregorian::date(
				PyDateTime_GET_YEAR( datetime ),
				PyDateTime_GET_MONTH( datetime ),
				PyDateTime_GET_DAY( datetime )
			),
			posix_time::time_duration(
				PyDateTime_DATE_GET_HOUR( datetime ),
				PyDateTime_DATE_GET_MINUTE( datetime ),
				PyDateTime_DATE_GET_SECOND( datetime ),
				0
			) + posix_time::microseconds( PyDateTime_DATE_GET_MICROSECOND( datetime ) )
		);

		void* storage = (( converter::rvalue_from_python_storage<posix_time::ptime>* ) data )->storage.bytes;
		new( storage ) posix_time::ptime( t );
		data->convertible = storage;
	}
};

struct ptime_to_python
{
	static PyObject *convert( const posix_time::ptime &t )
	{
		if ( t.is_special() )
		{
			PyErr_SetString(PyExc_ValueError, "Cannot convert out-of-range ptime to datetime");
			throw_error_already_set();
		}
		
		gregorian::date date = t.date();
		posix_time::time_duration dur = t.time_of_day();
		return PyDateTime_FromDateAndTime(
			static_cast<int>( date.year() ),
			static_cast<int>( date.month() ),
			static_cast<int>( date.day() ),
			static_cast<int>( dur.hours() ),
			static_cast<int>( dur.minutes() ),
			static_cast<int>( dur.seconds() ),
			getMicroseconds( dur )
		);
	}
};

template<>
static std::string repr<DateTimeData>( DateTimeData &x )
{
	object item( x.readable() );

	assert( item.attr( "__repr__" ) != object() );

	std::stringstream s;

	s << "IECore.DateTimeData( ";
	s << call_method< std::string >( item.ptr(), "__repr__" );
	s << " )";

	return s.str();
}

template<>
static std::string str<DateTimeData>( DateTimeData &x )
{
	return posix_time::to_simple_string( x.readable() );
}

static void setValue( DateTimeDataPtr data, const posix_time::ptime &v )
{
	assert( data );
	data->writable() = v;
}

static const posix_time::ptime &getValue( DateTimeDataPtr data )
{
	assert( data );
	return data->readable();
}

void bindDateTimeData()
{
	PyDateTime_IMPORT;

	ptime_from_python_datetime();
	to_python_converter<posix_time::ptime, ptime_to_python > ();

	typedef class_< DateTimeData, DateTimeDataPtr, noncopyable, bases<Data> > DateTimeDataPyClass;
	DateTimeDataPyClass( "DateTimeData", no_init )
		.def( init<>() )
		.def( init<const DateTimeData::ValueType &>() )
		.add_property( "value", make_function( &getValue, return_value_policy<copy_const_reference>() ), &setValue )
		.def( "__repr__", &repr<DateTimeData> )
		.def( "__str__", &str<DateTimeData> )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( DateTimeData );
	;

	INTRUSIVE_PTR_PATCH( DateTimeData, DateTimeDataPyClass );

	implicitly_convertible<DateTimeDataPtr, DataPtr>();
	implicitly_convertible<DateTimeDataPtr, ConstDateTimeDataPtr >();
}

} // namespace IECore
