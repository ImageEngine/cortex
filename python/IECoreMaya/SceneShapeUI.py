##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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
import maya.mel

import IECore
import IECoreMaya

__dagMenuCallbacks = []
## Registers a callback to be used when creating the right click dag
# menu for scene shapes. Callbacks should have the following signature :
#
# callback( menu, sceneShape ).
def addDagMenuCallback( callback ) :

	if not callback in __dagMenuCallbacks :
		__dagMenuCallbacks.append( callback )

## Removes a callback previously added with addDagMenuCallback.		
def removeDagMenuCallback( callback ) :

	__dagMenuCallbacks.remove( callback )

## This is forwarded to by the ieSceneShapeDagMenuProc function in
# ieSceneShape.mel
def _dagMenu( menu, sceneShape ) :

	if maya.cmds.nodeType( sceneShape )!="ieSceneShape" :
		children = maya.cmds.listRelatives( sceneShape, children=True, type="ieSceneShape", fullPath=True )
		if not children :
			return
		else :
			sceneShape = children[0]

	maya.cmds.setParent( menu, menu=True )
	maya.cmds.menuItem(
		label = "Component",
		radialPosition = "N",
		command = IECore.curry( __componentCallback, sceneShape )
	)
	maya.cmds.menuItem(
		label = "Object",
		radialPosition = "W",
		command = IECore.curry( __objectCallback, sceneShape ),
	)
	
	maya.cmds.menuItem(
		label = "Print Component Names",
		radialPosition = "NE",
		command = IECore.curry( __printComponents, sceneShape )
	)
	
	fnScS = IECoreMaya.FnSceneShape( sceneShape )
	if fnScS.selectedComponentNames() :
		maya.cmds.menuItem(
			label = "Print Selected Component Names",
			radialPosition = "E",
			command = IECore.curry( __printSelectedComponents, sceneShape )
		)

	for c in __dagMenuCallbacks :
	
		c( menu, sceneShape )
	
def __componentCallback( sceneShape, *unused ) :

	parent = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )[0]
	maya.cmds.selectMode( component=True )
	maya.cmds.hilite( parent )
	

def __objectCallback( sceneShape, *unused ) :

	parent = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )[0]
	maya.cmds.hilite( parent, unHilite=True )
	selection = maya.cmds.ls( selection=True )
	maya.cmds.selectMode( object=True )
	if selection :
		maya.cmds.select( selection, replace=True )
	else :
		maya.cmds.select( clear=True )

def __printComponents( sceneShape, *unused ) :

	fnS = IECoreMaya.FnSceneShape( sceneShape )
	names = fnS.childrenNames()
	names.sort()
	print " ".join( names ) ,

def __printSelectedComponents( sceneShape, *unused ) :

	fnS = IECoreMaya.FnSceneShape( sceneShape )
	selectedNames = fnS.selectedComponentNames()
	selectedNames = list( selectedNames )
	selectedNames.sort()
	print " ".join( selectedNames ) ,




