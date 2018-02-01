//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

/// \file GL.h
/// Include this file to include the system OpenGL headers. It's better this way as the
/// right way of including OpenGL varies from system to system.

#ifndef IECOREGL_GL_H
#define IECOREGL_GL_H

#include "IECoreGL/Export.h"

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "GL/glew.h"

namespace IECoreGL
{

IECOREGL_API inline void glColor( const Imath::V3f &c );
IECOREGL_API inline void glColor( const Imath::Color4f &c );
IECOREGL_API inline void glVertex( const Imath::V3f &v );
IECOREGL_API inline void glNormal( const Imath::V3f &n );
IECOREGL_API inline void glTranslate( const Imath::V2f &t );
IECOREGL_API inline void glTranslate( const Imath::V3f &t );

class IECOREGL_API PushAttrib
{
	public :

		PushAttrib( GLbitfield mask )
		{
			glPushAttrib( mask );
		}

		~PushAttrib()
		{
			glPopAttrib();
		}

};

} // namespace IECoreGL

#include "IECoreGL/GL.inl"

#endif // IECOREGL_GL_H
