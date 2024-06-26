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

#ifndef IECORESCENE_VISIBLERENDERABLE_H
#define IECORESCENE_VISIBLERENDERABLE_H

#include "IECoreScene/Export.h"
#include "IECoreScene/Renderable.h"

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "Imath/ImathBox.h"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECoreScene
{

/// The VisibleRenderable class is an abstract base class for all Renderables
/// which describe a visible object (primitive shape, procedural etc) to a Renderer.
/// VisibleRenderables guarantee to leave the Renderer state (attributes, transforms, shaders)
/// unchanged across calls to VisibleRenderable::render().
/// \ingroup renderingGroup
class IECORESCENE_API VisibleRenderable : public Renderable
{
	public:

		VisibleRenderable();
		~VisibleRenderable() override;

		IE_CORE_DECLAREEXTENSIONOBJECT( VisibleRenderable, VisibleRenderableTypeId, Renderable );

		/// Returns the bounding box for the rendered objects.
		virtual Imath::Box3f bound() const = 0;

	private:

		static const unsigned int m_ioVersion;

};

IE_CORE_DECLAREPTR( VisibleRenderable );

}

#endif // IECORESCENE_VISIBLERENDERABLE_H
