//////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/IECoreBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/FrameListBinding.h"
#include "IECorePython/ParameterBinding.h"
#include "IECorePython/Wrapper.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
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

		FrameListParameterWrap( PyObject *self, const std::string &n, const std::string &d, object dv = object( std::string("") ), bool allowEmptyList = true, const object &p = boost::python::tuple(), bool po = false, CompoundObjectPtr ud = 0 )
			:	FrameListParameter( n, d, makeDefault( dv ), allowEmptyList, parameterPresets<FrameListParameter::ObjectPresetsContainer>( p ), po, ud ), Wrapper< FrameListParameter >( self, this ) {};

		IECOREPYTHON_PARAMETERWRAPPERFNS( FrameListParameter );
};


void bindFrameListParameter()
{
	using boost::python::arg;

	FrameListPtr (FrameListParameter::*getFrameListValueInternalData)() const = &FrameListParameter::getFrameListValue;
	FrameListPtr (FrameListParameter::*getFrameListValueStringData)( const StringData *value ) const = &FrameListParameter::getFrameListValue;
	
	RunTimeTypedClass<FrameListParameter, FrameListParameterWrap>()
		.def(
			init< const std::string &, const std::string &, boost::python::optional< object, bool, const dict &, bool, CompoundObjectPtr > >
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "defaultValue" ) = object( std::string( "" ) ),
					arg( "allowEmptyList" ) = true,
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false ,
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.def( "getFrameListValue", getFrameListValueInternalData )
		.def( "getFrameListValue", getFrameListValueStringData )
		.def( "setFrameListValue", &FrameListParameter::setFrameListValue )
		.IECOREPYTHON_DEFPARAMETERWRAPPERFNS( FrameListParameter )
	;

}

}
