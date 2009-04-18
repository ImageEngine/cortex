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

#include "IECore/NumericParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/ParameterBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;

namespace IECore
{

template<class T>
static ParameterPtr numericConstructor( const std::string &name, const std::string &description, T defaultValue, T minValue, T maxValue,
	const object &presets, bool presetsOnly, object userData )
{
	typename NumericParameter<T>::PresetsContainer p = parameterPresets<typename NumericParameter<T>::PresetsContainer>( presets );
	
	// get the optional userData parameter.
	ConstCompoundObjectPtr ptrUserData = 0;
	if (userData != object()) {
		extract<CompoundObjectPtr> elem(userData);
		// try if elem is an exact CompoundObjectPtr
		if (elem.check()) {
			ptrUserData = elem();
		} else {
			// now try for ConstCompoundObjectPtr
			extract<ConstCompoundObjectPtr> elem(userData);
			if (elem.check()) {
				ptrUserData = elem();
			} else {
			   	PyErr_SetString(PyExc_TypeError, "Parameter userData is not an instance of CompoundObject!");
			  	throw_error_already_set();
				ParameterPtr res;
				return res;
			}
		}
	}
	return new NumericParameter<T>( name, description, defaultValue, minValue, maxValue, p, presetsOnly, ptrUserData );
}

template<typename T>
static void bindNumericParameter( const char *name )
{
	typedef class_< NumericParameter<T>, typename NumericParameter<T>::Ptr, boost::noncopyable, bases<Parameter> > NumericParameterPyClass;
	NumericParameterPyClass( name, no_init )
		.def( "__init__", make_constructor( &numericConstructor<T>, default_call_policies(), ( boost::python::arg_( "name" ), boost::python::arg_( "description" ), boost::python::arg_( "defaultValue" ) = T(), boost::python::arg_( "minValue" ) = Imath::limits<T>::min(), boost::python::arg_( "maxValue" ) = Imath::limits<T>::max(), boost::python::arg_( "presets" ) = boost::python::tuple(), boost::python::arg_( "presetsOnly" ) = false, boost::python::arg_( "userData" ) = object() ) ) )
		.add_property( "numericDefaultValue", &NumericParameter<T>::numericDefaultValue  )
		.def( "getNumericValue", &NumericParameter<T>::getNumericValue  )
		.def( "setNumericValue", &NumericParameter<T>::setNumericValue  )
		.def( "getTypedValue", &NumericParameter<T>::getNumericValue )		// added for consistency
		.def( "setTypedValue", &NumericParameter<T>::setNumericValue )		// added for consistency
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( NumericParameter<T> )
		.def( "hasMinValue", &NumericParameter<T>::hasMinValue )
		.def( "hasMaxValue", &NumericParameter<T>::hasMaxValue )		
		.add_property( "minValue", &NumericParameter<T>::minValue )
		.add_property( "maxValue", &NumericParameter<T>::maxValue )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( NumericParameter<T> )
	;
	INTRUSIVE_PTR_PATCH( NumericParameter<T>, typename NumericParameterPyClass );
	implicitly_convertible<typename NumericParameter<T>::Ptr, ParameterPtr>();
}

void bindNumericParameter()
{
	bindNumericParameter<int>( "IntParameter" );
	bindNumericParameter<float>( "FloatParameter" );
	bindNumericParameter<double>( "DoubleParameter" );
}

};
