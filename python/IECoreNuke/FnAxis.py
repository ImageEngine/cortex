##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import math
import nuke
import IECore
from KnobAccessors import getKnobValue

## This function set can be used to manipulate any nodes which have
# an axis knob. This includes the Axis, TransformGeo and Camera nodes.
## \todo Methods for dealing with TransformationMatrices, absolute transforms
# and for setting transforms.
class FnAxis :

	def __init__( self, node ) :
	
		if isinstance( node, str ) :
			
			axis = nuke.toNode( node )
			
		self.__node = node

	## Returns the transformation matrix for the local axis knob.
	# This ignores any parent transforms.
	def getLocalMatrix( self, resultType=IECore.M44f ) :
	
		vectorType = IECore.V3f
		eulerType = IECore.Eulerf
		if resultType==IECore.M44d :
			vectorType = IECore.V3d
			eulerType = IECore.Eulerd
			
		translate = getKnobValue( self.__node.knob( "translate" ), resultType=vectorType )
		translate = resultType.createTranslated( translate )

		pivot = getKnobValue( self.__node.knob( "pivot" ), resultType=vectorType )

		rotate = getKnobValue( self.__node.knob( "rotate" ), resultType=vectorType )
		rotate *= math.pi / 180.0

		rotOrderKnob = self.__node.knob( "rot_order" )
		rotateOrder = rotOrderKnob.enumName( int(rotOrderKnob.getValue()) )
		rotate = eulerType( rotate, getattr( eulerType.Order, rotateOrder ), eulerType.InputLayout.XYZLayout )
		rotate = rotate.toMatrix44()
		rotate = resultType.createTranslated( -pivot ) * rotate * resultType.createTranslated( pivot )

		scale = getKnobValue( self.__node.knob(  "scaling" ), resultType=vectorType )
		scale *= self.__node.knob( "uniform_scale" ).getValue()
		scale = resultType.createScaled( scale )
		scale = resultType.createTranslated( -pivot ) * scale * resultType.createTranslated( pivot )

		orderKnob = self.__node.knob( "xform_order" )
		order = orderKnob.enumName( int(orderKnob.getValue()) )
		matrices = {
			"T" : translate,
			"R" : rotate,
			"S" : scale
		}

		result = resultType()
		for m in order :
			result = result * matrices[m]

		return result
