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

#ifndef IE_COREPYTHON_PARAMETERBINDING_INL
#define IE_COREPYTHON_PARAMETERBINDING_INL

namespace IECore
{

template<typename T>
boost::python::tuple valueValid( const T &that, ConstObjectPtr value )
{
	std::string reason;
	bool valid = that.T::valueValid( value, &reason );
	return boost::python::make_tuple( valid, reason );
}

template<typename T>
T parameterPresets( const boost::python::object &o )
{
	/// \todo Remove support for the deprecated dictionary style argument for major version 5
	T result;
	boost::python::extract<boost::python::dict> ed( o );
	if( ed.check() )
	{
		if( PyErr_WarnEx( PyExc_DeprecationWarning, "Specifying presets as a dictionary is deprecated - pass a tuple of tuples instead.", 1 ) )
		{
			// warning converted to exception
			throw boost::python::error_already_set();
		}
	
		boost::python::dict dict = ed();
		boost::python::list keys = dict.keys();
		boost::python::list values = dict.values();
		for( int i = 0; i<keys.attr( "__len__" )(); i++ )
		{
			result.push_back( typename T::value_type( boost::python::extract<std::string>( keys[i] )(), boost::python::extract<typename T::value_type::second_type>( values[i] )() ) );
		}
		
		return result;
	}
		
	size_t s = boost::python::len( o );
	for( size_t i=0; i<s; i++ )
	{
		boost::python::tuple preset = boost::python::extract<boost::python::tuple>( o[i] )();
		size_t ts = boost::python::len( preset );
		if( ts!=2 )
		{
			PyErr_SetString( PyExc_ValueError, "Preset must be a tuple of the form ( name, value ).");
			throw boost::python::error_already_set();
		}
		result.push_back( typename T::value_type( boost::python::extract<std::string>( preset[0] )(), boost::python::extract<typename T::value_type::second_type>( preset[1] )() ) );
	}
	
	return result;	
}

}

#endif // IE_COREPYTHON_PARAMETERBINDING_INL
