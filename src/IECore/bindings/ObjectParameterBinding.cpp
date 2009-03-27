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
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECore/ObjectParameter.h"
#include "IECore/Object.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/ObjectParameterBinding.h"
#include "IECore/bindings/ParameterBinding.h"

#include <vector>
#include <algorithm>

using namespace std;
using namespace boost;
using namespace boost::python;

namespace IECore
{

static ObjectParameterPtr objectParameterConstructor( const std::string &name, const std::string &description, ObjectPtr defaultValue, TypeId type, const dict &presets, bool presetsOnly, CompoundObjectPtr userData )
{
	return new ObjectParameter( name, description, defaultValue, type, parameterPresetsFromDict( presets ), presetsOnly, userData );
}

static ObjectParameterPtr objectParameterConstructor2( const std::string &name, const std::string &description, ObjectPtr defaultValue, const boost::python::list &types, const dict &presets, bool presetsOnly, CompoundObjectPtr userData )
{
	vector<TypeId> tv;
	boost::python::container_utils::extend_container( tv, types );
	ObjectParameter::TypeIdSet t;
	copy( tv.begin(), tv.end(), insert_iterator<ObjectParameter::TypeIdSet>( t, t.begin() ) );
	return new ObjectParameter( name, description, defaultValue, t, parameterPresetsFromDict( presets ), presetsOnly, userData );
}

static boost::python::list validTypes( ObjectParameter &o )
{
	boost::python::list result;
	for( ObjectParameter::TypeIdSet::const_iterator it=o.validTypes().begin(); it!=o.validTypes().end(); it++ )
	{
		result.append( *it );
	}
	return result;
}

void bindObjectParameter()
{

	typedef class_< ObjectParameter, ObjectParameterPtr, boost::noncopyable, bases<Parameter> > ObjectParameterPyClass;
	ObjectParameterPyClass( "ObjectParameter", no_init )
		.def( "__init__", make_constructor( &objectParameterConstructor, default_call_policies(), ( boost::python::arg_( "name" ), boost::python::arg_( "description" ), boost::python::arg_( "defaultValue" ), boost::python::arg_( "type" ), boost::python::arg_( "presets" ) = dict(), boost::python::arg_( "presetsOnly" ) = false, boost::python::arg_( "userData" ) = object() ) ) )
		.def( "__init__", make_constructor( &objectParameterConstructor2, default_call_policies(), ( boost::python::arg_( "name" ), boost::python::arg_( "description" ), boost::python::arg_( "defaultValue" ), boost::python::arg_( "types" ), boost::python::arg_( "presets" ) = dict(), boost::python::arg_( "presetsOnly" ) = false, boost::python::arg_( "userData" ) = object() ) ) )
		.def( "validTypes", &validTypes )
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( ObjectParameter )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(ObjectParameter)
	;
	INTRUSIVE_PTR_PATCH( ObjectParameter, ObjectParameterPyClass );
	implicitly_convertible<ObjectParameterPtr, ParameterPtr>();

}

} // namespace IECore
