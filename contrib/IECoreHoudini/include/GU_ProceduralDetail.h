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

#ifndef GU_PROCEDURALDETAIL_H_
#define GU_PROCEDURALDETAIL_H_

// GL
#include <glew.h>

// Houdini
#include <RE/RE_Render.h>
#include <GU/GU_Detail.h>
#include <UT/UT_String.h>
#include <UT/UT_BoundingBox.h>

// Cortex
#include <IECore/ParameterisedProcedural.h>
#include "IECoreGL/IECoreGL.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/Renderer.h"
#include "IECoreGL/Scene.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECore/SimpleTypedData.h"
#include "IECorePython/ScopedGILLock.h"

namespace IECoreHoudini
{
	/// This class inherits from GU_Detail and is used to pass
	/// all the important information to our GR_Procedural render hook
	/// for visualisation of ParameterisedProcedurals.
    class GU_ProceduralDetail : public GU_Detail
    {
    	friend class SOP_ProceduralHolder;
    	friend class GR_Procedural;

        public:
			/// ctor
    		GU_ProceduralDetail();
    		/// dtor
            virtual ~GU_ProceduralDetail();

            /// returns a scene, rendering it if necessary
            IECoreGL::ConstScenePtr scene();

            /// make this procedural's scene as dirty
            void dirty();

            /// is this procedural's scene dirty?
            bool isDirty();

        private:
            IECore::ParameterisedProceduralPtr m_procedural;
    		IECoreGL::ScenePtr m_scene;
    		bool m_isDirty;
    };
}

#endif /* GU_PROCEDURALDETAIL_H_ */
