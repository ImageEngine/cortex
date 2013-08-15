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

#include "IECoreGL/Scene.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/Selector.h"
#include "IECoreGL/ShaderStateComponent.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( Scene );

Scene::Scene()
	:	m_root( new Group ), m_camera( 0 )
{
}

Scene::~Scene()
{
}

void Scene::render( State *state ) const
{
	if( m_camera )
	{
		m_camera->render( state );
	}

	GLint prevProgram;
	glGetIntegerv( GL_CURRENT_PROGRAM, &prevProgram );
	glPushAttrib( GL_ALL_ATTRIB_BITS );

		State::bindBaseState();
		state->bind();
		root()->render( state );

	glPopAttrib();
	glUseProgram( prevProgram );
}

void Scene::render() const
{
	/// \todo Can we avoid this cast?
	render( const_cast<State *>( State::defaultState().get() ) );
}

Imath::Box3f Scene::bound() const
{
	return root()->bound();
}

size_t Scene::select( Selector::Mode mode, const Imath::Box2f &region, std::vector<HitRecord> &hits ) const
{
	if( m_camera )
	{
		m_camera->render( const_cast<State *>( State::defaultState().get() ) );
	}

	Selector selector( region, mode, hits );
	
	State::bindBaseState();
	selector.baseState()->bind();
	root()->render( selector.baseState() );

	return hits.size();
}

void Scene::setCamera( CameraPtr camera )
{
	m_camera = camera;
}

CameraPtr Scene::getCamera()
{
	return m_camera;
}

ConstCameraPtr Scene::getCamera() const
{
	return m_camera;
}

GroupPtr Scene::root()
{
	return m_root;
}

ConstGroupPtr Scene::root() const
{
	return m_root;
}
