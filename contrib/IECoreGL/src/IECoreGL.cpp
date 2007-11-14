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

#include "IECoreGL/IECoreGL.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/GLUT.h"

#include "IECore/MessageHandler.h"

static void nullDisplayFunc()
{
}

void IECoreGL::init( bool glAlreadyInitialised )
{
	static bool init = false;
	if( !init )
	{
		if( !glAlreadyInitialised )
		{
			int argc = 2;
			// on the mac, glut changes working directory during initialisation
			// unless you pass the -useWorkingDir option
			char *argv[] = { "IECoreGL", "-useWorkingDir" };
			glutInit( &argc, argv );
			/// \todo We're making a window here to make glut initialise a gl context,
			/// so that glewInit() works. But we should figure out how to initialise
			/// GL ourselves and avoid the annoying window popping up at the beginning.
			int window = glutCreateWindow( "IECoreGL Initial Window" );
			glutDisplayFunc( nullDisplayFunc );
			glutDestroyWindow( window );
		}
		GLenum initStatus = glewInit();
		if( initStatus!=GLEW_OK )
		{
			IECore::msg( IECore::Msg::Error, "IECoreGL::init", boost::format( "GLEW initialisation failed (%s)." ) % glewGetErrorString( initStatus ) );
		}
		init = true;
	}
}

unsigned int IECoreGL::coreMajorVersion()
{
	return IE_CORE_MAJOR_VERSION;
}
