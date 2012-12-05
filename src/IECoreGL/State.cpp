//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

IE_CORE_DEFINERUNTIMETYPED( State );

State::ScopedBinding::ScopedBinding( const State &s, State &currentState )
	:	m_currentState( currentState )
{
	m_savedComponents.reserve( s.m_components.size() );

	for( ComponentMap::const_iterator it=s.m_components.begin(); it!=s.m_components.end(); it++ )
	{
		m_savedComponents.push_back( currentState.get( it->first ) );
		m_currentState.add( it->second );
	}
	s.bind();
}

State::ScopedBinding::~ScopedBinding()
{
	for( std::vector<StateComponentPtr>::const_iterator it=m_savedComponents.begin(); it!=m_savedComponents.end(); it++ )
	{
		(*it)->bind();
		m_currentState.add( *it );
	}
}

State::State( bool complete )
	:	m_userAttributes( 0 )
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
	:	m_userAttributes( other.m_userAttributes ? other.m_userAttributes->copy() : 0 )
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

void State::add( StatePtr s )
{
	for( ComponentMap::iterator it=s->m_components.begin(); it!=s->m_components.end(); it++ )
	{
		add( it->second );
	}
	if( s->m_userAttributes )
	{
		/// \todo Is it not a bit questionable that we don't take a copy here?
		IECore::CompoundDataMap &a = userAttributes()->writable();
		IECore::CompoundDataMap &ao = s->m_userAttributes->writable();
		for( IECore::CompoundDataMap::iterator it=ao.begin(); it!=ao.end(); it++ )
		{
			a.insert( *it );
		}
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

void State::remove( IECore::TypeId componentType )
{
	ComponentMap::iterator it = m_components.find( componentType );
	if( it==m_components.end() )
	{
		return;
	}
	m_components.erase( it );
}

IECore::CompoundData *State::userAttributes()
{
	if( !m_userAttributes )
	{
		m_userAttributes = new IECore::CompoundData;
	}
	return m_userAttributes.get();
}

const IECore::CompoundData *State::userAttributes() const
{
	if( !m_userAttributes )
	{
		m_userAttributes = new IECore::CompoundData;
	}
	return m_userAttributes.get();
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
	glCullFace( GL_BACK );
}
