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

	sceneShapes = __selectedSceneShapes()
	if not sceneShapes:
		return
	
	fnScS = []
	for target in sceneShapes:
		fnScS.append( IECoreMaya.FnSceneShape( target ) )
	
	maya.cmds.setParent( menu, menu=True )

	invalidSceneShapes = __invalidSceneShapes( sceneShapes )
	
	if invalidSceneShapes:
		maya.cmds.menuItem(
		label = "Invalid Inputs for selected SceneShapes!",
		radialPosition = "N",
		)
		
	# Component mode
	elif fnScS[0].selectedComponentNames():
		
			maya.cmds.menuItem(
			label = "Object",
			radialPosition = "N",
			command = IECore.curry( __objectCallback, sceneShapes[0] ),
			)
			
			maya.cmds.menuItem(
				label = "Print Component Names",
				radialPosition = "NW",
				command = IECore.curry( __printComponents, sceneShapes[0] )
			)

			maya.cmds.menuItem(
				label = "Print Selected Component Names",
				radialPosition = "NE",
				command = IECore.curry( __printSelectedComponents, sceneShapes[0] )
			)
		
			maya.cmds.menuItem(
				label = "Expand...",
				radialPosition = "SE",
				subMenu = True
			)

			maya.cmds.menuItem(
				label = "Expand to Selected Components",
				radialPosition = "S",
				command = IECore.curry( __expandToSelected, sceneShapes[0] )
			)
			maya.cmds.setParent( "..", menu=True )

			maya.cmds.menuItem(
				label = "Create Locator",
				radialPosition = "SW",
				subMenu = True,
			)
			
			maya.cmds.menuItem(
				label = "At Bound Min",
				radialPosition = "N",
				command = IECore.curry( __createLocatorAtPoints, sceneShapes[0], [ "Min" ] ),
			)
			
			maya.cmds.menuItem(
				label = "At Bound Max",
				radialPosition = "NE",
				command = IECore.curry( __createLocatorAtPoints, sceneShapes[0], [ "Max" ] ),
			)
			
			maya.cmds.menuItem(
				label = "At Bound Min And Max",
				radialPosition = "E",
				command = IECore.curry( __createLocatorAtPoints, sceneShapes[0], [ "Min", "Max" ] ),
			)
			
			maya.cmds.menuItem(
				label = "At Bound Centre",
				radialPosition = "SE",
				command = IECore.curry( __createLocatorAtPoints, sceneShapes[0], [ "Center" ] ),
			)
			
			maya.cmds.menuItem(
				label = "At Transform Origin",
				radialPosition = "S",
				command = IECore.curry( __createLocatorWithTransform, sceneShapes[0] ),
			)
			maya.cmds.setParent( "..", menu=True )
	
	# Object mode
	else:
		
		if len( sceneShapes ) == 1:
			if maya.cmds.getAttr( sceneShapes[0]+".drawGeometry" ) or maya.cmds.getAttr( sceneShapes[0]+".drawChildBounds" ):
				maya.cmds.menuItem(
					label = "Component",
					radialPosition = "N",
					command = IECore.curry( __componentCallback, sceneShapes[0] )
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

		
		commonTags = None
		for fn in fnScS:
			scene = fn.sceneInterface()
			tmpTags = scene.readTags(IECore.SceneInterface.EveryTag)
			if commonTags is None:
				commonTags = set( tmpTags )
			else:
				commonTags.intersection_update( set(tmpTags) )
				
		tagTree = dict()
		if not commonTags is None:
			tags = list(commonTags)
			for tag in tags :
				tag = str(tag)
				parts = tag.split(":")
				leftOverTag = tag[len(parts[0])+1:]
				if not parts[0] in tagTree :
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
				
				subtags = tagTree[tag]
				subtags.sort()
				
				if "" in subtags:
					maya.cmds.menuItem(
						label = tag,
						command = IECore.curry( __setTagsFilterPreviewAttributes, sceneShapes, tag )
					)
					subtags.remove("")
				
				if subtags:
					maya.cmds.menuItem(
						label = tag,
						subMenu = True
					)

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

		if any( map(lambda x: x.canBeExpanded(), fnScS) ):
			
			maya.cmds.menuItem(
				label = "Expand One Level",
				radialPosition = "E",
				command = IECore.curry( __expandOnce, sceneShapes )
			)
			
			maya.cmds.menuItem(
				label = "Recursive Expand",
				radialPosition = "N",
				command = IECore.curry( __expandAll, sceneShapes )
			)
			
			if len( sceneShapes ) == 1:
				if fnScS[0].selectedComponentNames() :
					maya.cmds.menuItem(
						label = "Expand to Selected Components",
						radialPosition = "S",
						command = IECore.curry( __expandToSelected, sceneShapes[0] )
					)
			
		maya.cmds.setParent( "..", menu=True )

		parentSceneShape = __parentSceneShape( sceneShapes )

		if any( map(lambda x: x.canBeCollapsed(), fnScS) ) or ( parentSceneShape and IECoreMaya.FnSceneShape( parentSceneShape ).canBeCollapsed() ):
			
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
						command = IECore.curry( __collapseChildren, [parentSceneShape] )
					)
			
			if any( map(lambda x: x.canBeCollapsed(), fnScS) ):
				maya.cmds.menuItem(
						label = "Collapse Children",
						radialPosition = "W",
						command = IECore.curry( __collapseChildren, sceneShapes )
					)
				
			maya.cmds.setParent( "..", menu=True )

	for c in __dagMenuCallbacks :
	
		c( menu, sceneShape )

