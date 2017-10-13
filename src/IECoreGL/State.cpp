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

#include "IECoreGL/State.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/StateComponent.h"

using namespace IECoreGL;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

class State::Implementation : public IECore::RefCounted
{

	public :

		Implementation( bool complete )
			:	m_userAttributes( nullptr )
		{
			if( complete )
			{
				State::CreatorMap &c = *State::creators();
				for( State::CreatorMap::const_iterator it = c.begin(); it!=c.end(); it++ )
				{
					add( it->second() );
				}
			}
		}

		Implementation( const Implementation &other )
			:	m_components( other.m_components ), m_userAttributes( other.m_userAttributes ? other.m_userAttributes->copy() : nullptr )
		{
		}

		virtual ~Implementation()
		{
		}

		void bind() const
		{
			for( ComponentMap::const_iterator it=m_components.begin(); it!=m_components.end(); it++ )
			{
				it->second.component->bind();
			}
		}

		void add( Implementation *s )
		{
			for( ComponentMap::iterator it=s->m_components.begin(); it!=s->m_components.end(); it++ )
			{
				add( it->second.component );
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

		void add( StateComponentPtr s, bool override = false )
		{
			m_components[s->typeId()] = Component( s, override );
		}

		StateComponent *get( IECore::TypeId componentType )
		{
			ComponentMap::const_iterator it = m_components.find( componentType );
			if( it==m_components.end() )
			{
				return nullptr;
			}
			return it->second.component.get();
		}

		const StateComponent *get( IECore::TypeId componentType ) const
		{
			ComponentMap::const_iterator it = m_components.find( componentType );
			if( it==m_components.end() )
			{
				return nullptr;
			}
			return it->second.component.get();
		}

		void remove( IECore::TypeId componentType )
		{
			ComponentMap::iterator it = m_components.find( componentType );
			if( it==m_components.end() )
			{
				return;
			}
			m_components.erase( it );
		}

		bool isComplete() const
		{
			return m_components.size()==creators()->size();
		}

		IECore::CompoundData *userAttributes()
		{
			if( !m_userAttributes )
			{
				m_userAttributes = new IECore::CompoundData;
			}
			return m_userAttributes.get();
		}

		const IECore::CompoundData *userAttributes() const
		{
			if( !m_userAttributes )
			{
				m_userAttributes = new IECore::CompoundData;
			}
			return m_userAttributes.get();
		}

	private :

		friend class ScopedBinding;

		struct Component
		{
			Component()
				: component( nullptr ), override( false )
			{
			}

			Component( StateComponentPtr c, bool o )
				:	component( c ), override( o )
			{
			}
			StateComponentPtr component;
			bool override;
		};

		typedef std::map<IECore::TypeId, Component> ComponentMap;
		ComponentMap m_components;
		mutable IECore::CompoundDataPtr m_userAttributes;

};

//////////////////////////////////////////////////////////////////////////
// ScopedBinding
//////////////////////////////////////////////////////////////////////////

State::ScopedBinding::ScopedBinding( const State &s, State &currentState )
	:	m_currentState( currentState )
{
	init( s );
}

State::ScopedBinding::ScopedBinding( const State &s, State &currentState, bool bind )
	:	m_currentState( currentState )
{
	init( s, bind );
}

void State::ScopedBinding::init( const State &s, bool bind )
{
	if( !bind )
	{
		return;
	}

	m_savedComponents.reserve( s.m_implementation->m_components.size() );

	for( Implementation::ComponentMap::const_iterator it=s.m_implementation->m_components.begin(); it!=s.m_implementation->m_components.end(); it++ )
	{
		Implementation::ComponentMap::iterator cIt = m_currentState.m_implementation->m_components.find( it->first );
		if( !cIt->second.override )
		{
			m_savedComponents.push_back( cIt->second.component );
			it->second.component->bind();
			cIt->second = it->second;
		}
	}
}

State::ScopedBinding::~ScopedBinding()
{
	for( std::vector<StateComponentPtr>::const_iterator it=m_savedComponents.begin(); it!=m_savedComponents.end(); it++ )
	{
		(*it)->bind();
		m_currentState.add( *it );
	}
}

//////////////////////////////////////////////////////////////////////////
// State
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( State );

State::State( bool complete )
	:	m_implementation( new Implementation( complete ) )
{
}

State::State( const State &other )
	:	m_implementation( new Implementation( *(other.m_implementation) ) )
{
}

State::~State()
{
}

void State::bind() const
{
	m_implementation->bind();
}

void State::add( StatePtr s )
{
	m_implementation->add( s->m_implementation.get() );
}

void State::add( StateComponentPtr s, bool override )
{
	m_implementation->add( s, override );
}

StateComponent *State::get( IECore::TypeId componentType )
{
	return m_implementation->get( componentType );
}

const StateComponent *State::get( IECore::TypeId componentType ) const
{
	return m_implementation->get( componentType );
}

void State::remove( IECore::TypeId componentType )
{
	return m_implementation->remove( componentType );
}

IECore::CompoundData *State::userAttributes()
{
	return m_implementation->userAttributes();
}

const IECore::CompoundData *State::userAttributes() const
{
	return m_implementation->userAttributes();
}

bool State::isComplete() const
{
	return m_implementation->isComplete();
}

const State *State::defaultState()
{
	static StatePtr s = new State( true );
	return s.get();
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
