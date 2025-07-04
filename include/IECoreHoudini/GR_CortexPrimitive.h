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

#ifndef IECOREHOUDINI_GRCORTEXPRIMITIVE_H
#define IECOREHOUDINI_GRCORTEXPRIMITIVE_H

#include "IECoreHoudini/CoreHoudiniVersion.h"
#include "IECoreHoudini/Export.h"

#include "IECoreScene/Renderable.h"

#include "GR/GR_Primitive.h"
#include "UT/UT_Version.h"

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
class IECOREHOUDINI_API GR_CortexPrimitive : public GR_Primitive
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

#if MIN_HOU_VERSION( 20, 0, 0 )
		virtual void update( RE_RenderContext r, const GT_PrimitiveHandle &primh, const GR_UpdateParms &p );
#else
		virtual void update( RE_Render *r, const GT_PrimitiveHandle &primh, const GR_UpdateParms &p );
#endif


#if MIN_HOU_VERSION( 20, 0, 0 )
		virtual void render( RE_RenderContext r, GR_RenderMode render_mode, GR_RenderFlags flags, GR_DrawParms parms);
#elif UT_MAJOR_VERSION_INT >= 16
		virtual void render( RE_Render *r, GR_RenderMode render_mode, GR_RenderFlags flags, GR_DrawParms parms);
#else
		virtual void render( RE_Render *r, GR_RenderMode render_mode, GR_RenderFlags flags, const GR_DisplayOption *opt, const UT_Array<RE_MaterialPtr> *materials );
#endif

#if MIN_HOU_VERSION( 20, 0, 0 )
		virtual int renderPick( RE_RenderContext r, const GR_DisplayOption *opt, unsigned int pick_type, GR_PickStyle pick_style, bool has_pick_map );
#else
		virtual void renderInstances( RE_Render *r, GR_RenderMode render_mode, GR_RenderFlags flags, const GR_DisplayOption *opt, const UT_Array<RE_MaterialPtr> *materials, int render_instance );
		virtual int renderPick( RE_Render *r, const GR_DisplayOption *opt, unsigned int pick_type, GR_PickStyle pick_style, bool has_pick_map );
#endif

	private :

		GA_Index m_primId;
		IECoreGL::ScenePtr m_scene;
		const IECoreScene::Renderable *m_renderable;

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
