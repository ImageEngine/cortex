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

#include "IECore/Parameterised.h"
#include "IECore/CompoundParameter.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/Wrapper.h"

using namespace boost;
using namespace boost::python;

namespace IECore {

class ParameterisedWrap : public Parameterised, public Wrapper<Parameterised>
{
	public :
		ParameterisedWrap( PyObject *self, const std::string &name, const std::string &description ) : Parameterised( name, description ), Wrapper<Parameterised>( self, this ) {};
		ParameterisedWrap( PyObject *self, const std::string &name, const std::string &description, CompoundParameterPtr compoundParameter ) : Parameterised( name, description, compoundParameter ), Wrapper<Parameterised>( self, this ) {};
};
IE_CORE_DECLAREPTR( ParameterisedWrap );

static ParameterPtr parameterisedGetItem( Parameterised &o, const std::string &n )
{
	ParameterPtr p = o.parameters()->parameter<Parameter>( n );
	if( !p )
	{
		throw Exception( std::string("Parameter ") + n + " doesn't exist" );
	}
	return p;
}

static object parameterisedGetAttr( Parameterised &o, const std::string &n )
{
	ParameterPtr p = o.parameters()->parameter<Parameter>( n );
	
	if ( p )
	{
		if( PyErr_WarnEx( PyExc_DeprecationWarning, "Access to Parameters as attributes is deprecated - please use item style access instead.", 1 ) )
		{
			// warning converted to exception;
			throw error_already_set();
		}
		return object( p );
	}
	
	PyErr_SetString( PyExc_KeyError, ( "'"  + n + "'" ).c_str() );
	throw error_already_set();
}

void bindParameterised()
{
	using boost::python::arg;

	RunTimeTypedClass<Parameterised, ParameterisedWrapPtr>()
		.def( init< const std::string, const std::string>( ( arg( "name" ), arg( "description") ) ) )
		.def( init< const std::string, const std::string, CompoundParameterPtr >( ( arg( "name" ), arg( "description") , arg( "compoundParameter") ) ) )
		.add_property( "name", make_function( &Parameterised::name, return_value_policy<copy_const_reference>() ) )
		.add_property( "description", make_function( &Parameterised::description, return_value_policy<copy_const_reference>() ) )
		.def( "parameters", (CompoundParameterPtr (Parameterised::*)())&Parameterised::parameters )
		.def( "__getitem__", &parameterisedGetItem )
		/// \todo Remove this in major version 5.
		.def( "__getattr__", &parameterisedGetAttr )
		.def( "userData", (CompoundObjectPtr (Parameterised::*)())&Parameterised::userData )
	;

}

} // namespace IECore
