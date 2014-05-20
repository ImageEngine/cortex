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

from __future__ import with_statement
import re

import maya.OpenMaya
import maya.cmds

import IECore
import IECoreMaya
import _IECoreMaya
from FnDagNode import FnDagNode
import StringUtil


## A function set for operating on the IECoreMaya::SceneShape type.
class FnSceneShape( maya.OpenMaya.MFnDependencyNode ) :

	## Initialise the function set for the given procedural object, which may
	# either be an MObject or a node name in string or unicode form.
	def __init__( self, object ) :

		if isinstance( object, str ) or isinstance( object, unicode ) :
			object = StringUtil.dependencyNodeFromString( object )

		maya.OpenMaya.MFnDependencyNode.__init__( self, object )

	## Creates a new node under a transform of the specified name. Returns a function set instance operating on this new node.
	@staticmethod
	def create( parentName, transformParent = None ) :
		
		try:
			parentNode = maya.cmds.createNode( "transform", name=parentName, skipSelect=True, parent = transformParent )
		except:
			# The parent name is supposed to be the children names in a sceneInterface, they could be numbers, maya doesn't like that. Use a prefix.
			parentNode = maya.cmds.createNode( "transform", name="sceneShape_"+parentName, skipSelect=True, parent = transformParent )
				
		return FnSceneShape.createShape( parentNode )
	
	## Create a scene shape under the given node. Returns a function set instance operating on this shape.
	@staticmethod
	def createShape( parentNode ) :

		parentShort = parentNode.rpartition( "|" )[-1]
		numbersMatch = re.search( "[0-9]+$", parentShort )
		if numbersMatch is not None :
			numbers = numbersMatch.group()
			shapeName = parentShort[:-len(numbers)] + "SceneShape" + numbers
		else :
			shapeName = parentShort + "SceneShape"
			
		shapeNode = maya.cmds.createNode( "ieSceneShape", name=shapeName, parent=parentNode, skipSelect=True )
		
		fnScS = FnSceneShape( shapeNode )
		maya.cmds.sets( fnScS.fullPathName(), add="initialShadingGroup" )
		maya.cmds.setAttr( fnScS.fullPathName()+".objectOnly", l=True )
		maya.cmds.connectAttr( "time1.outTime", fnScS.fullPathName()+'.time' )
		
		return fnScS

	## Returns a set of the names of any currently selected components.
	def selectedComponentNames( self ) :

		result = set()

		s = maya.OpenMaya.MSelectionList()
		maya.OpenMaya.MGlobal.getActiveSelectionList( s )
		
		allComponents = self.componentNames()

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

						result.add( allComponents[ a[j] ] )

			except :
				pass

		return result

	## Selects the components specified by the passed names. If replace is True
	# then the current selection is deselected first.
	def selectComponentNames( self, componentNames ) :

		if not isinstance( componentNames, set ) :
			componentNames = set( componentNames )

		fullPathName = self.fullPathName()
		allNames = self.componentNames()
		toSelect = []
		for i, name in enumerate( allNames ):
			if name in componentNames:
				toSelect.append( fullPathName + ".f[" + str( i ) + "]" )

		maya.cmds.select( clear=True )
		maya.cmds.selectMode( component=True )
		maya.cmds.hilite( fullPathName )
		if toSelect:
			maya.cmds.select( toSelect, r=True )
			
	
	## Returns the full path name to this node.
	def fullPathName( self ) :
		
		try :
			f = maya.OpenMaya.MFnDagNode( self.object() )
			return f.fullPathName()
		except :
			pass

		return self.name()
		
	def sceneInterface( self ) :

		return _IECoreMaya._sceneShapeSceneInterface( self )
	
	def componentNames( self ) :

		return _IECoreMaya._sceneShapeComponentNames( self )
	
	## Returns True if the scene shape can be expanded.
	# We assume that if the objectOnly flag is on, it means the scene shape has already been expanded so return False.
	# Can only be expanded if the scene interface for the scene shape has children.	
	def canBeExpanded( self ) :

		# An already expanded scene should have objectOnly on
		if not maya.cmds.getAttr( self.fullPathName()+".objectOnly" ):
			# Check if you have any children to expand to
			if self.sceneInterface().childNames():
				return True
		return False

	## Returns True if the scene shape can be collapsed.
	# We assume that if the objectOnly flag is off, the scene shape is already collapsed.
	def canBeCollapsed( self ) :

		# if already collapsed, objectOnly is off
		return maya.cmds.getAttr( self.fullPathName()+".objectOnly" )

	## Returns the index in the queryPaths which matches the given path.
	# If the path isn't already in the queries, add it and return the new index.
	def __queryIndexForPath( self, path ):

		node = self.fullPathName()
		index = None
		validIndices = maya.cmds.getAttr( node+".queryPaths", mi=True )
		if not validIndices:
			index = 0
		else:
			for id in validIndices:
				# Check if we can reuse a query path
				if maya.cmds.getAttr( node+".queryPaths["+str(id)+"]" ) == path:
					index = id
					break
			if index is None:
				# Didn't find path, get the next available index
				index = max( i for i in validIndices ) +1
				
		maya.cmds.setAttr( node+".queryPaths["+str(index)+"]", path, type="string" )
		return index
	
	## create the given child for the scene shape
	# Returns a the function set for the child scene shape.
	def createChild( self, childName, sceneFile, sceneRoot, drawGeo = False, drawChildBounds = False, drawRootBound = True, drawTagsFilter = "" ) :
		
		node = self.fullPathName()
		transform = maya.cmds.listRelatives( node, parent=True, f=True )[0]
		
		if maya.cmds.objExists( transform+"|"+childName ):
			shape = maya.cmds.listRelatives( transform+"|"+childName, f=True, type="ieSceneShape" )
			if shape:
				fnChild = IECoreMaya.FnSceneShape( shape[0] )
			else:
				fnChild = IECoreMaya.FnSceneShape.createShape( transform+"|"+childName )
		else:
			fnChild = IECoreMaya.FnSceneShape.create( childName, transformParent = transform )

		childNode = fnChild.fullPathName()
		childTransform = maya.cmds.listRelatives( childNode, parent=True, f=True )[0]
		maya.cmds.setAttr( childNode+".file", sceneFile, type="string" )
		sceneRootName = "/"+childName if sceneRoot == "/" else sceneRoot+"/"+childName
		maya.cmds.setAttr( childNode+".root", sceneRootName, type="string" )
		
		index = self.__queryIndexForPath( "/"+childName )
		outTransform = node+".outTransform["+str(index)+"]"
		if not maya.cmds.isConnected( outTransform+".outTranslate", childTransform+".translate" ):
			maya.cmds.connectAttr( outTransform+".outTranslate", childTransform+".translate", f=True )
		if not maya.cmds.isConnected( outTransform+".outRotate", childTransform+".rotate" ):
			maya.cmds.connectAttr( outTransform+".outRotate", childTransform+".rotate", f=True )
		if not maya.cmds.isConnected( outTransform+".outScale", childTransform+".scale" ):
			maya.cmds.connectAttr( outTransform+".outScale", childTransform+".scale", f=True )

		maya.cmds.setAttr( childNode+".drawGeometry", drawGeo )
		maya.cmds.setAttr( childNode+".drawChildBounds", drawChildBounds )
		maya.cmds.setAttr( childNode+".drawRootBound", drawRootBound )

		if drawTagsFilter:
			parentTags = drawTagsFilter.split()
			childTags = fnChild.sceneInterface().readTags(IECore.SceneInterface.EveryTag)
			commonTags = filter( lambda x: str(x) in childTags, parentTags )
			if not commonTags:
				# Hide that child since it doesn't match any filter
				maya.cmds.setAttr( childTransform+".visibility", 0 )
			else:
				maya.cmds.setAttr( childNode+".drawTagsFilter", " ".join(commonTags),type="string" )
		
		# Connect child time to its parent so they're in sync
		if not maya.cmds.isConnected( node+".outTime", childNode+".time" ):
			maya.cmds.connectAttr( node+".outTime", childNode+".time", f=True )
		
		return fnChild

	## Expands the scene shape one level down if possible.
	# Returns a list of function sets for the child scene shapes.
	# Missing child transforms and shapes will be created, missing connections and attribute values will be reset.
	def expandOnce( self ) :

		node = self.fullPathName()
		transform = maya.cmds.listRelatives( node, parent=True, f=True )[0]
		scene = self.sceneInterface()
		if not scene:
			return []
			
		sceneChildren = scene.childNames()
		
		if sceneChildren == []:
			# No children to expand to
			return []

		sceneFile = maya.cmds.getAttr( node+".file" )
		sceneRoot = maya.cmds.getAttr( node+".root" )
		
		# Set querySpace to world (which is world space starting from the root)
		maya.cmds.setAttr( node+".querySpace", 0 )
		maya.cmds.setAttr( node+".objectOnly", l=False )
		maya.cmds.setAttr( node+".objectOnly", 1 )
		maya.cmds.setAttr( node+".objectOnly", l=True )
		
		drawGeo = maya.cmds.getAttr( node+".drawGeometry" )
		drawChildBounds = maya.cmds.getAttr( node+".drawChildBounds" )
		drawRootBound = maya.cmds.getAttr( node+".drawRootBound" )
		drawTagsFilter = maya.cmds.getAttr( node+".drawTagsFilter" )
		
		newSceneShapeFns = []

		for i, child in enumerate( sceneChildren ):
			
			fnChild = self.createChild( child, sceneFile, sceneRoot, drawGeo, drawChildBounds, drawRootBound, drawTagsFilter )
			newSceneShapeFns.append( fnChild )
			
		return newSceneShapeFns
	
	## Recursively expands all levels starting from the scene shape.
	# Returns a list of function sets for all the child scene shapes.
	def expandAll( self ):

		newFn = []
		def recursiveExpand( fnSceneShape ):
			
			new = fnSceneShape.expandOnce()
			newFn.extend( new )
			for n in new:
				recursiveExpand( n )
			
		recursiveExpand( self )
		
		return newFn
	
	## Collapses all children up to this scene shape.
	def collapse( self ) :

		node = self.fullPathName()
		transform = maya.cmds.listRelatives( node, parent=True, f=True )[0]
		allTransformChildren = maya.cmds.listRelatives( transform, f=True, type = "transform" ) or []
		
		for child in allTransformChildren:
			# \todo check for connections and new parented nodes
			maya.cmds.delete( child )
		
		maya.cmds.setAttr( node+".objectOnly", l=False )
		maya.cmds.setAttr( node+".objectOnly", 0 )
		maya.cmds.setAttr( node+".objectOnly", l=True )
		maya.cmds.setAttr( node+".intermediateObject", 0 )
	
	## Returns tuple of maya type and input plug name that match the object in the scene interface, by checking the objectType tags.
	# Returns (None, None) if no object in the scene interface or the object isn't compatible with maya geometry we can create.
	def __mayaCompatibleShapeAndPlug( self ) :

		result = (None, None)
		if self.sceneInterface().hasObject():
			tags = self.sceneInterface().readTags( IECore.SceneInterface.LocalTag )
			if "ObjectType:MeshPrimitive" in tags:
				result = ( "mesh", "inMesh" )
			elif "ObjectType:CurvesPrimitive" in tags:
				result = ( "nurbsCurve", "create" )
			elif "ObjectType:CoordinateSystem" in tags:
				result = ( "locator", "localPosition" )

		return result
	
	## Recursively converts all objects in the scene interface to compatible maya geometry
	# All scene shape nodes in the hierarchy are turned into an intermediate object.
	def convertAllToGeometry( self ) :

		# Expand scene first, then for each scene shape we turn them into an intermediate object and connect a mesh
		self.expandAll()
		transform = maya.cmds.listRelatives( self.fullPathName(), parent=True, f=True )[0]
		
		allSceneShapes = maya.cmds.listRelatives( transform, ad=True, f=True, type="ieSceneShape" )

		for sceneShape in allSceneShapes:
			maya.cmds.setAttr( sceneShape+".querySpace", 1 )
			
			fn = FnSceneShape( sceneShape )
			if fn.sceneInterface() and fn.sceneInterface().hasObject():
				fn.convertObjectToGeometry()

			# turn the scene node an intermediateObject so it can't be seen by MayaScene
			maya.cmds.setAttr( sceneShape+".intermediateObject", 1 )
	
	## Converts the object (if any) in the scene interface into maya geometry.
	# If a shape with the expected name but incompatible type is found under the transform, we rename it and create a new proper shape.
	# The shape is connected to the scene shape object output only if it isn't already connected or locked.
	# transformNode parameter can be used to specify the parent of the geometry. If None, uses the transform of the scene shape.
	def convertObjectToGeometry( self, transformNode = None ):
		
		if not self.sceneInterface().hasObject():
			return

		node = self.fullPathName()
		if not transformNode:
			# No transform provided, use the transform of the reader
			transformNode = maya.cmds.listRelatives( node, f=True, p=True )[0]

		type, plug = self.__mayaCompatibleShapeAndPlug()
		if not (type and plug):
			raise Exception, "Scene interface at %s cannot be converted to Maya geometry." % self.sceneInterface().pathAsString()
		
		shapeName = IECoreMaya.FnDagNode.defaultShapeName( transformNode )
		shape = transformNode + "|" + shapeName
		create = False
		if not maya.cmds.objExists( shape ):
			create = True				
		elif maya.cmds.nodeType( shape ) != type:
			# Rename existing shape
			newName = shapeName + "_orig"
			maya.cmds.rename( shape, newName )
			IECore.msg( IECore.Msg.Level.Warning, "FnSceneShape.convertObjectToGeometry", "Renaming incompatible shape %s to %s." % shape, newName )
			create = True

		if create:
			maya.cmds.createNode( type, parent = transformNode, name = shapeName )
			if type == "mesh":
				maya.cmds.sets(shape, add="initialShadingGroup" )

		index = self.__queryIndexForPath( "/" )
		if not maya.cmds.listConnections( shape+"."+plug, source = True, destination = False ) and not maya.cmds.getAttr( shape+"."+plug, l=True ):
			maya.cmds.connectAttr( node+'.outObjects['+str(index)+']', shape+"."+plug, f=True )
			if type == "mesh":
				object = self.sceneInterface().readObject(0.0)
				interpolation = object.interpolation
				try:
					IECoreMaya.ToMayaMeshConverter.setMeshInterpolationAttribute( shape, interpolation )
				except:
					IECore.msg( IECore.Msg.Level.Warning, "FnSceneShape.convertObjectToGeometry", "Failed to set interpolation on %s." % shape )

	def createLocatorAtTransform( self, path ) :
		
		node = self.fullPathName()
		transform = maya.cmds.listRelatives( node, parent=True, f=True )[0]
		locator = "|" + maya.cmds.spaceLocator( name = path.replace( "/", "_" ) + "Transform" )[0]
		
		index = self.__queryIndexForPath( path )
		outTransform = node+".outTransform["+str(index)+"]"
		maya.cmds.connectAttr( outTransform+".outTranslate", locator + ".translate" )
		maya.cmds.connectAttr( outTransform+".outRotate", locator + ".rotate" )
		maya.cmds.connectAttr( outTransform+".outScale", locator + ".scale" )
		locator = transform + "|" + maya.cmds.parent( locator, transform, relative=True )[0]
		
		return locator
	
	def createLocatorAtPoints( self, path, childPlugSuffixes ) :
		
		node = self.fullPathName()
		transform = maya.cmds.listRelatives( node, parent=True, f=True )[0]
		
		locators = []
		for childPlugSuffix in childPlugSuffixes :
			index = self.__queryIndexForPath( path )
			outBound = node+".outBound["+str(index)+"]"
			locator = "|" + maya.cmds.spaceLocator( name = path.replace( "/", "_" ) + childPlugSuffix )[0]
			maya.cmds.connectAttr( outBound + ".outBound" + childPlugSuffix, locator + ".translate" )
			locators.append( transform + "|" + maya.cmds.parent( locator, transform, relative=True )[0] )
		
		return locators
			
	## Returns the maya node type that this function set operates on
	@classmethod
	def _mayaNodeType( cls ):
		
		return "ieSceneShape"

