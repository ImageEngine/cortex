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

	sceneShapes = __targets( sceneShape )
	if not sceneShapes:
		if maya.cmds.nodeType( sceneShape )!="ieSceneShape" :
			children = maya.cmds.listRelatives( sceneShape, children=True, type="ieSceneShape", fullPath=True )
			if not children :
				return
			else :
				sceneShape = children[0]
	else:
		sceneShape = sceneShapes[0]

	fnScS = IECoreMaya.FnSceneShape( sceneShape )
	
	maya.cmds.setParent( menu, menu=True )

	scene = fnScS.sceneInterface()
	
	if scene is None:
		maya.cmds.menuItem(
		label = "Invalid SceneShape Inputs!",
		radialPosition = "N",
		)
		
	# Component mode
	elif maya.cmds.selectMode( q=True, component=True ):
		
		maya.cmds.menuItem(
		label = "Object",
		radialPosition = "N",
		command = IECore.curry( __objectCallback, sceneShapes ),
		)
		
		maya.cmds.menuItem(
			label = "Print Component Names",
			radialPosition = "NW",
			command = IECore.curry( __printComponents, sceneShape )
		)
		
		# Check if any component is selected
		if fnScS.selectedComponentNames() :

			maya.cmds.menuItem(
				label = "Print Selected Component Names",
				radialPosition = "NE",
				command = IECore.curry( __printSelectedComponents, sceneShape )
			)
		
			maya.cmds.menuItem(
				label = "Expand...",
				radialPosition = "SE",
				subMenu = True
			)
			
			if fnScS.selectedComponentNames() :
				maya.cmds.menuItem(
					label = "Expand to Selected Components",
					radialPosition = "S",
					command = IECore.curry( __expandToSelected, sceneShapes )
				)
			maya.cmds.setParent( "..", menu=True )
	
	# Object mode
	elif maya.cmds.selectMode( q=True, object=True ):
		
		maya.cmds.menuItem(
			label = "Component",
			radialPosition = "N",
			command = IECore.curry( __componentCallback, sceneShapes )
			)

		maya.cmds.menuItem(
			label = "Preview...",
			radialPosition = "NW",
			subMenu = True		
		)
	
		maya.cmds.menuItem(
				label = "All Geometry On",
				radialPosition = "E",
				command = IECore.curry( __setChildrenPreviewAttributes, sceneShapes, "drawGeometry", True )
			)
		
		maya.cmds.menuItem(
				label = "All Child Bounds On",
				radialPosition = "SE",
				command = IECore.curry( __setChildrenPreviewAttributes, sceneShapes, "drawChildBounds", True )
			)
		
		maya.cmds.menuItem(
				label = "All Root Bound On",
				radialPosition = "NE",
				command = IECore.curry( __setChildrenPreviewAttributes, sceneShapes, "drawRootBound", True )
			)
		
		maya.cmds.menuItem(
				label = "All Geometry Off",
				radialPosition = "W",
				command = IECore.curry( __setChildrenPreviewAttributes, sceneShapes, "drawGeometry", False )
			)
		
		maya.cmds.menuItem(
				label = "All Child Bounds Off",
				radialPosition = "SW",
				command = IECore.curry( __setChildrenPreviewAttributes, sceneShapes, "drawChildBounds", False )
			)
		
		maya.cmds.menuItem(
				label = "All Root Bound Off",
				radialPosition = "NW",
				command = IECore.curry( __setChildrenPreviewAttributes, sceneShapes, "drawRootBound", False )
			)

		maya.cmds.setParent( "..", menu=True )

		tagTree = dict()
		tags = scene.readTags()
		for tag in tags :
			tag = str(tag)
			parts = tag.split(":")
			if len(parts) == 1 :
				if not tag in tagTree :
					tagTree[tag] = None
			else :
				leftOverTag = tag[len(parts[0])+1:]
				if not parts[0] in tagTree or tagTree[parts[0]] is None :
					tagTree[parts[0]] = [ leftOverTag ]
				else :
					tagTree[parts[0]].append( leftOverTag )

		if tagTree :

			maya.cmds.menuItem(
				label = "Tags filter...",
				radialPosition = "S",
				subMenu = True
			)

			maya.cmds.menuItem(
				label = "Display All",
				command = IECore.curry( __setTagsFilterPreviewAttributes, sceneShapes, "" )
			)

			tags = tagTree.keys()
			tags.sort()

			for tag in tags :

				if tagTree[tag] is None :

					maya.cmds.menuItem(
						label = tag,
						command = IECore.curry( __setTagsFilterPreviewAttributes, sceneShapes, tag )
					)

				else :

					maya.cmds.menuItem(
						label = tag,
						subMenu = True
					)
					subtags = tagTree[tag]
					subtags.sort()
					for tagSuffix in subtags :
						maya.cmds.menuItem(
							label = tagSuffix,
							command = IECore.curry( __setTagsFilterPreviewAttributes, sceneShapes, tag + ":" + tagSuffix )
						)
					maya.cmds.setParent( "..", menu=True )			
					
			maya.cmds.setParent( "..", menu=True )			
			
		maya.cmds.menuItem(
			label = "Expand...",
			radialPosition = "SE",
			subMenu = True
		)
	
		maya.cmds.menuItem(
				label = "Recursive Expand As Geometry",
				radialPosition = "W",
				command = IECore.curry( __expandAsGeometry, sceneShapes )
			)
		
		if fnScS.canBeExpanded():
			
			maya.cmds.menuItem(
				label = "Expand One Level",
				radialPosition = "E",
				command = IECore.curry( __expandScene, sceneShapes )
			)
			
			maya.cmds.menuItem(
				label = "Recursive Expand",
				radialPosition = "N",
				command = IECore.curry( __expandAll, sceneShapes )
			)
			
			if fnScS.selectedComponentNames() :
				maya.cmds.menuItem(
					label = "Expand to Selected Components",
					radialPosition = "S",
					command = IECore.curry( __expandToSelected, sceneShapes )
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
				
				parentName = maya.cmds.listRelatives( parentSceneShape, p=True )[0]
				maya.cmds.menuItem(
						label = "Collapse to Parent: "+parentName,
						radialPosition = "N",
						command = IECore.curry( __collapseToParentScene, sceneShape )
					)
			
			if fnScS.canBeCollapsed():
				maya.cmds.menuItem(
						label = "Collapse Children",
						radialPosition = "W",
						command = IECore.curry( __collapseChildren, sceneShapes )
					)
				
			maya.cmds.setParent( "..", menu=True )

	for c in __dagMenuCallbacks :
	
		c( menu, sceneShape )

def __targets( sceneShape ) :
	
	allSceneShapes = []
	
	selectedSceneShapes = maya.cmds.ls( sl=True, l=True )
	for shape in selectedSceneShapes:
		if maya.cmds.nodeType( shape ) == "ieSceneShape" and not shape in allSceneShapes:
			allSceneShapes.append( shape )
		else:
			children = maya.cmds.listRelatives( shape, children=True, type="ieSceneShape", fullPath=True )
			for child in children:
				if not child in allSceneShapes:
					allSceneShapes.append( child )
	return allSceneShapes


def __componentCallback( sceneShapes, *unused ) :

	for sceneShape in sceneShapes:
		# Make sure childBounds is turned on
		maya.cmds.setAttr( sceneShape + ".drawChildBounds", 1 )
		parent = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )[0]
		maya.cmds.selectMode( component=True )
		maya.cmds.hilite( parent )
	

