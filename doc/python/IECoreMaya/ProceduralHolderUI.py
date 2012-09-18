
import maya.cmds
import maya.mel

import IECore
import IECoreMaya

__dagMenuCallbacks = []
## Registers a callback to be used when creating the right click dag
# menu for procedural holders. Callbacks should have the following signature :
#
# callback( menu, proceduralHolder ).
def addDagMenuCallback( callback ) :

	if not callback in __dagMenuCallbacks :
		__dagMenuCallbacks.append( callback )

## Removes a callback previously added with addDagMenuCallback.		
def removeDagMenuCallback( callback ) :

	__dagMenuCallbacks.remove( callback )

## This is forwarded to by the ieProceduralHolderDagMenuProc function in
# ieProceduralHolder.mel
def _dagMenu( menu, proceduralHolder ) :

	if maya.cmds.nodeType( proceduralHolder )!="ieProceduralHolder" :
		children = maya.cmds.listRelatives( proceduralHolder, children=True, type="ieProceduralHolder", fullPath=True )
		if not children :
			return
		else :
			proceduralHolder = children[0]

	maya.cmds.setParent( menu, menu=True )
	maya.cmds.menuItem(
		label = "Component",
		radialPosition = "N",
		command = IECore.curry( __componentCallback, proceduralHolder )
	)
	maya.cmds.menuItem(
		label = "Object",
		radialPosition = "W",
		command = IECore.curry( __objectCallback, proceduralHolder ),
	)
	
	maya.cmds.menuItem(
		label = "Print Component Names",
		radialPosition = "NE",
		command = IECore.curry( __printComponents, proceduralHolder )
	)
	
	fnPH = IECoreMaya.FnProceduralHolder( proceduralHolder )
	if fnPH.selectedComponentNames() :
		maya.cmds.menuItem(
			label = "Print Selected Component Names",
			radialPosition = "E",
			command = IECore.curry( __printSelectedComponents, proceduralHolder )
		)
		
	if fnPH.selectedComponentNames() :
		
		maya.cmds.menuItem(
			label = "Create Locator",
			radialPosition = "SE",
			subMenu = True,
		)
		
		maya.cmds.menuItem(
			label = "At Bound Min",
			radialPosition = "N",
			command = IECore.curry( __createLocatorAtPoints, proceduralHolder, [ "Min" ] ),
		)
		
		maya.cmds.menuItem(
			label = "At Bound Max",
			radialPosition = "NE",
			command = IECore.curry( __createLocatorAtPoints, proceduralHolder, [ "Max" ] ),
		)
		
		maya.cmds.menuItem(
			label = "At Bound Min And Max",
			radialPosition = "E",
			command = IECore.curry( __createLocatorAtPoints, proceduralHolder, [ "Min", "Max" ] ),
		)
		
		maya.cmds.menuItem(
			label = "At Bound Centre",
			radialPosition = "SE",
			command = IECore.curry( __createLocatorAtPoints, proceduralHolder, [ "Center" ] ),
		)
		
		maya.cmds.menuItem(
			label = "At Transform Origin",
			radialPosition = "S",
			command = IECore.curry( __createLocatorWithTransform, proceduralHolder ),
		)
		
	maya.cmds.setParent( menu, menu=True )	
	
	maya.cmds.menuItem(
		label = "Convert To Geometry",
		radialPosition = "S",
		command = "import IECoreMaya; IECoreMaya.ProceduralHolderUI._convertToGeometry( \"" + proceduralHolder + "\" )",
	)
	
	for c in __dagMenuCallbacks :
	
		c( menu, proceduralHolder )
	
def __componentCallback( proceduralHolder, *unused ) :

	parent = maya.cmds.listRelatives( proceduralHolder, parent=True, fullPath=True )[0]
	maya.mel.eval( "doMenuComponentSelection( \"" + parent + "\", \"facet\" )" )

def __objectCallback( proceduralHolder, *unused ) :

	parent = maya.cmds.listRelatives( proceduralHolder, parent=True, fullPath=True )[0]
	maya.cmds.hilite( parent, unHilite=True )
	selection = maya.cmds.ls( selection=True )
	maya.mel.eval( "changeSelectMode -object" )
	if selection :
		maya.cmds.select( selection, replace=True )
	else :
		maya.cmds.select( clear=True )

def __printComponents( proceduralHolder, *unused ) :

	fnP = IECoreMaya.FnProceduralHolder( proceduralHolder )
	names = fnP.componentNames()
	names = list( names )
	names.sort()
	print " ".join( names ) ,

def __printSelectedComponents( proceduralHolder, *unused ) :

	fnP = IECoreMaya.FnProceduralHolder( proceduralHolder )
	selectedNames = fnP.selectedComponentNames()
	selectedNames = list( selectedNames )
	selectedNames.sort()
	print " ".join( selectedNames ) ,

def _convertToGeometry( proceduralHolder, *unused ) :

	fnP = IECoreMaya.FnProceduralHolder( proceduralHolder )
	
	proceduralParent = maya.cmds.listRelatives( fnP.fullPathName(), parent=True, fullPath=True )[0]
	geometryParent = maya.cmds.createNode( "transform", name = "convertedProcedural" )
	
	proceduralTransform = maya.cmds.xform( proceduralParent, query=True, worldSpace=True, matrix=True )
	maya.cmds.xform( geometryParent, worldSpace=True, matrix=proceduralTransform )
	
	fnP.convertToGeometry( parent=geometryParent )	

	maya.cmds.select( geometryParent, replace=True )

def __createLocatorAtPoints( proceduralHolder, childPlugSuffixes, *unused ) :
	
	fnPH = IECoreMaya.FnProceduralHolder( proceduralHolder )
	selectedNames = fnPH.selectedComponentNames()
	
	proceduralParent = maya.cmds.listRelatives( fnPH.fullPathName(), parent=True, fullPath=True )[0]

	locators = []
	for name in selectedNames :
		for childPlugSuffix in childPlugSuffixes :
			outputPlug = fnPH.componentBoundPlugPath( name )
			locator = "|" + maya.cmds.spaceLocator( name = name.replace( "/", "_" ) + childPlugSuffix )[0]
			maya.cmds.connectAttr( outputPlug + ".componentBound" + childPlugSuffix, locator + ".translate" )
			locators.extend( maya.cmds.parent( locator, proceduralParent, relative=True ) )
		
	maya.cmds.select( locators, replace=True )

def __createLocatorWithTransform( proceduralHolder, *unused ) :
	
	fnPH = IECoreMaya.FnProceduralHolder( proceduralHolder )
	selectedNames = fnPH.selectedComponentNames()

	proceduralParent = maya.cmds.listRelatives( fnPH.fullPathName(), parent=True, fullPath=True )[0]
	
	locators = []
	for name in selectedNames :
		outputPlug = fnPH.componentTransformPlugPath( name )
		locator = "|" + maya.cmds.spaceLocator( name = name.replace( "/", "_" ) + "Transform" )[0]
		maya.cmds.connectAttr( outputPlug + ".componentTranslate", locator + ".translate" )
		maya.cmds.connectAttr( outputPlug + ".componentRotate", locator + ".rotate" )
		maya.cmds.connectAttr( outputPlug + ".componentScale", locator + ".scale" )
		locators.extend( maya.cmds.parent( locator, proceduralParent, relative=True ) )

	maya.cmds.select( locators, replace=True )
