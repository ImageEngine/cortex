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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECore/Renderer.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECore/bindings/ParameterisedProceduralBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/Wrapper.h"

using namespace boost::python;

namespace IECore
{

class ParameterisedProceduralWrap : public ParameterisedProcedural, public Wrapper<ParameterisedProcedural>
{

	public :

		ParameterisedProceduralWrap( PyObject *self, const std::string &description="" )
			: ParameterisedProcedural( description ), Wrapper<ParameterisedProcedural>( self, this )
		{
		}
		
		virtual void doRenderState( RendererPtr renderer, ConstCompoundObjectPtr args ) const
		{
			try
			{
				override o = this->get_override( "doRenderState" );
				if( o )
				{
					o( renderer, boost::const_pointer_cast<CompoundObject>( args ) );
				}
				else
				{
					ParameterisedProcedural::doRenderState( renderer, args );
				}
			}
			catch( error_already_set )
			{
				PyErr_Print();
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "ParameterisedProceduralWrap::doRenderState", e.what() );
			}
			catch( ... )
			{
				msg( Msg::Error, "ParameterisedProceduralWrap::doRenderState", "Caught unknown exception" );
			}

		}

		virtual Imath::Box3f doBound( ConstCompoundObjectPtr args ) const
		{
			try
			{
				override o = this->get_override( "doBound" );
				if( o )
				{
					return o( boost::const_pointer_cast<CompoundObject>( args ) );
				}
				else
				{
					msg( Msg::Error, "ParameterisedProceduralWrap::doBound", "doBound() python method not defined" );
				}
			}
			catch( error_already_set )
			{
				PyErr_Print();
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "ParameterisedProceduralWrap::doBound", e.what() );
			}
			catch( ... )
			{
				msg( Msg::Error, "ParameterisedProceduralWrap::doBound", "Caught unknown exception" );
			}
			return Imath::Box3f(); // empty
		}

		virtual void doRender( RendererPtr r, ConstCompoundObjectPtr args ) const
		{
			// ideally we might not do any exception handling here, and always leave it to the host.
			// but in our case the host is mainly 3delight and that does no exception handling at all.
			try
			{
				override o = this->get_override( "doRender" );
				if( o )
				{
					o( r, boost::const_pointer_cast<CompoundObject>( args ) );
				}
				else
				{
					msg( Msg::Error, "ParameterisedProceduralWrap::doRender", "doRender() python method not defined" );
				}
			}
			catch( error_already_set )
			{
				PyErr_Print();
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "ParameterisedProceduralWrap::doRender", e.what() );
			}
			catch( ... )
			{
				msg( Msg::Error, "ParameterisedProceduralWrap::doRender", "Caught unknown exception" );
			}
		}

		IE_COREPYTHON_RUNTIMETYPEDWRAPPERFNS( ParameterisedProcedural );

};

IE_CORE_DECLAREPTR( ParameterisedProceduralWrap );

static ParameterPtr parameterisedProceduralGetItem( ParameterisedProcedural &o, const std::string &n )
{
	ParameterPtr p = o.parameters()->parameter<Parameter>( n );
	if( !p )
	{
		throw Exception( std::string("Parameter ") + n + " doesn't exist" );
	}
	return p;
}

void bindParameterisedProcedural()
{

	RunTimeTypedClass<ParameterisedProcedural, ParameterisedProceduralWrapPtr>()
		.def( init<>() )
		.def( init< const std::string >( arg( "description") ) )
		.add_property( "description", make_function( &ParameterisedProcedural::description, return_value_policy<copy_const_reference>() ) )
		.def( "parameters", (CompoundParameterPtr (ParameterisedProcedural::*)())&ParameterisedProcedural::parameters )
		.def( "render", (void (ParameterisedProcedural::*)( RendererPtr ) const )&ParameterisedProcedural::render )
		.def( "render", (void (ParameterisedProcedural::*)( RendererPtr, bool, bool, bool, bool ) const )&ParameterisedProcedural::render, ( arg( "renderer" ), arg( "inAttributeBlock" ) = true, arg( "withState" ) = true, arg( "withGeometry" ) = true, arg( "immediateGeometry" ) = false ) )
		.def( "__getitem__", &parameterisedProceduralGetItem )
	;

}

} // namespace IECore
