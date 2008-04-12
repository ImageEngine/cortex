//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <boost/python.hpp>

#include "IECoreGL/Renderer.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/BoxPrimitive.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Camera.h"

#include "IECoreMaya/ProceduralHolder.h"
#include "IECoreMaya/Convert.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "IECore/VectorOps.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "maya/MFnNumericAttribute.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreMaya;
using namespace boost;

MTypeId ProceduralHolder::id = ProceduralHolderId;
MObject ProceduralHolder::aGLPreview;
MObject ProceduralHolder::aTransparent;
MObject ProceduralHolder::aDrawBound;

ProceduralHolder::ProceduralHolder()
	:	m_boundDirty( true ), m_sceneDirty( true )
{
}

ProceduralHolder::~ProceduralHolder()
{
}

void ProceduralHolder::postConstructor()
{
	ParameterisedHolderComponentShape::postConstructor();
	setRenderable( true );
}

void *ProceduralHolder::creator()
{
	return new ProceduralHolder;
}
			 
MStatus ProceduralHolder::initialize()
{	
	MStatus s = inheritAttributesFrom( ParameterisedHolderComponentShape::typeName );
	assert( s );
	
	MFnNumericAttribute nAttr;
	
	aGLPreview = nAttr.create( "glPreview", "glpr", MFnNumericData::kBoolean, 1, &s );
	assert( s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );
	
	s = addAttribute( aGLPreview );
	assert( s );
	
	aTransparent = nAttr.create( "transparent", "trans", MFnNumericData::kBoolean, 0, &s );
	assert( s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );
	
	s = addAttribute( aTransparent );
	assert( s );
	
	aDrawBound = nAttr.create( "drawBound", "dbnd", MFnNumericData::kBoolean, 1, &s );
	assert( s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );
	
	s = addAttribute( aDrawBound );
	assert( s );
	
	return MS::kSuccess;
}

bool ProceduralHolder::isBounded() const
{
	return true;
}

MBoundingBox ProceduralHolder::boundingBox() const
{
	if( !m_boundDirty )
	{
		return m_bound;
	}
		
	m_bound = MBoundingBox( MPoint( -1, -1, -1 ), MPoint( 1, 1, 1 ) );

	Renderer::ProceduralPtr p = ((ProceduralHolder*)this)->getProcedural();
	if( p )
	{
		((ProceduralHolder*)this)->setParameterisedValues();
		try
		{
			Box3f b = p->bound();
			if( !b.isEmpty() )
			{
				m_bound = convert<MBoundingBox>( b );
			}
		}
		catch( boost::python::error_already_set )
		{
			PyErr_Print();
		}
		catch( IECore::Exception &e )
		{
			msg( Msg::Error, "ProceduralHolder::boundingBox", e.what() );
		}
		catch( ... )
		{
			msg( Msg::Error, "ProceduralHolder::boundingBox", "Exception thrown in Procedural::bound" );
		}
	}
	
	m_boundDirty = false;
	return m_bound;
}

MStatus ProceduralHolder::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	if( plugParameter( plug ) || (!plug.parent().isNull() && plugParameter( plug.parent() ) ) )
	{
		// it's an input to the procedural
		m_boundDirty = m_sceneDirty = true;
		childChanged( kBoundingBoxChanged ); // this is necessary to cause maya to redraw
	}
	
	return ParameterisedHolderComponentShape::setDependentsDirty( plug, plugArray );
}
				
MStatus ProceduralHolder::setProcedural( const std::string &className, int classVersion )
{
	return setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS" );
}

IECore::Renderer::ProceduralPtr ProceduralHolder::getProcedural( std::string *className, int *classVersion )
{
	return runTimeCast<Renderer::Procedural>( getParameterised( className, classVersion ) );
}

IECoreGL::ConstScenePtr ProceduralHolder::scene()
{
	if( !m_sceneDirty  )
	{
		return m_scene;
	}
	
	m_scene = 0;
	Renderer::ProceduralPtr p = ((ProceduralHolder*)this)->getProcedural();
	if( p )
	{
		setParameterisedValues();
		try
		{
			IECoreGL::RendererPtr renderer = new IECoreGL::Renderer;
			renderer->setOption( "gl:mode", new StringData( "deferred" ) );
			renderer->worldBegin();
			
				p->render( renderer );
			
			renderer->worldEnd();
			
			m_scene = renderer->scene();
			m_scene->setCamera( 0 );
		}
		catch( boost::python::error_already_set )
		{
			PyErr_Print();
		}
		catch( IECore::Exception &e )
		{
			msg( Msg::Error, "ProceduralHolder::scene", e.what() );
		}
		catch( ... )
		{
			msg( Msg::Error, "ProceduralHolder::scene", "Exception thrown in Procedural::render" );
		}
	}
	
	m_sceneDirty = false;
	return m_scene;
}
