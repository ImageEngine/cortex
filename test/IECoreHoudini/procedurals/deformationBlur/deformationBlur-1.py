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
#
# Deformation Blur
#
# This cookbook example injects two different geometries at different time
# two time samples creating deformation blur.
#
# Motion blur is defined in a very similar fashion to RenderMan. First we call
# motionBegin() with a list of time samples. You can have as many motion samples
# as your renderer will allow. For each time sample we then inject some
# geometry. It is important that the topology remain consistent between time
# samples, otherwise the renderer will complain. We finish by calling
# motionEnd(). Remember not to inject anything but geometry samples between
# motionBegin() and motionEnd().
#
# Don't forget to turn on motion blur in your renderer
#
# In OpenGL all samples will be rendered simultaneously. Refer to the
# RenderSwitch example for code that can differentiate based on which renderer
# is currently rendering.
#
# In general the code will look like:
#
#      renderer.motionBegin( [ sample1, sample2, ... ] )
#      sample1_geometry.render( renderer )
#      sample2_geometry.render( renderer )
#      ...
#      renderer.motionEnd()
#
#=====
from IECore import *

class deformationBlur(ParameterisedProcedural):

       #=====
       # Init
       def __init__(self) :
               ParameterisedProcedural.__init__( self, "DeformationBlur procedural." )
               geo1 = PathParameter( name="geo1", description="Geometry #1",
                                                       defaultValue="test_data/deform1.cob" )
               geo2 = PathParameter( name="geo2", description="Geometry #2",
                                                       defaultValue="test_data/deform2.cob" )
               self.parameters().addParameters( [geo1, geo2] )

       #=====
       # It's important that the bounding box extend to contain both geometry
       # samples.
       def doBound(self, args) :
               bbox = Box3f()
               geo1 = Reader.create( args['geo1'].value ).read()
               geo2 = Reader.create( args['geo2'].value ).read()
               bbox.extendBy( geo1.bound() )
               bbox.extendBy( geo2.bound() )
               return bbox

       #=====
       # Nothing to do
       def doRenderState(self, renderer, args) :
               pass

       #=====
       # Render our two motion samples
       def doRender(self, renderer, args):

               # load our geometry
               geo1 = Reader.create( args['geo1'].value ).read()
               geo2 = Reader.create( args['geo2'].value ).read()

               # get the shutter open/close values from the renderer
               shutter = renderer.getOption('shutter').value # this is a V2f

               # if motion blur is not enabled then both shutter open & close will
               # be zero.
               do_moblur = ( shutter.length() > 0 )

               # inject the motion samples
               renderer.motionBegin( [ shutter[0], shutter[1] ] )
               geo1.render( renderer )
               geo2.render( renderer )
               renderer.motionEnd()

#=====
# Register our procedural
registerRunTimeTyped( deformationBlur )
