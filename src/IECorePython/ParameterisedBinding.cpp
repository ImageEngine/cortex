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
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/Wrapper.h"

using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

class ParameterisedWrap : public Parameterised, public Wrapper<Parameterised>
{
	public :
		ParameterisedWrap( PyObject *self, const std::string &description ) : Parameterised( description ), Wrapper<Parameterised>( self, this ) {};
		ParameterisedWrap( PyObject *self, const std::string &description, CompoundParameterPtr compoundParameter ) : Parameterised( description, compoundParameter ), Wrapper<Parameterised>( self, this ) {};
};
IE_CORE_DECLAREPTR( ParameterisedWrap );

/// \todo We should consider deprecating this accessor and forcing all parameter access to go through
/// object.parameters()["name"]. Although it is convenient, it's also confusing that it only supports
/// a tiny subset of what can be done with parameters(), and it adds another way of doing the same thing
/// which can also cause confusion.
static ParameterPtr parameterisedGetItem( Parameterised &o, const std::string &n )
{
	ParameterPtr p = o.parameters()->parameter<Parameter>( n );
	if( !p )
	{
		throw Exception( std::string("Parameter ") + n + " doesn't exist" );
	}
	return p;
}

void bindParameterised()
{
	using boost::python::arg;

	CompoundParameter *(Parameterised::*parameters)() = &Parameterised::parameters;
	CompoundObject *(Parameterised::*userData)() = &Parameterised::userData;

	RunTimeTypedClass<Parameterised, ParameterisedWrap>()
		.def( init< const std::string &>( ( arg( "description") ) ) )
		.def( init< const std::string &, CompoundParameterPtr >( ( arg( "description") , arg( "compoundParameter") ) ) )
		.add_property( "description", make_function( &Parameterised::description, return_value_policy<copy_const_reference>() ) )
		.def( "parameters", parameters, return_value_policy<CastToIntrusivePtr>() )
		.def( "__getitem__", &parameterisedGetItem )
		.def( "userData", userData, return_value_policy<CastToIntrusivePtr>() )
	;

}

} // namespace IECorePython
