//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_STATE_H
#define IECOREGL_STATE_H

#include "IECore/CompoundDataBase.h"

#include "IECoreGL/Bindable.h"

#include "IECoreGL/TypeIds.h"

#include <vector>
#include <map>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( StateComponent );

class State : public Bindable
{
	public :

		typedef IECore::CompoundDataMap UserAttributesMap;

		// class that binds a state on the constructor and revert the changes on the destructor.
		/// \todo This is way too slow. We need to rewrite it to avoid all memory allocation, meaning
		/// we should modify currentState in place, and then restore the previous values to it in
		/// the destructor. We could also make State::userAttributes() lazily created.
		struct ScopedBinding : private boost::noncopyable
		{
			public :
				// binds the given state s and saving the modified values from current state.
				ScopedBinding( State &s, const State &currentState );

				ConstStatePtr boundState() const;
	
				// reverts the state changes.
				~ScopedBinding();

			private :

				std::vector< ConstStateComponentPtr > m_savedComponents;
				StatePtr m_boundState;
		};

		State( bool complete );
		State( const State &other );

		virtual ~State();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::State, StateTypeId, Bindable );

		// binds this state
		virtual void bind() const;
		
		/// Adds all the StateComponents and user attributes from s
		/// into this State.
		void add( StatePtr s );
		
		void add( StateComponentPtr s );
		template<typename T>
		typename T::Ptr get();
		template<typename T>
		typename T::ConstPtr get() const;
		StateComponentPtr get( IECore::TypeId componentType );
		ConstStateComponentPtr get( IECore::TypeId componentType ) const;
		template<typename T> void remove();
		void remove( IECore::TypeId componentType );

		/// Arbitrary state attributes for user manipulation.
		UserAttributesMap &userAttributes();
		const UserAttributesMap &userAttributes() const;

		bool isComplete() const;

		typedef StateComponentPtr (*CreatorFn)();
		static void registerComponent( IECore::TypeId, CreatorFn );

		/// Returns a complete State object with default settings. The
		/// same object is returned each time this is called.
		static ConstStatePtr defaultState();

		/// Even a complete State object doesn't specify the whole of the
		/// GL state - and in fact the State object and components assume
		/// that certain aspects of the GL state will be fixed at certain
		/// values. This function sets all those values. It is called
		/// for you by Scene::render(), but if using State objects
		/// without a Scene to coordinate rendering then you should
		/// call this function yourself.
		/// \todo This is incomplete - add more base state as we find it.
		/// \todo Also, remove any base state as we add StateComponents which
		/// specify it.
		/// \todo Should the StateComponents not get the chance to set the
		/// base state themselves? We've got a lot of random disparate stuff
		/// in this function that might be better placed alongside the
		/// code that's relying on that state.
		static void bindBaseState();

	private :

		typedef std::map<IECore::TypeId, CreatorFn> CreatorMap;
		static CreatorMap *creators();

	protected :

		typedef std::map<IECore::TypeId, StateComponentPtr> ComponentMap;
		ComponentMap m_components;
		UserAttributesMap m_userAttributes;
};

} // namespace IECoreGL

#include "IECoreGL/State.inl"

#endif // IECOREGL_STATE_H
