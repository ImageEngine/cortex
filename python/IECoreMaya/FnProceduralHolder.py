##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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
import re

import maya.OpenMaya
import maya.cmds

import IECore
import IECoreMaya
import _IECoreMaya
from FnParameterisedHolder import FnParameterisedHolder
from FnDagNode import FnDagNode

## A function set for operating on the IECoreMaya::ProceduralHolder type.
class FnProceduralHolder( FnParameterisedHolder ) :

	## Initialise the function set for the given procedural object, which may
	# either be an MObject or a node name in string or unicode form.
	def __init__( self, object ) :

		FnParameterisedHolder.__init__( self, object )

	## Creates a new node under a transform of the specified name and holding
	# the specified procedural class type. Returns a function set instance operating on this new node.
	@staticmethod
	def create( parentName, className, classVersion=-1 ) :

		fnDN = FnDagNode.createShapeWithParent( parentName, "ieProceduralHolder" )
		fnPH = FnProceduralHolder( fnDN.object() )
		maya.cmds.sets( fnPH.fullPathName(), add="initialShadingGroup" )
		fnPH.setProcedural( className, classVersion, undoable=False ) # undo for the node creation is all we need
		
		return fnPH

	## Convenience method to call setParameterised with the environment variable
	# for the searchpaths set to "IECORE_PROCEDURAL_PATHS".
	def setProcedural( self, className, classVersion=None, undoable=True ) :

		self.setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS", undoable )

	## Convenience method to return the ParameterisedProcedural class held inside this
	# node.
	def getProcedural( self ) :

		return self.getParameterised()[0]

	## Returns a set of the names of the components within the procedural. These names
	# are specified by the procedural by setting the "name" attribute in the
	# renderer
	def componentNames( self ) :
	
		# Makes sure that the scene has been built at least once with the
		# current node values, so there will be something in the plug
		# for us to read.
		self.scene()
		
		attributeName = "%s.proceduralComponents" % self.fullPathName()
	
		result = set()
		for i in range( maya.cmds.getAttr( attributeName, size=True ) ) :
			result.add( maya.cmds.getAttr( "%s[%i]" % ( attributeName, i ) ) )
		
		return result
		
	## Returns a set of the names of any currently selected components. These names
	# are specified by the procedural by setting the "name" attribute in the
	# renderer.
	def selectedComponentNames( self ) :

		result = set()

		s = maya.OpenMaya.MSelectionList()
		maya.OpenMaya.MGlobal.getActiveSelectionList( s )

		fullPathName = self.fullPathName()
		for i in range( 0, s.length() ) :

			try :

				p = maya.OpenMaya.MDagPath()
				c = maya.OpenMaya.MObject()
				s.getDagPath( i, p, c )

				if p.node()==self.object() :

					fnC = maya.OpenMaya.MFnSingleIndexedComponent( c )
					a = maya.OpenMaya.MIntArray()
					fnC.getElements( a )

					for j in range( 0, a.length() ) :

						result.add( maya.cmds.getAttr( fullPathName + ".proceduralComponents[" + str( a[j] ) + "]" ) )

			except :
				pass

		return result

	## Selects the components specified by the passed names. If replace is True
	# then the current selection is deselected first.
	def selectComponentNames( self, componentNames ) :

		if not isinstance( componentNames, set ) :
			componentNames = set( componentNames )

		fullPathName = self.fullPathName()
		validIndices = maya.cmds.getAttr( fullPathName + ".proceduralComponents", multiIndices=True )
		toSelect = []
		for i in validIndices :
			componentName = maya.cmds.getAttr( fullPathName + ".proceduralComponents[" + str( i ) + "]" )
			if componentName in componentNames :
				toSelect.append( fullPathName + ".f[" + str( i ) + "]" )

		maya.cmds.select( clear=True )
		maya.cmds.selectMode( component=True )
		maya.cmds.hilite( fullPathName )
		for s in toSelect :
			maya.cmds.select( s, add=True )

	## Returns the IECoreGL.Scene displayed by this node.
	def scene( self ) :

		return _IECoreMaya._proceduralHolderScene( self )

	## Creates a hierarchy of maya nodes representing the output of the procedural
	# at the current frame. If parent is specified then the geometry will be parented
	# under it, otherwise it will share the parent of the procedural. Returns the
	# path to the top level transform of the new geometry.
	def convertToGeometry( self, parent=None ) :
	
		procedural = self.getProcedural()
		if not procedural :
			return None
			
		renderer = IECore.CapturingRenderer()
		
		selected = self.selectedComponentNames()
		
		objectFilter = set()
		
		for sel in selected:
			# we want to output the selected components, and all of their children:
			objectFilter.add( sel )
			objectFilter.add( sel + "/*" )
		
		if len( objectFilter ):
			renderer.setOption( "cp:objectFilter", IECore.StringVectorData( list( objectFilter ) ) )
		
		with IECore.WorldBlock( renderer ) :
			procedural.render( renderer )
		
		world = renderer.world()
		
		# These things have a tendency to generate big, useless stacks of groups.
		# Only the tree structure is actually useful to us, so lets remove these if possible:
		self.__collapseGroups( world )
		self.__removeChildlessGroups( world )
		
		if parent is None :
			parent = maya.cmds.listRelatives( self.fullPathName(), fullPath=True, parent=True )
			if parent is not None :
				parent = parent[0]
			
		IECoreMaya.ToMayaGroupConverter( world ).convert( parent )
		
		return maya.cmds.listRelatives( parent, fullPath=True )[-1]
	
	## Returns a path to a plug which outputs the transform for the specified component. The plug
	# will have "componentTranslate", "componentRotate" and "componentScale" children with the appropriate values.
	def componentTransformPlugPath( self, componentName ) :
	
		i = self.__componentIndex( componentName )
		return self.fullPathName() + ".componentTransform[" + str( i ) + "]"

	## Returns a path to a plug which outputs the bounding box of the specified component. The 
	# plug will have "componentBoundMin", "componentBoundMax" and "componentBoundCenter" children with the
	# appropriate values.
	def componentBoundPlugPath( self, componentName ) :
	
		i = self.__componentIndex( componentName )
		return self.fullPathName() + ".componentBound[" + str( i ) + "]"

	def __componentIndex( self, componentName ) :
	
		fullPathName = self.fullPathName()

		queryIndices = maya.cmds.getAttr( fullPathName + ".componentQueries", multiIndices=True )
		if not queryIndices :
			i = 0
		else :
			for i in queryIndices :
				if maya.cmds.getAttr( fullPathName + ".componentQueries[" + str( i ) + "]" ) == componentName :
					return i
			i += 1
			
		maya.cmds.setAttr( fullPathName + ".componentQueries[" + str( i ) + "]", componentName, type="string" )
		return i
					
	def __removeChildlessGroups( self, group ) :
		
		children = group.children()
		
		for c in group.children():
			if isinstance( c, IECore.Group ) :
				if len( c.children() ) == 0:
					group.removeChild( c )
				else:
					self.__removeChildlessGroups( c )
		

	def __collapseGroups( self, group ) :

		children = group.children()

		if len( children ) == 0 :
			return

		# if this group is the parent of exactly one group, we merge it with
		# its child
		elif len( children ) == 1 :

			child = children[0]

			if isinstance( child, IECore.Group ):

				parentGlobalTransform = group.globalTransformMatrix()

				parentLocalTransform = group.transformMatrix()
				childLocalTransform = child.transformMatrix()

				group.setTransform( IECore.MatrixTransform( parentLocalTransform * childLocalTransform ) )

				childName = child.getAttribute("name")
				if childName:
					group.addState( IECore.AttributeState( { "name" : childName } ) )
					
				group.removeChild( child )

				for c in child.children():
					group.addChild( c )

				# ok - lets call the function again on this group if it's got children:
				if len( group.children() ) != 0:
					self.__collapseGroups( group )

				return

		else:
			# ok - we gots multiple children, so lets not mess around with it,
			# lets just recurse:

			for c in group.children():
				if isinstance( c, IECore.Group ) :
					self.__collapseGroups( c )
	
	def createLocatorAtTransform( self, path ):
		
		proceduralParent = maya.cmds.listRelatives( self.fullPathName(), parent=True, fullPath=True )[0]
		outputPlug = self.componentTransformPlugPath( path )
		locator = "|" + maya.cmds.spaceLocator( name = path.replace( "/", "_" ) + "Transform" )[0]
		maya.cmds.connectAttr( outputPlug + ".componentTranslate", locator + ".translate" )
		maya.cmds.connectAttr( outputPlug + ".componentRotate", locator + ".rotate" )
		maya.cmds.connectAttr( outputPlug + ".componentScale", locator + ".scale" )
		loc = proceduralParent + "|" + maya.cmds.parent( locator, proceduralParent, relative=True )[0]
		
		return loc
		
		
		
	def createLocatorAtPoints( self, path, childPlugSuffixes ) :
		
		proceduralParent = maya.cmds.listRelatives( self.fullPathName(), parent=True, fullPath=True )[0]
		
		locators = []
		for childPlugSuffix in childPlugSuffixes :
			outputPlug = self.componentBoundPlugPath( path )
			locator = "|" + maya.cmds.spaceLocator( name = path.replace( "/", "_" ) + childPlugSuffix )[0]
			maya.cmds.connectAttr( outputPlug + ".componentBound" + childPlugSuffix, locator + ".translate" )
			locators.append( proceduralParent + "|" + maya.cmds.parent( locator, proceduralParent, relative=True )[0] )
		return locators

	
	## Returns the maya node type that this function set operates on
	@classmethod
	def _mayaNodeType( cls ):
		
		return "ieProceduralHolder"
