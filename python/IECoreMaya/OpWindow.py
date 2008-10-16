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

import IECoreMaya

import maya.cmds
import traceback

## This class creates a window holding an instance of IECore.Op, presenting a ui
# for the modification of it's parameters. The user can then execute the op interactively.
## \todo We need to close this window if the node is deleted - for instance when the user makes a fresh scene.
class OpWindow( IECoreMaya.UIElement ) :

	def __init__( self, op ) :

		window = maya.cmds.window( title="Op Settings" )
		
		IECoreMaya.UIElement.__init__( self, window )
		
		form = maya.cmds.formLayout()
		
		infoRow = maya.cmds.rowLayout()
		maya.cmds.text( label = op.name, annotation = op.description, font = "boldLabelFont" )
		
		maya.cmds.setParent( form )
		
		scrollLayout = maya.cmds.scrollLayout( horizontalScrollBarThickness=0 )
		column = maya.cmds.columnLayout()
		maya.cmds.separator( parent=column, style="none" )
		maya.cmds.separator( parent=column, style="none" )
		
		maya.cmds.setParent( form )
		
		cancelButton = maya.cmds.button( label="Cancel", command="import maya.cmds; maya.cmds.deleteUI( \"%s\" )" % window )
		resetButton = maya.cmds.button( label="Reset", command=self._createCallback( self.__revertPressed ) )
		goButton = maya.cmds.button( label="Go!", command=self._createCallback( self.__goPressed ) )
		
		self.__fnTPH = IECoreMaya.FnTransientParameterisedHolderNode.create( column, op )
		
		maya.cmds.setParent( form )
		
		maya.cmds.formLayout(
			form,
			edit = True,
			attachForm = [
				( infoRow, "top", 5 ),
				( infoRow, "left", 5 ),
				( infoRow, "right", 5 ),
				( scrollLayout, "left", 5 ),
				( scrollLayout, "right", 5 ),
				( cancelButton, "left", 5 ),
				( goButton, "right", 5 ),
				( cancelButton, "bottom", 5 ),
				( resetButton, "bottom", 5 ),
				( goButton, "bottom", 5 ),
			],
			attachPosition = [
				( cancelButton, "right", 5, 33 ),
				( goButton, "left", 5, 66 ),
			],
			attachControl = [
				( scrollLayout, "top", 5, infoRow ),
				( scrollLayout, "bottom", 5, cancelButton ),
				( resetButton, "left", 5, cancelButton ),
				( resetButton, "right", 5, goButton ),
			],
			attachNone = [
				( cancelButton, "top" ),
				( resetButton, "top" ),
				( goButton, "top" ),
			]
		)
		
		maya.cmds.showWindow( window )

	def __revertPressed( self ) :
			
		op = self.__fnTPH.getParameterised()[0]
		op.parameters().setValue( op.parameters().defaultValue )
		self.__fnTPH.setNodeValues()
		
	def __goPressed( self ) :
			
		self.__fnTPH.setParameterisedValues()
		op = self.__fnTPH.getParameterised()[0]
		
		valid = op.parameters().valueValid()
		if not valid[0] :
			maya.cmds.confirmDialog(
				title="Parameter Error",
				message=valid[1], button="OK",
				defaultButton="OK",
				parent=self._topLevelUI()
			)
			return
		
		result = None
		try :
			result = op()
		except Exception, e :
			buttonPressed = maya.cmds.confirmDialog(
				title="Execution Error",
				message="Error : " + e.message,
				button=( "Print Details", "OK" ),
				defaultButton="OK",
				parent=self._topLevelUI()
			)
			if buttonPressed=="Print Details" :
				traceback.print_exc()
			
		if result is not None :	
			maya.cmds.deleteUI( self._topLevelUI() )
