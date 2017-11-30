import os
import math
import unittest

import IECore
import IECoreScene
import IECoreUSD


class USDSceneTest( unittest.TestCase ) :
	def testConstruction( self ) :

		fileName = os.path.dirname( __file__ ) + "/data/cube.usda"

		s = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( s.fileName(), fileName )

	def testHierarchy( self ) :

		# root
		r = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/hierarchy.usda", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( r.childNames(), ['group1'] )
		self.assertTrue( r.hasChild( 'group1' ) )
		self.assertFalse( r.hasChild( 'doesntExist' ) )
		self.assertEqual( r.path(), [] )

		self.assertEqual( r.child( "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, r.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, r.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

		# group1

		g = r.child( "group1" )
		self.assertTrue( g )
		self.assertEqual( g.name(), "group1" )
		self.assertEqual( g.path(), ["group1"] )

		self.assertEqual( g.childNames(), ["group2", "pPlane1"] )
		self.assertTrue( g.hasChild( "group2" ) )
		self.assertTrue( g.hasChild( "pPlane1" ) )

		self.assertFalse( g.hasChild( "doesntExist" ) )

		self.assertEqual( g.child( "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, g.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, g.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

		# group2
		g2 = g.child( "group2" )
		self.assertTrue( g2 )
		self.assertEqual( g2.name(), "group2" )
		self.assertEqual( g2.path(), ["group1", "group2"] )

		self.assertEqual( g2.childNames(), ["pCube1"] )
		self.assertTrue( g2.hasChild( "pCube1" ) )
		self.assertFalse( g2.hasChild( "doesntExist" ) )

		self.assertEqual( g2.child( "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, g2.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, g2.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

		# scene method

		cube = r.scene( ['group1', 'group2', 'pCube1'] )
		self.assertEqual( cube.path(), ['group1', 'group2', 'pCube1'] )

		plane = r.scene( ['group1', 'pPlane1'] )
		self.assertEqual( plane.path(), ['group1', 'pPlane1'] )

	def testStaticSceneHashes( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/hierarchy.usda", IECore.IndexedIO.OpenMode.Read )
		cube = root.scene( ['group1', 'group2', 'pCube1'] )

		# todo: childname & hierarchy hashes should also
		# todo: be constant but I'm unsure how to implement this right now.
		hashTypes = [cube.HashType.TransformHash, cube.HashType.ObjectHash, cube.HashType.BoundHash]
		# hashTypes = IECore.SceneInterface.HashType.values.values()

		hashes = [cube.hash( hashType, 0 ) for hashType in hashTypes]

		for i, hashType in enumerate( hashTypes ) :
			self.assertEqual( cube.hash( hashType, 1 ), hashes[i] )

	def testTransformingObjectHashes( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/transformAnim.usda", IECore.IndexedIO.OpenMode.Read )
		cube = root.scene( ['pCube1'] )

		staticHashes = [cube.HashType.ObjectHash, cube.HashType.BoundHash]
		staticHashValues = [cube.hash( hashType, 0 ) for hashType in staticHashes]

		for i, hashType in enumerate( staticHashes ) :
			self.assertEqual( cube.hash( hashType, 1 ), staticHashValues[i] )

		self.assertNotEqual( cube.hash( cube.HashType.TransformHash, 1 ), cube.hash( cube.HashType.TransformHash, 0 ) )

	def testDeformingObjectHashes( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/vertexAnim.usda", IECore.IndexedIO.OpenMode.Read )
		cube = root.scene( ['pCube1'] )

		staticHashes = [cube.HashType.TransformHash]
		staticHashValues = [cube.hash( hashType, 0 ) for hashType in staticHashes]

		varyingHashes = [cube.HashType.ObjectHash, cube.HashType.BoundHash]
		varyingHashValues = [cube.hash( hashType, 0 ) for hashType in varyingHashes]

		for i, hashType in enumerate( staticHashes ) :
			self.assertEqual( cube.hash( hashType, 1 ), staticHashValues[i] )

		for i, hashType in enumerate( varyingHashes ) :
			self.assertNotEqual( cube.hash( hashType, 1 ), varyingHashValues[i] )

	def testHasObject( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/hierarchy.usda", IECore.IndexedIO.OpenMode.Read )
		self.assertFalse( root.hasObject() )

		group1 = root.child( "group1" )
		self.assertFalse( group1.hasObject() )

		plane = group1.child( "pPlane1" )
		self.assertTrue( plane.hasObject() )

		group2 = group1.child( "group2" )
		self.assertFalse( group2.hasObject() )

		cube = group2.child( "pCube1" )
		self.assertTrue( cube.hasObject() )

	def testConvertMesh( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.usda", IECore.IndexedIO.OpenMode.Read )
		cube = root.child( "pCube1" )
		self.assertTrue( cube.hasObject() )

		cubeMesh = cube.readObject( 0.0 )

		self.failUnless( isinstance( cubeMesh, IECoreScene.MeshPrimitive ) )

	def testBound( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.usda", IECore.IndexedIO.OpenMode.Read )
		cube = root.child( "pCube1" )

		bound = cube.readBound( 0.0 )

		self.assertEqual( bound, IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )

	def testTransform ( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/hierarchy.usda", IECore.IndexedIO.OpenMode.Read )
		plane = root.child("group1").child("pPlane1")

		transform = plane.readTransformAsMatrix( 0.0 )

		expectedMatrix = IECore.M44d.createTranslated( IECore.V3d( 2.0, 0.0, 0.0 ) )

		self.assertEqual( transform, expectedMatrix )


