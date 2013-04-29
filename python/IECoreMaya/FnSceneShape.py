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
	def create( parentName ) :
		
		try:
			parentNode = maya.cmds.createNode( "transform", name=parentName, skipSelect=True )
		except:
			# The parent name is supposed to be the children names in a sceneInterface, they could be numbers, maya doesn't like that. Use a prefix.
			parentNode = maya.cmds.createNode( "transform", name="sceneShape_"+parentName, skipSelect=True )
				
		return FnSceneShape.createForTransform( parentNode )
	
	@staticmethod
	def createForTransform( parentNode ) :
		
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
		
		allChildren = self.childrenNames()

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

						result.add( allChildren[ a[j] ] )

			except :
				pass

		return result

	## Selects the components specified by the passed names. If replace is True
	# then the current selection is deselected first.
	def selectComponentNames( self, componentNames ) :

		if not isinstance( componentNames, set ) :
			componentNames = set( componentNames )

		fullPathName = self.fullPathName()
		allnames = self.childrenNames()
		for i, name in enumerate( allNames ):
			if name in componentNames:
				toSelect.append( fullPathName + ".f[" + str( i ) + "]" )

		maya.cmds.select( clear=True )
		maya.cmds.selectMode( component=True )
		maya.cmds.hilite( fullPathName )
		for s in toSelect :
			maya.cmds.select( s, add=True )
	
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
		
	def childrenNames( self ) :

		return _IECoreMaya._sceneShapeChildrenNames( self )
	
	def canBeExpanded( self ) :
		
		# An already expanded scene should have objectOnly on
		if not maya.cmds.getAttr( self.fullPathName()+".objectOnly" ):
			# Check if you have any children to expand to
			if len( self.childrenNames() ) > 1:
				return True
		return False

	def canBeCollapsed( self ) :
		
		# if already collapsed, objectOnly is off
		return maya.cmds.getAttr( self.fullPathName()+".objectOnly" )


	def expandScene( self, ignoreObjectOnlyFlag = False ) :

		node = self.fullPathName()
		transform = maya.cmds.listRelatives( node, parent=True, f=True )[0]
		scene = self.sceneInterface()
		if not scene:
			return []
			
		sceneChildren = scene.childNames()
		
		if sceneChildren == []:
			# No children to expand to
			return []
			
		newSceneShapeFns = []
		
		if not ignoreObjectOnlyFlag and maya.cmds.getAttr( node+".objectOnly"):
			# Already expanded, return existing scene shapes
			childTransforms = maya.cmds.listRelatives( transform, f=True, type="transform" ) or []
			for t in childTransforms:
				childSceneShape = maya.cmds.listRelatives( t, f=True, type="ieSceneShape" ) or []
				if childSceneShape:
					newSceneShapeFns.append( IECoreMaya.FnSceneShape( childSceneShape[0] ) )
		
		if len(newSceneShapeFns):
			return newSceneShapeFns

		sceneFile = maya.cmds.getAttr( node+".file" )
		sceneRoot = maya.cmds.getAttr( node+".root" )
		
		maya.cmds.setAttr( node+".querySpace", 1 )
		maya.cmds.setAttr( node+".objectOnly", l=False )
		maya.cmds.setAttr( node+".objectOnly", 1 )
		maya.cmds.setAttr( node+".objectOnly", l=True )
		
		drawGeo = maya.cmds.getAttr( node+".drawGeometry" )
		drawChildBounds = maya.cmds.getAttr( node+".drawChildBounds" )
		drawRootBound = maya.cmds.getAttr( node+".drawRootBound" )

		for i, child in enumerate( sceneChildren ):
			
			if maya.cmds.objExists( transform+"|"+child ):
				shape = maya.cmds.listRelatives( transform+"|"+child, f=True, type="ieSceneShape" )
				if shape:
					fnChild = IECoreMaya.FnSceneShape( shape[0] )
					newSceneShapeFns.append( fnChild )
					continue
				else:
					fnChild = IECoreMaya.FnSceneShape.createForTransform( transform+"|"+child )
			else:
				fnChild = IECoreMaya.FnSceneShape.create( child )

			maya.cmds.setAttr( node+".queryPaths["+str(i)+"]", "/"+child, type="string" )
			childNode = fnChild.fullPathName()
			childTransform = maya.cmds.listRelatives( childNode, parent=True, f=True )[0]
			maya.cmds.setAttr( childNode+".file", sceneFile, type="string" )
			sceneRootName = "/"+child if sceneRoot == "/" else sceneRoot+"/"+child
			maya.cmds.setAttr( childNode+".root", sceneRootName, type="string" )
			
			maya.cmds.connectAttr( node+".outTransform["+str(i)+"].outTranslate", childTransform+".translate" )
			maya.cmds.connectAttr( node+".outTransform["+str(i)+"].outRotate", childTransform+".rotate" )
			maya.cmds.connectAttr( node+".outTransform["+str(i)+"].outScale", childTransform+".scale" )

			maya.cmds.setAttr( childNode+".drawGeometry", drawGeo )
			maya.cmds.setAttr( childNode+".drawChildBounds", drawChildBounds )
			maya.cmds.setAttr( childNode+".drawRootBound", drawRootBound )

			maya.cmds.parent( childTransform, transform, relative=True )
			
			newSceneShapeFns.append( fnChild )
			
		return newSceneShapeFns
	

	def expandAllChildren( self, ignoreObjectOnlyFlag = False ):
		
		newFn = []
		def recursiveExpand( fnSceneShape ):
			
			new = fnSceneShape.expandScene( ignoreObjectOnlyFlag )
			newFn.extend( new )
			for n in new:
				recursiveExpand( n )
			
		recursiveExpand( self )
		
		return newFn
	
	
	def collapseScene( self ) :
		
		node = self.fullPathName()
		transform = maya.cmds.listRelatives( node, parent=True, f=True )[0]
		allTransformChildren = maya.cmds.listRelatives( transform, f=True, type = "transform" ) or []
		
		for child in allTransformChildren:
			# Do a bunch of tests first!
			maya.cmds.delete( child )
		
		maya.cmds.setAttr( node+".objectOnly", l=False )
		maya.cmds.setAttr( node+".objectOnly", 0 )
		maya.cmds.setAttr( node+".objectOnly", l=True )
		maya.cmds.setAttr( node+".intermediateObject", 0 )
	
	def convertToGeometry( self, ignoreObjectOnlyFlag = False ) :

		# Expand scene first, then for each scene shape we turn them into an intermediate object and connect a mesh
		self.expandAllChildren( ignoreObjectOnlyFlag )
		transform = maya.cmds.listRelatives( self.fullPathName(), parent=True, f=True )[0]
		
		allSceneShapes = maya.cmds.listRelatives( transform, ad=True, f=True, type="ieSceneShape" )

		def getObjectShapeName( parentNode ):
			
			parentShort = parentNode.rpartition( "|" )[-1]
			numbersMatch = re.search( "[0-9]+$", parentShort )
			if numbersMatch is not None :
				numbers = numbersMatch.group()
				shapeName = parentShort[:-len(numbers)] + "Shape" + numbers
			else :
				shapeName = parentShort + "Shape"
			
			return shapeName

		for sceneShape in allSceneShapes:
			maya.cmds.setAttr( sceneShape+".querySpace", 1 )
			
			fn = FnSceneShape( sceneShape )
			if fn.sceneInterface() and fn.sceneInterface().hasObject():

				parent = maya.cmds.listRelatives( sceneShape, parent=True, f=True )[0]
				object = fn.sceneInterface().readObject( 0.0 )

				if isinstance( object, IECore.MeshPrimitive ) or isinstance( object, IECore.SpherePrimitive ) or isinstance( object, IECore.CurvesPrimitive ) or isinstance( object, IECore.CoordinateSystem ):
					shapeName = getObjectShapeName( parent )
				else:
					# Not compatible with what can be in the outputObjects sceneShape plug
					continue
				if maya.cmds.objExists( parent+"|"+shapeName ):
					# Already there
					continue
				
				index = None
				validIndices = maya.cmds.getAttr( sceneShape+".queryPaths", mi=True )
				
				if validIndices == [] or validIndices is None:
					index = 0
				else:
					for i in validIndices:
						if maya.cmds.getAttr( sceneShape+".queryPaths["+str(i)+"]") == "/":
							index = i
					if index is None:
						# Didn't find "/", get the next available index
						index = max( i for i in validIndices ) +1
				
				maya.cmds.setAttr( sceneShape+".queryPaths["+str(index)+"]", "/", type="string" )
				
				if isinstance( object, IECore.MeshPrimitive ) or isinstance( object, IECore.SpherePrimitive ):
					mesh = maya.cmds.createNode( "mesh", parent = parent, name = shapeName )
					maya.cmds.connectAttr( sceneShape+'.outObjects['+str(index)+']', mesh+".inMesh" )
					maya.cmds.sets( mesh, add="initialShadingGroup" )
				elif isinstance( object, IECore.CurvesPrimitive ):
					curve = maya.cmds.createNode( "nurbsCurve", parent = parent, name = shapeName )
					maya.cmds.connectAttr( sceneShape+'.outObjects['+str(index)+']', curve+".create" )
				elif isinstance( object, IECore.CoordinateSystem ):
					loc = maya.cmds.spaceLocator( name = shapeName )
					maya.cmds.parent( loc[0], parent )
					maya.cmds.connectAttr( sceneShape+'.outObjects['+str(index)+']', loc[0]+".localPosition" )

			# turn the scene node an intermediateObject so it can't be seen by MayaScene
			maya.cmds.setAttr( sceneShape+".intermediateObject", 1 )

	## Returns the maya node type that this function set operates on
	@classmethod
	def _mayaNodeType( cls ):
		
		return "ieSceneShape"

