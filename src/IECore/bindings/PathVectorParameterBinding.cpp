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

#include "IECore/bindings/ParameterBinding.h"
#include "IECore/PathVectorParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/Wrapper.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace std;
using namespace boost;
using namespace boost::python;

namespace IECore
{

class PathVectorParameterWrap : public PathVectorParameter, public Wrapper<PathVectorParameter>
{
	public:
	
		PathVectorParameterWrap( PyObject *self, const std::string &n, const std::string &d, ConstStringVectorDataPtr dv, bool ae = true,
			PathVectorParameter::CheckType c = PathVectorParameter::DontCare, const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0 )	
			:	PathVectorParameter( n, d, dv->readable(), ae, c, parameterPresets<PathVectorParameter::PresetsContainer>( p ), po, ud ), Wrapper<PathVectorParameter>( self, this )
		{
		}
		
		IE_COREPYTHON_PARAMETERWRAPPERFNS( PathVectorParameter );

};
IE_CORE_DECLAREPTR( PathVectorParameterWrap );

void bindPathVectorParameter()
{
	using boost::python::arg;
	
	typedef class_<PathVectorParameter, PathVectorParameterWrapPtr, boost::noncopyable, bases<StringVectorParameter> > PathVectorParameterPyClass;
	PathVectorParameterPyClass pathVectorParamClass( "PathVectorParameter", no_init );
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
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( PathVectorParameter )
		.add_property( "mustExist", &PathVectorParameter::mustExist )
		.add_property( "mustNotExist", &PathVectorParameter::mustNotExist )
		.add_property( "allowEmptyList", &PathVectorParameter::allowEmptyList )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( PathVectorParameter )
	;	
	
	WrapperToPython<PathVectorParameterPtr>();
	
	INTRUSIVE_PTR_PATCH( PathVectorParameter, PathVectorParameterPyClass );
	implicitly_convertible<PathVectorParameterPtr, StringVectorParameterPtr>();

}

} // namespace IECore
