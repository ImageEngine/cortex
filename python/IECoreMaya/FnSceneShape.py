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
			fnDN = FnDagNode.createShapeWithParent( parentName, "ieSceneShape" )
		except:
			fnDN = FnDagNode.createShapeWithParent( "sceneShape_"+parentName, "ieSceneShape" )
		fnScS = FnSceneShape( fnDN.object() )
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


	def expandScene( self ) :

		node = self.fullPathName()
		transform = maya.cmds.listRelatives( node, parent=True, f=True )[0]
		scene = self.sceneInterface()
		sceneChildren = scene.childNames()
		
		if sceneChildren == []:
			# No children to expand to
			return []

		sceneFile = maya.cmds.getAttr( node+".sceneFile" )
		sceneRoot = maya.cmds.getAttr( node+".sceneRoot" )
		
		maya.cmds.setAttr( node+".querySpace", 1 )
		maya.cmds.setAttr( node+".objectOnly", l=False )
		maya.cmds.setAttr( node+".objectOnly", 1 )
		maya.cmds.setAttr( node+".objectOnly", l=True )
		
		drawGeo = maya.cmds.getAttr( node+".drawGeometry" )
		drawChildBounds = maya.cmds.getAttr( node+".drawChildBounds" )
		drawRootBound = maya.cmds.getAttr( node+".drawRootBound" )
		
		newSceneShapeFns = []
		
		for i, child in enumerate( sceneChildren ):
			
			maya.cmds.setAttr( node+".sceneQueries["+str(i)+"]", "/"+child, type="string" )
			
			# Create sceneShape file for child
			fnChild = IECoreMaya.FnSceneShape.create( child )
			childNode = fnChild.fullPathName()
			childTransform = maya.cmds.listRelatives( childNode, parent=True, f=True )[0]
			maya.cmds.setAttr( childNode+".sceneFile", sceneFile, type="string" )
			sceneRootName = "/"+child if sceneRoot == "/" else sceneRoot+"/"+child
			maya.cmds.setAttr( childNode+".sceneRoot", sceneRootName, type="string" )
			
			maya.cmds.connectAttr( node+".objectTransform["+str(i)+"].objectTranslate", childTransform+".translate" )
			maya.cmds.connectAttr( node+".objectTransform["+str(i)+"].objectRotate", childTransform+".rotate" )
			maya.cmds.connectAttr( node+".objectTransform["+str(i)+"].objectScale", childTransform+".scale" )
			
			maya.cmds.setAttr( childNode+".drawGeometry", drawGeo )
			maya.cmds.setAttr( childNode+".drawChildBounds", drawChildBounds )
			maya.cmds.setAttr( childNode+".drawRootBound", drawRootBound )

			maya.cmds.parent( childTransform, transform, relative=True )
			
			newSceneShapeFns.append( fnChild )
			
		return newSceneShapeFns
	
	
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


	## Returns the maya node type that this function set operates on
	@classmethod
	def _mayaNodeType( cls ):
		
		return "ieSceneShape"

