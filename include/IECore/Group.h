//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_GROUP_H
#define IE_CORE_GROUP_H

#include "IECore/StateRenderable.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/Transform.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( Group );
IE_CORE_FORWARDDECLARE( Data );

/// This class allows the grouping and rendering of a set
/// of VisibleRenderables, applying a specific set of Transforms
/// and StateRenderables to them.
/// \ingroup renderingGroup
class Group : public VisibleRenderable
{

	public:

		typedef std::vector<VisibleRenderablePtr> ChildContainer;
		typedef std::vector<StateRenderablePtr> StateContainer;

		/// \todo Add a constructor taking a list of children and state
		/// and an optional transform, and bind it.
		Group();
		virtual ~Group();

		IE_CORE_DECLAREOBJECT( Group, VisibleRenderable );

		/// Returns the Transform applied to the children of
		/// this Group. This is the local transform for the Group
		/// and takes no account of any transforms in the parents
		/// of the Group. May return 0 if no transform has been
		/// applied.
		Transform *getTransform();
		const Transform *getTransform() const;
		/// Sets the local transform applied to the children
		/// of this Group.
		void setTransform( TransformPtr transform );
		/// Returns the actual local transform matrix applied
		/// to this Group, evaluated at the specified time.
		Imath::M44f transformMatrix( float time = 0 ) const;
		/// Returns the final transform applied to the children
		/// of this Group when all parent transforms have been
		/// concatenated together with the local transform.
		Imath::M44f globalTransformMatrix( float time = 0 ) const;

		/// Adds some state to the Group. Throws an Exception if
		/// an attempt is made to add something deriving from Transform -
		/// setTransform() should be used for that.
		void addState( StateRenderablePtr state );
		/// Removes some state from the Group. If the state
		/// is not present already then an Exception is thrown.
		void removeState( StateRenderablePtr state );
		/// Removes all state from the Group.
		void clearState();
		/// Const access to the internal data structure used
		/// to hold the state.
		const StateContainer &state() const;
		/// Query an attribute from the group's scope. Tries to
		/// find the attribute in the group's state, then traverses
		/// the group's parents
		const Data *getAttribute( const std::string &name ) const;
		/// Set an attribute on this group.
		void setAttribute( const std::string &name, ConstDataPtr value );

		/// Adds a child to this Group. If the child is a Group itself
		/// and already has a parent then it will be removed from that
		/// parent before being added. Other VisibleRenderables may be
		/// parented under many Groups to provide instancing capabilities.
		void addChild( VisibleRenderablePtr child );
		/// Removes a child from the Group - has no effect if child is currently
		/// not a child. If the child is not a member of the group then
		/// an Exception is thrown.
		void removeChild( VisibleRenderablePtr child );
		/// Removes all children from the Group.
		void clearChildren();
		/// Const access to the internal data structure used to hold
		/// the children.
		const ChildContainer &children() const;

		/// Returns the parent for this Group, returning 0 if no
		/// parent exists.
		Group *parent();
		const Group *parent() const;
		
		/// Renders all the children in an attribute block along
		/// with the transform and state.
		virtual void render( Renderer *renderer ) const;
		/// As above, but can optionally omit the
		/// attribute block scoping.
		void render( Renderer *renderer, bool inAttributeBlock ) const;
		/// Renders all the held state, without any attributeBegin/End.
		/// Doesn't render the transform.
		void renderState( Renderer *renderer ) const;
		/// Renders all the held children, without any attributeBegin/End.
		void renderChildren( Renderer *renderer ) const;
		
		/// Returns the union of the bounds of the children, transformed
		/// by transformMatrix().
		virtual Imath::Box3f bound() const;

	private:
		
		static bool entryListCompare( const IECore::IndexedIO::EntryID& a, const IECore::IndexedIO::EntryID& b );
		
		static const unsigned int m_ioVersion;

		TransformPtr m_transform;
		Group *m_parent; // not a smart pointer to avoid cylic references
		StateContainer m_state;
		ChildContainer m_children;

};

} // namespace IECore

#endif // IE_CORE_GROUP_H