## Returns all the sceneShapes that do not have a valid scene interface
def __invalidSceneShapes( sceneShapes ):
	
	invalid = []
	for sceneShape in sceneShapes:
		fn = IECoreMaya.FnSceneShape( sceneShape )
		if fn.sceneInterface() is None:
			invalid.append( sceneShape )
	return invalid

## Returns all the selected scene shapes
def __selectedSceneShapes() :
	
	allSceneShapes = []
	
	selectedSceneShapes = maya.cmds.ls( sl=True, l=True )
	for shape in selectedSceneShapes:
		# Make sure we have the shape name, it could be a component 
		shapeName = shape.split(".f[")[0]
		if maya.cmds.nodeType( shapeName ) == "ieSceneShape" and not shapeName in allSceneShapes:
			allSceneShapes.append( shapeName )
		else:
			children = maya.cmds.listRelatives( shapeName, children=True, type="ieSceneShape", fullPath=True ) or []
			for child in children:
				if not child in allSceneShapes:
					allSceneShapes.append( child )
	return allSceneShapes

## Turns on child bounds and switches to component mode
def __componentCallback( sceneShape, *unused ) :

	parent = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )[0]
	maya.cmds.selectType( ocm=True, alc=False, facet=True )
	maya.cmds.hilite( parent )
	
## Switches to object mode
def __objectCallback( sceneShape, *unused ) :

	parent = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )[0]
	maya.cmds.hilite( parent, unHilite=True )
	selection = maya.cmds.ls( selection=True )
	maya.cmds.selectMode( object=True )
	if selection :
		maya.cmds.select( selection, replace=True )
	else :
		maya.cmds.select( clear=True )

## Print the existing component names for the scene shape
def __printComponents( sceneShape, *unused ) :

	fnS = IECoreMaya.FnSceneShape( sceneShape )
	names = fnS.componentNames()
	names.sort()
	print "\n"
	print " ".join( names ) ,
	print "\n"

## Print the selected component names for the scene shape
def __printSelectedComponents( sceneShape, *unused ) :

	fnS = IECoreMaya.FnSceneShape( sceneShape )
	selectedNames = fnS.selectedComponentNames()
	if selectedNames:
		selectedNames = list( selectedNames )
		selectedNames.sort()
		print "\n"
		print " ".join( selectedNames ) ,
		print "\n"

## Expand each scene shape one level down
def __expandOnce( sceneShapes, *unused ) :
	
	toSelect = []
	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		new = fnS.expandOnce( preserveNamespace=True )
		toSelect.extend( map( lambda x: x.fullPathName(), new ) )
	if toSelect:
		maya.cmds.select( toSelect, replace=True )

