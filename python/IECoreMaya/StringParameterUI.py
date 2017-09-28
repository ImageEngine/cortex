##########################################################################
#
#  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

import fnmatch
import re

import maya.cmds

import IECore
import IECoreMaya

## A UI for StringParameters. Supports the following parameter user data :
#
# BoolData ["UI"]["acceptsProceduralObjectName"] False
# When true, menu items will be created to set the parameter value to the name
# of an object in a procedural.
#
# BoolData ["UI"]["acceptsProceduralObjectNames"] False
# When true, menu items will be created to add and remove selected procedural
# components names.
#
# BoolData ["UI"]["acceptsCoordinateSystemName"] False
# When true, menu items will be created to set the parameter value to the name
# of a coordinate system in a procedural.
#
# BoolData ["UI"]["acceptsNodeName"] False
# When true, menu items will be created to set the value to the name of
# a node in the scene.
#
# BoolData ["UI"]["acceptsNodeNames"] False
# When true, menu items will be created to set the value to the names of
# a number of nodes in the scene.
#
# StringVectorData ["UI"]["acceptedNodeTypes"] []
# A list of node types to be considered by the "acceptsNodeName" and
# "acceptsNodeNames" features.
#
# StringData ["UI"]["acceptedNodeNameFormat"] "partial"
# Specifies either "partial" or "full", to define whether the shortest
# unique node name will be used for the features above, or whether
# the full path will be used. Specify "parent" or "parentPartial" or
# "parentFull" to get the direct transform parent node path. "parent"
# behaves the same as "parentPartial".
class StringParameterUI( IECoreMaya.ParameterUI ) :

	def __init__( self, node, parameter, **kw ):

		IECoreMaya.ParameterUI.__init__( self, node, parameter, maya.cmds.rowLayout( numberOfColumns = 2 ), **kw )

		self.__label = maya.cmds.text(
			label = self.label(),
			font = "smallPlainLabelFont",
			align = "right",
			annotation = self.description(),
		)

		self.__textField = maya.cmds.textField()

		maya.cmds.setParent("..")

		self.replace( self.node(), self.parameter )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		maya.cmds.connectControl( self.__textField, self.plugName() )
		self._addPopupMenu( parentUI=self.__textField, attributeName = self.plugName() )

		# The popup on the text field itself seems not to be working, so also add it to the
		# label in the mean time.
		self._addPopupMenu( parentUI=self.__label, attributeName = self.plugName(), button1 = True )

	def _popupMenuDefinition( self, **kw ) :

		definition = IECoreMaya.ParameterUI._popupMenuDefinition( self, **kw )

		if not maya.cmds.getAttr( self.plugName(), settable=True ) :
			return definition

		wantsComponentName = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsComponentName = self.parameter.userData()["UI"]["acceptsProceduralObjectName"].value

		wantsComponentNames = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsComponentNames = self.parameter.userData()["UI"]["acceptsProceduralObjectNames"].value

		if wantsComponentName or wantsComponentNames  :

			definition.append( "/ObjectsDivider", { "divider" : True } )
			definition.append( "/Objects/Set To Selected", { "command" : IECore.curry( self.__setToSelectedComponents, not wantsComponentNames ) } )
			if wantsComponentNames :
				definition.append( "/Objects/Add Selected", { "command" : self.__addSelectedComponents } )
				definition.append( "/Objects/Remove Selected", { "command" : self.__removeSelectedComponents } )

			definition.append( "/Objects/Select", { "command" : self.__selectComponents } )

		wantsNodeName = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsNodeName = self.parameter.userData()["UI"]["acceptsNodeName"].value

		wantsNodeNames = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsNodeNames = self.parameter.userData()["UI"]["acceptsNodeNames"].value

		if wantsNodeName or wantsNodeNames :

			lskw = {}
			with IECore.IgnoredExceptions( KeyError ) :
				lskw["type"] = list( self.parameter.userData()["UI"]["acceptedNodeTypes"] )
			with IECore.IgnoredExceptions( KeyError ) :
				if self.parameter.userData()["UI"]["acceptedNodeNameFormat"].value == "full" :
					lskw["long"] = True

			nodeNames = maya.cmds.ls( **lskw )
			with IECore.IgnoredExceptions( KeyError ) :
				if "parent" in self.parameter.userData()["UI"]["acceptedNodeNameFormat"].value :
					for i in range( len( nodeNames ) ) :
						nodeNames[i] = maya.cmds.listRelatives(
							nodeNames[i], parent=True, path=True,
						 	fullPath = (self.parameter.userData()["UI"]["acceptedNodeNameFormat"].value == "parentFull")
						)[0]

			if nodeNames :

				definition.append( "/NodesDivider", { "divider" : True } )

				if wantsNodeName :

					currentValue = maya.cmds.getAttr( self.plugName() )
					for nodeName in nodeNames :
						if nodeName!=currentValue :
							definition.append( "/Nodes/%s" % nodeName, { "command" : IECore.curry( self.__addNodeName, nodeName, clearFirst=True ) } )

				elif wantsNodeNames :

					currentNodes = set( maya.cmds.getAttr( self.plugName() ).split() )
					for nodeName in nodeNames :
						if nodeName in currentNodes :
							definition.append( "/Nodes/Remove/%s" % nodeName, { "command" : IECore.curry( self.__removeNodeName, nodeName ) } )
						else :
							definition.append( "/Nodes/Add/%s" % nodeName, { "command" : IECore.curry( self.__addNodeName, nodeName ) } )

		wantsCoordinateSystem = False
		with IECore.IgnoredExceptions( KeyError ) :
			wantsCoordinateSystem = self.parameter.userData()["UI"]["acceptsCoordinateSystemName"].value

		if wantsCoordinateSystem :

			definition.append( "/CoordinateSystemsDivider", { "divider" : True } )
			definition.append( "/Coordinate Systems/Set To Selected", { "command" : self.__setToSelectedCoordinateSystem } )
			definition.append( "/Coordinate Systems/Select", { "command" : self.__selectCoordinateSystem } )

		return definition

	def __setToSelectedComponents( self, oneOnly ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		components = fnPH.selectedComponentNames()
		components = list( components )
		components.sort()

		if oneOnly :
			maya.cmds.setAttr( self.plugName(), components[0] if components else "", type="string" )
		else :
			maya.cmds.setAttr( self.plugName(), " ".join( components ), type="string" )

	def __addSelectedComponents( self ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		components = fnPH.selectedComponentNames()
		components |= set( maya.cmds.getAttr( self.plugName() ).split() )
		components = list( components )
		components.sort()

		maya.cmds.setAttr( self.plugName(), " ".join( components ), type="string" )

	def __removeSelectedComponents( self ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		components = set( maya.cmds.getAttr( self.plugName() ).split() )
		components -= fnPH.selectedComponentNames()
		components = list( components )
		components.sort()

		maya.cmds.setAttr( self.plugName(), " ".join( components ), type="string" )

	def __selectComponents( self ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )

		regexes = [ re.compile( fnmatch.translate( x ) ) for x in maya.cmds.getAttr( self.plugName() ).split() ]

		toSelect = set()
		for name in fnPH.componentNames() :
			for r in regexes :
				if r.match( name ) is not None :
					toSelect.add( name )
					break

		fnPH.selectComponentNames( toSelect )

	def __setToSelectedCoordinateSystem( self ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		components = fnPH.selectedComponentNames()
		components = list( components )
		components = [ c for c in components if c.startswith( "coordinateSystem:" ) ]

		coordSys = ""
		if components :
			components.sort()
			coordSys = components[0][len( "coordinateSystem:" ):]

		maya.cmds.setAttr( self.plugName(), coordSys, type="string" )

	def __selectCoordinateSystem( self ) :

		fnPH = IECoreMaya.FnProceduralHolder( self.node() )
		fnPH.selectComponentNames( set( [ "coordinateSystem:" + maya.cmds.getAttr( self.plugName() ) ] ) )

	def __addNodeName( self, nodeName, clearFirst=False ) :

		names = set()
		if not clearFirst :
			names = set( maya.cmds.getAttr( self.plugName() ).split() )

		names.add( nodeName )
		names = list( names )
		names.sort()

		maya.cmds.setAttr( self.plugName(), " ".join( names ), type="string" )

	def __removeNodeName( self, nodeName ) :

		names = set( maya.cmds.getAttr( self.plugName() ).split() )
		names.remove( nodeName )
		names = list( names )
		names.sort()

		maya.cmds.setAttr( self.plugName(), " ".join( names ), type="string" )

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.StringParameter, StringParameterUI )
