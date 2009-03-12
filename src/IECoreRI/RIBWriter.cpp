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

#include "IECore/FileNameParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundData.h"
#include "IECore/Renderable.h"
#include "IECore/TypedParameter.h"

#include "IECoreRI/RIBWriter.h"
#include "IECoreRI/Renderer.h"

using namespace IECoreRI;
using namespace std;
using namespace boost;

const IECore::Writer::WriterDescription<RIBWriter> RIBWriter::g_writerDescription( "rib" );

RIBWriter::RIBWriter()
	:	IECore::Writer( "RIBWriter", "Writes Renderable objects in RIB format.", IECore::RenderableTypeId )
{
	constructParameters();
}

RIBWriter::RIBWriter( IECore::ObjectPtr object, const std::string &fileName )
	:	IECore::Writer( "RIBWriter", "Writes Renderable objects in RIB format.", IECore::RenderableTypeId )
{
	constructParameters();
	m_objectParameter->setValue( object );
	m_fileNameParameter->setTypedValue( fileName );
}

bool RIBWriter::canWrite( IECore::ConstObjectPtr object, const std::string &fileName )
{
	return object->isInstanceOf( IECore::Renderable::staticTypeId() );
}

IECore::BoolParameterPtr RIBWriter::worldBlockParameter()
{
	return m_worldBlockParameter;
}

IECore::ConstBoolParameterPtr RIBWriter::worldBlockParameter() const
{
	return m_worldBlockParameter;
}

void RIBWriter::doWrite()
{	
	RendererPtr renderer = new Renderer( fileName() );
	
	IECore::RenderablePtr renderable = static_pointer_cast<IECore::Renderable>( const_pointer_cast<IECore::Object>( object() ) );
	if( !m_worldBlockParameter->getTypedValue() )
	{
		renderable->render( renderer );
	}
	else
	{
		/// \todo When we have a Scene class or other Renderables which specify their own world block
		/// then we'll have to detect them and act appropriately.
		renderer->worldBegin();
			renderable->render( renderer );
		renderer->worldEnd();
	}
}

void RIBWriter::constructParameters()
{
	m_worldBlockParameter = new IECore::BoolParameter(
		"worldBlock",
		"If this is on, then a world block is emitted with the object within it, "
		"even if the object does not specify a world block itself.",
		false
	);
	parameters()->addParameter( m_worldBlockParameter );
}
