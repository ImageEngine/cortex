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

#include "IECore/DirNameParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/ParameterBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;

namespace IECore
{

static DirNameParameterPtr dirNameParameterConstructor( const std::string &name, const std::string &description,
	const std::string &defaultValue, bool allowEmptyString, PathParameter::CheckType check, const object &presets, bool presetsOnly, object userData )
{
 	DirNameParameter::PresetsContainer p = parameterPresets<DirNameParameter::PresetsContainer>( presets );

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
				DirNameParameterPtr res;
				return res;
			}
		}
	}
	return new DirNameParameter( name, description, defaultValue, allowEmptyString, check, p, presetsOnly, ptrUserData );
}

void bindDirNameParameter()
{

	typedef class_<DirNameParameter, DirNameParameterPtr, boost::noncopyable, bases<PathParameter> > DirNameParameterPyClass;
	DirNameParameterPyClass( "DirNameParameter", no_init )
		.def( "__init__", make_constructor( &dirNameParameterConstructor, default_call_policies(), ( boost::python::arg_( "name" ), boost::python::arg_( "description" ), boost::python::arg_( "defaultValue" ) = string( "" ), boost::python::arg_( "allowEmptyString" ) = true, boost::python::arg_( "check" ) = PathParameter::DontCare, boost::python::arg_( "presets" ) = boost::python::tuple(), boost::python::arg_( "presetsOnly") = false, boost::python::arg_( "userData" ) = object() ) ) )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( DirNameParameter )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( DirNameParameter )
	;
	INTRUSIVE_PTR_PATCH( DirNameParameter, DirNameParameterPyClass );
	implicitly_convertible<DirNameParameterPtr, PathParameterPtr>();

}


} // namespace IECore
