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

#include "IECorePython/TimeCodeDataBinding.h"

#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/IECoreBinding.h"

#include "IECore/TimeCodeData.h"

#include "boost/python.hpp"
#include "boost/python/make_constructor.hpp"

#include <sstream>

using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

template<>
std::string repr<TimeCodeData>( TimeCodeData &x )
{
	object item( x.readable() );

	assert( item.attr( "__repr__" ) != object() );

	std::stringstream s;

	s << "IECore.TimeCodeData( ";
	s << call_method< std::string >( item.ptr(), "__repr__" );
	s << " )";

	return s.str();
}

template<>
std::string str<TimeCodeData>( TimeCodeData &x )
{
	object item( x.readable() );

	assert( item.attr( "__str__" ) != object() );

	return call_method< std::string >( item.ptr(), "__str__" );
}

static void setValue( TimeCodeDataPtr data, const Imf::TimeCode &v )
{
	assert( data );
	data->writable() = v;
}

static const Imf::TimeCode &getValue( TimeCodeDataPtr data )
{
	assert( data );
	return data->readable();
}

void bindTimeCodeData()
{
	RunTimeTypedClass<TimeCodeData>()
		.def( init<>() )
		.def( init<const TimeCodeData::ValueType &>() )
		.add_property( "value", make_function( &getValue, return_value_policy<copy_const_reference>() ), &setValue )
		.def( "__repr__", &repr<TimeCodeData> )
		.def( "__str__", &str<TimeCodeData> )
		.def( "hasBase", &TimeCodeData::hasBase ).staticmethod( "hasBase" )
	;
}

} // namespace IECorePython
