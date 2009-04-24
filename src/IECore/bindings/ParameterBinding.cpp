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

#include "IECore/Parameter.h"
#include "IECore/bindings/ParameterBinding.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/Wrapper.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;

namespace IECore
{

/// \todo If this were inline, then IE_COREPYTHON_DEFPARAMETERWRAPPERFNS could be more readily used from extension modules
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
	const Parameter::PresetsContainer &p = that.presets();
	for( Parameter::PresetsContainer::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result[it->first] = it->second->copy();
	}
	return result;
}

static boost::python::tuple presetNames( const Parameter &that )
{
	boost::python::list result;
	const Parameter::PresetsContainer &p = that.presets();
	for( Parameter::PresetsContainer::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.append( it->first );
	}
	return boost::python::tuple( result );
}

static boost::python::tuple presetValues( const Parameter &that )
{
	boost::python::list result;
	const Parameter::PresetsContainer &p = that.presets();
	for( Parameter::PresetsContainer::const_iterator it=p.begin(); it!=p.end(); it++ )
	{
		result.append( it->second->copy() );
	}
	return boost::python::tuple( result );
}

class ParameterWrap : public Parameter, public Wrapper<Parameter>
{
	public:

		ParameterWrap( PyObject *self, const std::string &name, const std::string &description, ObjectPtr defaultValue,
						const object &presets = boost::python::tuple(), bool presetsOnly = false, CompoundObjectPtr userData = 0 ) :
				Parameter( name, description, defaultValue, parameterPresets<Parameter::PresetsContainer>( presets ), presetsOnly, userData ), Wrapper< Parameter >( self, this ) {};

		ParameterWrap( PyObject *self, const std::string &name, const std::string &description, ObjectPtr defaultValue, CompoundObjectPtr userData ) :
				Parameter( name, description, defaultValue, Parameter::PresetsContainer(), false, userData ), Wrapper< Parameter >( self, this ) {};

		IE_COREPYTHON_PARAMETERWRAPPERFNS( Parameter );

};

IE_CORE_DECLAREPTR( ParameterWrap );

void bindParameter()
{
	using boost::python::arg;
	
	RunTimeTypedClass<Parameter, ParameterWrapPtr>()
		.def( 
			init< const std::string &, const std::string &, ObjectPtr, boost::python::optional<const boost::python::object &, bool, CompoundObjectPtr > >
			( 
				( 
					arg( "name" ), 
					arg( "description" ), 
					arg( "defaultValue" ),
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false , 
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				) 
			)
		)
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
		.def( "presetNames", &presetNames, "Returns a tuple containing the names of all presets for the parameter." )
		.def( "presetValues", &presetValues, "Returns a tuple containing the values of all presets for the parameter." )
		.def( "userData", (CompoundObjectPtr (Parameter::*)())&Parameter::userData )
	;
	
}

} // namespace IECore
