//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/private/DeferredRendererImplementation.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/State.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/Primitive.h"
#include "IECoreGL/Camera.h"

#include "IECore/MessageHandler.h"

#include <cassert>

using namespace IECoreGL;
using namespace Imath;
using namespace std;

DeferredRendererImplementation::DeferredRendererImplementation()
{
	m_stateStack.push_back( new State( false ) );
	m_transformStack.push( M44f() );
	m_scene = new Scene;
}

DeferredRendererImplementation::~DeferredRendererImplementation()
{
}

void DeferredRendererImplementation::addCamera( CameraPtr camera )
{
	m_scene->setCamera( camera );
}

void DeferredRendererImplementation::addDisplay( ConstDisplayPtr display )
{
	IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::addDisplay", "Not implemented" );
}

void DeferredRendererImplementation::worldBegin()
{
	if( m_transformStack.size()>1 )
	{
		IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::worldBegin", "Mismatched transformBegin/transformEnd detected." );
	}
	m_transformStack = TransformStack();
	m_transformStack.push( M44f() );

	m_stateStack.push_back( new State( false ) );

	m_groupStack.push( m_scene->root() ); /// \todo this group should have the attribute state accumulated before worldBegin
}

void DeferredRendererImplementation::worldEnd()
{
	if( m_transformStack.size()!=1 )
	{
		IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::worldEnd", "Bad nesting of transformBegin/transformEnd detected." );
	}
	m_transformStack = TransformStack();
	m_transformStack.push( M44f() );

	/// \todo This is where we would do our rendering and saving of images.
}

void DeferredRendererImplementation::transformBegin()
{
	GroupPtr g = new Group;
	g->setTransform( m_transformStack.top() );
	m_groupStack.top()->addChild( g );
	m_groupStack.push( g );

	m_transformStack.push( M44f() );
}

void DeferredRendererImplementation::transformEnd()
{
	if( m_transformStack.size()<=1 )
	{
		IECore::msg( IECore::Msg::Warning, "DeferredRendererImplementation::transformEnd", "Bad nesting." );
		return;
	}
	m_transformStack.pop();
	m_groupStack.pop();

}

void DeferredRendererImplementation::concatTransform( const Imath::M44f &matrix )
{
	m_transformStack.top() = matrix * m_transformStack.top();
}

void DeferredRendererImplementation::attributeBegin()
{
	GroupPtr g = new Group;
	g->setTransform( m_transformStack.top() );
	g->setState( new State( **(m_stateStack.rbegin()) ) );
	m_groupStack.top()->addChild( g );
	m_groupStack.push( g );

	m_transformStack.push( M44f() );
	m_stateStack.push_back( new State( false ) );
}

void DeferredRendererImplementation::attributeEnd()
{
	if( m_stateStack.size()<=1 )
	{
		IECore::msg( IECore::Msg::Warning, "DeferredRendererImplementation::attributeEnd", "Bad nesting." );
		return;
	}
	m_transformStack.pop();
	m_stateStack.pop_back();
	m_groupStack.pop();
}

void DeferredRendererImplementation::addState( StateComponentPtr state )
{
	(*m_stateStack.rbegin())->add( state );
}

StateComponentPtr DeferredRendererImplementation::getState( IECore::TypeId type )
{
	for( StateStack::reverse_iterator it=m_stateStack.rbegin(); it!=m_stateStack.rend(); it++ )
	{
		StateComponentPtr c = (*it)->get( type );
		if( c )
		{
			return c;
		}
	}
	return boost::const_pointer_cast<StateComponent>( State::defaultState()->get( type ) );
}

void DeferredRendererImplementation::addPrimitive( PrimitivePtr primitive )
{
	GroupPtr g = new Group;
	g->setTransform( m_transformStack.top() );
	g->setState( new State( **(m_stateStack.rbegin()) ) );
	g->addChild( primitive );

	m_groupStack.top()->addChild( g );
}

ScenePtr DeferredRendererImplementation::scene()
{
	return m_scene;
}
