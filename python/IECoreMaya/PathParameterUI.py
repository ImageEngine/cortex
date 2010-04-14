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

import os.path

import maya.cmds

import IECore
import IECoreMaya

class PathParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ) :
	
		IECoreMaya.ParameterUI.__init__(
			
			self,
			node,
			parameter,
			maya.cmds.rowLayout(
				numberOfColumns = 3,
				columnWidth3 = [ IECoreMaya.ParameterUI.textColumnWidthIndex, IECoreMaya.ParameterUI.singleWidgetWidthIndex * 3, 26 ]
			),
			**kw
		
		)

		maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__textField = maya.cmds.textField()

		maya.cmds.iconTextButton(
			label = "",
			image = "fileOpen.xpm",
			command = self.openDialog,
			height = 23,
			style = "iconOnly"
		)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		self._addPopupMenu( parentUI=self.__textField, attributeName = self.plugName() )
		maya.cmds.connectControl( self.__textField, self.plugName() )

	def openDialog( self ) :
		
		uiUserData = self.parameter.userData().get( 'UI', {} )
		dialogPath = uiUserData.get( 'defaultPath', IECore.StringData() ).value
		obeyDefaultPath = uiUserData.get( 'obeyDefaultPath', IECore.BoolData() ).value
		currentPath = self.parameter.getTypedValue()
		
		if currentPath and not obeyDefaultPath :
			dialogPath = os.path.dirname( currentPath )
		
		dialogPath = os.path.expandvars( dialogPath )
		dialogPath = os.path.join( dialogPath, '*' )
		
		selection = maya.cmds.fileDialog( directoryMask=dialogPath ).encode('ascii')
		
		return selection
