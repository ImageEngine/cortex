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

#include "IECoreGL/Group.h"
#include "IECoreGL/State.h"

#include "OpenEXR/ImathBoxAlgo.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

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

StatePtr Group::getState()
{
	return m_state;
}

ConstStatePtr Group::getState() const
{
	return m_state;
}

void Group::setState( StatePtr state )
{
	m_state = state;
}

void Group::render( ConstStatePtr state ) const
{
	glPushMatrix();
		glMultMatrixf( m_transform.getValue() );
		GLbitfield mask = m_state->mask();
		// can't find a way of pushing the current program as part
		// of the attribute state, so we have to do it by hand.
		GLint oldProgram = 0;
		if( GLEW_VERSION_2_1 )
		{
			glGetIntegerv( GL_CURRENT_PROGRAM, &oldProgram );
		}
		if( mask )
		{
			glPushAttrib( mask );
		}
			m_state->bind();
			StatePtr s = new State( *state );
			s->add( m_state );
			for( ChildSet::const_iterator it=m_children.begin(); it!=m_children.end(); it++ )
			{
				(*it)->render( s );
			}
		if( mask )
		{
			glPopAttrib();
		}
		if( GLEW_VERSION_2_1 )
		{
			glUseProgram( oldProgram );
		}
	glPopMatrix();
}

Imath::Box3f Group::bound() const
{
	Box3f result;
	for( ChildSet::const_iterator it=children().begin(); it!=children().end(); it++ )
	{
		result.extendBy( (*it)->bound() );
	}
	return transform( result, m_transform );
}

void Group::addChild( RenderablePtr child )
{
	m_children.insert( child );
}

const Group::ChildSet &Group::children() const
{
	return m_children;
}
