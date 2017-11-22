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
# Nested Parent
#
# This cookbook example demonstrates how to inject many child procedurals from
# a single parent procedural.
#
# Here we create a random point cloud and inject the nestedChild procedural
# for each point.
#
#=====

import IECore
from random import *
import IECoreGL

class nestedParent(ParameterisedProcedural) :

        def __init__(self) :
                ParameterisedProcedural.__init__( self, "Description here." )
                self.__pdata = []
                seed(0)
                for i in range(100):
                        self.__pdata.append( IECore.V3f( random()*10, random()*10, random()*10 ) )

        def doBound(self, args) :
                return IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 10 ) )

        def doRenderState(self, renderer, args) :
                pass

        def doRender(self, renderer, args) :
                # loop through our points
                for p in self.__pdata:

                        # push the transform state
                        renderer.transformBegin()

                        # concatenate a transformation matrix
                        renderer.concatTransform( IECore.M44f().createTranslated( p ) )

                        # create an instance of our child procedural
                        procedural = IECore.ClassLoader.defaultProceduralLoader().load( "nestedChild", 1 )()

                        # do we want to draw our child procedural immediately or defer
                        # until later?
                        immediate_draw = False
                        if renderer.typeId()==IECoreGL.Renderer.staticTypeId():
                                immediate_draw = True

                        # render our child procedural
                        procedural.render( renderer, withGeometry=True, immediateGeometry=immediate_draw )

                        # pop the transform state
                        renderer.transformEnd()

# register
IECore.registerRunTimeTyped( nestedParent )
