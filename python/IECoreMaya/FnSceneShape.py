##########################################################################
#
#  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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
class FnSceneShape( maya.OpenMaya.MFnDagNode ) :

	## Initialise the function set for the given procedural object, which may
	# either be an MObject or a node name in string or unicode form.
	def __init__( self, object ) :

		if isinstance( object, str ) or isinstance( object, unicode ) :
			object = StringUtil.dependencyNodeFromString( object )

		maya.OpenMaya.MFnDagNode.__init__( self, object )

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
			
		dagMod = maya.OpenMaya.MDagModifier()
		shapeNode = dagMod.createNode( "ieSceneShape", IECoreMaya.StringUtil.dependencyNodeFromString( parentNode ) )
		dagMod.renameNode( shapeNode, shapeName )
		dagMod.doIt()
		
		fnScS = FnSceneShape( shapeNode )
		
		maya.cmds.sets( fnScS.fullPathName(), add="initialShadingGroup" )
		
		fnScS.findPlug( "objectOnly" ).setLocked( True )
		
		dgMod = maya.OpenMaya.MDGModifier()
		outTime = IECoreMaya.StringUtil.plugFromString( "time1.outTime" )
		dgMod.connect( outTime, fnScS.findPlug( "time" ) )
		dgMod.doIt()
		
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

		queryPaths = self.findPlug( "queryPaths" )
		validIndices = maya.OpenMaya.MIntArray()
		queryPaths.getExistingArrayAttributeIndices( validIndices )
		for i in validIndices:
			# Check if we can reuse a query path
			if queryPaths.elementByLogicalIndex( i ).asString() == path :
				return i

		# Didn't find path, get the next available index
		index = max(validIndices) + 1 if validIndices else 0
		queryPaths.elementByLogicalIndex( index ).setString( path )
		return index
	
	## create the given child for the scene shape
	# Returns a the function set for the child scene shape.
	def __createChild( self, childName, sceneFile, sceneRoot, drawGeo = False, drawChildBounds = False, drawRootBound = True, drawTagsFilter = "", namespace = "" ) :

		if namespace:
			namespace += ":"

		dag = maya.OpenMaya.MDagPath()
		self.getPath( dag )
		dag.pop()
		parentPath = dag.fullPathName()
		childPath = parentPath+"|"+namespace+childName
		try :
			childDag = IECoreMaya.dagPathFromString( childPath )
			childExists = True
		except RuntimeError :
			childExists = False
		
		if childExists :
			shape = maya.cmds.listRelatives( childPath, f=True, type="ieSceneShape" )
			if shape:
				fnChild = IECoreMaya.FnSceneShape( shape[0] )
			else:
				fnChild = IECoreMaya.FnSceneShape.createShape( childPath )
		else:
			fnChild = IECoreMaya.FnSceneShape.create( childName, transformParent = parentPath )

		fnChild.findPlug( "file" ).setString( sceneFile )
		sceneRootName = "/"+childName if sceneRoot == "/" else sceneRoot+"/"+childName
		fnChild.findPlug( "root" ).setString( sceneRootName )
		fnChildTransform = maya.OpenMaya.MFnDagNode( fnChild.parent( 0 ) )
		
		index = self.__queryIndexForPath( "/"+childName )
		outTransform = self.findPlug( "outTransform" ).elementByLogicalIndex( index )
		
		dgMod = maya.OpenMaya.MDGModifier()
		
		childTranslate = fnChildTransform.findPlug( "translate" )
		if childTranslate.isConnected() :
			connections = maya.OpenMaya.MPlugArray()
			childTranslate.connectedTo( connections, True, False )
			dgMod.disconnect( connections[0], childTranslate )
		dgMod.connect( outTransform.child( self.attribute( "outTranslate" ) ), childTranslate )
		
		childRotate = fnChildTransform.findPlug( "rotate" )
		if childRotate.isConnected() :
			connections = maya.OpenMaya.MPlugArray()
			childRotate.connectedTo( connections, True, False )
			dgMod.disconnect( connections[0], childRotate )
		dgMod.connect( outTransform.child( self.attribute( "outRotate" ) ), childRotate )
		
		childScale = fnChildTransform.findPlug( "scale" )
		if childScale.isConnected() :
			connections = maya.OpenMaya.MPlugArray()
			childScale.connectedTo( connections, True, False )
			dgMod.disconnect( connections[0], childScale )
		dgMod.connect( outTransform.child( self.attribute( "outScale" ) ), childScale )
		
		childTime = fnChild.findPlug( "time" )
		if childTime.isConnected() :
			connections = maya.OpenMaya.MPlugArray()
			childTime.connectedTo( connections, True, False )
			dgMod.disconnect( connections[0], childTime )
		dgMod.connect( self.findPlug( "outTime" ), childTime )
		
		dgMod.doIt()

		fnChild.findPlug( "drawGeometry" ).setBool( drawGeo )
		fnChild.findPlug( "drawChildBounds" ).setBool( drawChildBounds )
		fnChild.findPlug( "drawRootBound" ).setBool( drawRootBound )

		if drawTagsFilter:
			parentTags = drawTagsFilter.split()
			childTags = fnChild.sceneInterface().readTags(IECore.SceneInterface.EveryTag)
			commonTags = filter( lambda x: str(x) in childTags, parentTags )
			if not commonTags:
				# Hide that child since it doesn't match any filter
				fnChildTransform.findPlug( "visibility" ).setBool( False )
			else:
				fnChild.findPlug( "drawTagsFilter" ).setString( " ".join( commonTags ) )
		
		return fnChild

	## create the given child for the scene shape
	# Returns a the function set for the child scene shape.
	# If preserveNamespace is True, it creates the child with the same namespace as the one this sceneShape node has.
	def createChild( self, childName, sceneFile, sceneRoot, drawGeo = False, drawChildBounds = False, drawRootBound = True, drawTagsFilter = "", preserveNamespace=False) :

		if preserveNamespace:

			selfNamespaceList = self.fullPathName().split("|")[-1].split( ":" )[:-1]
			selfNamespace = ":".join(selfNamespaceList)
			if selfNamespace:

				originalNS = maya.cmds.namespaceInfo( cur=True, absoluteName=True )
				maya.cmds.namespace( set=":" + selfNamespace )

				try:
					return self.__createChild(childName, sceneFile, sceneRoot, drawGeo, drawChildBounds, drawRootBound, drawTagsFilter, selfNamespace)
				finally:
					maya.cmds.namespace( set=originalNS )

		return self.__createChild(childName, sceneFile, sceneRoot, drawGeo, drawChildBounds, drawRootBound, drawTagsFilter)

	## Expands the scene shape one level down if possible.
	# Returns a list of function sets for the child scene shapes.
	# Missing child transforms and shapes will be created, missing connections and attribute values will be reset.
	# If preserveNamespace is True, it creates transforms and shapes with the same namespace as the one this sceneShape node has.
	def expandOnce( self, preserveNamespace=False ) :

		scene = self.sceneInterface()
		if not scene:
			return []
			
		sceneChildren = sorted( scene.childNames() )
		
		if sceneChildren == []:
			# No children to expand to
			return []

		sceneFile = self.findPlug( "file" ).asString()
		sceneRoot = self.findPlug( "root" ).asString()
		
		# Set querySpace to world (which is world space starting from the root)
		self.findPlug( "querySpace" ).setInt( 0 )
		objectOnlyPlug = self.findPlug( "objectOnly" )
		objectOnlyPlug.setLocked( False )
		objectOnlyPlug.setBool( True )
		objectOnlyPlug.setLocked( True )
		
		drawGeo = self.findPlug( "drawGeometry" ).asBool()
		drawChildBounds = self.findPlug( "drawChildBounds" ).asBool()
		drawRootBound = self.findPlug( "drawRootBound" ).asBool()
		drawTagsFilter = self.findPlug( "drawTagsFilter" ).asString()
		
		newSceneShapeFns = []

		for i, child in enumerate( sceneChildren ):
			
			fnChild = self.createChild( child, sceneFile, sceneRoot, drawGeo, drawChildBounds, drawRootBound, drawTagsFilter, preserveNamespace )
			newSceneShapeFns.append( fnChild )
			
		return newSceneShapeFns
		
	## Recursively expands all levels starting from the scene shape.
	# Returns a list of function sets for all the child scene shapes.
	# If preserveNamespace is True, it creates transforms and shapes with the same namespace as the one this sceneShape node has.
	# If tagName is specfied, each scene in the hierarchy expands only if at least one child has the tag
	def expandAll( self, preserveNamespace=False, tagName=None ):

		newFn = []
		def recursiveExpand( fnSceneShape ):
			
			if tagName and tagName not in fnSceneShape.sceneInterface().readTags( IECore.SceneInterface.DescendantTag ):
				return

			new = fnSceneShape.expandOnce( preserveNamespace )
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
	def convertAllToGeometry( self, preserveNamespace=False ) :

		# Expand scene first, then for each scene shape we turn them into an intermediate object and connect a mesh
		self.expandAll( preserveNamespace )
		transform = maya.cmds.listRelatives( self.fullPathName(), parent=True, f=True )[0]
		
		allSceneShapes = maya.cmds.listRelatives( transform, ad=True, f=True, type="ieSceneShape" )

		for sceneShape in allSceneShapes:
			fn = FnSceneShape( sceneShape )
			fn.findPlug( "querySpace" ).setInt( 1 )
			
			if fn.sceneInterface() and fn.sceneInterface().hasObject():
				fn.convertObjectToGeometry()

			# turn the scene node an intermediateObject so it can't be seen by LiveScene
			fn.findPlug( "intermediateObject" ).setBool( True )

	## Update parameters based on index'th element of queryConvertParameters.
	def __readConvertParams( self, index, parameters ):

		queryConvertParametersPlug = self.findPlug( "queryConvertParameters" )
		convertParamIndices = maya.OpenMaya.MIntArray()
		queryConvertParametersPlug.getExistingArrayAttributeIndices( convertParamIndices )
		values = queryConvertParametersPlug.elementByLogicalIndex( index ).asString().split()
		values = [ str(x) for x in values ] # unicode to str
		IECore.ParameterParser().parse( values, parameters )

	## Set queryConvertParameters attribute from a parametrized object.
	def __setConvertParams( self, index, parameters ):
		paramsStr = " ".join( IECore.ParameterParser().serialise( parameters ) )
		queryConvertParameters = self.findPlug( "queryConvertParameters" )
		queryConvertParameters.elementByLogicalIndex( index ).setString( paramsStr )

	@staticmethod
	def __getFnShape( pathToShape ):

		try :
			return maya.OpenMaya.MFnDagNode( IECoreMaya.StringUtil.dagPathFromString( pathToShape ) )
		except RuntimeError :
			pass

	def __findOrCreateShape( self, transformNode, shapeName, shapeType ):

		pathToShape = transformNode + "|" + shapeName
		fnShape = FnSceneShape.__getFnShape( pathToShape )

		if fnShape and maya.cmds.nodeType( pathToShape ) != shapeType :
			# Rename existing shape
			newName = shapeName + "_orig"
			maya.cmds.rename( pathToShape, newName )
			IECore.msg( IECore.Msg.Level.Warning, "FnSceneShape.__findOrCreateShape", "Renaming incompatible shape %s to %s." % pathToShape, newName )
			fnShape = None

		if not fnShape :
			dagMod = maya.OpenMaya.MDagModifier()
			shapeNode = dagMod.createNode( shapeType, IECoreMaya.StringUtil.dependencyNodeFromString( transformNode ) )
			dagMod.renameNode( shapeNode, shapeName )
			dagMod.doIt()
			
			fnShape = maya.OpenMaya.MFnDagNode( shapeNode )

			if shapeType == "mesh":
				maya.cmds.sets( pathToShape, add="initialShadingGroup" )

		if shapeType == "mesh":
			object = self.sceneInterface().readObject(0.0)
			interpolation = object.interpolation
			shape = fnShape.fullPathName()
			try:
				IECoreMaya.ToMayaMeshConverter.setMeshInterpolationAttribute( shape, interpolation )
			except:
				IECore.msg( IECore.Msg.Level.Warning, "FnSceneShape.__findOrCreateShape", "Failed to set interpolation on %s." % shape )

		return fnShape

	def __findOrCreateShapes( self, transformNode ):

		shapeType, plugStr = self.__mayaCompatibleShapeAndPlug()
		if not (shapeType and plugStr):
			raise Exception, "Scene interface at %s cannot be converted to Maya geometry." % self.sceneInterface().pathAsString()

		shapeName = IECoreMaya.FnDagNode.defaultShapeName( transformNode )

		if shapeType == "nurbsCurve":
			curvesPrimitive = self.sceneInterface().readObject( 0.0 )
			numShapes = curvesPrimitive.numCurves()
			for shapeId in range( numShapes ):
				nameId = '' if numShapes == 1 else str( shapeId ) # Do not add number to the name if there's only one curve, for backward compatibility.
				self.__findOrCreateShape( transformNode, shapeName + nameId, shapeType )

		else:
			self.__findOrCreateShape( transformNode, shapeName, shapeType )

	def __connectShape( self, pathToShape, plugStr, arrayIndex ):

			fnShape = FnSceneShape.__getFnShape( pathToShape )

			plug = fnShape.findPlug( plugStr )
			if plug.isLocked() :
				return

			connections = maya.OpenMaya.MPlugArray()
			if plug.isConnected() :
				plug.connectedTo( connections, True, False )
			if connections.length():
				return

			# Connect this node to the shape.
			dgMod = maya.OpenMaya.MDGModifier()
			dgMod.connect( self.findPlug( "outObjects" ).elementByLogicalIndex( arrayIndex ), plug )
			dgMod.doIt()

	def __connectShapes( self, transformNode = None ):

		shapeType, plugStr = self.__mayaCompatibleShapeAndPlug()
		if not (shapeType and plugStr):
			return

		shapeName = IECoreMaya.FnDagNode.defaultShapeName( transformNode )
		pathToShape = transformNode + "|" + shapeName

		if shapeType == "nurbsCurve":
			curvesPrimitive = self.sceneInterface().readObject( 0.0 )
			numShapes = curvesPrimitive.numCurves()

			# Connect this node to shapes. Set query attributes.

			queryPathsPlug = self.findPlug( "queryPaths" )
			validIndices = maya.OpenMaya.MIntArray()
			queryPathsPlug.getExistingArrayAttributeIndices( validIndices )

			# Find an unused query attributes array index for the time we need to make one.
			if validIndices:
				nextArrayIndex = max( validIndices ) + 1
			else:
				nextArrayIndex = 0

			parameters = IECoreMaya.ToMayaObjectConverter.create( curvesPrimitive ).parameters()

			# { "index" convert param value : queryPaths array index}, to be used to check if we can reuse existing query attributes element.
			existingQueryIndices = {}
			for i in validIndices:
				if queryPathsPlug.elementByLogicalIndex( i ).asString() == "/":
					self.__readConvertParams( i, parameters )
					existingQueryIndices[ parameters["index"].getTypedValue() ] = i

			for shapeId in range( numShapes ):

				# Set query attributes.
				# We need multiple Maya shape nodes to fully convert a curvesPrimitive.
				# To feed geometry data into multiple shapes, ieSceneShape node's "outObjects" attribute needs to output multiple geometries.
				# This can be done using query attributes "queryConvertParameters".
				# e.g. when you want outObjects[ i ] to output j's curve of the CurvesPrimitive,
				#	maya.cmds.setAttr( "mySceneShape.queryConvertParameters[ i ]", "-index %d" % j, type="string" )
				# You also need to set a valid path to the i'th queryPaths element.
				attrIndex = existingQueryIndices.get( shapeId )
				if attrIndex != None:
					arrayIndex = attrIndex # Reuse an existing array element.
				else:
					arrayIndex = nextArrayIndex # Create a new array element.
					nextArrayIndex += 1

					queryPathsPlug.elementByLogicalIndex( arrayIndex ).setString( "/" )
					parameters[ "index" ].setTypedValue( shapeId )
					if "src" in parameters:
						del parameters[ "src" ]
					self.__setConvertParams( arrayIndex, parameters )

				nameId = '' if numShapes == 1 else str( shapeId )
				self.__connectShape( pathToShape + nameId, plugStr, arrayIndex )

		else:
			# Connect this node to one shape.
			arrayIndex = self.__queryIndexForPath( "/" )
			self.__connectShape( pathToShape, plugStr, arrayIndex )

	## Converts the object (if any) in the scene interface into maya geometry.
	# If a shape with the expected name but incompatible type is found under the transform, we rename it and create a new proper shape.
	# The shape is connected to the scene shape object output only if it isn't already connected or locked.
	# transformNode parameter can be used to specify the parent of the geometry. If None, uses the transform of the scene shape.
	def convertObjectToGeometry( self, transformNode = None ):

		if not self.sceneInterface().hasObject():
			return

		if not transformNode:
			# No transform provided, use the transform of the reader
			dag = maya.OpenMaya.MDagPath()
			self.getPath( dag )
			dag.pop()
			transformNode = dag.fullPathName()

		self.__findOrCreateShapes( transformNode )
		self.__connectShapes( transformNode )

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

