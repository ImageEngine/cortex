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

#include "IECoreGL/IECoreGL.h"

#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/DepthTexture.h"
#include "IECoreGL/FrameBuffer.h"
#include "IECoreGL/GL.h"

#include "IECore/MessageHandler.h"

#if defined( __APPLE__ )
#include <OpenGL/OpenGL.h>
#elif defined( __linux__ )
#include "GL/glx.h"
#elif defined( _MSC_VER )
#include <windows.h>
#include "GL/GL.h"
#include "GL/wglew.h"
#include "boost/algorithm/string.hpp"
#endif

#include <stdio.h>

static int g_glslVersion = 0;

#if defined ( _MSC_VER )

#define INIT_ENTRY_POINT( funcName, funcType )							\
	funcName = ( funcType ) wglGetProcAddress( #funcName );				\
	if( !funcName )														\
	{																	\
		IECore::msg(													\
			IECore::Msg::Error,											\
			"IECoreGL::init",											\
			"Failed to get \"{}\" procedure.", #funcName				\
		);																\
		return;															\
	}																	\

static HINSTANCE hInstance = NULL;

BOOL WINAPI DllMain(
	HINSTANCE hInstanceDLL,
	DWORD fdwReason,
	LPVOID lpvReserved
)
{
	if( fdwReason == DLL_PROCESS_ATTACH )
	{
		hInstance = hInstanceDLL;
		WNDCLASSEX wndClass = {
			sizeof( WNDCLASSEX ),
			CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
			DefWindowProc,  // Use default windows message handler
			0,  // extra bytes to allocate
			0,  // extra bytes to allocate following window instance
			hInstance,
			NULL,  // icon
			NULL,  // cursor
			NULL,  // background brush
			NULL,  // menu
			"IECoreGL",  // class name
			NULL  // small icon
		};

		if( !RegisterClassEx( &wndClass ) )
		{
			IECore::msg(
				IECore::Msg::Error,
				"IECoreGL::init::DllMain",
				"Failed to register window class."
			);
			return NULL;
		}

		return true;
	}
}
#endif

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
			CGLCreateContext( pixelFormat, nullptr, &context );
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

#elif defined ( _MSC_VER )

			// Create a simple context with whatever OpenGL provides

			if( !hInstance )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to get instance handle." );
				return;
			}

			HWND hWnd = CreateWindow(
				"IECoreGL",  // class name
				NULL,  // window name
				0,  // style
				0, 0, 32, 32,  // x, y, width, height
				NULL,  // hWndParent
				NULL,  // hMenu
				hInstance,
				NULL  // extra parameters
			);
			if( !hWnd )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to get window handle." );
				return;
			}

			PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof( PIXELFORMATDESCRIPTOR ),
				1,  // version
				PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,  // pixel type
				32,  // color bits
				0, 0, 0, 0, 0, 0,  // not used
				32,  // alpha bits
				0,  // not used
				0,  // accumulation buffer bits
				0, 0, 0, 0,  // not used
				32,  // depth buffer bits
				0,  // stencil buffer bits
				0,  // number of aux buffers
				PFD_MAIN_PLANE,  // image plane, current implementations ignore this
				0, 0, 0, 0  // not used
			};

			HDC hDC = GetDC( hWnd );
			if( !hDC )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to get device context handle." );
				return;
			}

			int pixelFormat = ChoosePixelFormat(hDC, &pfd );
			if( !pixelFormat )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to get pixel format." );
				return;
			}

			if( !SetPixelFormat( hDC, pixelFormat, &pfd ) )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to set pixel format." );
				return;
			}

			// Create a context so we can get ARB extensions
			HGLRC starterContext = wglCreateContext( hDC );
			if( !starterContext )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to get starter context." );
				return;
			}

			if( !wglMakeCurrent( hDC, starterContext ) )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to make starter context current." );
				return;
			}

			// Create a Pbuffer for offscreen rendering

			INIT_ENTRY_POINT( wglGetExtensionsStringARB, PFNWGLGETEXTENSIONSSTRINGARBPROC );
			std::string extensions = wglGetExtensionsStringARB( hDC );
			if( extensions.find( "WGL_ARB_pixel_format" ) == std::string::npos )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Extension \"WGL_ARB_pixel_format\" not available." );
				return;
			}
			if( extensions.find( "WGL_ARB_pbuffer" ) == std::string::npos )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Extension \"WGL_ARB_pbuffer\" not available." );
				return;
			}

			INIT_ENTRY_POINT( wglCreatePbufferARB, PFNWGLCREATEPBUFFERARBPROC );
			INIT_ENTRY_POINT( wglGetPbufferDCARB, PFNWGLGETPBUFFERDCARBPROC );
			INIT_ENTRY_POINT( wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC );

			int pbufferIntAttribs[] = {
				WGL_DRAW_TO_PBUFFER_ARB, true,
				0
			};

			int formats[1];
			unsigned int formatCount;
			bool pfResult = wglChoosePixelFormatARB(
				hDC,
				pbufferIntAttribs,
				nullptr,
				1,
				formats,
				&formatCount
			);
			if( !pfResult || formatCount == 0 )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "No compatible formats available for Pbuffer." );
				return;
			}

			HPBUFFERARB hPbuffer = wglCreatePbufferARB( hDC, formats[0], 32, 32, NULL );
			if( !hPbuffer )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to create Pbuffer." );
				return;
			}

			HDC hPbufferDC = wglGetPbufferDCARB( hPbuffer );
			if( !hPbufferDC )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to get Pbuffer device context." );
				return;
			}

			// Create a new context requesting OpenGL version 3.3
			int contextAttribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
				0
			};

			INIT_ENTRY_POINT( wglCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC );

			HGLRC pbufferContext = wglCreateContextAttribsARB( hPbufferDC, 0, contextAttribs );
			if( !pbufferContext )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to create Pbuffer context." );
				return;
			}

			if( !wglMakeCurrent( hPbufferDC, pbufferContext ) )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::init", "Failed to set current context to Pbuffer context." );
				return;
			}

#endif

		}

		const GLenum initStatus = glewInit();
		if( initStatus!=GLEW_OK )
		{
			IECore::msg( IECore::Msg::Error, "IECoreGL::init", "GLEW initialisation failed ({}).", *glewGetErrorString( initStatus ) );
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
			g_frameBuffer->setColor( new ColorTexture( 32, 32 ) );
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
