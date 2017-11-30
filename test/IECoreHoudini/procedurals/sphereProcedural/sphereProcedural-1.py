##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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
##########################################################################

from __future__ import with_statement
import IECore
import IECoreScene

# renders a sphere
class sphereProcedural( IECoreScene.ParameterisedProcedural ) :
        def __init__( self ) :
                IECoreScene.ParameterisedProcedural.__init__( self, "Renders a sphere." )

		rad_param = IECore.FloatParameter(
			name = "radius",
			description = "Sphere radius.",
			defaultValue = 1,
			minValue = 0.01,
			maxValue = 100.0,
			userData = { 'UI': { "update" : IECore.BoolData( True ) } }
		)

		theta_param = IECore.FloatParameter(
			name = "theta",
			description = "Sphere theta.",
			defaultValue = 360,
			minValue = 1,
			maxValue = 360,
			userData = { 'UI': { "update" : IECore.BoolData( True ) } }
		)

		self.parameters().addParameters( [rad_param, theta_param] )

	def doBound( self, args ) :
		rad = args["radius"].value
		return IECore.Box3f( IECore.V3f(-rad,-rad,-rad), IECore.V3f(rad,rad,rad) )

	def doRenderState( self, renderer, args ) :
		pass

	def doRender( self, renderer, args ) :
		rad = args["radius"].value
		theta = args["theta"].value
		with IECoreScene.AttributeBlock( renderer ):
			renderer.sphere( rad, -1, 1, theta, {} )

# register
IECore.registerRunTimeTyped( sphereProcedural )
