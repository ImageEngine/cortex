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

import maya.cmds
import maya.OpenMaya as OpenMaya

import IECore
import IECoreMaya

class LiveSceneTest( IECoreMaya.TestCase ) :
	
	def setUp( self ) :

		maya.cmds.file( new=True, f=True )
	
	def testFileName( self ) :

		scene = IECoreMaya.LiveScene()
		self.assertRaises( RuntimeError, scene.fileName )

	def testChildNames( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.LiveScene()
		child = scene.child( "pSphere1" )
		
		self.assertEqual( set( child.childNames() ), set( [ "pSphere2", "pSphere3" ] ) )
		
		self.assertEqual( scene.child( "pSphere1" ).child( "pSphere2" ).childNames(), [] )

	def testHasChild( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.LiveScene()
		child = scene.child( "pSphere1" )
		
		self.assertEqual( scene.hasChild("pSphere1"), True )
		
		self.assertEqual( child.hasChild("pSphere1Shape"), False )
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
		
		scene = IECoreMaya.LiveScene()
		
		sphere1 = scene.child( "pSphere1" )
		
		sphere2 = sphere1.child( "pSphere2" )
		
		sphere3 = sphere1.child( "pSphere3" )
		
		self.assertEqual( str( scene.name() ), "/" )
		self.assertEqual( str( sphere1.name() ), "pSphere1" )
		
		self.assertEqual( str( sphere2.name() ), "pSphere2" )
		
		self.assertEqual( str( sphere3.name() ), "pSphere3" )
	
	def testPaths( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.LiveScene()
		
		sphere1 = scene.child( "pSphere1" )
		
		sphere2 = sphere1.child( "pSphere2" )
		
		sphere3 = sphere1.child( "pSphere3" )
	
		self.assertEqual( scene.path(), [] )
		self.assertEqual( sphere1.path(), [ "pSphere1" ] )
		
		self.assertEqual( sphere2.path(), [ "pSphere1", "pSphere2" ] )
		
		self.assertEqual( sphere3.path(), [ "pSphere1", "pSphere3" ] )
	
	def testSceneMethod( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		sphere2 = maya.cmds.polySphere( name="pSphere2" )
		sphere3 = maya.cmds.polySphere( name="pSphere3" )
		
		maya.cmds.parent( "pSphere2", "pSphere1" )
		maya.cmds.parent( "pSphere3", "pSphere1" )
		
		scene = IECoreMaya.LiveScene()
		
		self.assertEqual( str( scene.scene( ["pSphere1"] ).name() ), "pSphere1" )
		
		# does it still return absolute paths if we've gone to another location?
		scene = scene.scene( ["pSphere1"] )
		self.assertEqual( str( scene.scene( [] ).name() ), "/" )
		self.assertEqual( str( scene.scene( ["pSphere1", "pSphere2"] ).name() ), "pSphere2" )
		self.assertEqual( str( scene.scene( ["pSphere1", "pSphere3"] ).name() ), "pSphere3" )
		
		self.assertEqual( scene.scene( ["idontexist"], IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, IECore.curry( scene.scene, ["idontexist"] ) )
		
	def testHasObject( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		scene = IECoreMaya.LiveScene()
		child = scene.child( "pSphere1" )
		
		self.assertEqual( scene.hasObject(), False )
		self.assertEqual( child.hasObject(), True )
	
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
		
		scene = IECoreMaya.LiveScene()
		transformChild = scene.child( "transform1" ).child( "pSphere1" )
		
		# test it returns the correct transform in local space
		maya.cmds.currentTime( "0.0sec" )
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

	def testTimeException( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		maya.cmds.setKeyframe( "pSphere1", attribute="tx", t="0sec", v=1 )
		maya.cmds.setKeyframe( "pSphere1", attribute="ty", t="0sec", v=2 )
		maya.cmds.setKeyframe( "pSphere1", attribute="tz", t="0sec", v=3 )
		
		maya.cmds.setKeyframe( "pSphere1", attribute="tx", t="1sec", v=4 )
		maya.cmds.setKeyframe( "pSphere1", attribute="ty", t="1sec", v=5 )
		maya.cmds.setKeyframe( "pSphere1", attribute="tz", t="1sec", v=6 )
		
		scene = IECoreMaya.LiveScene()
		transformChild = scene.child( "pSphere1" )
		
		# move to frame -1:
		maya.cmds.currentTime( -1 )
		
		# test it returns the correct transform in local space
		self.assertRaises( RuntimeError, IECore.curry( transformChild.readTransform, 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( transformChild.readTransform, 0.5 ) )
		self.assertRaises( RuntimeError, IECore.curry( transformChild.readTransform, 1.0 ) )

		
	def testAnimatedTransform( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		maya.cmds.setKeyframe( "pSphere1", attribute="tx", t="0sec", v=1 )
		maya.cmds.setKeyframe( "pSphere1", attribute="ty", t="0sec", v=2 )
		maya.cmds.setKeyframe( "pSphere1", attribute="tz", t="0sec", v=3 )
		
		maya.cmds.setKeyframe( "pSphere1", attribute="tx", t="1sec", v=4 )
		maya.cmds.setKeyframe( "pSphere1", attribute="ty", t="1sec", v=5 )
		maya.cmds.setKeyframe( "pSphere1", attribute="tz", t="1sec", v=6 )
		
		scene = IECoreMaya.LiveScene()
		transformChild = scene.child( "pSphere1" )
		
		# test it returns the correct transform in local space
		maya.cmds.currentTime( "0sec" )
		transform0 = transformChild.readTransform( 0 ).value
		maya.cmds.currentTime( "0.5sec" )
		transform0_5 = transformChild.readTransform( 0.5 ).value
		maya.cmds.currentTime( "1sec" )
		transform1 = transformChild.readTransform( 1 ).value
		
		self.assertEqual( transform0.translate, IECore.V3d( 1, 2, 3 ) )
		
		self.assertAlmostEqual( transform0_5.translate.x, 2.5, 5 )
		self.assertAlmostEqual( transform0_5.translate.y, 3.5, 5 )
		self.assertAlmostEqual( transform0_5.translate.z, 4.5, 5 )
		
		self.assertEqual( transform1.translate, IECore.V3d( 4, 5, 6 ) )
		
		
	def testDeletedDagPath( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		scene = IECoreMaya.LiveScene()
		child = scene.child( "pSphere1" )
		
		maya.cmds.delete( "pSphere1" )
		
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
		
		# this doesn't need to throw an exception does it?
		self.assertEqual( child.scene( [ "pSphere1", "pSphereShape1" ], IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		
		# I guess this does...
		self.assertRaises( RuntimeError, IECore.curry( child.scene, [ "pSphere1", "pSphereShape1" ] ) )
	
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
		
		scene = IECoreMaya.LiveScene()
		cube = scene.child( "pCube1" )
		
		# read mesh at time 0:
		maya.cmds.currentTime( "0.0sec" )
		mesh = cube.readObject( 0 )
		
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

		# check read primvars
		self.assertEqual( mesh["P"], cube.readObjectPrimitiveVariables( [ "P" ], 0 )["P"] )

	def testAnimatedMesh( self ) :
		
		cube = maya.cmds.polyCube( name = "pCube1" )
		
		# create a skin cluster to animate vertex 0:
		maya.cmds.select( cl=True )
		maya.cmds.select( "pCube1.vtx[0]", r=True )
		cluster = maya.mel.eval( 'newCluster "-envelope 1"' )[1]
		
		maya.cmds.setKeyframe( cluster, attribute="tx", t="0sec" )
		maya.cmds.setKeyframe( cluster, attribute="tx", t="1sec", v=-1 )
		
		scene = IECoreMaya.LiveScene()
		cube = scene.child( "pCube1" )
		
		# read mesh at different times:
		maya.cmds.currentTime( "0.0sec" )
		mesh0   = cube.readObject( 0 )
		maya.cmds.currentTime( "0.5sec" )
		mesh0_5 = cube.readObject( 0.5 )
		maya.cmds.currentTime( "1.0sec" )
		mesh1   = cube.readObject( 1 )
		
		# have we moved vertex 0?
		self.assertEqual( mesh0["P"].data[0].x, -0.5 )
		self.assertEqual( mesh0_5["P"].data[0].x, -1 )
		self.assertEqual( mesh1["P"].data[0].x, -1.5 )
	
	def testReadBound( self ) :
		
		# create some cubes:
		maya.cmds.polyCube( name = "pCube1" )
		maya.cmds.polyCube( name = "pCube2" )
		maya.cmds.polyCube( name = "pCube3" )
		maya.cmds.polyCube( name = "pCube4" )
		
		maya.cmds.parent( "pCube2", "pCube1" )
		maya.cmds.parent( "pCube3", "pCube1" )
		
		maya.cmds.setAttr( "pCube4.tx", 3 )
		maya.cmds.setAttr( "pCube4.ty", 3 )
		maya.cmds.setAttr( "pCube4.tz", 3 )
		
		maya.cmds.setAttr( "pCube2.tx", 1 )
		maya.cmds.setAttr( "pCube2.ty", 1 )
		maya.cmds.setAttr( "pCube2.tz", 1 )
		
		maya.cmds.setAttr( "pCube3.tx", -1 )
		maya.cmds.setAttr( "pCube3.ty", -1 )
		maya.cmds.setAttr( "pCube3.tz", -1 )
		
		scene = IECoreMaya.LiveScene()
		cube4Transform = scene.child( "pCube4" )
		cube1Transform = scene.child( "pCube1" )

		maya.cmds.currentTime( "0.0sec" )		
		self.assertEqual( scene.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -1.5, -1.5, -1.5 ), IECore.V3d( 3.5, 3.5, 3.5 ) ) )
		
		self.assertEqual( cube4Transform.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		
		# check it's including its children:
		self.assertEqual( cube1Transform.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -1.5, -1.5, -1.5 ), IECore.V3d( 1.5, 1.5, 1.5 ) ) )
		
		maya.cmds.setAttr( "pCube1.tx", 1 )
		maya.cmds.setAttr( "pCube1.ty", 1 )
		maya.cmds.setAttr( "pCube1.tz", 1 )
		
		# should be in object space!!!
		self.assertEqual( cube1Transform.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -1.5, -1.5, -1.5 ), IECore.V3d( 1.5, 1.5, 1.5 ) ) )
		
		cube2Transform = cube1Transform.child( "pCube2" )
		self.assertEqual( cube2Transform.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		
		cube3Transform = cube1Transform.child( "pCube3" )
		self.assertEqual( cube3Transform.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
	
	def testAnimatedMeshBound( self ) :
		
		# Currently fails, because I'm pulling on the boundingBox plugs at arbitrary
		# times, and that doesn't work, although it kind of should!
		
		maya.cmds.polyCube( name = "pCube2" )
		
		# create a skin cluster to animate vertex 0:
		maya.cmds.select( cl=True )
		maya.cmds.select( "pCube2.vtx[0]", r=True )
		cluster = maya.mel.eval( 'newCluster "-envelope 1"' )[1]
		
		maya.cmds.setKeyframe( cluster, attribute="tx", t="0sec" )
		maya.cmds.setKeyframe( cluster, attribute="tx", t="1sec", v=-1 )
		
		scene = IECoreMaya.LiveScene()
		transformChild = scene.child( "pCube2" )

		maya.cmds.currentTime( "0.0sec" )		
		self.assertEqual( transformChild.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		maya.cmds.currentTime( "0.5sec" )
		self.assertEqual( transformChild.readBound( 0.5 ), IECore.Box3d( IECore.V3d( -1.0, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		maya.cmds.currentTime( "1.0sec" )
		self.assertEqual( transformChild.readBound( 1.0 ), IECore.Box3d( IECore.V3d( -1.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		
	def testAnimatedBound( self ) :
		
		# Currently fails, because I'm pulling on the boundingBox plugs at arbitrary
		# times, and that doesn't work, although it kind of should!
		
		maya.cmds.polyCube( name = "pCube1" )
		maya.cmds.createNode( "transform", name = "pCube1Parent" )
		
		maya.cmds.parent( "pCube1", "pCube1Parent" )
		
		maya.cmds.setKeyframe( "pCube1", attribute="tx", t="0sec", v=0 )
		maya.cmds.setKeyframe( "pCube1", attribute="tx", t="1sec", v=-1 )
		
		scene = IECoreMaya.LiveScene()
		transformChild = scene.child( "pCube1Parent" )

		maya.cmds.currentTime( "0.0sec" )
		self.assertEqual( transformChild.readBound( 0.0 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 0.5, 0.5 ) ) )
		maya.cmds.currentTime( "0.5sec" )
		self.assertEqual( transformChild.readBound( 0.5 ), IECore.Box3d( IECore.V3d( -1.0, -0.5, -0.5 ), IECore.V3d( 0.0, 0.5, 0.5 ) ) )
		maya.cmds.currentTime( "1.0sec" )
		self.assertEqual( transformChild.readBound( 1.0 ), IECore.Box3d( IECore.V3d( -1.5, -0.5, -0.5 ), IECore.V3d( -0.5, 0.5, 0.5 ) ) )
		
	def testCameraTransform( self ) :
		
		# camera must be output with an identity transform, because of the hierarchical
		# nature of this class...
		
		scene = IECoreMaya.LiveScene()
		cameraTransform = scene.child( "persp" )
		maya.cmds.currentTime( "0.0sec" )
		camera = cameraTransform.readObject( 0 )
		
		# sanity check: camera transform is not identity?
		self.assertNotEqual( cameraTransform.readTransformAsMatrix( 0 ), IECore.M44f() )
		
		# this transform must be identity...
		self.assertEqual( camera.getTransform().transform(), IECore.M44f() )
		
	def testMeshChange( self ) :
		
		sphere = maya.cmds.polySphere( name="pSphere1" )
		
		scene = IECoreMaya.LiveScene()
		sphere = scene.child( "pSphere1" )
		
		maya.cmds.currentTime( "0.0sec" )
		mesh = sphere.readObject( 0 )
		
		# should default to 382 verts:
		self.assertEqual( len( mesh["P"].data ), 382 )
		
		maya.cmds.setAttr( "polySphere1.subdivisionsAxis", 3 )
		maya.cmds.setAttr( "polySphere1.subdivisionsHeight", 3 )
		
		mesh = sphere.readObject( 0 )
		
		# should be 8 verts now:
		self.assertEqual( len( mesh["P"].data ), 8 )
	
	def testWriteExceptions( self ) :
		
		scene = IECoreMaya.LiveScene()
		
		self.assertRaises( RuntimeError, IECore.curry( scene.writeBound, IECore.Box3d(), 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( scene.writeTransform, IECore.M44dData( IECore.M44d() ), 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( scene.writeAttribute, "asdfs", IECore.BoolData( False ), 0.0 ) )
		self.assertRaises( RuntimeError, IECore.curry( scene.writeObject, IECore.SpherePrimitive(), 0.0 ) )
		
	def testSceneShapeCustomReaders( self ):
		
		# make sure we are at time 0
		maya.cmds.currentTime( "0sec" )
		scene = IECoreMaya.LiveScene()

		envShape = str( IECoreMaya.FnSceneShape.create( "ieScene1" ).fullPathName() )
		envNode = 'ieScene1'

		envScene = scene.child( envNode )
		self.assertFalse( envScene.hasAttribute( IECore.LinkedScene.linkAttribute ) )

		maya.cmds.setAttr( envShape+'.file', 'test/IECore/data/sccFiles/environment.lscc',type='string' )

		self.assertTrue( envScene.hasAttribute( IECore.LinkedScene.linkAttribute ) )

		spheresShape = str( IECoreMaya.FnSceneShape.create( "ieScene2" ).fullPathName() )
		spheresNode = 'ieScene2'
		maya.cmds.setAttr( spheresShape+'.file', 'test/IECore/data/sccFiles/animatedSpheres.scc',type='string' )

		self.assertEqual( set( scene.childNames() ).intersection([ envNode, spheresNode ]) , set( [ envNode, spheresNode ] ) )
		self.assertTrue( IECore.LinkedScene.linkAttribute in envScene.attributeNames() )
		self.assertEqual( envScene.readAttribute( IECore.LinkedScene.linkAttribute, 0 ), IECore.CompoundData( { "fileName":IECore.StringData('test/IECore/data/sccFiles/environment.lscc'), "root":IECore.InternedStringVectorData() } ) )
		self.assertFalse( envScene.hasObject() )

		spheresScene = scene.child( spheresNode )
		self.assertTrue( spheresScene.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertEqual( spheresScene.readAttribute( IECore.LinkedScene.linkAttribute, 0 ), IECore.CompoundData( { "fileName":IECore.StringData('test/IECore/data/sccFiles/animatedSpheres.scc'), "root":IECore.InternedStringVectorData() } ) )
		self.assertFalse( spheresScene.hasObject() )

		# expand the scene
		fnSpheres = IECoreMaya.FnSceneShape( spheresShape )
		fnSpheres.expandAll()

		self.assertFalse( spheresScene.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		leafScene = spheresScene.child("A").child("a")
		self.assertTrue( leafScene.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		# When expanding, we connect the child time attributes to their scene shape parent time attribute to propagate time remapping. When checking for time remapping, the scene shape
		# currently only checks the direct connection, so we have here time in the link attributes. Will have to look out for performance issues.
		self.assertEqual( leafScene.readAttribute( IECore.LinkedScene.linkAttribute, 0 ), IECore.CompoundData( { "fileName":IECore.StringData('test/IECore/data/sccFiles/animatedSpheres.scc'), "root":IECore.InternedStringVectorData([ 'A', 'a' ]), 'time':IECore.DoubleData( 0 ) } ) )
		self.assertFalse( leafScene.hasObject() )

		# expand scene to meshes
		fnSpheres.convertAllToGeometry()
		self.assertFalse( leafScene.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertTrue( leafScene.hasObject() )
		self.assertTrue( isinstance( leafScene.readObject(0), IECore.MeshPrimitive) )

		# test time remapped scene readers...
		spheresShape = str( maya.cmds.createNode( 'ieSceneShape' ) )
		maya.cmds.setAttr( spheresShape+'.file', 'test/IECore/data/sccFiles/animatedSpheres.scc',type='string' )
		maya.cmds.setAttr( spheresShape+'.time', 24.0*10 )

		spheresScene = scene.child( 'ieScene3' )

		self.assertTrue( spheresScene.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertEqual( spheresScene.readAttribute( IECore.LinkedScene.linkAttribute, 0 ), IECore.CompoundData( { "fileName":IECore.StringData('test/IECore/data/sccFiles/animatedSpheres.scc'), "root":IECore.InternedStringVectorData(), "time":IECore.DoubleData(10.0) } ) )		
	
	def testReadRootAttribute( self ):
		
		maya.cmds.file( new=True, f=True )
		# make sure we are at time 0
		maya.cmds.currentTime( "0sec" )
		scene = IECoreMaya.LiveScene()
		
		# tests a bug where calling attributeNames at the root raised an exception
		scene.attributeNames()
	
	def testCustomTags( self ) :

		t = maya.cmds.createNode( "transform" )
		maya.cmds.select( clear = True )
		sphere = maya.cmds.polySphere( name="pSphere" )

		doTest = True
		
		def hasMyTags( node, tag, tagFilter ) :
			#'archivable' should be on all transforms and 'renderable' only at shape transforms.

			if not doTest:
				return False
			
			if tag not in ( "renderable", "archivable" ) :
				return False
			
			if tag == "archivable"  :
				return True

			dagPath = IECoreMaya.StringUtil.dagPathFromString(node)
			try:
				dagPath.extendToShapeDirectlyBelow(0)
			except:
				return False

			if not ( tagFilter & IECore.SceneInterface.TagFilter.LocalTag ) :
				return False
			
			if dagPath.apiType() != maya.OpenMaya.MFn.kMesh :
				return False
			
			return dagPath.fullPathName().endswith("Shape")

		def readMyTags( node, tagFilter ) :
			#'archivable' should be on all transforms and 'renderable' only at shape transforms.
			
			if not doTest:
				return []

			result = [ "archivable" ]
			
			dagPath = IECoreMaya.StringUtil.dagPathFromString(node)
			try:
				dagPath.extendToShapeDirectlyBelow(0)
			except:
				return result

			if tagFilter & IECore.SceneInterface.TagFilter.LocalTag and dagPath.apiType() == maya.OpenMaya.MFn.kMesh :
				result.append( "renderable" )

			return result
		
		IECoreMaya.LiveScene.registerCustomTags( hasMyTags, readMyTags )

		scene = IECoreMaya.LiveScene()
		transformScene = scene.child(str(t))
		sphereScene = scene.child('pSphere')
		self.assertFalse( scene.hasTag( 'renderable' ) )
		self.assertFalse( scene.hasTag( 'archivable' ) )
		self.assertEqual( scene.readTags(), [] )
		self.assertFalse( transformScene.hasTag( 'renderable' ) )
		self.assertTrue( transformScene.hasTag( 'archivable' ) )
		self.assertEqual( transformScene.readTags(), [ IECore.InternedString('archivable') ] )
		self.assertEqual( set(sphereScene.readTags()), set([ IECore.InternedString('renderable'), IECore.InternedString('archivable') ]) )
		self.assertEqual( set(sphereScene.readTags( IECore.SceneInterface.TagFilter.EveryTag )), set([ IECore.InternedString('renderable'), IECore.InternedString('archivable') ]) )
		self.assertEqual( sphereScene.readTags( IECore.SceneInterface.TagFilter.AncestorTag ), [ IECore.InternedString('archivable') ] )
		self.assertTrue( sphereScene.hasTag( 'renderable') )
		self.assertTrue( sphereScene.hasTag( 'archivable') )
		
		# Disable custom tag functions so they don't mess with other tests
		doTest = False
	
	
	def testCustomAttributes( self ) :
	
		t = maya.cmds.createNode( "transform" )
		maya.cmds.select( clear = True )
		sphere = maya.cmds.polySphere( name="pSphere" )
		maya.cmds.currentTime( "0sec" )
		
		doTest = True
	
		def myAttributeNames( node ) :

			if not doTest:
				return []
			if not node:
				return ["root"]
			
			dagPath = IECoreMaya.StringUtil.dagPathFromString(node)
			try:
				dagPath.extendToShapeDirectlyBelow(0)
			except:
				return ["transformAttribute"]
			
			if dagPath.apiType() != maya.OpenMaya.MFn.kMesh :
				return []
	
			return ["shapeAttribute"]
	
		def readMyAttribute( node, attr ) :
			
			if not doTest:
				return None
			
			if not node :
				if attr == "root":
					return IECore.BoolData( True )
				return None
			
			dagPath = IECoreMaya.StringUtil.dagPathFromString(node)
			try:
				dagPath.extendToShapeDirectlyBelow(0)
			except:
				if attr == "shapeAttribute":
					return None
				return IECore.FloatData( 5 )
			
			if attr == "transformAttribute":
				return None
			
			if dagPath.apiType() != maya.OpenMaya.MFn.kMesh :
				return None
	
			return IECore.StringData("mesh")
		
		IECoreMaya.LiveScene.registerCustomAttributes( myAttributeNames, readMyAttribute )
	
		scene = IECoreMaya.LiveScene()
		transformScene = scene.child(str(t))
		sphereScene = scene.child('pSphere')
		self.assertEqual( scene.attributeNames(), [ "scene:visible", "root" ] )
		self.assertEqual( scene.readAttribute("anyAttr", 0.0), None )
		self.assertEqual( scene.readAttribute("scene:visible", 0.0), IECore.BoolData(True) )
		self.assertEqual( scene.readAttribute("root", 0.0), IECore.BoolData(True) )
		
		self.assertEqual( transformScene.attributeNames(), [ IECore.InternedString("scene:visible"), IECore.InternedString("transformAttribute") ] )
		self.assertEqual( transformScene.hasAttribute("shapeAttribute"), False )
		self.assertEqual( transformScene.readAttribute("shapeAttribute", 0.0), None )
		self.assertEqual( transformScene.readAttribute( "transformAttribute", 0.0), IECore.FloatData(5) )
		self.assertEqual( sphereScene.attributeNames(), [ IECore.InternedString("scene:visible"), IECore.InternedString('shapeAttribute') ] )
		self.assertEqual( sphereScene.readAttribute( "shapeAttribute", 0.0), IECore.StringData("mesh") )
		
		# Disable custom attribute functions so they don't mess with other tests
		doTest = False
	
	def testSceneVisible( self ) :
		
		maya.cmds.createNode( "transform", name = "t1" )
		maya.cmds.createNode( "transform", name = "t2" )
		
		scene = IECoreMaya.LiveScene()
		t1 = scene.child( "t1" )
		t2 = scene.child( "t2" )
		
		self.assertEqual( t1.attributeNames(), ["scene:visible"] )
		self.assertEqual( t2.attributeNames(), ["scene:visible"] )
		
		self.assertEqual( t1.hasAttribute( "scene:visible" ), True )
		self.assertEqual( t2.hasAttribute( "scene:visible" ), True )
		
		self.assertEqual( t1.readAttribute( "scene:visible", 0 ), IECore.BoolData( True ) )
		self.assertEqual( t2.readAttribute( "scene:visible", 0 ), IECore.BoolData( True ) )
		
		# test visibility on transform:
		maya.cmds.setAttr( "t1.visibility", False )
		
		self.assertEqual( t1.readAttribute( "scene:visible", 0 ), IECore.BoolData( False ) )
		self.assertEqual( t2.readAttribute( "scene:visible", 0 ), IECore.BoolData( True ) )
		
		maya.cmds.setAttr( "t1.visibility", True )
		
		# test visibility on shape. Add a mesh to the two transforms, along with an image plane to confuse it:
		maya.cmds.createNode( "mesh", name = "m1", parent="t1" )
		maya.cmds.createNode( "imagePlane", name = "i1", parent="t1" )
		
		maya.cmds.createNode( "imagePlane", name = "i2", parent="t2" )
		maya.cmds.createNode( "mesh", name = "m2", parent="t2" )
		
		maya.cmds.setAttr( "i1.visibility", False )
		maya.cmds.setAttr( "i2.visibility", False )
		
		# these should both be visible, as cortex ignores image planes and the meshes are visible:
		self.assertEqual( t1.readAttribute( "scene:visible", 0 ), IECore.BoolData( True ) )
		self.assertEqual( t2.readAttribute( "scene:visible", 0 ), IECore.BoolData( True ) )
	
		maya.cmds.setAttr( "m1.visibility", False )
		maya.cmds.setAttr( "m2.visibility", False )
		
		# should both be invisible now, as we've hidden the meshes:
		self.assertEqual( t1.readAttribute( "scene:visible", 0 ), IECore.BoolData( False ) )
		self.assertEqual( t2.readAttribute( "scene:visible", 0 ), IECore.BoolData( False ) )
		
	def testSceneShapeVisible( self ) :
		
		# make sure we are at time 0
		maya.cmds.currentTime( "0sec" )
		scene = IECoreMaya.LiveScene()

		envShape = str( IECoreMaya.FnSceneShape.create( "ieScene1" ).fullPathName() )
		envNode = 'ieScene1'

		envScene = scene.child( envNode )
		self.failUnless( IECore.InternedString( "scene:visible" ) in envScene.attributeNames() )
		
		maya.cmds.setAttr( envShape+'.file', 'test/IECore/data/sccFiles/animatedSpheres.scc',type='string' )
		self.failUnless( IECore.InternedString( "scene:visible" ) in envScene.attributeNames() )
		
		maya.cmds.setAttr( "ieScene1.visibility", False )
		self.assertEqual( envScene.readAttribute( "scene:visible", 0 ), IECore.BoolData( False ) )
		
		maya.cmds.setAttr( "ieScene1.visibility", True )
		self.assertEqual( envScene.readAttribute( "scene:visible", 0 ), IECore.BoolData( True ) )
		
		maya.cmds.setAttr( envShape + ".visibility", False )
		self.assertEqual( envScene.readAttribute( "scene:visible", 0 ), IECore.BoolData( False ) )
	
	
if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
