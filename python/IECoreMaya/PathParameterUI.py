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
import warnings

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

		self.__label = maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__textField = maya.cmds.textField()

		maya.cmds.iconTextButton(
			label = "",
			image = "fileOpen.xpm",
			command = self._fileDialog,
			height = 23,
			style = "iconOnly"
		)

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )
		# We can't see the menu if its on the field...
		self._addPopupMenu( parentUI=self.__label, attributeName = self.plugName() )
		maya.cmds.connectControl( self.__textField, self.plugName() )
	
	## This can be implemented in derived classes to show a customised file picker.
	## Typically, you would call the base class passing additional kw arguments,
	## which are passed to the FileDialog call. If omitted, "path", "key" and
	## "callback" will be set to appropriate values depending on the parameter.
	def _fileDialog( self, **kw ) :
		
		# Allow a class to enforce a path if the default behaviour for
		# using existing paths or the default userData path is not desired.
		if "path" not in kw:
			kw["path"] = self._initialPath()

		# Allow the parameter userData to override the key from the caller.
		# This is a little different in concept to the path, but they key is
		# never dynamic in terms of the parameter, so I think it makes sense.
		uiUserData = self.parameter.userData().get( 'UI', {} )
		key = uiUserData.get( 'fileDialogKey', IECore.StringData() ).value
		if key:
			kw["key"] = key

		if "callback" not in kw:
			kw["callback"] = self.__defaultFileDialogCallback

		IECoreMaya.FileDialog( **kw )
	
	# Simply sets the parameter value
	def __defaultFileDialogCallback( self, selection ) :
		
		if selection:

			self.parameter.setValue( IECore.StringData( selection[0] ) )
			fnPH = IECoreMaya.FnParameterisedHolder( self.node() )
			fnPH.setNodeValue( self.parameter )
	
	## Returns the initial path for a FileDialog, implied by the current
	## parameter value, and the status of the following userData["UI"] entries:
	##  - defaultPath (IECore.StringData()) A path to use as a default.
	##  - obeyDefaultPath (IECore.BoolData()) If True, the default path will
	##     be used, regardless of the curent parameter value. Otherwise, the
	##     parent directory of the current path is used.
	def _initialPath( self, parameter=None ) :
		
		if not parameter:
			parameter = self.parameter
		
		uiUserData = parameter.userData().get( 'UI', {} )
		dialogPath = uiUserData.get( 'defaultPath', IECore.StringData() ).value
		obeyDefaultPath = uiUserData.get( 'obeyDefaultPath', IECore.BoolData( False ) ).value
		currentPath = parameter.getTypedValue()
		
		if currentPath and not obeyDefaultPath :
			dialogPath = os.path.dirname( currentPath )
			
		return dialogPath
		
		
