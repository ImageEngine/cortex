##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

import maya.cmds
import maya.OpenMayaUI

import IECoreMaya

## A ui for any parameter for which parameter.presetsOnly is True.
class PresetsOnlyParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw  ) :

		IECoreMaya.ParameterUI.__init__(

			self,
			node,
			parameter,
			maya.cmds.rowLayout(
				numberOfColumns = 2,
			),
			**kw

		)

		maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__popupControl = maya.cmds.iconTextStaticLabel(
			image = "arrowDown.xpm",
			font = "smallBoldLabelFont",
			style = "iconAndTextHorizontal",
			height = 23
		)

		self.replace( node, parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		self.__updateLabel()

		self._addPopupMenu( parentUI=self.__popupControl, attributeName = self.plugName(), button1=True )

		self.__attributeChangedCallbackId = IECoreMaya.CallbackId(
			maya.OpenMaya.MNodeMessage.addAttributeChangedCallback( self.node(), self.__attributeChanged )
		)

	def _topLevelUIDeleted( self ) :

		self.__attributeChangedCallbackId = None

	def __attributeChanged( self, changeType, plug, otherPlug, userData ) :

		if not ( changeType & maya.OpenMaya.MNodeMessage.kAttributeSet ) :
			return

		try :
			myPlug = self.plug()
		except :
			# this situation can occur when our parameter has been removed but the
			# ui we represent is not quite yet dead
			return

		if plug == myPlug :
			self.__updateLabel()
			return

		if plug.isChild():
			if plug.parent() == myPlug :
				self.__updateLabel()
				return

	def __updateLabel( self ) :

		IECoreMaya.FnParameterisedHolder( self.node() ).setParameterisedValues()

		maya.cmds.iconTextStaticLabel(
			self.__popupControl,
			edit = True,
			label = self.parameter.getCurrentPresetName(),
		)
