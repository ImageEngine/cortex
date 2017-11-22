#=====
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#=====
# Render Mode
#
# This cookbook example demonstrates how to render based on the current
# rendering context - specifically differentiating between OpenGL and RenderMan.
# This procedural renders a cube in OpenGL but a sphere in RenderMan.
#
#=====

import IECore
import IECoreGL
import IECoreRI

class renderMode(ParameterisedProcedural) :

       def __init__(self) :
               ParameterisedProcedural.__init__( self, "RenderMode cookbook example." )

       def doBound(self, args) :
               return IECore.Box3f( IECore.V3f( -1, -1, -1 ), IECore.V3f( 1, 1, 1 ) )

       def doRenderState(self, renderer, args) :
               pass

       def doRender(self, renderer, args) :

               # This checks the renderer against the GL renderer type
               if renderer.typeId()==IECoreGL.Renderer.staticTypeId():
                       MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) ).render( renderer )

               # This checks the renderer against the RenderMan renderer type
               if renderer.typeId()==IECoreRI.Renderer.staticTypeId():
                       renderer.sphere( -1, 1, -1, 360, {} )

# register
IECore.registerRunTimeTyped( renderMode )
