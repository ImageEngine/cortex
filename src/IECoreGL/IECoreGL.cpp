//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/GL.h"
#include "IECoreGL/IECoreGL.h"
#include "IECoreGL/FrameBuffer.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/DepthTexture.h"

#if defined( __APPLE__ )

#include <OpenGL/OpenGL.h>

#elif defined( __linux__ )

#include "GL/glx.h"

#endif

#include "IECore/MessageHandler.h"

static int g_glslVersion = 0;

void IECoreGL::init( bool glAlreadyInitialised )
{
	static bool init = false;
	if( !init )
	{
		if( !glAlreadyInitialised )
		{

#if defined( __APPLE__ )

			CGLPixelFormatAttribute attributes[2] =
			{
				kCGLPFAAccelerated, // no software rendering
				(CGLPixelFormatAttribute)0
			};
			CGLPixelFormatObj pixelFormat;
			GLint numVirtualScreens;
			CGLChoosePixelFormat( attributes, &pixelFormat, &numVirtualScreens );

			CGLContextObj context;
			CGLCreateContext( pixelFormat, 0, &context );
			CGLDestroyPixelFormat( pixelFormat );

			CGLSetCurrentContext( context );

#elif defined( __linux__ )
						
			int numFBConfigs = 0;
			Display *display = XOpenDisplay( NULL );
			GLXFBConfig *fbConfigs = glXChooseFBConfig( display, DefaultScreen( display ), NULL, &numFBConfigs );
			
			int contextAttribs[] =
			{
    			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
    			None
			};
			
			typedef GLXContext (*glXCreateContextAttribsARBProc)( Display *, GLXFBConfig, GLXContext, Bool, const int * );
			glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
			
			GLXContext openGLContext = glXCreateContextAttribsARB( display, fbConfigs[0], 0, True, contextAttribs );
			
			int pbufferAttribs[] = {
    			GLX_PBUFFER_WIDTH,  32,
    			GLX_PBUFFER_HEIGHT, 32,
    			None
			};

			GLXPbuffer pbuffer = glXCreatePbuffer( display, fbConfigs[0], pbufferAttribs );

			glXMakeContextCurrent( display, pbuffer, pbuffer, openGLContext );

			XFree( fbConfigs );
			XSync( display, False );

#endif

		}

		const GLenum initStatus = glewInit();
		if( initStatus!=GLEW_OK )
		{
			IECore::msg( IECore::Msg::Error, "IECoreGL::init", boost::format( "GLEW initialisation failed (%s)." ) % glewGetErrorString( initStatus ) );
		}
		init = true;
		
		const char *s = (const char *)glGetString( GL_SHADING_LANGUAGE_VERSION );
		int major = 0; int minor = 0;
		sscanf( s, "%d.%d", &major, &minor );
		g_glslVersion = major * 100 + minor;
	
#if defined( __APPLE__ )

		if( !glAlreadyInitialised )
		{
			// we have to do this bit after GLEW initialisation.
			static FrameBufferPtr g_frameBuffer = new FrameBuffer();
			g_frameBuffer->setColor( new ColorTexture( 32, 23 ) );
			g_frameBuffer->setDepth( new DepthTexture( 32, 32 ) );
			g_frameBuffer->validate();
			glBindFramebuffer( GL_FRAMEBUFFER, g_frameBuffer->frameBuffer() );
		}

#endif
		
	}
}

int IECoreGL::glslVersion()
{
	return g_glslVersion;
}
