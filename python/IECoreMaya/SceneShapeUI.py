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
import functools

import maya.cmds

import IECore
import IECoreScene
import IECoreMaya

__dagMenuCallbacks = []
## Registers a callback to be used when creating the right click dag
# menu for scene shapes. Callbacks should have the following signature :
#
# callback( menuDefinition, sceneShape ).
def addDagMenuCallback( callback ) :

	if not callback in __dagMenuCallbacks :
		__dagMenuCallbacks.append( callback )

## Removes a callback previously added with addDagMenuCallback.
def removeDagMenuCallback( callback ) :

	__dagMenuCallbacks.remove( callback )

def _menuDefinition( callbackShape ) :
	sceneShapes = __selectedSceneShapes()
	if not sceneShapes :
		return

	mainDef = IECore.MenuDefinition()

	fnShapes = [ IECoreMaya.FnSceneShape( shape ) for shape in sceneShapes ]

	# INVALID SHAPES
	invalidSceneShapes = __invalidSceneShapes( sceneShapes )
	if invalidSceneShapes :
		mainDef.append( "/Invalid Inputs for selected SceneShapes!", { "blindData" : { "maya" : { "radialPosition" : "N" } } } )
		return mainDef

	# COMPONENT MODE
	if fnShapes[ 0 ].selectedComponentNames() :
		mainDef.append( "/Object", { "blindData" : { "maya" : { "radialPosition" : "N" } }, "command" : functools.partial( __objectCallback, sceneShapes[ 0 ] ) } )
		mainDef.append( "/Print Component Names", { "blindData" : { "maya" : { "radialPosition" : "NW" } }, "command" : functools.partial( __printComponents, sceneShapes[ 0 ] ) } )
		mainDef.append( "/Print Selected Component Names", { "blindData" : { "maya" : { "radialPosition" : "NE" } }, "command" : functools.partial( __printSelectedComponents, sceneShapes[ 0 ] ) } )

		# EXPAND
		expandDef = IECore.MenuDefinition( [
			("/Expand to Selected Components", { "blindData" : { "maya" : { "radialPosition" : "S" } }, "command" : functools.partial( __expandToSelected, sceneShapes[ 0 ] ) }),
		] )
		mainDef.append( "/Expand...", { "blindData" : { "maya" : { "radialPosition" : "SE" } }, "subMenu" : expandDef } )

		locatorDef = IECore.MenuDefinition( [
			("/At Bound Min", { "blindData" : { "maya" : { "radialPosition" : "N" } }, "command" : functools.partial( __createLocatorAtPoints, sceneShapes[ 0 ], [ "Min" ] ) }),
			("/At Bound Max", { "blindData" : { "maya" : { "radialPosition" : "NE" } }, "command" : functools.partial( __createLocatorAtPoints, sceneShapes[ 0 ], [ "Max" ] ) }),
			("/At Bound Min And Max", { "blindData" : { "maya" : { "radialPosition" : "E" } }, "command" : functools.partial( __createLocatorAtPoints, sceneShapes[ 0 ], [ "Min", "Max" ] ) }),
			("/At Bound Centre", { "blindData" : { "maya" : { "radialPosition" : "SE" } }, "command" : functools.partial( __createLocatorAtPoints, sceneShapes[ 0 ], [ "Center" ] ) }),
			("/At Transform Origin", { "blindData" : { "maya" : { "radialPosition" : "S" } }, "command" : functools.partial( __createLocatorWithTransform, sceneShapes[ 0 ] ) }),
		] )
		mainDef.append( "/Create Locator", { "blindData" : { "maya" : { "radialPosition" : "SW" } }, "subMenu" : locatorDef } )

	# OBJECT MODE
	else :
		# PREVIEW
		if len( sceneShapes ) == 1 and (maya.cmds.getAttr( sceneShapes[ 0 ] + ".drawGeometry" ) or maya.cmds.getAttr( sceneShapes[ 0 ] + ".drawChildBounds" )) :
			mainDef.append( "/Component", { "blindData" : { "maya" : { "radialPosition" : "N" } }, "command" : functools.partial( __componentCallback, sceneShapes[ 0 ] ) } )

		previewDef = IECore.MenuDefinition( [
			("/All Geometry On", { "blindData" : { "maya" : { "radialPosition" : "E" } }, "command" : functools.partial( __setChildrenPreviewAttributes, sceneShapes, "drawGeometry", True ) }),
			("/All Child Bounds On", { "blindData" : { "maya" : { "radialPosition" : "SE" } }, "command" : functools.partial( __setChildrenPreviewAttributes, sceneShapes, "drawChildBounds", True ) }),
			("/All Root Bound On", { "blindData" : { "maya" : { "radialPosition" : "NE" } }, "command" : functools.partial( __setChildrenPreviewAttributes, sceneShapes, "drawRootBound", True ) }),
			("/All Geometry Off", { "blindData" : { "maya" : { "radialPosition" : "W" } }, "command" : functools.partial( __setChildrenPreviewAttributes, sceneShapes, "drawGeometry", False ) }),
			("/All Child Bounds Off", { "blindData" : { "maya" : { "radialPosition" : "SW" } }, "command" : functools.partial( __setChildrenPreviewAttributes, sceneShapes, "drawChildBounds", False ) }),
			("/All Root Bound Off", { "blindData" : { "maya" : { "radialPosition" : "NE" } }, "command" : functools.partial( __setChildrenPreviewAttributes, sceneShapes, "drawRootBound", False ) })
		] )

		mainDef.append( "/Preview...", { "blindData" : { "maya" : { "radialPosition" : "NW" } }, "subMenu" : previewDef } )

		# get all tags that are shared between all shapes
		commonTags = None
		for fn in fnShapes :
			scene = fn.sceneInterface()
			tmpTags = scene.readTags( IECoreScene.SceneInterface.EveryTag )
			if commonTags is None :
				commonTags = set( tmpTags )
			else :
				commonTags.intersection_update( set( tmpTags ) )

		tagTree = dict()
		if commonTags :
			for tag in commonTags :
				tag = str( tag )
				namespace, _, subTagsString = tag.partition( ':' )
				subTags = set( subTagsString.split( ':' ) )
				if not namespace in tagTree :
					tagTree[ namespace ] = subTags
				else :
					tagTree[ namespace ].update( subTags )

		# EXPAND
		expandDef = IECore.MenuDefinition(
			[ ("/Recursive Expand As Geometry", { "blindData" : { "maya" : { "radialPosition" : "W" } }, "command" : functools.partial( _expandAsGeometry, sceneShapes)})] )
		mainDef.append( "/Expand...", { "blindData" : { "maya" : { "radialPosition" : "SE" } }, "subMenu" : expandDef } )

		if any( map( lambda x : x.canBeExpanded(), fnShapes ) ) :

			expandDef.append( "/Expand One Level", { "blindData" : { "maya" : { "radialPosition" : "E" } }, "command" : functools.partial( __expandOnce, sceneShapes ) } )
			expandDef.append( "/Recursive Expand", { "blindData" : { "maya" : { "radialPosition" : "N" } }, "command" : functools.partial( _expandAll, sceneShapes)})

			if len( sceneShapes ) == 1 and fnShapes[ 0 ].selectedComponentNames() :
				expandDef.append( "/Expand to Selected Components", { "blindData" : { "maya" : { "radialPosition" : "S" } }, "command" : functools.partial( __expandToSelected, sceneShapes[ 0 ] ) } )

		if tagTree :
			tags = tagTree.keys()
			tags.sort()

			def addTagSubMenuItems( menuDef, command ) :
				import copy
				copiedTagTree = copy.deepcopy( tagTree )
				for tag in tags :
					subtags = list( copiedTagTree[ tag ] )
					subtags.sort()

					for subtag in subtags :
						if subtag == '' :
							label = "/{}".format( tag )
							expandTag = tag
						else :
							label = "/{}/{}".format( tag, subtag )
							expandTag = "{}:{}".format( tag, subtag )
						menuDef.append( label, { "command" : functools.partial( command, sceneShapes, expandTag ) } )

			filterDef = IECore.MenuDefinition( [
				("/Display All", { "command" : functools.partial( _setTagsFilterPreviewAttributes, sceneShapes, "")})
			] )
			expandTagDef = IECore.MenuDefinition()
			expandTagGeoDef = IECore.MenuDefinition()
			mainDef.append( "/Tags filter...", { "blindData" : { "maya" : { "radialPosition" : "S" } }, "subMenu" : filterDef } )

			addTagSubMenuItems( filterDef, _setTagsFilterPreviewAttributes)
			addTagSubMenuItems( expandTagDef, _expandAll)
			addTagSubMenuItems( expandTagGeoDef, _expandAsGeometry)

			expandDef.append( "/Expand by Tag...", { "blindData" : { "maya" : { "radialPosition" : "SW" } }, "subMenu" : expandTagDef } )
			expandDef.append( "/Expand by Tag as Geo...", { "blindData" : { "maya" : { "radialPosition" : "SE" } }, "subMenu" : expandTagGeoDef } )

		parentSceneShape = __parentSceneShape( sceneShapes )

		# COLLAPSE
		if any( map( lambda x : x.canBeCollapsed(), fnShapes ) ) or (parentSceneShape and IECoreMaya.FnSceneShape( parentSceneShape ).canBeCollapsed()) :

			collapseDef = IECore.MenuDefinition()

			if parentSceneShape and IECoreMaya.FnSceneShape( parentSceneShape ).canBeCollapsed() :
				parentName = maya.cmds.listRelatives( parentSceneShape, p = True )[ 0 ]
				collapseDef.append( "/Collapse to Parent: {}".format( parentName ),
					{ "blindData" : { "maya" : { "radialPosition" : "N" } }, "command" : functools.partial( __collapseChildren, [ parentSceneShape ] ) } )

			if any( map( lambda x : x.canBeCollapsed(), fnShapes ) ) :
				collapseDef.append( "/Collapse Children", { "blindData" : { "maya" : { "radialPosition" : "W" } }, "command" : functools.partial( __collapseChildren, sceneShapes ) } )

			mainDef.append( "/Collapse...", { "blindData" : { "maya" : { "radialPosition" : "SW" } }, "subMenu" : collapseDef } )

	return mainDef


