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

#include "IECoreGL/State.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/StateComponent.h"

#include <iostream>

using namespace IECoreGL;
using namespace std;

State::State( bool complete )
{
	if( complete )
	{
		CreatorMap &c = *creators();
		for( CreatorMap::const_iterator it = c.begin(); it!=c.end(); it++ )
		{
			add( it->second() );
		}
	}
}

State::State( const State &other )
{
	m_components = other.m_components;
}

State::~State()
{
}

void State::bind() const
{
	for( ComponentMap::const_iterator it=m_components.begin(); it!=m_components.end(); it++ )
	{
		it->second->bind();
	}
}

GLbitfield State::mask() const
{
	GLbitfield result = 0;
	for( ComponentMap::const_iterator it=m_components.begin(); it!=m_components.end(); it++ )
	{
		result |= it->second->mask();
	}
	return result;
}

void State::add( StatePtr s )
{
	for( ComponentMap::iterator it=s->m_components.begin(); it!=s->m_components.end(); it++ )
	{
		add( it->second );
	}
}

void State::add( StateComponentPtr s )
{
	m_components[s->typeId()] = s;
}

StateComponentPtr State::get( IECore::TypeId componentType )
{
	ComponentMap::const_iterator it = m_components.find( componentType );
	if( it==m_components.end() )
	{
		return 0;
	}
	return it->second;
}

ConstStateComponentPtr State::get( IECore::TypeId componentType ) const
{
	ComponentMap::const_iterator it = m_components.find( componentType );
	if( it==m_components.end() )
	{
		return 0;
	}
	return it->second;
}

bool State::isComplete() const
{
	return m_components.size()==creators()->size();
}

ConstStatePtr State::defaultState()
{
	static StatePtr s = new State( true );
	return s;
}

void State::registerComponent( IECore::TypeId type, CreatorFn creator )
{
	(*creators())[type] = creator;
}

State::CreatorMap *State::creators()
{
	static CreatorMap *m = new CreatorMap;
	return m;
}

void State::bindBaseState()
{
	glEnable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
	float black[4] = { 0, 0, 0, 0 };
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, black );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, black );
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, black );
	glShadeModel( GL_SMOOTH );
	glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
}
