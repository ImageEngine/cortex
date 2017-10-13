//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2013, Image Engine Design Inc. All rights reserved.
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
#include "IECore/AttributeBlock.h"

using namespace IECore;

IE_CORE_DEFINEABSTRACTOBJECTTYPEDESCRIPTION( ParameterisedProcedural );

static IndexedIO::EntryID g_parametersEntry("parameters");
const unsigned int ParameterisedProcedural::m_ioVersion = 0;

ParameterisedProcedural::ParameterisedProcedural( const std::string &description )
	:	m_description( description ), m_parameters( new CompoundParameter )
{
}


ParameterisedProcedural::~ParameterisedProcedural()
{
}

const std::string &ParameterisedProcedural::description() const
{
	return m_description;
}


void ParameterisedProcedural::copyFrom( const Object *other, CopyContext *context )
{
	VisibleRenderable::copyFrom( other, context );
	const ParameterisedProcedural *tOther = static_cast<const ParameterisedProcedural *>( other );
	m_parameters->setValue( tOther->m_parameters->getValue()->copy() );
}

void ParameterisedProcedural::save( SaveContext *context ) const
{
	VisibleRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_parameters->getValue(), container.get(), g_parametersEntry );
}

void ParameterisedProcedural::load( LoadContextPtr context )
{
	VisibleRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	m_parameters->setValue( context->load<Object>( container.get(), g_parametersEntry ) );
}

bool ParameterisedProcedural::isEqualTo( const Object *other ) const
{
	if( !VisibleRenderable::isEqualTo( other ) )
	{
		return false;
	}

	const ParameterisedProcedural *tOther = static_cast<const ParameterisedProcedural *>( other );
	return m_parameters->getValue()->isEqualTo( tOther->m_parameters->getValue() );
}

void ParameterisedProcedural::memoryUsage( Object::MemoryAccumulator &a ) const
{
	VisibleRenderable::memoryUsage( a );
	a.accumulate( m_parameters->getValue() );
}

void ParameterisedProcedural::hash( MurmurHash &h ) const
{
	VisibleRenderable::hash( h );
	m_parameters->getValue()->hash( h );
}

class ParameterisedProcedural::Forwarder : public Renderer::Procedural
{
	public :

		Forwarder( ConstParameterisedProceduralPtr p, ConstCompoundObjectPtr a )
			:	parameterisedProcedural( p ), validatedArgs( a )
		{
		}

		Imath::Box3f bound() const override
		{
			return parameterisedProcedural->doBound( validatedArgs );
		}

		void render( Renderer *renderer ) const override
		{
			parameterisedProcedural->doRender( renderer, validatedArgs );
		}

		/// returns a hash of the parameters, so the renderer can instance procedurals with
		/// identical parameters
		MurmurHash hash() const override
		{
			MurmurHash h;
			parameterisedProcedural->hash( h );
			return h;
		}

		ConstParameterisedProceduralPtr parameterisedProcedural;
		ConstCompoundObjectPtr validatedArgs;
};

void ParameterisedProcedural::render( Renderer *renderer ) const
{
	render( renderer, true, true, true, false );
}

void ParameterisedProcedural::render( Renderer *renderer, bool inAttributeBlock, bool withState, bool withGeometry, bool immediateGeometry ) const
{
	ConstCompoundObjectPtr validatedArgs = parameters()->getTypedValidatedValue<CompoundObject>();

	AttributeBlock attributeBlock( renderer, inAttributeBlock );

		if( withState )
		{
			doRenderState( renderer, validatedArgs );
		}

		if( withGeometry )
		{

			if( immediateGeometry )
			{
				doRender( renderer, validatedArgs  );
			}
			else
			{
				renderer->procedural( new Forwarder( this, validatedArgs ) );
			}

		}

}

Imath::Box3f ParameterisedProcedural::bound() const
{
	ConstCompoundObjectPtr args = parameters()->getTypedValidatedValue<CompoundObject>();
	return doBound( args );
}

CompoundParameter *ParameterisedProcedural::parameters()
{
	return m_parameters.get();
}

const CompoundParameter *ParameterisedProcedural::parameters() const
{
	return m_parameters.get();
}

void ParameterisedProcedural::doRenderState( RendererPtr renderer, ConstCompoundObjectPtr args ) const
{
}