## This is forwarded to by the ieSceneShapeDagMenuProc function in
# ieSceneShape.mel
def _dagMenu( parentMenu, sceneShape ) :
	menuDef = _menuDefinition( sceneShape )
	if not menuDef :
		return

	# Pass menu definition to registered callbacks to collect changes / additions
	for callback in __dagMenuCallbacks :
		callback( menuDef, sceneShape )

	# build menu from menu definition
	cortexMenu = IECoreMaya.Menu( menuDef, parentMenu, keepCallback = True )


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

	allSceneShapes = set()

	for shape in maya.cmds.ls( sl=True, l=True ):
		# Make sure we have the shape name, it could be a component
		shapeName, _, _ = shape.partition(".f[")
		if maya.cmds.objectType( shapeName ) == "ieSceneShape":
			allSceneShapes.add( shapeName )
		else:
			children = maya.cmds.listRelatives( shapeName, children=True, type="ieSceneShape", fullPath=True ) or []
			allSceneShapes.update(children)
	return list(allSceneShapes)

## Turns on child bounds and switches to component mode
def __componentCallback( sceneShape, *unused ) :

	parent = maya.cmds.listRelatives( sceneShape, parent=True, fullPath=True )[0]
	maya.cmds.selectType( objectComponent=True, allComponents=False, facet=True )
	maya.cmds.hilite( parent )

	maya.cmds.select( clear=True )

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
def _expandAll( sceneShapes, tagName=None, *unused) :

	toSelect = []
	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		newFn = fnS.expandAll( preserveNamespace=True, tagName=tagName )

		toSelect.extend( map( lambda x: x.fullPathName(), newFn ) )
	if toSelect:
		maya.cmds.select( toSelect, replace=True )

## Recursively expand the scene shapes and converts objects to geometry
def _expandAsGeometry( sceneShapes, tagName=None, *unused) :

	for sceneShape in sceneShapes:
		fnS = IECoreMaya.FnSceneShape( sceneShape )
		fnS.convertAllToGeometry( True, tagName )

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
def _setTagsFilterPreviewAttributes( sceneShapes, tagName, *unused) :

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