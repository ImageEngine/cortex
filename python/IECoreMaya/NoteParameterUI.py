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

import IECore
import IECoreMaya

## A UI for a StringParameters that uses the "note" type hint.
#
# This is for displaying user annotations

class NoteParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ):


		IECoreMaya.ParameterUI.__init__( self, node, parameter, maya.cmds.rowLayout( numberOfColumns = 3, columnWidth3 = [ 30, 300, 30 ] ), **kw )

		self.__editWindow = ""

		maya.cmds.text( label = "" )

		self.__label = maya.cmds.text(
			font = "smallPlainLabelFont",
			align = "left",
			annotation = "Notes",
		)

		maya.cmds.iconTextButton(
			label = "",
			image = "ie_editTextIcon.xpm",
			command = self._launchEditWindow,
			height = 23,
			style = "iconOnly"
		)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		maya.cmds.text(
			self.__label,
			e = True,
			label = str( parameter.getValue() ),
		)

	def _launchEditWindow( self ):

		if not maya.cmds.window( self.__editWindow, exists=True ):

			self.__editWindow = maya.cmds.window(
				title = "Edit Annotation",
				iconName = "Edit Annotation",
				width = 400,
				height = 500
			)

			form = maya.cmds.formLayout(numberOfDivisions=100)

			button = maya.cmds.button(
				label='Edit',
				command = self._editText,
			)

			self.__textField = maya.cmds.scrollField(
				editable = True,
				height = 50,
				wordWrap = True,
				text = str( self.parameter.getValue() ),
			)

			maya.cmds.formLayout(
				form,
				edit=True,
				attachForm=[(self.__textField, 'top', 5), (self.__textField, 'left', 5), (self.__textField, 'right', 5), (button, 'left', 5), (button, 'bottom', 5), (button, 'right', 5) ],
				attachControl=[ ( self.__textField, 'bottom', 5, button ) ],
				#attachPosition=[ ( self.__textField, 'right', 5, 75 ) ],
				attachNone=(button, 'top'),
			)

			maya.cmds.showWindow( self.__editWindow )

	def _editText( self, state ):

		newStr = maya.cmds.scrollField(
			self.__textField,
			q = True,
			text = True,
		)

		self.parameter.setValue( IECore.StringData( newStr ) )

		maya.cmds.deleteUI( self.__editWindow , window=True )

		maya.cmds.text(
			self.__label,
			e = True,
			label = str( self.parameter.getValue() ),
		)


IECoreMaya.ParameterUI.registerUI( IECore.TypeId.StringParameter, NoteParameterUI, "note" )
