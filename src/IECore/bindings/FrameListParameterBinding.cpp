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

		static FrameListParameter::ObjectPresetsMap makePresets( const dict &d )
		{
			FrameListParameter::ObjectPresetsMap p;
			boost::python::list keys = d.keys();
			boost::python::list values = d.values();
			for( int i = 0; i<keys.attr( "__len__" )(); i++ )
			{			
				const std::string key = extract<std::string>( keys[i] )();
				extract<std::string> de( values[i] );
				if( de.check() )
				{
					p.insert( FrameListParameter::ObjectPresetsMap::value_type( key, new StringData( de() ) ) );
				}
				else
				{				
					extract<StringData *> de( values[i] );
					if( de.check() )
					{
						p.insert( FrameListParameter::ObjectPresetsMap::value_type( key, de() ) );
					}				
					else	
					{
						extract<FrameList *> de( values[i] );
						if( de.check() )
						{
							p.insert( FrameListParameter::ObjectPresetsMap::value_type( key, new StringData( de()->asString() ) ) );
						}
						else
						{
							throw InvalidArgumentException( "FrameListParameter: Invalid preset value" );
						}
					}
				}
			}
			return p;
		}

	public :

		FrameListParameterWrap( PyObject *self, const std::string &n, const std::string &d, object dv, bool allowEmptyList = true, const dict &p = dict(), bool po = false, CompoundObjectPtr ud = 0 )	
			:	FrameListParameter( n, d, makeDefault( dv ), allowEmptyList, makePresets( p ), po, ud ), Wrapper< FrameListParameter >( self, this ) {};
		
		FrameListParameterWrap( PyObject *self, const std::string &n, const std::string &d, object dv, CompoundObjectPtr ud )	
			:	FrameListParameter( n, d, makeDefault( dv ), true, FrameListParameter::ObjectPresetsMap(), false, ud ), Wrapper< FrameListParameter >( self, this ) {};

		IE_COREPYTHON_PARAMETERWRAPPERFNS( FrameListParameter );
};


void bindFrameListParameter()
{	
	typedef class_< FrameListParameter, FrameListParameterWrap::Ptr, boost::noncopyable, bases< StringParameter > > FrameListParameterPyClass;
	FrameListParameterPyClass ( "FrameListParameter", no_init )
		.def( init< const std::string &, const std::string &, object, boost::python::optional< bool, const dict &, bool, CompoundObjectPtr > >( args( "name", "description", "defaultValue", "allowEmptyList", "presets", "presetsOnly", "userData") ) )
		.def( init< const std::string &, const std::string &, object, CompoundObjectPtr >( args( "name", "description", "defaultValue", "userData") ) )	
		.def( "getFrameListValue", &FrameListParameter::getFrameListValue )	
		.def( "setFrameListValue", &FrameListParameter::setFrameListValue )	
		.IE_COREPYTHON_DEFPARAMETERWRAPPERFNS( FrameListParameter )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( FrameListParameter )
	;
	
	INTRUSIVE_PTR_PATCH( FrameListParameter, FrameListParameterPyClass );
	implicitly_convertible<FrameListParameterWrap::Ptr, FrameListParameterPtr>();
	implicitly_convertible<FrameListParameterPtr, StringParameterPtr>();
}

}
