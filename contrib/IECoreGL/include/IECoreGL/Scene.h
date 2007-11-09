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

#ifndef IECOREGL_SCENE_H
#define IECOREGL_SCENE_H

#include "IECoreGL/Renderable.h"

#include <list>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Group );

class Scene : public Renderable
{
	public :

		Scene();
		virtual ~Scene();

		/// Renders the scene, using the passed state as
		/// the root state - as with all Renderable calls
		/// this state must already have been bound.
		virtual void render( ConstStatePtr state ) const;
		/// Convenience function to bind a default state
		/// and then call render() with it.
		void render() const;
		virtual Imath::Box3f bound() const;
		
		/// Returns the root node for the scene. The
		/// scene can be edited by editing the root node.
		GroupPtr root();
		/// Returns the root node for the scene.
		ConstGroupPtr root() const;
	
	private :
	
		GroupPtr m_root;
		
};

IE_CORE_DECLAREPTR( Scene );

} // namespace IECoreGL

#endif // IECOREGL_SCENE_H
