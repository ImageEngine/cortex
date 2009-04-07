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

#include "IECore/ParameterisedProcedural.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Renderer.h"

using namespace IECore;

IE_CORE_DEFINEABSTRACTOBJECTTYPEDESCRIPTION( ParameterisedProcedural );

const unsigned int ParameterisedProcedural::m_ioVersion = 0;

ParameterisedProcedural::ParameterisedProcedural()
	:	m_parameters( new CompoundParameter )
{
}

ParameterisedProcedural::~ParameterisedProcedural()
{
}
				
void ParameterisedProcedural::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	VisibleRenderable::copyFrom( other, context );
	const ParameterisedProcedural *tOther = static_cast<const ParameterisedProcedural *>( other.get() );
	m_parameters->setValue( tOther->m_parameters->getValue()->copy() );
}

void ParameterisedProcedural::save( SaveContext *context ) const
{
	VisibleRenderable::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_parameters->getValue(), container, "parameters" );
}

void ParameterisedProcedural::load( LoadContextPtr context )
{
	VisibleRenderable::load( context );
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	m_parameters->setValue( context->load<Object>( container, "parameters" ) );
}

bool ParameterisedProcedural::isEqualTo( ConstObjectPtr other ) const
{
	if( !VisibleRenderable::isEqualTo( other ) )
	{
		return false;
	}

	const ParameterisedProcedural *tOther = static_cast<const ParameterisedProcedural *>( other.get() );
	return m_parameters->getValue()->isEqualTo( tOther->m_parameters->getValue() );
}

void ParameterisedProcedural::memoryUsage( Object::MemoryAccumulator &a ) const
{
	VisibleRenderable::memoryUsage( a );
	a.accumulate( m_parameters->getValue() );
}

class ParameterisedProcedural::Forwarder : public Renderer::Procedural
{
	public :
		
		Forwarder( ConstParameterisedProceduralPtr p )
			:	parameterisedProcedural( p )
		{
		}
		
		virtual Imath::Box3f bound() const
		{
			return parameterisedProcedural->bound();
		}
		
		virtual void render( RendererPtr renderer ) const
		{
			ConstCompoundObjectPtr args = parameterisedProcedural->parameters()->getTypedValidatedValue<CompoundObject>();
			parameterisedProcedural->doRender( renderer, args );
		}
		
		ConstParameterisedProceduralPtr parameterisedProcedural;
};
				
void ParameterisedProcedural::render( RendererPtr renderer ) const
{
	renderer->procedural( new Forwarder( this ) );
}

Imath::Box3f ParameterisedProcedural::bound() const
{
	ConstCompoundObjectPtr args = parameters()->getTypedValidatedValue<CompoundObject>();
	return doBound( args );
}

CompoundParameterPtr ParameterisedProcedural::parameters()
{
	return m_parameters;
}

ConstCompoundParameterPtr ParameterisedProcedural::parameters() const
{
	return m_parameters;
}
