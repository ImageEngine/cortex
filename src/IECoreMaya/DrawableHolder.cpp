//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/SimpleTypedData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreGL/Scene.h"
#include "IECoreGL/Renderer.h"
#include "IECoreGL/Camera.h"

#include "IECoreMaya/DrawableHolder.h"
#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/Convert.h"

#include "maya/MFnNumericAttribute.h"
#include "maya/MFnNumericData.h"

using namespace IECoreMaya;

const MTypeId DrawableHolder::id = DrawableHolderId;
const MString DrawableHolder::typeName = "ieDrawable";

MObject DrawableHolder::aDraw;

DrawableHolder::DrawableHolder()
	:	m_scene( 0 )
{
}

DrawableHolder::~DrawableHolder()
{
}

void *DrawableHolder::creator()
{
	return new DrawableHolder;
}

MStatus DrawableHolder::initialize()
{
	MStatus s = inheritAttributesFrom( ParameterisedHolderSurfaceShape::typeName );
	assert( s );
	
	MFnNumericAttribute nAttr;
	
	aDraw = nAttr.create( "draw", "draw", MFnNumericData::kBoolean, 1, &s );
	assert( s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );
	
	s = addAttribute( aDraw );
	assert( s );
	
	return s;
}

bool DrawableHolder::isBounded() const
{
	return true;
}

MBoundingBox DrawableHolder::boundingBox() const
{
	IECoreGL::ConstScenePtr s = ((DrawableHolder *)this)->scene();
	if( s )
	{
		Imath::Box3f b = s->bound();
		return IECore::convert<MBoundingBox>( b );
	}
	return MBoundingBox();
}

MStatus DrawableHolder::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	if( std::string( plug.partialName().substring( 0, 4 ).asChar() ) == ParameterisedHolderSurfaceShape::g_attributeNamePrefix )
	{
		m_scene = 0;
	}
	return ParameterisedHolderSurfaceShape::setDependentsDirty( plug, plugArray );
}

IECoreGL::ConstScenePtr DrawableHolder::scene()
{
	if( m_scene  )
	{
		return m_scene;
	}
	
	m_scene = 0;
	IECore::RunTimeTypedPtr drawable = getParameterised();
	IECore::ParameterisedInterface *drawableInterface = dynamic_cast<IECore::ParameterisedInterface *>( drawable.get() );
	if( drawableInterface )
	{
		setParameterisedValues();
		try
		{
			IECoreGL::RendererPtr renderer = new IECoreGL::Renderer;
			renderer->setOption( "gl:mode", new IECore::StringData( "deferred" ) );
			renderer->worldBegin();
			
				{
					IECorePython::ScopedGILLock gilLock;
					boost::python::object pythonDrawable( drawable );
					pythonDrawable.attr( "draw" )( renderer );
				}
				
			renderer->worldEnd();
			
			m_scene = renderer->scene();
			m_scene->setCamera( 0 );
			
		}
		catch( boost::python::error_already_set )
		{
			IECorePython::ScopedGILLock gilLock;
			PyErr_Print();
		}
		catch( IECore::Exception &e )
		{
			IECore::msg( IECore::Msg::Error, "DrawableHolder::scene", e.what() );
		}
		catch( std::exception &e )
		{
			IECore::msg( IECore::Msg::Error, "DrawableHolder::scene", e.what() );
		}
		catch( ... )
		{
			IECore::msg( IECore::Msg::Error, "DrawableHolder::scene", "Exception thrown in MapGenerator::draw" );
		}
	}
	
	return m_scene;

}
