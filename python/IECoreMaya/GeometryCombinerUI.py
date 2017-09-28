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

from __future__ import with_statement

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
