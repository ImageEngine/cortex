//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2012 Electric Theatre Collective Limited. All rights reserved.
//
//  Copyright (c) 2015, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//	 * Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the following disclaimer.
//
//	 * Redistributions in binary form must reproduce the above copyright
//	   notice, this list of conditions and the following disclaimer in the
//	   documentation and/or other materials provided with the distribution.
//
//	 * Neither the name of Image Engine Design nor the names of any
//	   other contributors to this software may be used to endorse or
//	   promote products derived from this software without specific prior
//	   written permission.
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

#ifndef IECOREMANTRA_PROCEDURALPRIMITIVE_H
#define IECOREMANTRA_PROCEDURALPRIMITIVE_H

#include "VRAY/VRAY_Procedural.h"
#include "UT/UT_Version.h"

#include "IECore/VisibleRenderable.h"


namespace IECoreMantra
{
	
IE_CORE_FORWARDDECLARE( RendererImplementation )

class ProceduralPrimitive : public VRAY_Procedural
{
	public:
		ProceduralPrimitive();
		virtual ~ProceduralPrimitive();

#if UT_MAJOR_VERSION_INT >= 14

		virtual const char *className() const;

#else

		virtual const char *getClassName();

#endif

		virtual int initialize( const UT_BoundingBox * );
		virtual void getBoundingBox( UT_BoundingBox &box );
		virtual void render();
		// cortex types interface
		virtual void addVisibleRenderable ( IECore::VisibleRenderablePtr renderable );
		
        // mantra data for procedurals
		Imath::Box3f m_bound;
#if UT_MAJOR_VERSION_INT >= 16
		fpreal64 m_cameraShutter[2];
		fpreal64 m_fps;
		fpreal64 m_preBlur, m_postBlur;
#else
		fpreal m_cameraShutter[2];
		fpreal m_fps;
		fpreal m_preBlur, m_postBlur;
#endif

	
    private:
		void applySettings(VRAY_ProceduralChildPtr child);
		// cortex data
		RendererImplementationPtr m_renderer;
		IECore::Renderer::ProceduralPtr m_procedural;

        friend class IECoreMantra::RendererImplementation;
};
		
} // end namespace

#endif // IECOREMANTRA_PROCEDURALPRIMITIVE_H
