//////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//   * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//   * Neither the name of Image Engine Design nor the names of any
//    other contributors to this software may be used to endorse or
//    promote products derived from this software without specific prior
//    written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "boost/python.hpp"
#include <iostream>

#include "IECore/FrameListParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Exception.h"

#include "IECore/bindings/IECoreBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/FrameListBinding.h"
#include "IECore/bindings/ParameterBinding.h"

using namespace boost::python;

namespace IECore 
{

class FrameListParameterWrap : public FrameListParameter, public Wrapper< FrameListParameter >
{
	public:
	
		IE_CORE_DECLAREMEMBERPTR( FrameListParameterWrap );
	
	protected:

		/// Allow construction from either a string, StringData, or a FrameList
		static StringDataPtr makeDefault( object defaultValue )
		{
			extract<std::string> de( defaultValue );
			if( de.check() )
			{
				return new StringData( de() );
			}
			else			
			{
				extract<StringData *> de( defaultValue );
				if( de.check() )
				{
					return de();
				}
				else
				{
					extract<FrameList *> de( defaultValue );
					if( de.check() )
					{
						return new StringData( de()->asString() );
					}
					else
					{
						throw InvalidArgumentException( "FrameListParameter: Invalid default value" );
					}
				}
			}
		}

	public :

		FrameListParameterWrap( PyObject *self, const std::string &n, const std::string &d, object dv, bool allowEmptyList = true, const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0 )	
			:	FrameListParameter( n, d, makeDefault( dv ), allowEmptyList, parameterPresets<FrameListParameter::ObjectPresetsContainer>( p ), po, ud ), Wrapper< FrameListParameter >( self, this ) {};
		
		IE_COREPYTHON_PARAMETERWRAPPERFNS( FrameListParameter );
};


void bindFrameListParameter()
{	
	using boost::python::arg;

	typedef class_< FrameListParameter, FrameListParameterWrap::Ptr, boost::noncopyable, bases< StringParameter > > FrameListParameterPyClass;
	FrameListParameterPyClass ( "FrameListParameter", no_init )
		.def( 
			init< const std::string &, const std::string &, object, boost::python::optional< bool, const dict &, bool, CompoundObjectPtr > >
			( 
				( 
					arg( "name" ), 
					arg( "description" ), 
					arg( "defaultValue" ),
					arg( "allowEmptyList" ) = true,
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false , 
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				) 
			)
		)
		.def( "getFrameListValue", &FrameListParameter::getFrameListValue )	
		.def( "setFrameListValue", &FrameListParameter::setFrameListValue )	
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( FrameListParameter )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( FrameListParameter )
	;

	WrapperToPython< FrameListParameterWrap::Ptr >();	
	INTRUSIVE_PTR_PATCH( FrameListParameter, FrameListParameterPyClass );
	implicitly_convertible<FrameListParameterWrap::Ptr, FrameListParameterPtr>();
	implicitly_convertible<FrameListParameterPtr, StringParameterPtr>();
}

}
