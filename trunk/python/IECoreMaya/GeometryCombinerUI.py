import maya.cmds

import IECore
import IECoreMaya

# we don't want to export anything, we just want to register callbacks
__all__ = []

def __geometryCombiner( plugPath, createIfMissing=False ) :

	connections = maya.cmds.listConnections( plugPath, source=True, destination=False, type="ieGeometryCombiner" )
	
	if connections :
		return connections[0]
		
	if not createIfMissing :
		return ""
	
	result = maya.cmds.createNode( "ieGeometryCombiner", skipSelect=True )
	maya.cmds.connectAttr( result + ".outputGroup", plugPath, force=True )
	maya.cmds.setAttr( result + ".convertPrimVars", 1 )
	maya.cmds.setAttr( result + ".convertBlindData", 1 )
	
	return result

def __selectedGeometry() :

	return maya.cmds.ls( selection=True, type=( "mesh", "nurbsCurve" ), noIntermediate=True, leaf=True, dag=True )

def __addSelected( plugPath ) :

	selection = __selectedGeometry()
	if not selection :
		return
			
	combiner = __geometryCombiner( plugPath, createIfMissing=True )
	existingInputs = __inputGeometry( plugPath )
	
	for s in selection :
	
		if s in existingInputs :
			continue
	
		if maya.cmds.nodeType( s )=="mesh" :
			maya.cmds.connectAttr( s + ".worldMesh", combiner + ".inputGeometry", nextAvailable=True, force=True )
		if maya.cmds.nodeType( s )=="nurbsCurve" :
			maya.cmds.connectAttr( s + ".worldSpace", combiner + ".inputGeometry", nextAvailable=True, force=True )	

def __inputGeometry( plugPath, plugs=False ) :

	combiner = __geometryCombiner( plugPath )
	if not combiner :
		return []
		
	connections = maya.cmds.listConnections( combiner + ".inputGeometry", source=True, destination=False, shapes=True, plugs=plugs )
	if connections is None :
		return []
	else :
		return connections
	
def __removeSelected( plugPath ) :

	combiner = __geometryCombiner( plugPath )
	if not combiner :
		return
		
	inputPlugs = __inputGeometry( plugPath, plugs=True )
	inputNodes = [ IECoreMaya.StringUtil.nodeFromAttributePath( x ) for x in inputPlugs ]
	
	for s in __selectedGeometry() :
		i = -1
		with IECore.IgnoredExceptions( ValueError ) :
			i = inputNodes.index( s )
		if s != -1 :
			maya.cmds.disconnectAttr( inputPlugs[i], combiner + ".inputGeometry", nextAvailable=True )
	
def __select( plugPath ) :

	maya.cmds.select( __inputGeometry( plugPath ), replace=True )

def __menuCallback( definition, parameter, node ) :
	
	active = False
	with IECore.IgnoredExceptions( KeyError ) :
		active = parameter.userData()["maya"]["useGeometryCombiner"].value
		
	if not active :
		return
			
	definition.append( "/InputGeometryDivider", { "divider" : True } )
	
	plugPath = IECoreMaya.FnParameterisedHolder( node ).parameterPlugPath( parameter )
	
	definition.append( "/Input Geometry/Add Selected", { "command" : IECore.curry( __addSelected, plugPath ) } )
	definition.append( "/Input Geometry/Remove Selected", { "command" : IECore.curry( __removeSelected, plugPath ) } )
	definition.append( "/Input Geometry/Select", { "command" : IECore.curry( __select, plugPath ) } )
	
IECoreMaya.ParameterUI.registerPopupMenuCallback( __menuCallback )
