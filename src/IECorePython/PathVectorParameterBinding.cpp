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
#include "IECore/PathVectorParameter.h"
#include "IECore/CompoundObject.h"
#include "IECorePython/PathVectorParameterBinding.h"
#include "IECorePython/Wrapper.h"
#include "IECorePython/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

class PathVectorParameterWrap : public PathVectorParameter, public Wrapper<PathVectorParameter>
{
	public:

		PathVectorParameterWrap( PyObject *self, const std::string &n, const std::string &d, ConstStringVectorDataPtr dv, bool ae = true,
			PathVectorParameter::CheckType c = PathVectorParameter::DontCare, const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0 )
			:	PathVectorParameter( n, d, dv->copy(), ae, c, parameterPresets<PathVectorParameter::ObjectPresetsContainer>( p ), po, ud ), Wrapper<PathVectorParameter>( self, this )
		{
		}

		IECOREPYTHON_PARAMETERWRAPPERFNS( PathVectorParameter );

};

void bindPathVectorParameter()
{
	using boost::python::arg;

	RunTimeTypedClass<PathVectorParameter, PathVectorParameterWrap> pathVectorParamClass;
	{
		// define enum before functions.
		scope varScope = pathVectorParamClass;
		enum_<PathVectorParameter::CheckType>( "CheckType" )
			.value( "DontCare", PathVectorParameter::DontCare )
			.value( "MustExist", PathVectorParameter::MustExist )
			.value( "MustNotExist", PathVectorParameter::MustNotExist )
		;
	}
	pathVectorParamClass
		.def(
			init< const std::string &, const std::string &, ConstStringVectorDataPtr, boost::python::optional<bool, PathVectorParameter::CheckType, const object &, bool, CompoundObjectPtr > >
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "defaultValue" ),
					arg( "allowEmptyList" ) = true,
					arg( "check" ) = PathVectorParameter::DontCare,
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false ,
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.IECOREPYTHON_DEFPARAMETERWRAPPERFNS( PathVectorParameter )
		.add_property( "mustExist", &PathVectorParameter::mustExist )
		.add_property( "mustNotExist", &PathVectorParameter::mustNotExist )
		.add_property( "allowEmptyList", &PathVectorParameter::allowEmptyList )
	;
}

} // namespace IECorePython
