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
# Point Render
#
# This cookbook example demonstrates how to create and render a Cortex
# PointsPrimitive. The procedural creates a points primitive and fills it with
# a specified number of points, within a specified bounding box.
#
#=====
from IECore import *
from random import *

#generate a points primitive filling the bbox with npoints
def generatePoints( bbox, npoints ):
        seed(0)
        size = bbox.size()
        pdata = V3fVectorData()
        for i in range(npoints):
                pdata.append( V3f( random() * size.x + bbox.min.x,
                                                random() * size.y + bbox.min.y,
                                                random() * size.z + bbox.min.z ) )
        return PointsPrimitive( pdata )

#our point render procedural
class pointRender(ParameterisedProcedural) :
        def __init__(self) :
                ParameterisedProcedural.__init__( self, "Description here." )
                bbox = Box3fParameter( "bbox", "Bounds for points.", Box3f(V3f(0), V3f(1)) )
                npoints = IntParameter( "npoints", "Number of points.", 100, minValue=0, maxValue=10000 )
                width = FloatParameter( "width", "Point width", 0.05  )
                self.parameters().addParameters( [ bbox, npoints, width ] )
                self.__points = None
                self.__npoints = None
                self.__bbox = None

        def generatePoints(self, args):
                if args['npoints'].value!=self.__npoints or args['bbox'].value!=self.__bbox:
                        self.__points = generatePoints( args['bbox'].value, args['npoints'].value )
                        self.__npoints = args['npoints'].value
                        self.__bbox = args['bbox'].value
                return self.__points

        def doBound(self, args) :
                self.generatePoints(args)
                return self.__points.bound()

        def doRenderState(self, renderer, args) :
                pass

        def doRender(self, renderer, args) :
                self.generatePoints(args)
                self.__points['width'] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, args['width'] )
                self.__points.render( renderer )

#register
registerRunTimeTyped( pointRender )
