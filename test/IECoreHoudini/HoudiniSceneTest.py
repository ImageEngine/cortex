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

import unittest

import hou

import IECore
import IECoreHoudini

class HoudiniSceneTest( IECoreHoudini.TestCase ) :
	
	def buildScene( self ) :
		
		obj = hou.node( "/obj" )
		sub1 = obj.createNode( "subnet", "sub1" )
		sub2 = obj.createNode( "subnet", "sub2" )
		box1 = sub1.createOutputNode( "geo", "box1", run_init_scripts=False )
		box1.createNode( "box", "actualBox" )
		actualBox = box1.children()[0]
		bname = actualBox.createOutputNode( "name" )
		bname.parm( "name1" ).set( "/" )
		torus = box1.createNode( "torus" )
		tname = torus.createOutputNode( "name" )
		tname.parm( "name1" ).set( "/gap/torus" )
		merge = bname.createOutputNode( "merge" )
		merge.setInput( 1, tname )
		merge.setRenderFlag( True )
		box2 = obj.createNode( "geo", "box2", run_init_scripts=False )
		box2.createNode( "box", "actualBox" )
		torus1 = sub1.createNode( "geo", "torus1", run_init_scripts=False )
		torus1.createNode( "torus", "actualTorus" )
		torus2 = torus1.createOutputNode( "geo", "torus2", run_init_scripts=False )
		torus2.createNode( "torus", "actualTorus" )
		
		return IECoreHoudini.HoudiniScene()
	
	def testChildNames( self ) :
	
		scene = self.buildScene()
		self.assertEqual( sorted( scene.childNames() ), [ "box2", "sub1", "sub2" ] )
		
		child = scene.child( "sub1" )
		self.assertEqual( sorted( child.childNames() ), [ "box1", "torus1" ] )
		
		child2 = child.child( "torus1" )
		self.assertEqual( sorted( child2.childNames() ), [ "torus2" ] )
		
		child3 = child2.child( "torus2" )
		self.assertEqual( sorted( child3.childNames() ), [] )
		
		box1 = child.child( "box1" )
		self.assertEqual( sorted( box1.childNames() ), [ "gap" ] )
		
		gap = box1.child( "gap" )
		self.assertEqual( sorted( gap.childNames() ), [ "torus" ] )
		
		self.assertEqual( gap.child( "torus" ).childNames(), [] )
		self.assertEqual( scene.child( "box2" ).childNames(), [] )
		self.assertEqual( scene.child( "sub2" ).childNames(), [] )
	
	def testIndirectInputs( self ) :
		
		scene = self.buildScene()
		hou.node( "/obj/sub1/torus1" ).setInput( 0, hou.node( "/obj/sub1" ).indirectInputs()[0] )
		
		self.assertEqual( sorted( scene.childNames() ), [ "box2", "sub1", "sub2" ] )
		
		child = scene.child( "sub1" )
		self.assertEqual( sorted( child.childNames() ), [ "box1", "torus1" ] )
		
		child2 = child.child( "torus1" )
		self.assertEqual( sorted( child2.childNames() ), [ "torus2" ] )
		
		child3 = child2.child( "torus2" )
		self.assertEqual( sorted( child3.childNames() ), [] )
		
		box1 = child.child( "box1" )
		self.assertEqual( sorted( box1.childNames() ), [ "gap" ] )
		
		gap = box1.child( "gap" )
		self.assertEqual( sorted( gap.childNames() ), [ "torus" ] )
		
		self.assertEqual( gap.child( "torus" ).childNames(), [] )
		self.assertEqual( scene.child( "box2" ).childNames(), [] )
		self.assertEqual( scene.child( "sub2" ).childNames(), [] )
	
	def testHasChild( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.hasChild( "box2" ), True )
		self.assertEqual( scene.hasChild( "sub1" ), True )
		self.assertEqual( scene.hasChild( "sub2" ), True )
		self.assertEqual( scene.hasChild( "fake" ), False )
		
		child = scene.child( "sub1" )
		self.assertEqual( child.hasChild( "torus1" ), True )
		self.assertEqual( child.hasChild( "torus2" ), False )
		self.assertEqual( child.child( "torus1" ).hasChild( "torus2" ), True )
		self.assertEqual( child.hasChild( "fake" ), False )
		
		self.assertEqual( child.hasChild( "box1" ), True )
		self.assertEqual( child.child( "box1" ).hasChild( "gap" ), True )
		self.assertEqual( child.child( "box1" ).hasChild( "torus" ), False )
		self.assertEqual( child.child( "box1" ).child( "gap" ).hasChild( "torus" ), True )
	
	def testNames( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.name(), "/" )
		self.assertEqual( scene.child( "box2" ).name(), "box2" )
		self.assertEqual( scene.child( "sub2" ).name(), "sub2" )
		
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.name(), "sub1" )
		self.assertEqual( sub1.child( "box1" ).name(), "box1" )
		self.assertEqual( sub1.child( "box1" ).child( "gap" ).name(), "gap" )
		self.assertEqual( sub1.child( "box1" ).child( "gap" ).child( "torus" ).name(), "torus" )
		
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.name(), "torus1" )
		self.assertEqual( torus1.child( "torus2" ).name(), "torus2" )
	
	def testPaths( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.path(), [] )
		self.assertEqual( scene.pathAsString(), "/" )
		self.assertEqual( scene.child( "box2" ).path(), [ "box2" ] )
		self.assertEqual( scene.child( "box2" ).pathAsString(), "/box2" )
		self.assertEqual( scene.child( "sub2" ).path(), [ "sub2" ] )
		self.assertEqual( scene.child( "sub2" ).pathAsString(), "/sub2" )
		
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.path(), [ "sub1" ] )
		self.assertEqual( sub1.pathAsString(), "/sub1" )
		self.assertEqual( sub1.child( "box1" ).path(), [ "sub1", "box1" ] )
		self.assertEqual( sub1.child( "box1" ).pathAsString(), "/sub1/box1" )
		self.assertEqual( sub1.child( "box1" ).child( "gap" ).path(), [ "sub1", "box1", "gap" ] )
		self.assertEqual( sub1.child( "box1" ).child( "gap" ).pathAsString(), "/sub1/box1/gap" )
		self.assertEqual( sub1.child( "box1" ).child( "gap" ).child( "torus" ).path(), [ "sub1", "box1", "gap", "torus" ] )
		self.assertEqual( sub1.child( "box1" ).child( "gap" ).child( "torus" ).pathAsString(), "/sub1/box1/gap/torus" )
		
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.path(), [ "sub1", "torus1" ] )
		self.assertEqual( torus1.pathAsString(), "/sub1/torus1" )
		self.assertEqual( torus1.child( "torus2" ).path(), [ "sub1", "torus1", "torus2" ] )
		self.assertEqual( torus1.child( "torus2" ).pathAsString(), "/sub1/torus1/torus2" )
		
		## \todo: add a harder test. 3 connections at the same level, and 3 one level deep
		
		self.assertRaises( RuntimeError, scene.child, "idontexist" )
		self.assertEqual( scene.child( "idontexist", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		
		# test the node that does exist according to houdini, but is actually a grandchild in our world
		self.assertRaises( RuntimeError, scene.child, "box1" )
		self.assertEqual( scene.child( "box1", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )		
		
	def testSceneMethod( self ) :
		
		scene = self.buildScene()
		
		self.assertEqual( scene.scene( [ "sub2" ] ).pathAsString(), "/sub2" )
		
		torus1 = scene.scene( [ "sub1", "torus1" ] )
		self.assertEqual( torus1.name(), "torus1" )
		self.assertEqual( torus1.path(), [ "sub1", "torus1" ] )
		self.assertEqual( torus1.pathAsString(), "/sub1/torus1" )
		self.assertEqual( torus1.hasChild( "torus2" ), True )
		self.assertEqual( torus1.childNames(), [ "torus2" ] )
		
		# does it still return absolute paths if we've gone to another location?
		sub1 = scene.scene( [ "sub1" ] )
		self.assertEqual( sub1.scene( [] ).name(), "/" )
		self.assertEqual( sub1.scene( [ "sub1", "torus1", "torus2" ] ).pathAsString(), "/sub1/torus1/torus2" )
		self.assertEqual( sub1.scene( [ "box2" ] ).pathAsString(), "/box2" )
		
		self.assertRaises( RuntimeError, scene.scene, [ "idontexist" ] )
		self.assertEqual( scene.scene( [ "idontexist" ], IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		
		# test the node that does exist according to houdini, but is actually a grandchild in our world
		self.assertRaises( RuntimeError, scene.scene, [ "box1" ] )
		self.assertEqual( scene.scene( [ "box1" ], IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )

	def testHasObject( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.hasObject(), False )
		self.assertEqual( scene.child( "box2" ).hasObject(), True )
		self.assertEqual( scene.child( "sub2" ).hasObject(), False )
		
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.hasObject(), False )
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.hasObject(), True )
		self.assertEqual( torus1.child( "torus2" ).hasObject(), True )
		self.assertEqual( sub1.child( "box1" ).hasObject(), True )
		self.assertEqual( sub1.child( "box1" ).child( "gap" ).hasObject(), False )
		self.assertEqual( sub1.child( "box1" ).child( "gap" ).child( "torus" ).hasObject(), True )
	
	def testTags( self ) :
		
		# no tags by default
		scene = self.buildScene()
		self.assertEqual( scene.readTags(), [] )
		self.assertFalse( scene.hasTag( "any" ) )
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.readTags(), [] )
		self.assertFalse( sub1.hasTag( "any" ) )
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.readTags(), [] )
		self.assertFalse( torus1.hasTag( "any" ) )
		
		def addTags( node, tags ) :
			
			parm = node.addSpareParmTuple( hou.StringParmTemplate( "ieTags", "ieTags", 1, "" ) )
			parm.set( [ tags ] )
		
		# we can add tags to OBJ nodes, but they do not trickle up automatically
		addTags( hou.node( "/obj/sub1/torus1" ), "yellow" )
		addTags( hou.node( "/obj/box1" ), "sop top" )
		self.assertEqual( scene.readTags(), [] )
		self.assertFalse( scene.hasTag( "yellow" ) )
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.readTags(), [] )
		self.assertFalse( sub1.hasTag( "yellow" ) )
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.readTags(), [ "yellow" ] )
		self.assertTrue( torus1.hasTag( "yellow" ) )
		box1 = sub1.child( "box1" )
		self.assertEqual( box1.readTags(), [ "sop", "top" ] )
		self.assertTrue( box1.hasTag( "sop" ) )
		self.assertTrue( box1.hasTag( "top" ) )
		self.assertFalse( box1.hasTag( "yellow" ) )
		
		def addSopTags( node, tag, primRange ) :
			
			group = node.createOutputNode( "group" )
			group.parm( "crname" ).set( tag )
			group.parm( "groupop" ).set( 1 ) # by range
			group.parm( "rangestart" ).set( primRange[0] )
			group.parm( "rangeend" ).deleteAllKeyframes()
			group.parm( "rangeend" ).set( primRange[1] )
			group.parm( "select2" ).set( 1 )
			group.setRenderFlag( True )
		
		# we can add tags to SOPs using groups, but they do not trickle up automatically
		boxObj = hou.node( "/obj/box1" )
		addSopTags( boxObj.renderNode(), "ieTag_itsABox", ( 0, 5 ) ) # box only
		addSopTags( boxObj.renderNode(), "notATag", ( 0, 5 ) ) # box only
		addSopTags( boxObj.renderNode(), "ieTag_itsATorus", ( 6, 105 ) ) # torus only
		addSopTags( boxObj.renderNode(), "ieTag_both:and", ( 3, 50 ) ) # parts of each
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.readTags(), [] )
		self.assertFalse( sub1.hasTag( "yellow" ) )
		box1 = sub1.child( "box1" )
		self.assertEqual( box1.readTags(), [ "sop", "top", "itsABox", "both:and" ] )
		self.assertTrue( box1.hasTag( "sop" ) )
		self.assertTrue( box1.hasTag( "top" ) )
		self.assertTrue( box1.hasTag( "itsABox" ) )
		self.assertTrue( box1.hasTag( "both:and" ) )
		self.assertFalse( box1.hasTag( "itsATorus" ) )
		gap = box1.child( "gap" )
		self.assertEqual( gap.readTags(), [] )
		self.assertFalse( gap.hasTag( "sop" ) )
		self.assertFalse( gap.hasTag( "top" ) )
		self.assertFalse( gap.hasTag( "itsATorus" ) )
		torus = gap.child( "torus" )
		self.assertEqual( torus.readTags(), [ "itsATorus", "both:and" ] )
		self.assertTrue( torus.hasTag( "itsATorus" ) )
		self.assertTrue( torus.hasTag( "both:and" ) )
		self.assertFalse( torus.hasTag( "sop" ) )
		self.assertFalse( torus.hasTag( "top" ) )
		self.assertFalse( torus.hasTag( "itsABox" ) )
	
	def testLinks( self ) :
		
		# at this point, only SceneCacheNodes can define links
		scene = self.buildScene()
		self.assertFalse( scene.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertEqual( scene.readAttribute( IECore.LinkedScene.linkAttribute, 0 ), None )
		sub1 = scene.child( "sub1" )
		self.assertFalse( sub1.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertEqual( sub1.readAttribute( IECore.LinkedScene.linkAttribute, 0 ), None )
		torus1 = sub1.child( "torus1" )
		self.assertFalse( torus1.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertEqual( torus1.readAttribute( IECore.LinkedScene.linkAttribute, 0 ), None )
	
	def testDeletedPath( self ) :
		
		scene = self.buildScene()
		sub1 = scene.child( "sub1" )
		torus1 = sub1.child( "torus1" )
		
		hou.node( "/obj/sub1/torus1" ).destroy()
		
		self.assertRaises( RuntimeError, IECore.curry( torus1.scene, [ "sub1", "torus1" ] ) )
		self.assertEqual( torus1.scene( [ "sub1", "torus1" ], IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, IECore.curry( torus1.child, "torus2" ) )
		self.assertEqual( torus1.child( "torus2", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, torus1.childNames )
		self.assertRaises( RuntimeError, torus1.hasObject )
		self.assertRaises( RuntimeError, IECore.curry( torus1.readBound, 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( torus1.readObject, 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( torus1.readTransform, 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( torus1.readTransformAsMatrix, 0.0 ) )
	
	def testReadMesh( self ) :
		
		scene = self.buildScene()
		hou.node( "/obj/sub1" ).parmTuple( "t" ).set( [ 1, 2, 3 ] )
		hou.node( "/obj/sub1" ).parmTuple( "r" ).set( [ 10, 20, 30 ] )
		
		box1 = scene.child( "sub1" ).child( "box1" )
		mesh = box1.readObject( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		
		vertList = list( mesh["P"].data )
		self.assertEqual( len( vertList ), 8 )
		
		# check the verts are in local space
		self.assertEqual( vertList.count( IECore.V3f( -0.5, -0.5, 0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( 0.5, -0.5, 0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( -0.5, 0.5, 0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( 0.5, 0.5, 0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( -0.5, 0.5, -0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( 0.5, 0.5, -0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( -0.5, -0.5, -0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( 0.5, -0.5, -0.5 ) ), 1 )
	
		# check read primvars
		self.assertEqual( mesh["P"], box1.readObjectPrimitiveVariables( [ "P" ], 0 )["P"] )

	def testAnimatedMesh( self ) :
		
		scene = self.buildScene()
		shape = hou.node( "/obj/box1/actualBox" )
		deformer = shape.createOutputNode( "twist" )
		deformer.parm( "paxis" ).set( 1 )
		deformer.parm( "strength" ).setExpression( "10*($T+(1.0/$FPS))" )
		
		box1 = scene.child( "sub1" ).child( "box1" )
		mesh0   = box1.readObject( 0 )
		mesh0_5 = box1.readObject( 0.5 )
		mesh1   = box1.readObject( 1 )
		# the mesh hasn't moved because the deformer isn't the renderable SOP
		self.assertEqual( mesh0, mesh0_5 )
		self.assertEqual( mesh0, mesh1 )
		self.assertEqual( len(mesh0["P"].data), 8 )
		self.assertEqual( mesh0["P"].data[0].x, -0.5 )
		self.assertEqual( mesh0_5["P"].data[0].x, -0.5 )
		self.assertEqual( mesh1["P"].data[0].x, -0.5 )
		
		deformer.setRenderFlag( True )
		mesh0   = box1.readObject( 0 )
		mesh0_5 = box1.readObject( 0.5 )
		mesh1   = box1.readObject( 1 )
		self.assertEqual( len(mesh0["P"].data), 8 )
		self.assertEqual( len(mesh0_5["P"].data), 8 )
		self.assertEqual( len(mesh1["P"].data), 8 )
		self.assertEqual( mesh0["P"].data[0].x, -0.5 )
		self.assertAlmostEqual( mesh0_5["P"].data[0].x, -0.521334, 6 )
		self.assertAlmostEqual( mesh1["P"].data[0].x, -0.541675, 6 )
	
	def testReadBound( self ) :
		
		scene = self.buildScene()
		hou.node( "/obj/sub1" ).parmTuple( "t" ).set( [ 1, 1, 1 ] )
		hou.node( "/obj/sub1/torus1" ).parmTuple( "t" ).set( [ 2, 2, 2 ] )
		hou.node( "/obj/sub1/torus2" ).parmTuple( "t" ).set( [ -1, 0, 2 ] )
		hou.node( "/obj/box1" ).parmTuple( "t" ).set( [ -1, -1, -1 ] )
		# to make the bounds nice round numbers
		hou.node( "/obj/sub1/torus1/actualTorus" ).parm( "rows" ).set( 100 )
		hou.node( "/obj/sub1/torus1/actualTorus" ).parm( "cols" ).set( 100 )
		hou.node( "/obj/sub1/torus2/actualTorus" ).parm( "rows" ).set( 100 )
		hou.node( "/obj/sub1/torus2/actualTorus" ).parm( "cols" ).set( 100 )
		
		self.assertEqual( scene.child( "box2" ).readBound( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		
		sub1 = scene.child( "sub1" )
		box1 = sub1.child( "box1" )
		self.assertEqual( box1.readBound( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		
		torus1 = sub1.child( "torus1" )
		torus2 = torus1.child( "torus2" )
		self.assertEqual( torus2.readBound( 0 ), IECore.Box3d( IECore.V3d( -1.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 1.5 ) ) )
		self.assertEqual( torus1.readBound( 0 ), IECore.Box3d( IECore.V3d( -2.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 3.5 ) ) )
		self.assertEqual( sub1.readBound( 0 ), IECore.Box3d( IECore.V3d( -1.5 ), IECore.V3d( 3.5, 2.5, 5.5 ) ) )
		self.assertEqual( scene.readBound( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 4.5, 3.5, 6.5 ) ) )
		
	def testAnimatedBound( self ) :
		
		scene = self.buildScene()
		hou.node( "/obj/sub1" ).parmTuple( "t" ).set( [ 1, 1, 1 ] )
		hou.node( "/obj/sub1/torus1" ).parm( "tx" ).setExpression( "$T+(1.0/$FPS)" )
		hou.node( "/obj/sub1/torus1" ).parm( "ty" ).setExpression( "$T+(1.0/$FPS)" )
		hou.node( "/obj/sub1/torus1" ).parm( "tz" ).setExpression( "$T+(1.0/$FPS)" )
		hou.node( "/obj/sub1/torus2" ).parmTuple( "t" ).set( [ -1, 0, 2 ] )
		hou.node( "/obj/box1" ).parm( "tx" ).setExpression( "-($T+(1.0/$FPS))" )
		hou.node( "/obj/box1" ).parm( "ty" ).setExpression( "-($T+(1.0/$FPS))" )
		hou.node( "/obj/box1" ).parm( "tz" ).setExpression( "-($T+(1.0/$FPS))" )
		# to make the bounds nice round numbers
		hou.node( "/obj/sub1/torus1/actualTorus" ).parm( "rows" ).set( 100 )
		hou.node( "/obj/sub1/torus1/actualTorus" ).parm( "cols" ).set( 100 )
		hou.node( "/obj/sub1/torus2/actualTorus" ).parm( "rows" ).set( 100 )
		hou.node( "/obj/sub1/torus2/actualTorus" ).parm( "cols" ).set( 100 )
		
		self.assertEqual( scene.child( "box2" ).readBound( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		
		sub1 = scene.child( "sub1" )
		box1 = sub1.child( "box1" )
		self.assertEqual( box1.readBound( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		
		torus1 = sub1.child( "torus1" )
		torus2 = torus1.child( "torus2" )
		self.assertEqual( torus2.readBound( 0 ), IECore.Box3d( IECore.V3d( -1.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 1.5 ) ) )
		self.assertEqual( torus1.readBound( 0 ), IECore.Box3d( IECore.V3d( -2.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 3.5 ) ) )
		self.assertEqual( sub1.readBound( 0 ), IECore.Box3d( IECore.V3d( -2.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 3.5 ) ) )
		self.assertEqual( scene.readBound( 0 ), IECore.Box3d( IECore.V3d( -1.5, -0.5, -0.5 ), IECore.V3d( 2.5, 1.5, 4.5 ) ) )
		
		# time 1
		self.assertEqual( box1.readBound( 1 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		self.assertEqual( torus2.readBound( 1 ), IECore.Box3d( IECore.V3d( -1.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 1.5 ) ) )
		self.assertEqual( torus1.readBound( 1 ), IECore.Box3d( IECore.V3d( -2.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 3.5 ) ) )
		self.assertEqual( sub1.readBound( 1 ), IECore.Box3d( IECore.V3d( -1.5 ), IECore.V3d( 2.5, 1.5, 4.5 ) ) )
		self.assertEqual( scene.readBound( 1 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 3.5, 2.5, 5.5 ) ) )
		
		# time 1.5
		self.assertEqual( box1.readBound( 1.5 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		self.assertEqual( torus2.readBound( 1.5 ), IECore.Box3d( IECore.V3d( -1.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 1.5 ) ) )
		self.assertEqual( torus1.readBound( 1.5 ), IECore.Box3d( IECore.V3d( -2.5, -0.5, -1.5 ), IECore.V3d( 1.5, 0.5, 3.5 ) ) )
		self.assertEqual( sub1.readBound( 1.5 ), IECore.Box3d( IECore.V3d( -2 ), IECore.V3d( 3, 2, 5 ) ) )
		self.assertEqual( scene.readBound( 1.5 ), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 4, 3, 6 ) ) )
	
	def testReadTransformMethods( self ) :
		
		scene = self.buildScene()
		hou.node( "/obj/sub1/torus1" ).parmTuple( "t" ).set( [ 1, 2, 3 ] )
		hou.node( "/obj/sub1/torus1" ).parmTuple( "r" ).set( [ 10, 20, 30 ] )
		hou.node( "/obj/sub1/torus1" ).parmTuple( "s" ).set( [ 4, 5, 6 ] )
		
		torus1 = scene.child( "sub1" ).child( "torus1" )
		transform = torus1.readTransform( 0 ).value
		
		self.assertEqual( transform.translate.x, 1 )
		self.assertEqual( transform.translate.y, 2 )
		self.assertEqual( transform.translate.z, 3 )
		self.assertAlmostEqual( IECore.radiansToDegrees( transform.rotate.x ), 10.0, 5 )
		self.assertAlmostEqual( IECore.radiansToDegrees( transform.rotate.y ), 20.0, 5 )
		self.assertAlmostEqual( IECore.radiansToDegrees( transform.rotate.z ), 30.0, 5 )
		self.assertAlmostEqual( transform.scale.x, 4, 6 )
		self.assertAlmostEqual( transform.scale.y, 5, 6 )
		self.assertAlmostEqual( transform.scale.z, 6, 6 )
		self.failUnless( torus1.readTransformAsMatrix( 0 ).equalWithAbsError( transform.transform, 1e-6 ) )
	
	def testReadWorldTransformMethods( self ) :
		
		scene = self.buildScene()
		hou.node( "/obj/sub1" ).parmTuple( "t" ).set( [ 10, 20, 30 ] )
		hou.node( "/obj/sub1/torus1" ).parmTuple( "t" ).set( [ 1, 2, 3 ] )
		hou.node( "/obj/sub1/torus1" ).parmTuple( "r" ).set( [ 10, 20, 30 ] )
		hou.node( "/obj/sub1/torus1" ).parmTuple( "s" ).set( [ 4, 5, 6 ] )
		
		torus1 = scene.child( "sub1" ).child( "torus1" )
		transform = torus1.readWorldTransform( 0 ).value
		
		self.assertEqual( transform.translate.x, 11 )
		self.assertEqual( transform.translate.y, 22 )
		self.assertEqual( transform.translate.z, 33 )
		self.assertAlmostEqual( IECore.radiansToDegrees( transform.rotate.x ), 10.0, 5 )
		self.assertAlmostEqual( IECore.radiansToDegrees( transform.rotate.y ), 20.0, 5 )
		self.assertAlmostEqual( IECore.radiansToDegrees( transform.rotate.z ), 30.0, 5 )
		self.assertAlmostEqual( transform.scale.x, 4, 6 )
		self.assertAlmostEqual( transform.scale.y, 5, 6 )
		self.assertAlmostEqual( transform.scale.z, 6, 6 )
		self.failUnless( torus1.readWorldTransformAsMatrix( 0 ).equalWithAbsError( transform.transform, 1e-6 ) )
	
	def testAnimatedTransform( self ) :
		
		scene = self.buildScene()
		hou.node( "/obj/sub1/torus1" ).parm( "tx" ).setExpression( "$T+(1.0/$FPS)" )
		hou.node( "/obj/sub1/torus1" ).parm( "ty" ).setExpression( "$T+(1.0/$FPS)+1" )
		hou.node( "/obj/sub1/torus1" ).parm( "tz" ).setExpression( "$T+(1.0/$FPS)+2" )
		
		torus1 = scene.child( "sub1" ).child( "torus1" )
		transform0 = torus1.readTransform( 0 ).value
		transform0_5 = torus1.readTransform( 0.5 ).value
		transform1 = torus1.readTransform( 1 ).value
		
		self.assertEqual( transform0.translate, IECore.V3d( 0, 1, 2 ) )
		self.assertAlmostEqual( transform0_5.translate.x, 0.5, 5 )
		self.assertAlmostEqual( transform0_5.translate.y, 1.5, 5 )
		self.assertAlmostEqual( transform0_5.translate.z, 2.5, 5 )
		self.assertEqual( transform1.translate, IECore.V3d( 1, 2, 3 ) )
	
	def testFlatGeoWithGap( self ) :	
	
		scene = self.buildScene()
		hou.node( "/obj/box1/name1" ).parm( "name1" ).set( "/gap/box" )
		
		box1 = scene.child( "sub1" ).child( "box1" )
		self.assertEqual( box1.path(), [ "sub1", "box1" ] )
		self.assertEqual( box1.pathAsString(), "/sub1/box1" )
		self.assertEqual( box1.name(), "box1" )
		self.assertEqual( sorted( box1.childNames() ), [ "gap" ] )
		self.assertEqual( box1.hasChild( "gap" ), True )
		self.assertEqual( box1.hasObject(), False )
		
		gap = box1.child( "gap" )
		self.assertEqual( gap.path(), [ "sub1", "box1", "gap" ] )
		self.assertEqual( gap.pathAsString(), "/sub1/box1/gap" )
		self.assertEqual( gap.name(), "gap" )
		self.assertEqual( sorted( gap.childNames() ), [ "box", "torus" ] )
		self.assertEqual( gap.hasChild( "torus" ), True )
		self.assertEqual( gap.hasChild( "box" ), True )
		self.assertEqual( gap.hasObject(), False )
		
		boxTorus = gap.child( "torus" )
		self.assertEqual( boxTorus.path(), [ "sub1", "box1", "gap", "torus" ] )
		self.assertEqual( boxTorus.pathAsString(), "/sub1/box1/gap/torus" )
		self.assertEqual( boxTorus.name(), "torus" )
		self.assertEqual( boxTorus.childNames(), [] )
		self.assertEqual( boxTorus.hasObject(), True )
		mesh = boxTorus.readObject( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		self.assertEqual( mesh["P"].data.size(), 100 )
		self.assertEqual( mesh.blindData(), IECore.CompoundData() )
		
		boxBox = gap.child( "box" )
		self.assertEqual( boxBox.path(), [ "sub1", "box1", "gap", "box" ] )
		self.assertEqual( boxBox.pathAsString(), "/sub1/box1/gap/box" )
		self.assertEqual( boxBox.name(), "box" )
		self.assertEqual( boxBox.childNames(), [] )
		self.assertEqual( boxBox.hasObject(), True )
		mesh = boxBox.readObject( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		self.assertEqual( mesh["P"].data.size(), 8 )
		self.assertEqual( mesh.blindData(), IECore.CompoundData() )
	
	def testRerooting( self ) :	
		
		self.buildScene()
		scene = IECoreHoudini.HoudiniScene( "/obj/sub1", rootPath = [ "sub1" ] )
		self.assertEqual( scene.path(), [] )
		self.assertEqual( scene.pathAsString(), "/" )
		self.assertEqual( scene.name(), "/" )
		self.assertEqual( sorted( scene.childNames() ), [ "box1", "torus1" ] )
		self.assertEqual( scene.hasChild( "box1" ), True )
		self.assertEqual( scene.hasChild( "torus1" ), True )
		self.assertEqual( scene.hasChild( "torus2" ), False )
		self.assertEqual( scene.hasObject(), False )
		
		torus1 = scene.child( "torus1" )
		self.assertEqual( torus1.path(), [ "torus1" ] )
		self.assertEqual( torus1.pathAsString(), "/torus1" )
		self.assertEqual( torus1.name(), "torus1" )
		self.assertEqual( sorted( torus1.childNames() ), [ "torus2" ] )
		self.assertEqual( torus1.hasChild( "torus2" ), True )
		self.assertEqual( torus1.hasObject(), True )
		mesh = torus1.readObject( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		self.assertEqual( mesh["P"].data.size(), 100 )
		self.assertEqual( mesh.blindData(), IECore.CompoundData() )
		
		torus2 = torus1.child( "torus2" )
		self.assertEqual( torus2.path(), [ "torus1", "torus2" ] )
		self.assertEqual( torus2.pathAsString(), "/torus1/torus2" )
		self.assertEqual( torus2.name(), "torus2" )
		self.assertEqual( sorted( torus2.childNames() ), [] )
		self.assertEqual( torus2.hasObject(), True )
		mesh = torus2.readObject( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		self.assertEqual( mesh["P"].data.size(), 100 )
		self.assertEqual( mesh.blindData(), IECore.CompoundData() )
		
		box1 = scene.child( "box1" )
		self.assertEqual( box1.path(), [ "box1" ] )
		self.assertEqual( box1.pathAsString(), "/box1" )
		self.assertEqual( box1.name(), "box1" )
		self.assertEqual( sorted( box1.childNames() ), [ "gap" ] )
		self.assertEqual( box1.hasChild( "gap" ), True )
		self.assertEqual( box1.hasObject(), True )
		mesh = box1.readObject( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		self.assertEqual( mesh["P"].data.size(), 8 )
		self.assertEqual( mesh.blindData(), IECore.CompoundData() )
		
		gap = box1.child( "gap" )
		self.assertEqual( gap.path(), [ "box1", "gap" ] )
		self.assertEqual( gap.pathAsString(), "/box1/gap" )
		self.assertEqual( gap.name(), "gap" )
		self.assertEqual( sorted( gap.childNames() ), [ "torus" ] )
		self.assertEqual( gap.hasChild( "torus" ), True )
		self.assertEqual( gap.hasObject(), False )
		
		boxTorus = gap.child( "torus" )
		self.assertEqual( boxTorus.path(), [ "box1", "gap", "torus" ] )
		self.assertEqual( boxTorus.pathAsString(), "/box1/gap/torus" )
		self.assertEqual( boxTorus.name(), "torus" )
		self.assertEqual( boxTorus.childNames(), [] )
		self.assertEqual( boxTorus.hasObject(), True )
		mesh = boxTorus.readObject( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		self.assertEqual( mesh["P"].data.size(), 100 )
		self.assertEqual( mesh.blindData(), IECore.CompoundData() )
		
		self.assertRaises( RuntimeError, scene.child, "box2" )
		self.assertEqual( scene.child( "box2", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		
		# test the node that does exist according to houdini, but is actually a grandchild in our world
		self.assertRaises( RuntimeError, scene.child, "torus2" )
		self.assertEqual( scene.child( "torus2", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		
		torus1 = scene.scene( [ "torus1" ] )
		self.assertEqual( torus1.pathAsString(), "/torus1" )
		
		# can't use scene() to go outside of the re-rooting
		self.assertRaises( RuntimeError, scene.scene, [ "sub2" ] )
		self.assertEqual( scene.scene( [ "sub2" ], IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
	
	def testHiddenObjects( self ) :
		
		scene = self.buildScene()
		hou.node( "/obj/sub1" ).setDisplayFlag( False )
		self.assertFalse(  hou.node( "/obj/sub1" ).isObjectDisplayed() )
		self.assertFalse(  hou.node( "/obj/sub1/torus1" ).isObjectDisplayed() )
		
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.childNames(), [ "torus1", "box1" ] )
		self.assertFalse( sub1.hasObject() )
		
		torus = sub1.child( "torus1" )
		self.assertEqual( torus.childNames(), [ "torus2" ] )
		self.assertTrue( torus.hasObject() )
		self.assertTrue( isinstance( torus.readObject( 0 ), IECore.MeshPrimitive ) )

	def testDefaultTime( self ) :
		
		self.buildScene()
		box = hou.node( "/obj/box1" )
		deformer = box.renderNode().createOutputNode( "twist" )
		deformer.parm( "paxis" ).set( 1 )
		deformer.parm( "strength" ).setExpression( "10*($T+1.0/$FPS)" )
		deformer.setRenderFlag( True )
		
		self.assertNotEqual( hou.time(), 0.5 )
		self.assertEqual( deformer.cookCount(), 0 )
		scene = IECoreHoudini.HoudiniScene( box.path(), defaultTime = 0.5 )
		self.assertEqual( scene.getDefaultTime(), 0.5 )
		self.assertEqual( deformer.cookCount(), 1 )
		self.assertTrue( scene.hasObject() )
		self.assertEqual( deformer.cookCount(), 1 )
		self.assertEqual( scene.childNames(), [ "gap" ] )
		self.assertEqual( deformer.cookCount(), 1 )
		mesh0_5 = scene.readObject( 0.5 )
		self.assertEqual( deformer.cookCount(), 1 )
		self.assertEqual( len(mesh0_5["P"].data), 8 )
		self.assertAlmostEqual( mesh0_5["P"].data[0].x, -0.521334, 6 )
		
		scene.setDefaultTime( 0 )
		self.assertEqual( scene.getDefaultTime(), 0 )
		self.assertEqual( deformer.cookCount(), 1 )
		self.assertTrue( scene.hasObject() )
		self.assertEqual( deformer.cookCount(), 2 )
		self.assertEqual( scene.childNames(), [ "gap" ] )
		self.assertEqual( deformer.cookCount(), 2 )
		mesh0 = scene.readObject( 0 )
		self.assertEqual( deformer.cookCount(), 2 )
		self.assertEqual( len(mesh0["P"].data), 8 )
		self.assertEqual( mesh0["P"].data[0].x, -0.5 )
	
	def testNode( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.node(), hou.node( "/obj" ) )
		
		child = scene.child( "sub1" )
		self.assertEqual( child.node(), hou.node( "/obj/sub1" ) )
		
		child2 = child.child( "torus1" )
		self.assertEqual( child2.node(), hou.node( "/obj/sub1/torus1" ) )
		
		child3 = child2.child( "torus2" )
		self.assertEqual( child3.node(), hou.node( "/obj/sub1/torus2" ) )
		
		box1 = child.child( "box1" )
		self.assertEqual( box1.node(), hou.node( "/obj/box1" ) )
		
		# flattened geo still points to the parent OBJ
		gap = box1.child( "gap" )
		self.assertEqual( gap.node(), hou.node( "/obj/box1" ) )
		self.assertEqual( gap.child( "torus" ).node(), hou.node( "/obj/box1" ) )
		
		self.assertEqual( scene.child( "box2" ).node(), hou.node( "/obj/box2" ) )
		self.assertEqual( scene.child( "sub2" ).node(), hou.node( "/obj/sub2" ) )
	
	def testEmbedded( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.embedded(), False )
		
		child = scene.child( "sub1" )
		self.assertEqual( child.embedded(), False )
		
		child2 = child.child( "torus1" )
		self.assertEqual( child2.embedded(), False )
		
		child3 = child2.child( "torus2" )
		self.assertEqual( child3.embedded(), False )
		
		box1 = child.child( "box1" )
		self.assertEqual( box1.embedded(), False )
		
		gap = box1.child( "gap" )
		self.assertEqual( gap.embedded(), True )
		self.assertEqual( gap.child( "torus" ).embedded(), True )
		
		self.assertEqual( scene.child( "box2" ).embedded(), False )
		self.assertEqual( scene.child( "sub2" ).embedded(), False )

	def testSimilarNamesInFlatGeo( self ) :
		
		scene = self.buildScene()
		name = hou.node( "/obj/box1" ).renderNode().createInputNode( 2, "name" )
		name.parm( "name1" ).set( "/gap/torus2" )
		name.createInputNode( 0, "torus" )
		
		box1 = scene.scene( [ "sub1", "box1" ] )
		self.assertEqual( box1.childNames(), [ "gap" ] )
		
		gap = box1.child( "gap" )
		self.assertEqual( gap.childNames(), [ "torus", "torus2" ] )
		
		torus = gap.child( "torus" )
		self.assertEqual( torus.childNames(), [] )
		self.assertTrue( torus.hasObject() )
		self.assertTrue( torus.readObject( 0 ).isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( torus.readObject( 0 ).variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 100 )
		
		torus2 = gap.child( "torus2" )
		self.assertEqual( torus2.childNames(), [] )
		self.assertTrue( torus2.hasObject() )
		self.assertTrue( torus2.readObject( 0 ).isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( torus2.readObject( 0 ).variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 100 )
	
	def testCustomAttributes( self ) : 
		
		scene = self.buildScene()
		self.assertEqual( scene.attributeNames(), [] )
		self.assertFalse( scene.hasAttribute( "custom" ) )
		self.assertEqual( scene.readAttribute( "custom", 0 ), None )
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.attributeNames(), [] )
		self.assertFalse( sub1.hasAttribute( "custom" ) )
		self.assertEqual( sub1.readAttribute( "custom", 0 ), None )
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.attributeNames(), [] )
		self.assertFalse( torus1.hasAttribute( "custom" ) )
		self.assertEqual( torus1.readAttribute( "custom", 0 ), None )
		box1 = sub1.child( "box1" )
		self.assertEqual( box1.attributeNames(), [] )
		self.assertFalse( box1.hasAttribute( "custom" ) )
		self.assertEqual( box1.readAttribute( "custom", 0 ), None )
		gap = box1.child( "gap" )
		self.assertEqual( gap.attributeNames(), [] )
		self.assertFalse( gap.hasAttribute( "custom" ) )
		self.assertEqual( gap.readAttribute( "custom", 0 ), None )
		
		doTest = True
		
		def names( node ) :
			
			if not doTest :
				return []
			
			if node.type().name() == "geo" :
				return [ "custom" ]
			
			return []
		
		def readName( node, name, time ) :
			
			if not doTest :
				return None
			
			if node.type().name() == "geo" and name == "custom" :
				return IECore.StringData( node.path() )
			
			return None
		
		IECoreHoudini.HoudiniScene.registerCustomAttributes( names, readName )
		
		# subnets do not have the new attribute
		self.assertEqual( scene.attributeNames(), [] )
		self.assertFalse( scene.hasAttribute( "custom" ) )
		self.assertEqual( scene.readAttribute( "custom", 0 ), None )
		self.assertEqual( sub1.attributeNames(), [] )
		self.assertFalse( sub1.hasAttribute( "custom" ) )
		self.assertEqual( sub1.readAttribute( "custom", 0 ), None )
		# geo nodes have the new attribute
		self.assertEqual( torus1.attributeNames(), [ "custom" ] )
		self.assertTrue( torus1.hasAttribute( "custom" ) )
		self.assertEqual( torus1.readAttribute( "custom", 0 ), IECore.StringData( torus1.node().path() ) )
		self.assertEqual( box1.attributeNames(), [ "custom" ] )
		self.assertTrue( box1.hasAttribute( "custom" ) )
		self.assertEqual( box1.readAttribute( "custom", 0 ), IECore.StringData( box1.node().path() ) )
		# embedded shapes do as well
		self.assertEqual( gap.attributeNames(), [ "custom" ] )
		self.assertTrue( gap.hasAttribute( "custom" ) )
		self.assertEqual( gap.readAttribute( "custom", 0 ), IECore.StringData( gap.node().path() ) )
		
		# Disable custom attribute functions so they don't mess with other tests
		doTest = False
	
	def testCustomTags( self ) : 
		
		scene = self.buildScene()
		self.assertEqual( scene.readTags(), [] )
		self.assertFalse( scene.hasTag( "custom" ) )
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.readTags(), [] )
		self.assertFalse( sub1.hasTag( "custom" ) )
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.readTags(), [] )
		self.assertFalse( torus1.hasTag( "custom" ) )
		box1 = sub1.child( "box1" )
		self.assertEqual( box1.readTags(), [] )
		self.assertFalse( box1.hasTag( "custom" ) )
		gap = box1.child( "gap" )
		self.assertEqual( gap.readTags(), [] )
		self.assertFalse( gap.hasTag( "custom" ) )
		
		doTest = True
		
		def readTags( node, includeChildren ) :
			
			if not doTest :
				return []
			
			if node.type().name() == "geo" :
				return [ "custom" ]
			
			if includeChildren :
				
				def recurse( node ) :
					if node.type().name() == "geo" :
						return True
					for child in node.children() :
						if recurse( child ) :
							return True
					return False
				
				if recurse( node ) :
					return [ "custom" ]
			
			return []
		
		def hasTag( node, name, includeChildren ) :
			
			if not doTest :
				return False
			
			if name != "custom" :
				return False
			
			if node.type().name() == "geo" :
				return True
			
			if includeChildren :
				
				def recurse( node ) :
					if node.type().name() == "geo" :
						return True
					for child in node.children() :
						if recurse( child ) :
							return True
					return False
				
				return recurse( node )
			
			return False
		
		IECoreHoudini.HoudiniScene.registerCustomTags( hasTag, readTags )
		
		# subnets do not have the new tag directly
		self.assertEqual( scene.readTags( False ), [] )
		self.assertFalse( scene.hasTag( "custom", False ) )
		self.assertEqual( sub1.readTags( False ), [] )
		self.assertFalse( sub1.hasTag( "custom", False ) )
		# but they do have them on children
		self.assertEqual( scene.readTags(), [ "custom" ] )
		self.assertTrue( scene.hasTag( "custom" ) )
		self.assertEqual( sub1.readTags(), [ "custom" ] )
		self.assertTrue( sub1.hasTag( "custom" ) )
		# geo nodes have the new tag directly and on children
		self.assertEqual( torus1.readTags( False ), [ "custom" ] )
		self.assertEqual( torus1.readTags(), [ "custom" ] )
		self.assertTrue( torus1.hasTag( "custom", False ) )
		self.assertTrue( torus1.hasTag( "custom" ) )
		self.assertEqual( box1.readTags( False ), [ "custom" ] )
		self.assertEqual( box1.readTags(), [ "custom" ] )
		self.assertTrue( box1.hasTag( "custom", False ) )
		self.assertTrue( box1.hasTag( "custom" ) )
		# embedded shapes do as well
		self.assertEqual( gap.readTags( False ), [ "custom" ] )
		self.assertEqual( gap.readTags(), [ "custom" ] )
		self.assertTrue( gap.hasTag( "custom", False ) )
		self.assertTrue( gap.hasTag( "custom" ) )
		
		# Disable custom tag functions so they don't mess with other tests
		doTest = False
	
	def testBrokenSop( self ) :
		
		scene = self.buildScene()
		boxNode = hou.node( "/obj/box1" )
		box1 = scene.scene( [ "sub1", "box1" ] )
		self.assertEqual( box1.hasObject(), True )
		mesh = box1.readObject( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		self.assertEqual( mesh["P"].data.size(), 8 )
		self.assertEqual( box1.childNames(), [ "gap" ] )
		self.assertTrue( isinstance( box1.child( "gap" ), IECoreHoudini.HoudiniScene ) )
		
		# forcing a cook error
		hou.parm('/obj/box1/actualBox/sizex').setExpression( "fake" )
		self.assertEqual( box1.hasObject(), False )
		self.assertEqual( box1.readObject( 0 ), None )
		self.assertEqual( box1.childNames(), [] )
		self.assertRaises( RuntimeError, box1.child, "gap" )
		self.assertEqual( box1.child( "gap", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( hou.OperationFailed, box1.node().renderNode().cook )

if __name__ == "__main__":
	unittest.main()
