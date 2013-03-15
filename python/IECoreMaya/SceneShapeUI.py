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
		radialPosition = "NW",
		command = IECore.curry( __objectCallback, sceneShape ),
	)
	
	maya.cmds.menuItem(
		label = "Print Component Names",
		radialPosition = "NE",
		command = IECore.curry( __printComponents, sceneShape )
	)
	
	maya.cmds.menuItem(
		label = "Preview...",
		radialPosition = "W",
		subMenu = True		
	)
	
	maya.cmds.menuItem(
			label = "All Geometry On",
			radialPosition = "E",
			command = IECore.curry( __setChildrenPreviewAttributes, sceneShape, "drawGeometry", True )
		)
	
	maya.cmds.menuItem(
			label = "All Child Bounds On",
			radialPosition = "SE",
			command = IECore.curry( __setChildrenPreviewAttributes, sceneShape, "drawChildBounds", True )
		)
	
	maya.cmds.menuItem(
			label = "All Root Bound On",
			radialPosition = "NE",
			command = IECore.curry( __setChildrenPreviewAttributes, sceneShape, "drawRootBound", True )
		)
	
	maya.cmds.menuItem(
			label = "All Geometry Off",
			radialPosition = "W",
			command = IECore.curry( __setChildrenPreviewAttributes, sceneShape, "drawGeometry", False )
		)
	
	maya.cmds.menuItem(
			label = "All Child Bounds Off",
			radialPosition = "SW",
			command = IECore.curry( __setChildrenPreviewAttributes, sceneShape, "drawChildBounds", False )
		)
	
	maya.cmds.menuItem(
			label = "All Root Bound Off",
			radialPosition = "NW",
			command = IECore.curry( __setChildrenPreviewAttributes, sceneShape, "drawRootBound", False )
		)
	
	maya.cmds.setParent( "..", menu=True )
	
	# If the objectOnly flag is on, assume it's already expanded
	fnScS = IECoreMaya.FnSceneShape( sceneShape )
	if fnScS.canBeExpanded():
		maya.cmds.menuItem(
			label = "Expand...",
			radialPosition = "SE",
			subMenu = True
		)
		
		maya.cmds.menuItem(
			label = "Expand Scene",
			radialPosition = "E",
			command = IECore.curry( __expandScene, sceneShape )
		)
		
		maya.cmds.menuItem(
			label = "Expand All Children",
			radialPosition = "N",
			command = IECore.curry( __expandAll, sceneShape )
		)
		
		if fnScS.selectedComponentNames() :
			maya.cmds.menuItem(
				label = "Expand to Selected Components",
				radialPosition = "S",
				command = IECore.curry( __expandToSelected, sceneShape )
			)
			
		maya.cmds.setParent( "..", menu=True )

	parentSceneShape = __parentSceneShape( sceneShape )
	if fnScS.canBeCollapsed() or ( parentSceneShape and IECoreMaya.FnSceneShape( parentSceneShape ).canBeCollapsed() ):
		
		maya.cmds.menuItem(
				label = "Collapse...",
				radialPosition = "SW",
				subMenu = True
			)
	
		if parentSceneShape and IECoreMaya.FnSceneShape( parentSceneShape ).canBeCollapsed():
			maya.cmds.menuItem(
					label = "Collapse to Parent Scene",
					radialPosition = "N",
					command = IECore.curry( __collapseToParentScene, sceneShape )
				)
		
		if fnScS.canBeCollapsed():
			maya.cmds.menuItem(
					label = "Collapse Children",
					radialPosition = "W",
					command = IECore.curry( __collapseChildren, sceneShape )
				)
			
		maya.cmds.setParent( "..", menu=True )
	
	# Check if any component is selected
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

def __expandScene( sceneShape, *unused ) :
	
	fnS = IECoreMaya.FnSceneShape( sceneShape )
	fnS.expandScene()
	
def __expandAll( sceneShape, *unused ) :
	
	def recursiveExpand( fnSceneShape ):
		
		new = fnSceneShape.expandScene()
		for n in new:
			recursiveExpand( n )
	
	fnS = IECoreMaya.FnSceneShape( sceneShape )
	recursiveExpand( fnS )
	
def __expandToSelected( sceneShape, *unused ) :
	
	fnScS = IECoreMaya.FnSceneShape( sceneShape )
	selectedNames = fnScS.selectedComponentNames()
	if "/" in selectedNames:
		selectedNames.remove("/")
	if selectedNames == []:
		return

	for selected in selectedNames:

		transformName = "|".join( sceneShape.split("|")[:-1] )
		transformNames = [ transformName ]
		for item in selected.split("/")[1:]:
			transformName = transformName + "|" + item
			if not transformName in transformNames:
				transformNames.append( transformName )
		
		for transform in transformNames:
			shape = maya.cmds.listRelatives( transform, fullPath=True, type = "ieSceneShape" )[0]
			fnS = IECoreMaya.FnSceneShape( shape )
			fnS.expandScene()

	

def __collapseChildren( sceneShape, *unused ) :
	
	fnS = IECoreMaya.FnSceneShape( sceneShape )
	fnS.collapseScene()
	
def __collapseToParentScene( sceneShape, *unused ) :
	
	transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
	if transform:
		parent = maya.cmds.listRelatives( transform[0], parent=True, fullPath=True )
		if parent:
			parentShape = maya.cmds.listRelatives( parent[0], fullPath=True, type = "ieSceneShape" )
			if parentShape:
				fnParent = IECoreMaya.FnSceneShape( parentShape[0] )
				fnParent.collapseScene()

def __parentSceneShape( sceneShape ):
	
	transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
	if transform:
		parent = maya.cmds.listRelatives( transform[0], parent=True, fullPath=True )
		if parent:
			parentShape = maya.cmds.listRelatives( parent[0], fullPath=True, type = "ieSceneShape" )
			if parentShape:
				return parentShape[0]
	return None

def __setChildrenPreviewAttributes( sceneShape, attributeName, value, *unused ) :

	transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
	if transform:
		allChildren = maya.cmds.listRelatives( transform[0], ad=True, fullPath=True, type = "ieSceneShape" ) or []
		for node in allChildren:
			maya.cmds.setAttr( node+"."+attributeName, value )





