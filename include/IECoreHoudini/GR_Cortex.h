//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

/// GR_RenderHooks are used in Houdini 12.0 and cause problems in 12.5.
/// Check GR_CortexPrimitive.h for Cortex viewport rendering in Houdini 12.5.
#include "UT/UT_Version.h"
#if UT_MAJOR_VERSION_INT >= 12 && UT_MINOR_VERSION_INT <= 1

#ifndef IECOREHOUDINI_GRCORTEX_H
#define IECOREHOUDINI_GRCORTEX_H

#include "GR/GR_Detail.h"
#include "GR/GR_DisplayOption.h"
#include "GR/GR_RenderHook.h"

#include "IECoreGL/GL.h"
#include "IECoreGL/IECoreGL.h"
#include "IECoreGL/Renderer.h"

#include "ieHoudini.h"

namespace IECoreHoudini
{

/// Custom GL render hook for Houdini. This class is responsible for
/// OpenGL rendering of our Cortex primitives and GL scenes.
class CortexHOUAPI GR_Cortex : public GR_RenderHook
{

	public :

		GR_Cortex();
		virtual ~GR_Cortex();

		/// Tell Houdini if a particular detail should be rendered using this render hook.
		virtual GA_PrimCompat::TypeMask getWireMask( GU_Detail *gdp, const GR_DisplayOption *dopt ) const;

		/// Tell Houdini if a particular detail should be rendered using this render hook.
		virtual GA_PrimCompat::TypeMask getShadedMask( GU_Detail *gdp, const GR_DisplayOption *dopt ) const;

		/// Renders the ParameterisedProcedural in wireframe
		virtual void renderWire( GU_Detail *gdp, RE_Render &ren, const GR_AttribOffset &ptinfo, const GR_DisplayOption *dopt, float lod, const GU_PrimGroupClosure *hidden_geometry );

		/// Renders the ParameterisedProcedural in shaded
		virtual void renderShaded( GU_Detail *gdp, RE_Render &ren, const GR_AttribOffset &ptinfo, const GR_DisplayOption *dopt, float lod, const GU_PrimGroupClosure *hidden_geometry );

		/// Methods to render stuff in openGL
		void render( GU_Detail *gdp, const IECoreGL::State *displayState );
		void renderObject( const IECore::Object *object, const IECoreGL::State *displayState );

		/// Tells Houdini what our render hook is called.
		virtual const char *getName() const
		{
			return "IECoreHoudini::GR_Cortex";
		}

	private :

		/// Utility method used to create a valid GL state, based
		/// on the display options (wireframe/shaded) etc.
		IECoreGL::ConstStatePtr getDisplayState( const GR_DisplayOption *dopt, bool wireframe=false );

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_GRCORTEX_H

#endif // 12.1 or earlier
