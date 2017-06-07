//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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

#ifndef IECOREPYTHON_PARAMETERBINDING_INL
#define IECOREPYTHON_PARAMETERBINDING_INL

#include "IECorePython/IECoreBinding.h"

namespace
{

template<typename T>
boost::python::tuple valueValid( const T &that, IECore::ConstObjectPtr value )
{
	std::string reason;
	bool valid = that.T::valueValid( value.get(), &reason );
	return boost::python::make_tuple( valid, reason );
}

boost::python::tuple valueValid2( const IECore::Parameter &that )
{
	std::string reason;
	bool valid = that.valueValid( &reason );
	return boost::python::make_tuple( valid, reason );
}

} // namespace

namespace IECorePython
{

template<typename T>
T parameterPresets( const boost::python::object &o )
{
	T result;
	size_t s = IECorePython::len( o );
	for( size_t i=0; i<s; i++ )
	{
		boost::python::tuple preset = boost::python::extract<boost::python::tuple>( o[i] )();
		size_t ts = IECorePython::len( preset );
		if( ts!=2 )
		{
			PyErr_SetString( PyExc_ValueError, "Preset must be a tuple of the form ( name, value ).");
			throw boost::python::error_already_set();
		}
		result.push_back( typename T::value_type( boost::python::extract<std::string>( preset[0] )(), boost::python::extract<typename T::value_type::second_type>( preset[1] )() ) );
	}

	return result;
}

template<typename T, typename TWrapper>
ParameterClass<T, TWrapper>::ParameterClass( const char *docString )
	:	IECorePython::RunTimeTypedClass<T, TWrapper>( docString )
{
	this->def( "valueValid", &valueValid<T>, "Returns a tuple containing a bool specifying validity and a string giving a reason for invalidity." );
	this->def( "valueValid", &valueValid2, "Returns a tuple containing a bool specifying validity and a string giving a reason for invalidity." );
}

} // namespace IECorePython

#endif // IECOREPYTHON_PARAMETERBINDING_INL
