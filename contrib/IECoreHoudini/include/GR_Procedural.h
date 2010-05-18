//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#ifndef GR_PROCEDURAL_H_
#define GR_PROCEDURAL_H_

// Houdini
#include <GR/GR_Detail.h>
#include <GR/GR_RenderHook.h>
#include <GR/GR_DisplayOption.h>

// Cortex
#include <IECoreGL/IECoreGL.h>
#include <IECoreGL/GL.h>
#include <IECoreGL/Renderer.h>

namespace IECoreHoudini
{
	/// forward declare our detail
	class GU_ProceduralDetail;

	/// Custom GL render hook for Houdini.
	/// This class is responsible for rendering in OpenGL our
	/// ParameterisedProcedural.
	class GR_Procedural : public GR_RenderHook
    {
        public:
			/// ctor
			GR_Procedural();
			/// dtor
            virtual ~GR_Procedural();

            /// Tell Houdini if a particular detail should be
            /// rendered using this render hook.
            virtual int getWireMask( GU_Detail *gdp,
            		const GR_DisplayOption *dopt
            		) const;

            /// Tell Houdini if a particular detail should be
            /// rendered using this render hook.
            virtual int getShadedMask( GU_Detail *gdp,
            		const GR_DisplayOption *dopt
            		) const;

            /// Renders the ParameterisedProcedural in wireframe
            virtual void renderWire( GU_Detail *gdp,
                    RE_Render &ren,
                    const GR_AttribOffset &ptinfo,
                    const GR_DisplayOption *dopt,
                    float lod,
                    const GU_PrimGroupClosure *hidden_geometry );

            /// Renders the ParameterisedProcedural in shaded
            virtual void renderShaded( GU_Detail *gdp,
                    RE_Render &ren,
                    const GR_AttribOffset &ptinfo,
                    const GR_DisplayOption *dopt,
                    float lod,
                    const GU_PrimGroupClosure *hidden_geometry );

            /// Tells Houdini what our render hook is called.
            virtual const char *getName() const
            {
            	return "IECoreHoudini::GR_Procedural";
            }

        private:
            /// Utility method used to create a valid GL state, based
            /// on the display options (wireframe/shaded) etc.
            IECoreGL::ConstStatePtr getDisplayState(
            		const GR_DisplayOption *dopt,
            		bool wireframe=false );
    };

} // namespace IECoreHoudini

#endif /* GR_PROCEDURAL_H_ */
