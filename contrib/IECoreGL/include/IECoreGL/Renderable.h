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

#ifndef IECOREGL_RENDERABLE_H
#define IECOREGL_RENDERABLE_H

#include "IECoreGL/TypeIds.h"

#include "IECore/RunTimeTyped.h"

#include "OpenEXR/ImathBox.h"

#include "boost/utility.hpp"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( State );

/// The Renderable class provides an abstract base class
/// for all classes capable of producing a visible result in
/// an OpenGL context.
class Renderable : public IECore::RunTimeTyped, boost::noncopyable
{

	public :

		Renderable();
		virtual ~Renderable();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Renderable, RenderableTypeId, IECore::RunTimeTyped );

		/// Renders the object in the current OpenGL context, using
		/// the state specified. The state should already have been
		/// bound into the current context, and state->isComplete()
		/// must be true. Upon leaving render() the OpenGL state is
		/// unchanged.
		virtual void render( ConstStatePtr state ) const = 0;
		/// Returns the bounding box for the Renderable.
		virtual Imath::Box3f bound() const = 0;
};

IE_CORE_DECLAREPTR( Renderable );

} // namespace IECoreGL

#endif // IECOREGL_RENDERABLE_H