def __objectCallback( sceneShapes, *unused ) :

	for sceneShape in sceneShapes:
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
	print "\n"
	print " ".join( names ) ,
	print "\n"

def __printSelectedComponents( sceneShape, *unused ) :

	fnS = IECoreMaya.FnSceneShape( sceneShape )
	selectedNames = fnS.selectedComponentNames()
	selectedNames = list( selectedNames )
	selectedNames.sort()
	print "\n"
	print " ".join( selectedNames ) ,
	print "\n"

def __expandScene( sceneShapes, *unused ) :
	
	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		new = fnS.expandScene()
		toSelect = map( lambda x: x.fullPathName(), new )
		maya.cmds.select( toSelect, replace=True )
	
def __expandAll( sceneShapes, *unused ) :
	
	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		newFn = fnS.expandAllChildren()
		
		toSelect = map( lambda x: x.fullPathName(), newFn )
		maya.cmds.select( toSelect, replace=True )

def __expandAsGeometry( sceneShapes, *unused ) :
	
	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		fnS.convertToGeometry()
	
def __expandToSelected( sceneShapes, *unused ) :
	
	for sceneShape in sceneShapes:
		fnScS = IECoreMaya.FnSceneShape( sceneShape )
		selectedNames = fnScS.selectedComponentNames()
		if "/" in selectedNames:
			selectedNames.remove("/")
		
		# Go back to object mode
		parent =  maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )[0]
		maya.cmds.hilite( parent, unHilite=True )
		maya.cmds.selectMode( object=True )
		
		if selectedNames == []:
			return
		
		toSelect = []	
	
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
			
			toSelect.append( transformNames[-1] )
		
		maya.cmds.select( toSelect, replace=True )


def __collapseChildren( sceneShapes, *unused ) :
	
	for sceneShape in sceneShapes:
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
				maya.cmds.select( fnParent.fullPathName(), replace=True )

def __parentSceneShape( sceneShape ):
	
	transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
	if transform:
		parent = maya.cmds.listRelatives( transform[0], parent=True, fullPath=True )
		if parent:
			parentShape = maya.cmds.listRelatives( parent[0], fullPath=True, type = "ieSceneShape" )
			if parentShape:
				return parentShape[0]
	return None

def __setChildrenPreviewAttributes( sceneShapes, attributeName, value, *unused ) :

	for sceneShape in sceneShapes:
		transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
		if transform:
			allChildren = maya.cmds.listRelatives( transform[0], ad=True, fullPath=True, type = "ieSceneShape" ) or []
			for node in allChildren:
				maya.cmds.setAttr( node+"."+attributeName, value )


def __setTagsFilterPreviewAttributes( sceneShapes, tagName, *unused ) :

	for sceneShape in sceneShapes:
		transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
		if transform:
			allChildren = maya.cmds.listRelatives( transform[0], ad=False, fullPath=True, type = "ieSceneShape" ) or []
			for node in allChildren:
				maya.cmds.setAttr( node+".drawTagsFilter", tagName, type = "string" )
