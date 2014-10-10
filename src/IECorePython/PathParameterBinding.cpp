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

#include "IECorePython/ParameterBinding.h"
#include "IECore/PathParameter.h"
#include "IECore/CompoundObject.h"
#include "IECorePython/Wrapper.h"
#include "IECorePython/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

class PathParameterWrap : public PathParameter, public Wrapper<PathParameter>
{
	public :

		PathParameterWrap( PyObject *self, const std::string &n, const std::string &d, const std::string &dv, bool ae,
			PathParameter::CheckType c, object &p, bool po, CompoundObjectPtr ud )
			:	PathParameter( n, d, dv, ae, c, parameterPresets<PathParameter::PresetsContainer>( p ), po, ud ), Wrapper<PathParameter>( self, this ) {};

		IECOREPYTHON_PARAMETERWRAPPERFNS( PathParameter );

};

void bindPathParameter()
{
	using boost::python::arg;

	RunTimeTypedClass<PathParameter, PathParameterWrap> pathParamClass;
	{
		// define enum before functions.
		scope varScope = pathParamClass;
		enum_<PathParameter::CheckType>( "CheckType" )
			.value( "DontCare", PathParameter::DontCare )
			.value( "MustExist", PathParameter::MustExist )
			.value( "MustNotExist", PathParameter::MustNotExist )
		;
	}
	pathParamClass
		.def(
			init<const std::string &, const std::string &, const std::string &, bool, PathParameter::CheckType, object &, bool, CompoundObjectPtr>
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "defaultValue" ) = std::string( "" ),
					arg( "allowEmptyString" ) = true,
					arg( "check" ) = PathParameter::DontCare,
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false,
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.IECOREPYTHON_DEFPARAMETERWRAPPERFNS( PathParameter )
		.add_property( "mustExist", &PathParameter::mustExist )
		.add_property( "mustNotExist", &PathParameter::mustNotExist )
		.add_property( "allowEmptyString", &PathParameter::allowEmptyString )
	;
}

} // namespace IECorePython
