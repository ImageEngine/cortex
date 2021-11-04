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

#ifndef IECOREGL_GROUP_H
#define IECOREGL_GROUP_H

#include "IECoreGL/Export.h"
#include "IECoreGL/Renderable.h"

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathMatrix.h"
IECORE_POP_DEFAULT_VISIBILITY

#include <list>
#include <mutex>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( State );

class IECOREGL_API Group : public Renderable
{

	public :

		typedef std::recursive_mutex Mutex;
		typedef std::list<RenderablePtr> ChildContainer;

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Group, GroupTypeId, Renderable );

		/// Newly created Groups have empty state and
		/// identity transforms.
		Group();
		/// Makes a copy of a Group - this is a shallow
		/// copy so any State and children are just referenced.
		Group( const Group &other );
		~Group() override;

		void setTransform( const Imath::M44f &matrix );
		const Imath::M44f &getTransform() const;

		State *getState();
		const State *getState() const;
		void setState( StatePtr state );

		// render method ( assumes there's no threads modifying the group ).
		void render( State *currentState ) const override;
		Imath::Box3f bound() const override;

		void addChild( RenderablePtr child );
		void removeChild( Renderable *child );
		void clearChildren();
		const ChildContainer &children() const;

		// Returns a mutex for this group object.
		// It should be used if the group is manipulated from different threads.
		/// \todo Can we remove this?
		Mutex &mutex() const;

	private :

		StatePtr m_state;
		Imath::M44f m_transform;
		ChildContainer m_children;
		mutable Mutex m_mutex;

};

IE_CORE_DECLAREPTR( Group );

} // namespace IECoreGL

#endif // IECOREGL_GROUP_H
