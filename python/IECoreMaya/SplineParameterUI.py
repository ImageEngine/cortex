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

import IECore
import IECoreMaya
import maya.cmds
from ParameterUI import ParameterUI

class SplineParameterUI( ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :

		ParameterUI.__init__( self, node, parameter, **kw )

		self._layout = maya.cmds.rowLayout(
			numberOfColumns = 3,
			rowAttach = [ ( 1, "top", 0 ), ( 2, "both", 0 ), ( 3, "both", 0 ) ]
		)

		maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__gradientControl = maya.cmds.gradientControl()
		self.__button = maya.cmds.button( label = ">")
		self.__editWindow = None

		self.replace( node, parameter )

	def replace( self, node, parameter ) :

		ParameterUI.replace( self, node, parameter )
		maya.cmds.gradientControl( self.__gradientControl, edit=True, attribute=self.plugName() )
		maya.cmds.button( self.__button, edit=True, command=self.__openEditWindow )
		self.__editWindow = None

	## Returns True if we're a color ramp and False if we're a greyscale curve.
	def __colored( self ) :

		plugName = self.plugName()
		attrName = plugName.split( "." )[-1]
		return maya.cmds.objExists( plugName + "[0]." + attrName + "_ColorR" )

	def __openEditWindow( self, unused ) :

		if not self.__editWindow :

			self.__editWindow = maya.cmds.window( self.nodeName() + " " + self.label(), retain=True, widthHeight=[ 600, 300 ] )

			layout = maya.cmds.formLayout()

			positionControl = maya.cmds.attrFieldSliderGrp( label = "Selected position", columnWidth=[ ( 1, 100 ) ] )

			if self.__colored() :
				valueControl = maya.cmds.attrColorSliderGrp( label = "Selected colour", showButton=False, columnWidth=[ ( 1, 90 ) ] )
			else :
				valueControl = maya.cmds.attrFieldSliderGrp( label = "Selected value", columnWidth=[ ( 1, 90 ) ] )

			gradientControl = maya.cmds.gradientControl(
				attribute=self.plugName(),
				selectedColorControl=valueControl,
				selectedPositionControl=positionControl
			)

			maya.cmds.formLayout( layout,
				edit=True,
				attachForm = [
					( positionControl, "left", 5 ),
					( positionControl, "bottom", 15 ),
					( valueControl, "bottom", 15 ),
					( gradientControl, "top", 5 ),
					( gradientControl, "left", 5 ),
					( gradientControl, "right", 5 ),
				],
				attachControl = [
					( gradientControl, "bottom", 5, positionControl ),
					( valueControl, "left", 5, positionControl ),
				]
			)
		maya.cmds.showWindow( self.__editWindow )

ParameterUI.registerUI( IECore.TypeId.SplinefColor3fParameter, SplineParameterUI )
ParameterUI.registerUI( IECore.TypeId.SplinefColor4fParameter, SplineParameterUI )
ParameterUI.registerUI( IECore.TypeId.SplineffParameter, SplineParameterUI )
ParameterUI.registerUI( IECore.TypeId.SplineddParameter, SplineParameterUI )
