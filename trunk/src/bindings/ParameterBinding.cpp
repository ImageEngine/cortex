//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <boost/python.hpp>

#include "IECore/Parameter.h"
#include "IECore/bindings/ParameterBinding.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/Wrapper.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;

namespace IECore
{

boost::python::tuple valueValid2( const Parameter &that )
{
	std::string reason;
	bool valid = that.valueValid( &reason );
	return boost::python::make_tuple( valid, reason );
}

static ObjectPtr defaultValue( Parameter &that )
{
	return that.defaultValue()->copy();
}

static dict presets( Parameter &that )
{
	dict result;
	const Parameter::PresetsMap &p = that.presets();
	for( Parameter::PresetsMap::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result[it->first] = it->second->copy();
	}
	return result;
}

Parameter::PresetsMap parameterPresetsFromDict( const dict &presets )
{
	Parameter::PresetsMap p;
	boost::python::list keys = presets.keys();
	boost::python::list values = presets.values();
	for( int i = 0; i<keys.attr( "__len__" )(); i++ )
	{
		p.insert( Parameter::PresetsMap::value_type( extract<string>( keys[i] )(), extract<ObjectPtr>( values[i] )() ) );
	}
	return p;
}

class ParameterWrap : public Parameter, public Wrapper<Parameter>
{
	public:

		ParameterWrap( PyObject *self, const std::string &name, const std::string &description, ObjectPtr defaultValue,
						const dict & presets = dict(), bool presetsOnly = false, CompoundObjectPtr userData = 0 ) :
				Parameter( name, description, defaultValue, parameterPresetsFromDict( presets ), presetsOnly, userData ), Wrapper< Parameter >( self, this ) {};

		ParameterWrap( PyObject *self, const std::string &name, const std::string &description, ObjectPtr defaultValue, CompoundObjectPtr userData ) :
				Parameter( name, description, defaultValue, Parameter::PresetsMap(), false, userData ), Wrapper< Parameter >( self, this ) {};

		IE_COREPYTHON_PARAMETERWRAPPERFNS( Parameter );

};

IE_CORE_DECLAREPTR( ParameterWrap );

void bindParameter()
{
	typedef class_< Parameter, ParameterWrapPtr, boost::noncopyable, bases<RunTimeTyped> > ParameterPyClass;
	ParameterPyClass( "Parameter", no_init )
		.def( init< const std::string &, const std::string &, ObjectPtr, optional<const dict &, bool, CompoundObjectPtr > >( args( "name", "description", "defaultValue", "presets", "presetsOnly", "userData") ) )
		.def( init< const std::string &, const std::string &, ObjectPtr, CompoundObjectPtr >( args( "name", "description", "defaultValue", "userData") ) )
		.add_property( "name", make_function( &Parameter::name, return_value_policy<copy_const_reference>() ) )
		.add_property( "description", make_function( &Parameter::description, return_value_policy<copy_const_reference>() ) )
		.add_property( "defaultValue", &defaultValue )
		.def( "setValue", (void (Parameter::*)( ObjectPtr ))&Parameter::setValue )
		.def( "setValue", (void (Parameter::*)( const std::string & ))&Parameter::setValue )
		.def( "setValidatedValue", &Parameter::setValidatedValue )
		.def( "getValue", (ObjectPtr (Parameter::*)())&Parameter::getValue )
		.def( "getValidatedValue", (ObjectPtr (Parameter::*)())&Parameter::getValidatedValue )
		.def( "getCurrentPresetName", &Parameter::getCurrentPresetName )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( Parameter )
		.def( "validate", (void (Parameter::*)() const)&Parameter::validate )
		.def( "validate", (void (Parameter::*)( ConstObjectPtr ) const)&Parameter::validate )
		.add_property( "presetsOnly", &Parameter::presetsOnly )
		.def( "presets", &presets, "Returns a dictionary containing presets for the parameter." )
		.def( "userData", (CompoundObjectPtr (Parameter::*)())&Parameter::userData )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(Parameter)
	;
	
	WrapperToPython<ParameterPtr>();
	INTRUSIVE_PTR_PATCH( Parameter, ParameterPyClass );
	implicitly_convertible<ParameterPtr, RunTimeTypedPtr>();
	implicitly_convertible<ParameterPtr, ConstParameterPtr>();

}

} // namespace IECore
