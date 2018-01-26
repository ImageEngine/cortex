//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/Group.h"

#include "IECoreGL/GL.h"
#include "IECoreGL/State.h"

#include "OpenEXR/ImathBoxAlgo.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( Group );

Group::Group()
	:	m_state( new State( false ) ), m_transform( M44f() )
{
}

Group::Group( const Group &other )
	:	m_state( new State( *(other.m_state) ) ), m_transform( other.m_transform ), m_children( other.m_children )
{
}

Group::~Group()
{
}

void Group::setTransform( const Imath::M44f &matrix )
{
	m_transform = matrix;
}

const Imath::M44f &Group::getTransform() const
{
	return m_transform;
}

State *Group::getState()
{
	return m_state.get();
}

const State *Group::getState() const
{
	return m_state.get();
}

void Group::setState( StatePtr state )
{
	m_state = state;
}

void Group::render( State *currentState ) const
{
	const bool haveTransform = m_transform != M44f();
	if( haveTransform )
	{
		// We only call glPushMatrix() if the matrix is
		// non-identity, in an attempt to avoid using all
		// the available stack depth.
		/// \todo Stop using the gl matrix stack. The proper
		/// solution is to define the transform entirely ourselves
		/// and pass it to a named shader parameter ourselves
		/// via glUniform. If we hit the matrix stack limit before
		/// we get the chance to implement the proper solution, an
		/// improved interim solution might be to track the transform
		/// ourselves without using the gl stack, and call glLoadMatrix
		/// to load the matrix. That way we'd be moving towards the
		/// proper solution, but without requiring all the shaders to
		/// be rewritten to use a different variable for receiving the
		/// matrix.
		glPushMatrix();
		glMultMatrixf( m_transform.getValue() );
	}

	{
		State::ScopedBinding scope( *m_state, *currentState );
		for( ChildContainer::const_iterator it=m_children.begin(); it!=m_children.end(); it++ )
		{
			(*it)->render( currentState );
		}
	}

	if( haveTransform )
	{
		glPopMatrix();
	}
}

Imath::Box3f Group::bound() const
{
	Box3f result;
	for( ChildContainer::const_iterator it=children().begin(); it!=children().end(); it++ )
	{
		result.extendBy( (*it)->bound() );
	}
	return transform( result, m_transform );
}

void Group::addChild( RenderablePtr child )
{
	m_children.push_back( child );
}

void Group::removeChild( Renderable *child )
{
	m_children.remove( child );
}

void Group::clearChildren()
{
	m_children.clear();
}

const Group::ChildContainer &Group::children() const
{
	return m_children;
}

Group::Mutex &Group::mutex() const
{
	return m_mutex;
}