## Recursively expand the scene shapes
def __expandAll( sceneShapes, *unused ) :
	
	toSelect = []
	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		newFn = fnS.expandAll( preserveNamespace=True )
		
		toSelect.extend( map( lambda x: x.fullPathName(), newFn ) )
	if toSelect:
		maya.cmds.select( toSelect, replace=True )

## Recursively expand the scene shapes and converts objects to geometry
def __expandAsGeometry( sceneShapes, *unused ) :
	
	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		fnS.convertAllToGeometry( True )

## Expand the scene shape the minimal amount to reach the selected components
def __expandToSelected( sceneShape, *unused ) :

	fnScS = IECoreMaya.FnSceneShape( sceneShape )
	sceneShape = fnScS.fullPathName()
	selectedNames = fnScS.selectedComponentNames()
	if not selectedNames:
		return
	
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
		transformName = parent
		transformNames = [ transformName ]
		for item in selected.split("/")[1:-1]:
			transformName = transformName + "|" + item
			if not transformName in transformNames:
				transformNames.append( transformName )
		
		for transform in transformNames:
			shape = maya.cmds.listRelatives( transform, fullPath=True, type = "ieSceneShape" )[0]
			fnS = IECoreMaya.FnSceneShape( shape )
			fnS.expandOnce()
		
		toSelect.append( transformNames[-1] )
	if toSelect:
		maya.cmds.select( toSelect, replace=True )

## Collapse all the children of the scene shapes
def __collapseChildren( sceneShapes, *unused ) :
	
	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		fnS.collapse()

## Returns the first common parent scene shape for the given scene shapes
# Returns None if no parent found.
def __parentSceneShape( sceneShapes ):
	
	def getParentShapes( transform, allParentShapes ):
		parent = maya.cmds.listRelatives( transform, p=True, fullPath=True )
		if parent:
			parentShape = maya.cmds.listRelatives( parent[0], fullPath=True, type = "ieSceneShape" )
			if parentShape:
			    allParentShapes.append( parentShape[0] )
			    getParentShapes( parent[0], allParentShapes )
		
	parents = None
	for sceneShape in sceneShapes:
		transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
		if transform:
			allParentShapes = []
			getParentShapes( transform[0], allParentShapes )
			if parents is None:
				parents = set( allParentShapes )
			else:
				parents.intersection_update( set(allParentShapes) )
	if parents:
		parent = ""
		for p in parents:
			if p.count("|") > parent.count("|"):
				parent = p
		return parent

	return None

## Sets the given preview attribute on the scene shapes with the given boolean value
# Preview attributes can be drawGeometry, drawLocators, drawRootBound and drawChildBounds
def __setChildrenPreviewAttributes( sceneShapes, attributeName, value, *unused ) :

	for sceneShape in sceneShapes:
		transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
		if transform:
			allChildren = maya.cmds.listRelatives( transform[0], ad=True, fullPath=True, type = "ieSceneShape" ) or []
			for node in allChildren:
				maya.cmds.setAttr( node+"."+attributeName, value )

## Sets the given tags filter attribute on the scene shapes with the given string value
def __setTagsFilterPreviewAttributes( sceneShapes, tagName, *unused ) :

	for sceneShape in sceneShapes:
		transform = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )
		if transform:
			allChildren = maya.cmds.listRelatives( transform[0], ad=False, fullPath=True, type = "ieSceneShape" ) or []
			for node in allChildren:
				maya.cmds.setAttr( node+".drawTagsFilter", tagName, type = "string" )

def __createLocatorAtPoints( sceneShape, childPlugSuffixes, *unused ) :
	
	fnSc = IECoreMaya.FnSceneShape( sceneShape )
	selectedNames = fnSc.selectedComponentNames()

	locators = []
	for name in selectedNames :
		locators.extend( fnSc.createLocatorAtPoints( name, childPlugSuffixes ) )
		
	maya.cmds.select( locators, replace=True )

def __createLocatorWithTransform( sceneShape, *unused ) :
	
	fnSc = IECoreMaya.FnSceneShape( sceneShape )
	selectedNames = fnSc.selectedComponentNames()

	locators = []
	for name in selectedNames :
		locators.append( fnSc.createLocatorAtTransform( name ) )

	maya.cmds.select( locators, replace=True )

