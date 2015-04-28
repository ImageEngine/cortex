//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

/// GR_Primitives are used in Houdini 12.5, but do not exist in earlier versions.
/// Check GR_Cortex.h for Cortex viewport rendering in Houdini 12.0.
#include "UT/UT_Version.h"
#if UT_MAJOR_VERSION_INT > 12 || UT_MINOR_VERSION_INT >= 5

#ifndef IECOREHOUDINI_GRCORTEXPRIMITIVE_H
#define IECOREHOUDINI_GRCORTEXPRIMITIVE_H

#include "GR/GR_Primitive.h"

#include "IECore/Renderable.h"

// We can't include any IECoreGL files here, because it
// causes an issue with gl/glew initialization order.
namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Scene );
IE_CORE_FORWARDDECLARE( State );

}

namespace IECoreHoudini
{

/// Wrapper for drawing GU_CortexPrimitives in OpenGL
class GR_CortexPrimitive : public GR_Primitive
{
	public :
		
		GR_CortexPrimitive( const GR_RenderInfo *info, const char *cache_name, const GEO_Primitive *prim );
		virtual ~GR_CortexPrimitive();
		
		virtual const char *className() const
		{
			return "GR_CortexPrimitive";
		};
		
		virtual GR_PrimAcceptResult acceptPrimitive( GT_PrimitiveType t, int geo_type, const GT_PrimitiveHandle &ph, const GEO_Primitive *prim );
		virtual void resetPrimitives();
	
	protected :

		virtual void update( RE_Render *r, const GT_PrimitiveHandle &primh, const GR_UpdateParms &p );
		virtual void render( RE_Render *r, GR_RenderMode render_mode, GR_RenderFlags flags, const GR_DisplayOption *opt, const RE_MaterialList *materials );
		virtual void renderInstances( RE_Render *r, GR_RenderMode render_mode, GR_RenderFlags flags, const GR_DisplayOption *opt, const RE_MaterialList *materials, int render_instance );
		virtual int renderPick( RE_Render *r, const GR_DisplayOption *opt, unsigned int pick_type, GR_PickStyle pick_style, bool has_pick_map );
	
	private :
		
		GA_Index m_primId;
		IECoreGL::ScenePtr m_scene;
		const IECore::Renderable *m_renderable;		
		
		IECoreGL::State *getState( GR_RenderMode mode, GR_RenderFlags flags, const GR_DisplayOption *opt );
		
		const std::string &pickFragmentSource();
		
		static IECoreGL::StatePtr g_lit;
		static IECoreGL::StatePtr g_shaded;
		static IECoreGL::StatePtr g_wire;
		static IECoreGL::StatePtr g_wireLit;
		static IECoreGL::StatePtr g_wireShaded;
		static IECoreGL::StatePtr g_wireConstGhost;
		static IECoreGL::StatePtr g_wireConstBG;
		static IECoreGL::StatePtr g_pick;
		static IECoreGL::StatePtr g_selected;
		static IECoreGL::StatePtr g_wireSelected;
		static IECoreGL::StatePtr g_wireConstBGSelected;
		static IECoreGL::StatePtr g_wireConstGhostSelected;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_GRCORTEXPRIMITIVE_H

#endif // 12.5 or later
