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

import maya.cmds

import IECore
import IECoreMaya

class MayaSceneTest( IECoreMaya.TestCase ) :
	
	def testCreateMethod( self ) :
		m = IECore.SceneInterface.create( "x.ma", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( isinstance( m, IECoreMaya.MayaScene ), True )
		
	
	def testChildNames( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.MayaScene()
		child = scene.child( "pSphere1" )
		
		self.assertEqual( set( child.childNames() ), set( [ "pSphere2", "pSphere3", "pSphere1Shape" ] ) )

	def testHasChild( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.MayaScene()
		child = scene.child( "pSphere1" )
		
		self.assertEqual( scene.hasChild("pSphere1"), True )
		self.assertEqual( scene.hasChild("pSphere1Shape"), False )
		
		self.assertEqual( child.hasChild("pSphere2"), True )
		self.assertEqual( child.hasChild("pSphere3"), True )
		self.assertEqual( child.hasChild("pSphere3Shape"), False )
		self.assertEqual( child.hasChild("pSphere2Shape"), False )
		
		self.assertEqual( child.hasChild("asdfasdf"), False )

	
	def testNames( self ) :
	
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.MayaScene()
		
		sphere1 = scene.child( "pSphere1" )
		sphere1Shape = sphere1.child( "pSphere1Shape" )
		
		sphere2 = sphere1.child( "pSphere2" )
		sphere2Shape = sphere2.child( "pSphere2Shape" )
		
		sphere3 = sphere1.child( "pSphere3" )
		sphere3Shape = sphere3.child( "pSphere3Shape" )
		
		self.assertEqual( scene.name(), "/" )
		self.assertEqual( sphere1.name(), "/pSphere1" )
		self.assertEqual( sphere1Shape.name(), "/pSphere1/pSphere1Shape" )
		
		self.assertEqual( sphere2.name(), "/pSphere1/pSphere2" )
		self.assertEqual( sphere2Shape.name(), "/pSphere1/pSphere2/pSphere2Shape" )
		
		self.assertEqual( sphere3.name(), "/pSphere1/pSphere3" )
		self.assertEqual( sphere3Shape.name(), "/pSphere1/pSphere3/pSphere3Shape" )
	
	def testPaths( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.MayaScene()
		
		sphere1 = scene.child( "pSphere1" )
		sphere1Shape = sphere1.child( "pSphere1Shape" )
		
		sphere2 = sphere1.child( "pSphere2" )
		sphere2Shape = sphere2.child( "pSphere2Shape" )
		
		sphere3 = sphere1.child( "pSphere3" )
		sphere3Shape = sphere3.child( "pSphere3Shape" )
	
		self.assertEqual( scene.path(), [] )
		self.assertEqual( sphere1.path(), [ "pSphere1" ] )
		self.assertEqual( sphere1Shape.path(), [ "pSphere1", "pSphere1Shape" ] )
		
		self.assertEqual( sphere2.path(), [ "pSphere1", "pSphere2" ] )
		self.assertEqual( sphere2Shape.path(), [ "pSphere1", "pSphere2", "pSphere2Shape" ] )
		
		self.assertEqual( sphere3.path(), [ "pSphere1", "pSphere3" ] )
		self.assertEqual( sphere3Shape.path(), [ "pSphere1", "pSphere3", "pSphere3Shape" ] )
	
	def testSceneMethod( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.MayaScene()
		
		self.assertEqual( scene.scene( ["pSphere1"] ).name(), "/pSphere1" )
		self.assertEqual( scene.scene( ["pSphere1", "pSphere1Shape"] ).name(), "/pSphere1/pSphere1Shape" )
		
		# does it still return absolute paths if we've gone to another location?
		scene = scene.scene( ["pSphere1"] )
		self.assertEqual( scene.scene( [] ).name(), "/" )
		self.assertEqual( scene.scene( ["pSphere1", "pSphere2"] ).name(), "/pSphere1/pSphere2" )
		self.assertEqual( scene.scene( ["pSphere1", "pSphere2", "pSphere2Shape"] ).name(), "/pSphere1/pSphere2/pSphere2Shape" )
		self.assertEqual( scene.scene( ["pSphere1", "pSphere3"] ).name(), "/pSphere1/pSphere3" )
		self.assertEqual( scene.scene( ["pSphere1", "pSphere3", "pSphere3Shape"] ).name(), "/pSphere1/pSphere3/pSphere3Shape" )
		
		self.assertEqual( scene.scene( ["idontexist"] ), None )
		
	def testHasObject( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		scene = IECoreMaya.MayaScene()
		transformChild = scene.child( "pSphere1" )
		shapeChild = transformChild.child( "pSphere1Shape" )
		
		self.assertEqual( scene.hasObject(), False )
		self.assertEqual( transformChild.hasObject(), False )
		self.assertEqual( shapeChild.hasObject(), True )
	
	def testReadTransformMethods( self ) :
		
		# create a little hierarchy
		transfromythingy = maya.cmds.createNode( "transform", name="transform1" )
		maya.cmds.setAttr( "transform1.tx", 0.1 )
		maya.cmds.setAttr( "transform1.ty", 0.2 )
		maya.cmds.setAttr( "transform1.tz", 0.3 )
		
		maya.cmds.setAttr( "transform1.rx", 0.1 )
		maya.cmds.setAttr( "transform1.ry", 0.2 )
		maya.cmds.setAttr( "transform1.rz", 0.3 )
		
		maya.cmds.setAttr( "transform1.sx", 0.1 )
		maya.cmds.setAttr( "transform1.sy", 0.2 )
		maya.cmds.setAttr( "transform1.sz", 0.3 )
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		maya.cmds.parent( "pSphere1", "transform1" )
		
		maya.cmds.setAttr( "pSphere1.tx", 1 )
		maya.cmds.setAttr( "pSphere1.ty", 2 )
		maya.cmds.setAttr( "pSphere1.tz", 3 )
		
		maya.cmds.setAttr( "pSphere1.rx", 10 )
		maya.cmds.setAttr( "pSphere1.ry", 20 )
		maya.cmds.setAttr( "pSphere1.rz", 30 )
		
		maya.cmds.setAttr( "pSphere1.sx", 4 )
		maya.cmds.setAttr( "pSphere1.sy", 5 )
		maya.cmds.setAttr( "pSphere1.sz", 6 )
		
		scene = IECoreMaya.MayaScene()
		transformChild = scene.child( "transform1" ).child( "pSphere1" )
		
		# test it returns the correct transform in local space
		transform = transformChild.readTransform( 0 ).value
		
		import math
		
		self.assertAlmostEqual( transform.translate.x, 1, 5 )
		self.assertAlmostEqual( transform.translate.y, 2, 5 )
		self.assertAlmostEqual( transform.translate.z, 3, 5 )
		
		self.assertAlmostEqual( transform.rotate.x * 180.0 / math.pi, 10.0, 5 )
		self.assertAlmostEqual( transform.rotate.y * 180.0 / math.pi, 20.0, 5 )
		self.assertAlmostEqual( transform.rotate.z * 180.0 / math.pi, 30.0, 5 )
		
		self.assertAlmostEqual( transform.scale.x, 4, 5 )
		self.assertAlmostEqual( transform.scale.y, 5, 5 )
		self.assertAlmostEqual( transform.scale.z, 6, 5 )
		
		self.assertEqual( transform.transform, transformChild.readTransformAsMatrix( 0 ) )
		
	def testAnimatedTransform( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		maya.cmds.setKeyframe( "pSphere1", attribute="tx", t="0sec", v=1 )
		maya.cmds.setKeyframe( "pSphere1", attribute="ty", t="0sec", v=2 )
		maya.cmds.setKeyframe( "pSphere1", attribute="tz", t="0sec", v=3 )
		
		maya.cmds.setKeyframe( "pSphere1", attribute="tx", t="1sec", v=4 )
		maya.cmds.setKeyframe( "pSphere1", attribute="ty", t="1sec", v=5 )
		maya.cmds.setKeyframe( "pSphere1", attribute="tz", t="1sec", v=6 )
		
		scene = IECoreMaya.MayaScene()
		transformChild = scene.child( "pSphere1" )
		
		# test it returns the correct transform in local space
		transform0 = transformChild.readTransform( 0 ).value
		transform0_5 = transformChild.readTransform( 0.5 ).value
		transform1 = transformChild.readTransform( 1 ).value
		
		self.assertEqual( transform0.translate, IECore.V3d( 1, 2, 3 ) )
		
		self.assertAlmostEqual( transform0_5.translate.x, 2.5, 5 )
		self.assertAlmostEqual( transform0_5.translate.y, 3.5, 5 )
		self.assertAlmostEqual( transform0_5.translate.z, 4.5, 5 )
		
		self.assertEqual( transform1.translate, IECore.V3d( 4, 5, 6 ) )
		
		
	def testDeletedDagPath( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		scene = IECoreMaya.MayaScene()
		child = scene.child( "pSphere1" )
		
		maya.cmds.delete( "pSphere1" )
		
		# this doesn't need to throw an exception does it?
		self.assertEqual( child.scene( [ "pSphere1", "pSphereShape1" ] ), None )
		
		self.assertRaises( RuntimeError, IECore.curry( child.child, "pSphereShape1" ) )
		self.assertRaises( RuntimeError, child.childNames )
		self.assertRaises( RuntimeError, IECore.curry( child.hasChild, "asdd" ) )
		self.assertRaises( RuntimeError, child.name )
		self.assertRaises( RuntimeError, child.path )
		self.assertRaises( RuntimeError, child.hasObject )
		self.assertRaises( RuntimeError, IECore.curry( child.readBound, 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( child.readObject, 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( child.readTransform, 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( child.readTransformAsMatrix, 0.0 ) )
	
	
	def testReadMesh( self ) :
		
		# create a cube:
		maya.cmds.polyCube( name = "pCube1" )
		
		# transform a bit, so we can check it's returning the mesh in world space:
		maya.cmds.setAttr( "pCube1.tx", 0.1 )
		maya.cmds.setAttr( "pCube1.ty", 0.2 )
		maya.cmds.setAttr( "pCube1.tz", 0.3 )
		
		maya.cmds.setAttr( "pCube1.rx", 10 )
		maya.cmds.setAttr( "pCube1.ry", 20 )
		maya.cmds.setAttr( "pCube1.rz", 30 )
		
		scene = IECoreMaya.MayaScene()
		shapeChild = scene.child( "pCube1" ).child( "pCube1Shape" )
		
		# read mesh at time 0:
		mesh = shapeChild.readObject( 0 )
		
		vertList = list( mesh["P"].data )
		
		# check it's got the right length:
		self.assertEqual( len( vertList ), 8 )
		
		# check it's got the right verts:
		self.assertEqual( vertList.count( IECore.V3f( -0.5, -0.5, 0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( 0.5, -0.5, 0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( -0.5, 0.5, 0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( 0.5, 0.5, 0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( -0.5, 0.5, -0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( 0.5, 0.5, -0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( -0.5, -0.5, -0.5 ) ), 1 )
		self.assertEqual( vertList.count( IECore.V3f( 0.5, -0.5, -0.5 ) ), 1 )

	def testAnimatedMesh( self ) :
		
		cube = maya.cmds.polyCube( name = "pCube1" )
		
		# create a skin cluster to animate vertex 0:
		maya.cmds.select( cl=True )
		maya.cmds.select( "pCube1.vtx[0]", r=True )
		cluster = maya.mel.eval( 'newCluster "-envelope 1"' )[1]
		
		maya.cmds.setKeyframe( cluster, attribute="tx", t="0sec" )
		maya.cmds.setKeyframe( cluster, attribute="tx", t="1sec", v=-1 )
		
		scene = IECoreMaya.MayaScene()
		shapeChild = scene.child( "pCube1" ).child( "pCube1Shape" )
		
		# read mesh at different times:
		mesh0   = shapeChild.readObject( 0 )
		mesh0_5 = shapeChild.readObject( 0.5 )
		mesh1   = shapeChild.readObject( 1 )
		
		# have we moved vertex 0?
		self.assertEqual( mesh0["P"].data[0].x, -0.5 )
		self.assertEqual( mesh0_5["P"].data[0].x, -1 )
		self.assertEqual( mesh1["P"].data[0].x, -1.5 )


	def testReadBound( self ) :
		
		# create a cube:
		maya.cmds.polyCube( name = "pCube1" )
		maya.cmds.polyCube( name = "pCube2" )
		maya.cmds.polyCube( name = "pCube3" )
		
		maya.cmds.parent( "pCube2", "pCube1" )
		maya.cmds.parent( "pCube3", "pCube1" )
		
		
		maya.cmds.setAttr( "pCube2.tx", 1 )
		maya.cmds.setAttr( "pCube2.ty", 1 )
		maya.cmds.setAttr( "pCube2.tz", 1 )
		
		maya.cmds.setAttr( "pCube3.tx", -1 )
		maya.cmds.setAttr( "pCube3.ty", -1 )
		maya.cmds.setAttr( "pCube3.tz", -1 )
		
		scene = IECoreMaya.MayaScene()
		transformChild = scene.child( "pCube1" )
		
		# check it's including its children:
		self.assertEqual( transformChild.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -1.5, -1.5, -1.5 ), IECore.V3d( 1.5, 1.5, 1.5 ) ) )
		
		maya.cmds.setAttr( "pCube1.tx", 1 )
		maya.cmds.setAttr( "pCube1.ty", 1 )
		maya.cmds.setAttr( "pCube1.tz", 1 )
		
		self.assertEqual( transformChild.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 2.5, 2.5, 2.5 ) ) )
		
		transformChild = transformChild.child( "pCube1Shape" )
		self.assertEqual( transformChild.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		
		transformChild = transformChild.scene( [ "pCube1", "pCube2" ] )
		self.assertEqual( transformChild.readBound( 0.0 ), IECore.Box3d( IECore.V3d( 0.5, 0.5, 0.5 ), IECore.V3d( 1.5, 1.5, 1.5 ) ) )
		
		transformChild = transformChild.scene( [ "pCube1", "pCube3" ] )
		self.assertEqual( transformChild.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -1.5, -1.5, -1.5 ), IECore.V3d( -0.5, -0.5, -0.5 ) ) )
		

	def testAnimatedBound( self ) :
		
		# Currently fails, because I'm pulling on the boundingBox plugs at arbitrary
		# times, and that doesn't work, although it kind of should!
		
		maya.cmds.polyCube( name = "pCube2" )
		
		# create a skin cluster to animate vertex 0:
		maya.cmds.select( cl=True )
		maya.cmds.select( "pCube2.vtx[0]", r=True )
		cluster = maya.mel.eval( 'newCluster "-envelope 1"' )[1]
		
		maya.cmds.setKeyframe( cluster, attribute="tx", t="0sec" )
		maya.cmds.setKeyframe( cluster, attribute="tx", t="1sec", v=-1 )
		
		scene = IECoreMaya.MayaScene()
		transformChild = scene.child( "pCube2" )
		
		self.assertEqual( transformChild.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		self.assertEqual( transformChild.readBound( 0.5 ), IECore.Box3d( IECore.V3d( -1.0, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		self.assertEqual( transformChild.readBound( 1.0 ), IECore.Box3d( IECore.V3d( -1.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		
		

if __name__ == "__main__":
	IECoreMaya.TestProgram()
