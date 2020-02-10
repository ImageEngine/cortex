##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

import os
import math
import unittest
import imath

import IECore
import IECoreScene
import IECoreUSD


class USDSceneTest( unittest.TestCase ) :

	def testConstruction( self ) :

		fileName = os.path.dirname( __file__ ) + "/data/cube.usda"

		s = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( s.fileName(), fileName )

	def testMissingFileThrowsException( self ) :

		fileName = os.path.dirname( __file__ ) + "/data/cubeMissing.usda"

		with self.assertRaises(RuntimeError):
			s = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )


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
		# hashTypes = IECoreScene.SceneInterface.HashType.values.values()

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

		self.assertEqual( bound, imath.Box3d( imath.V3d( -0.5 ), imath.V3d( 0.5 ) ) )

	def testTransform ( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/hierarchy.usda", IECore.IndexedIO.OpenMode.Read )
		plane = root.child("group1").child("pPlane1")

		transform = plane.readTransformAsMatrix( 0.0 )

		expectedMatrix = imath.M44d().translate( imath.V3d( 2.0, 0.0, 0.0 ) )

		self.assertEqual( transform, expectedMatrix )

	def testPrimVarTypes ( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/primVars.usda", IECore.IndexedIO.OpenMode.Read )

		object = root.child("root").child("sphere").readObject(0.0)

		expected = {
			'test_Bool_Scalar_constant' : IECore.BoolData( 0 ),
			'test_Double2_Array_constant' : IECore.V2dVectorData( [imath.V2d( 1.1, 1.2 ), imath.V2d( 2.1, 2.2 ), imath.V2d( 3.1, 3.2 )] ),
			'test_Double2_Scalar_constant' : IECore.V2dData( imath.V2d( 0.1, 0.2 ) ),
			'test_Double3_Array_constant' : IECore.V3dVectorData( [imath.V3d( 1.1, 1.2, 1.3 ), imath.V3d( 2.1, 2.2, 2.3 ), imath.V3d( 3.1, 3.2, 3.3 )] ),
			'test_Double3_Scalar_constant' : IECore.V3dData( imath.V3d( 0.1, 0.2, 0.3 ) ),
			'test_Double_Array_constant' : IECore.DoubleVectorData([1.2, 1.3, 1.4]),
			'test_Double_Scalar_constant' : IECore.DoubleData( 1.1 ),
			'test_Float2_Array_constant' : IECore.V2fVectorData( [imath.V2f( 1.1, 1.2 ), imath.V2f( 2.1, 2.2 ), imath.V2f( 3.1, 3.2 )] ),
			'test_Float2_Scalar_constant' : IECore.V2fData( imath.V2f( 0.1, 0.2 ) ),
			'test_Float3_Array_constant' : IECore.V3fVectorData( [imath.V3f( 1.1, 1.2, 1.3 ), imath.V3f( 2.1, 2.2, 2.3 ), imath.V3f( 3.1, 3.2, 3.3 )] ),
			'test_Float3_Scalar_constant' : IECore.V3fData( imath.V3f( 0.1, 0.2, 0.3 ) ),
			'test_Float_Array_constant' : IECore.FloatVectorData( [0.7, 0.8, 0.9] ),
			'test_Float_Scalar_constant' : IECore.FloatData( 0.6 ),
			'test_Half_Array_constant' : IECore.HalfVectorData( [0.0999756, 0.199951, 0.300049] ),
			'test_Half_Scalar_constant' : IECore.HalfData( 0.5 ),
			'test_Int2_Array_constant' : IECore.V2iVectorData( [imath.V2i( 3, 4 ), imath.V2i( 5, 6 ), imath.V2i( 7, 8 )] ),
			'test_Int2_Scalar_constant' : IECore.V2iData( imath.V2i( 1, 2 ) ),
			'test_Int3_Array_constant' : IECore.V3iVectorData([imath.V3i(3, 4, 5), imath.V3i(5, 6, 7), imath.V3i(7, 8, 9)]),
			'test_Int3_Scalar_constant' : IECore.V3iData(imath.V3i(1, 2, 3)),
			'test_Int64_Array_constant' : IECore.Int64VectorData([9223372036854775805, 9223372036854775806, 9223372036854775807]),
			'test_Int64_Scalar_constant' : IECore.Int64Data(-9223372036854775808),
			'test_Int_Array_constant' : IECore.IntVectorData([0, -1, -2]),
			'test_Int_Scalar_constant' : IECore.IntData(-1),
			'test_String_Array_constant' : IECore.StringVectorData(["is", "a", "test"]),
			'test_String_Scalar_constant': IECore.StringData('this'),
			'test_Token_Array_constant' : IECore.InternedStringVectorData([IECore.InternedString("t-is"), IECore.InternedString("t-a"), IECore.InternedString("t-test")]),
			'test_Token_Scalar_constant' : IECore.InternedStringData(IECore.InternedString("t-this")),
			'test_UChar_Array_constant' : IECore.UCharVectorData([0,1,2]),
			'test_UChar_Scalar_constant' : IECore.UCharData(0),
			'test_UInt64_Array_constant' : IECore.UInt64VectorData([18446744073709551613, 18446744073709551614, 18446744073709551615]),
			'test_UInt64_Scalar_constant' : IECore.UInt64Data(18446744073709551615),
			'test_UInt_Array_constant' : IECore.UIntVectorData([4294967293, 4294967294, 4294967295]),
			'test_UInt_Scalar_constant' : IECore.UIntData(4294967295),
			#'test_color3d_Array_constant' : IECore.Color3dVectorData([IECore.Color3d(1.1, 1.2, 1.3), IECore.Color3d(2.1, 2.2, 2.3), IECore.Color3d(3.1, 3.2, 3.3)]),
			#'test_color3d_Scalar_constant' : IECore.Color3dData(IECore.Color3d(0.1, 0.2, 0.3)),
			'test_color3f_Array_constant' : IECore.Color3fVectorData([imath.Color3f(1.1, 1.2, 1.3), imath.Color3f(2.1, 2.2, 2.3), imath.Color3f(3.1, 3.2, 3.3)]),
			'test_color3f_Scalar_constant' :  IECore.Color3fData(imath.Color3f(0.1, 0.2, 0.3)),
			#'test_color4d_Array_constant' : IECore.Color4dVectorData([IECore.Color4d(1.1, 1.2, 1.3, 1.4), IECore.Color4d(2.1, 2.2, 2.3, 2.4), IECore.Color4d(3.1, 3.2, 3.3, 3.4)]),
			#'test_color4d_Scalar_constant' : IECore.Color4dData(IECore.Color4d(0.1, 0.2, 0.3, 0.4)),
			'test_color4f_Array_constant' : IECore.Color4fVectorData([imath.Color4f(1.1, 1.2, 1.3, 1.4), imath.Color4f(2.1, 2.2, 2.3, 2.4), imath.Color4f(3.1, 3.2, 3.3, 3.4)]),
			'test_color4f_Scalar_constant' : IECore.Color4fData(imath.Color4f(0.1, 0.2, 0.3, 0.4)),
			'test_matrix3d_Array_constant' : IECore.M33dVectorData(
				[
					imath.M33d(0, 0, 0, 0, 1, 0,0, 0, 0),
					imath.M33d(0, 0, 0, 0, 0, 0,0, 0, 2),
					imath.M33d(0, 0, 4, 0, 0, 0,0, 0, 0)
				]
			),
			'test_matrix3d_Scalar_constant' : IECore.M33dData(imath.M33d(0, 0, 0, 0, 0, 0, 0, 0, 0)),
			'test_matrix4d_Array_constant' : IECore.M44dVectorData(
				[
					imath.M44d(1, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0),
					imath.M44d(1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
					imath.M44d(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0)
				]
			),
			'test_matrix4d_Scalar_constant' : IECore.M44dData(imath.M44d(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)),
			'test_normal3d_Array_constant' : IECore.V3dVectorData([imath.V3d( 1.1, 1.2, 1.3 ), imath.V3d( 2.1, 2.2, 2.3 ), imath.V3d( 3.1, 3.2, 3.3 )]),
			'test_normal3d_Scalar_constant' : IECore.V3dData (imath.V3d( 0.1, 0.2, 0.3 )),
			'test_normal3f_Array_constant': IECore.V3fVectorData([imath.V3f( 1.1, 1.2, 1.3 ), imath.V3f( 2.1, 2.2, 2.3 ), imath.V3f( 3.1, 3.2, 3.3 )]),
			'test_normal3f_Scalar_constant' : IECore.V3fData (imath.V3f( 0.1, 0.2, 0.3 )),
			'test_point3d_Array_constant' : IECore.V3dVectorData([imath.V3d( 1.1, 1.2, 1.3 ), imath.V3d( 2.1, 2.2, 2.3 ), imath.V3d( 3.1, 3.2, 3.3 )]),
			'test_point3d_Scalar_constant' : IECore.V3dData (imath.V3d( 0.1, 0.2, 0.3 )),
			'test_point3f_Array_constant' : IECore.V3fVectorData([imath.V3f( 1.1, 1.2, 1.3 ), imath.V3f( 2.1, 2.2, 2.3 ), imath.V3f( 3.1, 3.2, 3.3 )]),
			'test_point3f_Scalar_constant' : IECore.V3fData(imath.V3f(0.1, 0.2, 0.3)),
			'test_quatd_Array_constant' : IECore.QuatdVectorData([imath.Quatd(1, 0, 0, 0), imath.Quatd(0, 1, 0, 0), imath.Quatd(0, 0, 1, 0)]),
			'test_quatd_Scalar_constant' : IECore.QuatdData(imath.Quatd(0, 0, 0, 1)),
			'test_quatf_Array_constant' : IECore.QuatfVectorData([imath.Quatf(1, 0, 0, 0), imath.Quatf(0, 1, 0, 0), imath.Quatf(0, 0, 1, 0)]),
			'test_quatf_Scalar_constant' : IECore.QuatfData(imath.Quatf(0, 0, 0, 1)),
			'test_vector3d_Array_constant' : IECore.V3dVectorData([imath.V3d( 1.1, 1.2, 1.3 ), imath.V3d( 2.1, 2.2, 2.3 ), imath.V3d( 3.1, 3.2, 3.3 )]),
			'test_vector3d_Scalar_constant' : IECore.V3dData (imath.V3d( 0.1, 0.2, 0.3 )),
			'test_vector3f_Array_constant' : IECore.V3fVectorData([imath.V3f( 1.1, 1.2, 1.3 ), imath.V3f( 2.1, 2.2, 2.3 ), imath.V3f( 3.1, 3.2, 3.3 )]),
			'test_vector3f_Scalar_constant' : IECore.V3fData (imath.V3f( 0.1, 0.2, 0.3 )),
		}


		for primVarName, primVarExpectedValue in expected.items():
			self.assertTrue(primVarName in object.keys())
			p = object[primVarName]
			self.assertEqual(p.data, primVarExpectedValue)

	def testSpherePrimitiveReadWrite ( self ) :

		# verify we can round trip a sphere

		root = IECoreScene.SceneInterface.create( "/tmp/sphereWriteTest.usda", IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "sphere" )
		child.writeObject( IECoreScene.SpherePrimitive( 3.0 ), 0 )

		del root, child

		root = IECoreScene.SceneInterface.create( "/tmp/sphereWriteTest.usda", IECore.IndexedIO.OpenMode.Read )

		sphere = root.child( "sphere" ).readObject( 0.0 )
		self.failUnless( isinstance( sphere, IECoreScene.SpherePrimitive ) )
		self.assertEqual( 3.0, sphere.radius() )

	def testTraverseInstancedScene ( self ) :

		# Verify we can load a usd file which uses scene proxies

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/instances.usda", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( root.childNames(), ['InstanceSources', 'instance_0', 'instance_1'] )

		instance0Object = root.child("instance_0").child("world").readObject( 0.0 )
		instance1Object = root.child("instance_1").child("world").readObject( 0.0 )

		self.failUnless( isinstance( instance0Object, IECoreScene.SpherePrimitive ) )
		self.failUnless( isinstance( instance1Object, IECoreScene.SpherePrimitive ) )

if __name__ == "__main__":
	unittest.main()
