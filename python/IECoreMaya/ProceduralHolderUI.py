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
	geometryParent = maya.cmds.createNode( "transform", name = "convertedProcedural", skipSelect=True )
	
	proceduralTransform = maya.cmds.xform( proceduralParent, query=True, worldSpace=True, matrix=True )
	maya.cmds.xform( geometryParent, worldSpace=True, matrix=proceduralTransform )
	
	fnP.convertToGeometry( parent=geometryParent )	

	maya.cmds.select( geometryParent, replace=True )

def __createLocatorAtPoints( proceduralHolder, childPlugSuffixes, *unused ) :
	
	fnPH = IECoreMaya.FnProceduralHolder( proceduralHolder )
	selectedNames = fnPH.selectedComponentNames()

	locators = []
	for name in selectedNames :
		locators.extend( fnPH.createLocatorAtPoints( name, childPlugSuffixes ) )
		
	maya.cmds.select( locators, replace=True )

def __createLocatorWithTransform( proceduralHolder, *unused ) :
	
	fnPH = IECoreMaya.FnProceduralHolder( proceduralHolder )
	selectedNames = fnPH.selectedComponentNames()

	locators = []
	for name in selectedNames :
		locators.append( fnPH.createLocatorAtTransform( name ) )

	maya.cmds.select( locators, replace=True )
