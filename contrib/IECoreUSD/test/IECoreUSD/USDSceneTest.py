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
import shutil
import tempfile
import imath

import IECore
import IECoreScene
import IECoreUSD

import pxr.Usd
import pxr.UsdGeom

if pxr.Usd.GetVersion() < ( 0, 19, 3 ) :
	pxr.Usd.Attribute.HasAuthoredValue = pxr.Usd.Attribute.HasAuthoredValueOpinion

class USDSceneTest( unittest.TestCase ) :

	def setUp( self ) :

		self.__temporaryDirectory = None

	def tearDown( self ) :

		if self.__temporaryDirectory is not None :
			shutil.rmtree( self.__temporaryDirectory )

	## \todo Make an IECoreTest module that we can move GafferTest.TestCase
	# functionality to, and then we can get this from there.
	def temporaryDirectory( self ) :

		if self.__temporaryDirectory is None :
			 self.__temporaryDirectory = tempfile.mkdtemp( prefix = "ieCoreTest" )

		return self.__temporaryDirectory

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

		self.assertTrue( isinstance( cubeMesh, IECoreScene.MeshPrimitive ) )
		self.assertIsInstance( cubeMesh["P"].data, IECore.V3fVectorData )
		self.assertEqual( cubeMesh["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

	def testReadCurves( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/curves.usda", IECore.IndexedIO.OpenMode.Read )
		child = root.child( "borderLines" )
		self.assertTrue( child.hasObject() )

		curves = child.readObject( 0.0 )
		self.assertTrue( isinstance( curves, IECoreScene.CurvesPrimitive ) )
		self.assertIsInstance( curves["P"].data, IECore.V3fVectorData )
		self.assertEqual( curves["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

	def testReadPoints( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/points.usda", IECore.IndexedIO.OpenMode.Read )
		child = root.child( "plane" )
		self.assertTrue( child.hasObject() )

		points = child.readObject( 0.0 )
		self.assertTrue( isinstance( points, IECoreScene.PointsPrimitive ) )
		self.assertEqual( points.numPoints, 4 )
		self.assertIsInstance( points["P"].data, IECore.V3fVectorData )
		self.assertEqual( points["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

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
			'test_normal3d_Array_constant' : IECore.V3dVectorData([imath.V3d( 1.1, 1.2, 1.3 ), imath.V3d( 2.1, 2.2, 2.3 ), imath.V3d( 3.1, 3.2, 3.3 )], IECore.GeometricData.Interpretation.Normal ),
			'test_normal3d_Scalar_constant' : IECore.V3dData (imath.V3d( 0.1, 0.2, 0.3 ), IECore.GeometricData.Interpretation.Normal),
			'test_normal3f_Array_constant': IECore.V3fVectorData([imath.V3f( 1.1, 1.2, 1.3 ), imath.V3f( 2.1, 2.2, 2.3 ), imath.V3f( 3.1, 3.2, 3.3 )], IECore.GeometricData.Interpretation.Normal),
			'test_normal3f_Scalar_constant' : IECore.V3fData (imath.V3f( 0.1, 0.2, 0.3 ), IECore.GeometricData.Interpretation.Normal),
			'test_point3d_Array_constant' : IECore.V3dVectorData([imath.V3d( 1.1, 1.2, 1.3 ), imath.V3d( 2.1, 2.2, 2.3 ), imath.V3d( 3.1, 3.2, 3.3 )], IECore.GeometricData.Interpretation.Point ),
			'test_point3d_Scalar_constant' : IECore.V3dData (imath.V3d( 0.1, 0.2, 0.3 ), IECore.GeometricData.Interpretation.Point ),
			'test_point3f_Array_constant' : IECore.V3fVectorData([imath.V3f( 1.1, 1.2, 1.3 ), imath.V3f( 2.1, 2.2, 2.3 ), imath.V3f( 3.1, 3.2, 3.3 )], IECore.GeometricData.Interpretation.Point ),
			'test_point3f_Scalar_constant' : IECore.V3fData(imath.V3f(0.1, 0.2, 0.3), IECore.GeometricData.Interpretation.Point ),
			'test_quatd_Array_constant' : IECore.QuatdVectorData([imath.Quatd(1, 0, 0, 0), imath.Quatd(0, 1, 0, 0), imath.Quatd(0, 0, 1, 0)]),
			'test_quatd_Scalar_constant' : IECore.QuatdData(imath.Quatd(0, 0, 0, 1)),
			'test_quatf_Array_constant' : IECore.QuatfVectorData([imath.Quatf(1, 0, 0, 0), imath.Quatf(0, 1, 0, 0), imath.Quatf(0, 0, 1, 0)]),
			'test_quatf_Scalar_constant' : IECore.QuatfData(imath.Quatf(0, 0, 0, 1)),
			'test_vector3d_Array_constant' : IECore.V3dVectorData([imath.V3d( 1.1, 1.2, 1.3 ), imath.V3d( 2.1, 2.2, 2.3 ), imath.V3d( 3.1, 3.2, 3.3 )], IECore.GeometricData.Interpretation.Vector ),
			'test_vector3d_Scalar_constant' : IECore.V3dData (imath.V3d( 0.1, 0.2, 0.3 ), IECore.GeometricData.Interpretation.Vector ),
			'test_vector3f_Array_constant' : IECore.V3fVectorData([imath.V3f( 1.1, 1.2, 1.3 ), imath.V3f( 2.1, 2.2, 2.3 ), imath.V3f( 3.1, 3.2, 3.3 )], IECore.GeometricData.Interpretation.Vector ),
			'test_vector3f_Scalar_constant' : IECore.V3fData (imath.V3f( 0.1, 0.2, 0.3 ), IECore.GeometricData.Interpretation.Vector ),
		}


		for primVarName, primVarExpectedValue in expected.items():
			self.assertIn( primVarName, object )
			p = object[primVarName]
			self.assertEqual(p.data, primVarExpectedValue)

	def testSpherePrimitiveReadWrite ( self ) :

		# verify we can round trip a sphere

		fileName = os.path.join( self.temporaryDirectory(), "sphereWriteTest.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "sphere" )
		child.writeObject( IECoreScene.SpherePrimitive( 3.0 ), 0 )

		del root, child

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		sphere = root.child( "sphere" ).readObject( 0.0 )
		self.assertTrue( isinstance( sphere, IECoreScene.SpherePrimitive ) )
		self.assertEqual( 3.0, sphere.radius() )

	def testTraverseInstancedScene ( self ) :

		# Verify we can load a usd file which uses scene proxies

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/instances.usda", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( root.childNames(), ['InstanceSources', 'instance_0', 'instance_1'] )

		instance0Object = root.child("instance_0").child("world").readObject( 0.0 )
		instance1Object = root.child("instance_1").child("world").readObject( 0.0 )

		self.assertTrue( isinstance( instance0Object, IECoreScene.SpherePrimitive ) )
		self.assertTrue( isinstance( instance1Object, IECoreScene.SpherePrimitive ) )

	def testGeometricInterpretation( self ) :

		primitive = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 0 ) ] ) )
		self.assertEqual( primitive["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		for data, interpolation in [
			( IECore.V3fData( imath.V3f( 0 ) ), IECoreScene.PrimitiveVariable.Interpolation.Constant ),
			( IECore.V3fVectorData( [ imath.V3f( 0 ) ] ), IECoreScene.PrimitiveVariable.Interpolation.Vertex ),
		] :

			for interpretation in [
				IECore.GeometricData.Interpretation.Point,
				IECore.GeometricData.Interpretation.Vector,
				IECore.GeometricData.Interpretation.Normal,
				IECore.GeometricData.Interpretation.None_,
			] :

				dataCopy = data.copy()
				dataCopy.setInterpretation( interpretation )

				name = dataCopy.typeName() + str( interpretation )
				primitive[name] = IECoreScene.PrimitiveVariable( interpolation, dataCopy )

		fileName = os.path.join( self.temporaryDirectory(), "interpretationTest.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root.createChild( "points" ).writeObject( primitive, 0 )
		del root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		loadedPrimitive = root.child( "points" ).readObject( 0 )

		self.assertEqual( loadedPrimitive.keys(), primitive.keys() )
		for key in loadedPrimitive.keys() :

			self.assertEqual(
				loadedPrimitive[key].data.getInterpretation(),
				primitive[key].data.getInterpretation(),
				key + " interpretation not preserved"
			)

	def testUVs( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.usda", IECore.IndexedIO.OpenMode.Read )
		cube = root.child( "pCube1" ).readObject( 0.0 )

		self.assertIn( "uv", cube )
		self.assertEqual( cube["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( cube["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		self.assertNotIn( "st", cube )

	def testCanWriteEmptyUSDFile( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "usd_empty.usda" )
		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), [] )

	def testCanWriteTransformHierarchy( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "usd_transform.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		numberOneSon = sceneWrite.createChild( "nakamoto" )
		numberOneGrandson = numberOneSon.createChild( "satoshi" )
		testTransform = IECore.M44dData( imath.M44d( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 2, 3, 1 ) )
		numberOneGrandson.writeTransform( testTransform , 0.0 )

		del numberOneGrandson
		del numberOneSon
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), [ "nakamoto" ] )
		sceneReadNumberOneSon = sceneRead.child("nakamoto")
		self.assertEqual( sceneReadNumberOneSon.childNames(), [ "satoshi" ] )
		sceneReadNumberOneGrandson = sceneReadNumberOneSon.child("satoshi")

		self.assertEqual( sceneReadNumberOneGrandson.childNames(), [] )
		self.assertEqual( sceneReadNumberOneGrandson.readTransform( 0.0 ), testTransform)

	def testCanWriteSimpleMesh( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "usd_simple_mesh.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		flatEarth = sceneWrite.createChild( "flatearth" )

		planeDimensions = imath.Box2f( imath.V2f( 0, 0 ), imath.V2f( 1, 1 ) )
		planeDivisions = imath.V2i( 16, 16 )

		flatEarthObject = IECoreScene.MeshPrimitive.createPlane( planeDimensions, planeDivisions )

		flatEarth.writeObject( flatEarthObject, 0.0 )

		del flatEarth
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), ["flatearth"] )
		otherWorld = sceneRead.child( "flatearth" )

		self.assertTrue( otherWorld.hasObject() )

		object = otherWorld.readObject( 0.0 )
		self.assertIsInstance( object, IECoreScene.MeshPrimitive )

		otherMesh = object

		self.assertTrue("P" in otherMesh)

	def testCanWritePrimitiveVariables( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "usd_primvars.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		flatEarth = sceneWrite.createChild( "flatearth" )

		planeDimensions = imath.Box2f( imath.V2f( 0, 0 ), imath.V2f( 1, 1 ) )
		planeDivisions = imath.V2i( 1, 1 )

		quad = IECoreScene.MeshPrimitive.createPlane( planeDimensions, planeDivisions )

		uvData = IECore.V2fVectorData( [ imath.V2f( 1.0, 2.0 ) ], IECore.GeometricData.Interpretation.UV )
		uvIndexData = IECore.IntVectorData( [0, 0, 0, 0] )

		scalarTest = IECore.IntData( 123 )

		quad["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uvData, uvIndexData )
		quad["scalarTest"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, scalarTest )

		flatEarth.writeObject( quad, 0.0 )

		del flatEarth
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), ["flatearth"] )
		otherWorld = sceneRead.child( "flatearth" )

		readMesh = otherWorld.readObject( 0.0 )

		self.assertIsInstance( readMesh, IECoreScene.MeshPrimitive )

		self.assertTrue( "P" in readMesh )
		self.assertTrue( "uv" in readMesh )
		self.assertTrue( "scalarTest" in readMesh)

		roundTripData = readMesh["uv"].data
		roundTripIndices = readMesh["uv"].indices

		self.assertEqual(roundTripData, IECore.V2fVectorData( [ imath.V2f( 1.0, 2.0) ], IECore.GeometricData.Interpretation.UV ) )
		self.assertEqual(roundTripIndices, IECore.IntVectorData( [ 0, 0, 0, 0 ] ) )

		roundTripScalar = readMesh["scalarTest"].data
		self.assertEqual(roundTripScalar, IECore.IntData( 123 ) )

	def testCanWriteAttributes( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "usd_attributes.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		numberOneSon = sceneWrite.createChild( "nakamoto" )

		numberOneSon.writeAttribute( "s32", IECore.IntData( 1 ), 0.0 )
		numberOneSon.writeAttribute( "f32", IECore.FloatData( 2.0 ), 0.0 )
		numberOneSon.writeAttribute( "str", IECore.StringData( "hey-ho" ), 0.0 )
		numberOneSon.writeAttribute( "boo", IECore.BoolData( True ), 0.0 )

		del numberOneSon
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), [ "nakamoto" ] )
		sceneReadNumberOneSon = sceneRead.child("nakamoto")

		self.assertEqual( sceneReadNumberOneSon.readAttribute( "s32", 0.0 ), IECore.IntData( 1 ) )
		self.assertEqual( sceneReadNumberOneSon.readAttribute( "f32", 0.0 ), IECore.FloatData( 2.0 ) )
		self.assertEqual( sceneReadNumberOneSon.readAttribute( "str", 0.0 ), IECore.StringData( "hey-ho" ) )
		self.assertEqual( sceneReadNumberOneSon.readAttribute( "boo", 0.0 ), IECore.BoolData( True ) )

	def testCanWritePoints ( self ):

		fileName = os.path.join( self.temporaryDirectory(), "usd_points.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root = sceneWrite.createChild( "root" )

		positionData = []
		for i in range( 16 ) :
			for j in range( 16 ) :
				positionData.append( imath.V3f( [i, j, 0] ) );

		positions = IECore.V3fVectorData( positionData )

		pointsPrimitive = IECoreScene.PointsPrimitive( positions )
		root.writeObject( pointsPrimitive, 0.0 )

		del root
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), ["root"] )
		readRoot = sceneRead.child( "root" )

		self.assertTrue( readRoot.hasObject() )

		readPoints = readRoot.readObject( 0.0 )
		self.assertIsInstance( readPoints, IECoreScene.PointsPrimitive )

		roundTripPositions = readPoints["P"].data
		self.assertEqual( roundTripPositions, positions )

	def testCanWriteCurves( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "usd_curves.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root = sceneWrite.createChild( "root" )

		vertsPerCurveData = []
		positionData = []
		for i in range( 16 ) :
			for j in range( 16 ) :
				vertsPerCurveData.append( 4 );
				for k in range( 3 ) :
					positionData.append( imath.V3f( [i, j, k] ) )

		curvesPrimitive = IECoreScene.CurvesPrimitive( IECore.IntVectorData( vertsPerCurveData ), IECore.CubicBasisf.linear(), False, IECore.V3fVectorData( positionData ) )

		root.writeObject( curvesPrimitive, 0.0 )

		del root
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), ["root"] )
		readRoot = sceneRead.child( "root" )

		self.assertTrue( readRoot.hasObject() )

		readCurves = readRoot.readObject( 0.0 )
		self.assertIsInstance( readCurves, IECoreScene.CurvesPrimitive )

		self.assertEqual( readCurves.verticesPerCurve(), IECore.IntVectorData( vertsPerCurveData ) )
		roundTripPositions = readCurves["P"].data
		for a, b in zip(roundTripPositions, IECore.V3fVectorData( positionData )):
			self.assertEqual( a, b )

	def testCanWriteAnimatedTransforms( self ):

		fileName = os.path.join( self.temporaryDirectory(), "usd_animated_transforms.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root = sceneWrite.createChild( "root" )
		child = root.createChild( "child" )

		box = IECoreScene.MeshPrimitive.createBox( imath.Box3f ( imath.V3f(0,0,0), imath.V3f(1,1,1)))

		child.writeObject( box, 0)

		for t in range( 64 ):

			translation = IECore.M44dData( imath.M44d( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, t, 0, 0, 1 ) )
			root.writeTransform( translation , t )

		del box
		del child
		del root
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		readRoot = sceneRead.createChild( "root" )

		for t in range(64):
			self.assertEqual( readRoot.readTransform( t ), IECore.M44dData( imath.M44d( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, t, 0, 0, 1 ) ) )

	def testCanWriteAnimatedPositions( self ):

		fileName = os.path.join( self.temporaryDirectory(), "usd_animated_positions.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root = sceneWrite.createChild( "root" )
		child = root.createChild( "child" )

		for t in range( 64 ):

			planeDimensions = imath.Box2f( imath.V2f( 0, 0 ), imath.V2f( 1 + t, 1 ) )
			planeDivisions = imath.V2i( 1, 1 )

			plane = IECoreScene.MeshPrimitive.createPlane( planeDimensions, planeDivisions )
			child.writeObject( plane, t )

		del child
		del root
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		root = sceneRead.child( "root" )
		child = root.child( "child" )

		for t in range( 64 ):
			readObject = child.readObject ( t )

			self.assertEqual(readObject["P"].data[1][0], 1 + t)


	def testCanWriteSubD( self ):

		fileName = os.path.join( self.temporaryDirectory(), "usd_subd.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root = sceneWrite.createChild( "root" )
		child = root.createChild( "child" )

		box = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0, 0, 0 ), imath.V3f( 1, 1, 1 ) ) )
		box.interpolation = "catmullClark"

		child.writeObject ( box, 0.0 )

		del child
		del root
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		readRoot = sceneRead.child( "root" )
		readChild = readRoot.child( "child" )

		self.assertEqual(readChild.readObject( 0.0 ).interpolation, "catmullClark")

	def testCanWriteAnimatedPrimitiveVariable ( self ):

		fileName = os.path.join( self.temporaryDirectory(), "usd_animated_primvar.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root = sceneWrite.createChild( "root" )

		for t in range(32):
			positionData = []
			primvarData = []
			for i in range( 16 ) :
				for j in range( 16 ) :
					positionData.append( imath.V3f( [i, j, 0] ) )
					primvarData.append ( i * 16 + j + t )

			positions = IECore.V3fVectorData( positionData )
			primvar = IECore.IntVectorData( primvarData)

			pointsPrimitive = IECoreScene.PointsPrimitive( positions )
			pointsPrimitive["index"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, primvar )
			root.writeObject( pointsPrimitive, t )

		del root
		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), ["root"] )
		readRoot = sceneRead.child( "root" )

		self.assertTrue( readRoot.hasObject() )

		readPoints = readRoot.readObject( 0.0 )
		self.assertIsInstance( readPoints, IECoreScene.PointsPrimitive )

		roundTripPositions = readPoints["P"].data
		self.assertEqual( roundTripPositions, positions )

	def testCanWriteVariousPrimVarTypes ( self ):

		fileName = os.path.join( self.temporaryDirectory(), "usd_various_primvars.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root = sceneWrite.createChild( "root" )
		child = root.createChild( "child" )

		positions = IECore.V3fVectorData( [ imath.V3f(0, 0, 0 ) ] )
		c3f = IECore.Color3fVectorData( [ imath.Color3f( 0, 0, 0 ) ] )
		intStr = IECore.InternedStringVectorData ( [ IECore.InternedString("a")] )
		quat = IECore.QuatfVectorData( [ imath.Quatf(0,1,2,3)] )

		pointsPrimitive = IECoreScene.PointsPrimitive( positions )
		pointsPrimitive["c3f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, c3f)
		pointsPrimitive["token"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, intStr)
		pointsPrimitive["quat"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, quat)

		child.writeObject( pointsPrimitive, 0.0 )

		del root, sceneWrite, child

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), ["root"] )
		readRoot = sceneRead.child( "root" )
		readChild = readRoot.child( "child" )

		readObject = readChild.readObject( 0.0 )

		self.assertTrue("c3f" in readObject)
		self.assertIsInstance( readObject["c3f"].data, IECore.Color3fVectorData )

		self.assertTrue("token" in readObject)
		self.assertIsInstance( readObject["token"].data, IECore.InternedStringVectorData )

		self.assertTrue("quat" in readObject)
		self.assertIsInstance( readObject["quat"].data, IECore.QuatfVectorData )

	def testCanWriteTags( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "usd_sets.usda" )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		a = sceneWrite.createChild( "a" )
		b = a.createChild( "b" )
		b.writeTags( ["b_set"] )
		c = a.createChild( "c" )
		c.writeTags( ["cd_set"] )
		d = a.createChild( "d" )
		d.writeTags( ["cd_set"] )

		e = d.createChild("e")

		e.writeTags ( ["e_set"] )

		del e, d, c, b, a, sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), ["a"] )
		a = sceneRead.child( "a" )

		def sortedStringList(a):
			return sorted( [str( i ) for i in a] )

		self.assertEqual( sortedStringList( a.readTags( IECoreScene.SceneInterface.EveryTag ) ), sortedStringList( [IECore.InternedString( 'b_set' ), IECore.InternedString( 'cd_set' ), IECore.InternedString( 'e_set' )] ) )
		self.assertEqual( a.childNames(), ["b", "c", "d"] )

		b = a.child( "b" )
		self.assertEqual( b.readTags( IECoreScene.SceneInterface.LocalTag ), [IECore.InternedString('b_set')] )
		self.assertFalse( b.hasTag('b_set', IECoreScene.SceneInterface.AncestorTag) )
		self.assertFalse( b.hasTag('b_set', IECoreScene.SceneInterface.DescendantTag) )
		self.assertTrue( b.hasTag('b_set', IECoreScene.SceneInterface.LocalTag) )
		self.assertTrue( b.hasTag('b_set', IECoreScene.SceneInterface.EveryTag) )

		c = a.child( "c" )
		self.assertEqual( c.readTags( IECoreScene.SceneInterface.LocalTag), [IECore.InternedString('cd_set')] )
		self.assertFalse( c.hasTag('cd_set', IECoreScene.SceneInterface.AncestorTag) )
		self.assertFalse( c.hasTag('cd_set', IECoreScene.SceneInterface.DescendantTag) )
		self.assertTrue( c.hasTag('cd_set', IECoreScene.SceneInterface.LocalTag) )
		self.assertTrue( c.hasTag('cd_set', IECoreScene.SceneInterface.EveryTag) )

		d = a.child( "d" )
		self.assertEqual( d.readTags( IECoreScene.SceneInterface.LocalTag ), [IECore.InternedString('cd_set')] )

		self.assertEqual( d.childNames(), ["e"] )

		e = d.child("e")

		self.assertEqual( e.readTags( IECoreScene.SceneInterface.AncestorTag ), [IECore.InternedString('cd_set')] )
		self.assertEqual( e.readTags( IECoreScene.SceneInterface.DescendantTag ), [] )
		self.assertEqual( e.readTags( IECoreScene.SceneInterface.LocalTag ), [IECore.InternedString('e_set')] )
		self.assertEqual( sortedStringList (e.readTags( IECoreScene.SceneInterface.EveryTag )), sortedStringList([IECore.InternedString('e_set'), IECore.InternedString('cd_set')]) )

		self.assertTrue( e.hasTag('cd_set', IECoreScene.SceneInterface.AncestorTag ) )
		self.assertFalse( e.hasTag('cd_set', IECoreScene.SceneInterface.DescendantTag ) )
		self.assertFalse( e.hasTag('cd_set', IECoreScene.SceneInterface.LocalTag ) )
		self.assertTrue( e.hasTag('cd_set', IECoreScene.SceneInterface.EveryTag ) )

		self.assertFalse( e.hasTag('e_set', IECoreScene.SceneInterface.AncestorTag ) )
		self.assertFalse( e.hasTag('e_set', IECoreScene.SceneInterface.DescendantTag ) )
		self.assertTrue( e.hasTag('e_set', IECoreScene.SceneInterface.LocalTag ) )
		self.assertTrue( e.hasTag('e_set', IECoreScene.SceneInterface.EveryTag ) )

		self.assertFalse( e.hasTag('not_found', IECoreScene.SceneInterface.AncestorTag ) )

	def testSets( self ) :

		# Based on IECoreScene/SceneCacheTest.py
		# There is a difference in that we can't add the current location to a set written at the same location.
		# A
		#   B { 'don': ['/E'], 'john'; ['/F'] }
		#      E
		#      F
		#   C { 'don' : ['/O'] }
		#      O
		#   D { 'john' : ['/G] }
		#      G {'matti' : ['/'] }  this will not get written - added here so we ensure the other set information is writen inspite of
		# H
		#    I
		#       J
		#          K {'foo',['/L/M/N'] }
		#             L
		#                M
		#                   N

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		writeRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		A = writeRoot.createChild("A")
		B = A.createChild("B")
		C = A.createChild("C")
		D = A.createChild("D")
		E = B.createChild("E")
		F = B.createChild("F")
		G = D.createChild("G")

		H = writeRoot.createChild("H")
		I = H.createChild("I")
		J = I.createChild("J")
		K = J.createChild("K")
		L = K.createChild("L")
		M = L.createChild("M")
		N = M.createChild("N")

		O = C.createChild("O")

		B.writeSet( "don", IECore.PathMatcher( ['/E'] ) )
		B.writeSet( "john", IECore.PathMatcher( ['/F'] ) )
		C.writeSet( "don", IECore.PathMatcher( ['/O'] ) )
		D.writeSet( "john", IECore.PathMatcher( ['/G'] ) )
		K.writeSet( "foo", IECore.PathMatcher( ['/L/M/N'] ) )
		G.writeSet( "matti", IECore.PathMatcher( ['/'] ) )

		del O, N, M, L, K, J, I, H, G, F, E, D, C, B, A, writeRoot

		readRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set(readRoot.childNames()), set (['A', 'H']) )

		A = readRoot.child('A')

		self.assertEqual( set( A.childNames() ), set( ['B', 'C', 'D'] ) )
		B = A.child('B')
		C = A.child('C')
		D = A.child('D')
		E = B.child('E')
		F = B.child('F')
		H = readRoot.child('H')

		self.assertEqual( set( B.childNames() ), set( ['E', 'F'] ) )
		self.assertEqual( D.childNames(), ['G'] )

		self.assertEqual( set(B.readSet("don").paths() ), set(['/E'] ) )
		self.assertEqual( set(B.readSet("john").paths() ), set(['/F'] ) )
		self.assertEqual( set(C.readSet("don").paths() ), set(['/O'] ) )
		self.assertEqual( set(D.readSet("john").paths() ), set(['/G'] ) )

		self.assertEqual( set(E.readSet("don").paths() ), set([] ) )

		# Check the setNames returns all the sets in it's subtree
		self.assertEqual( set( B.setNames() ), set( ['don', 'john'] ) )
		self.assertEqual( set( C.setNames() ), set( ['don'] ) )
		self.assertEqual( set( D.setNames() ), set( ['john', 'matti'] ) )
		self.assertEqual( set( E.setNames() ), set() )
		self.assertEqual( set( F.setNames() ), set() )

		self.assertEqual( len( A.setNames() ), 3)
		self.assertEqual( set( A.setNames() ), set( ['don', 'john', 'matti'] ) )
		self.assertEqual( set( A.readSet( "don" ).paths() ), set( ['/B/E', '/C/O'] ) )
		self.assertEqual( set( A.readSet( "john" ).paths() ), set( ['/B/F', '/D/G'] ) )

		self.assertEqual( set( H.readSet( "foo" ).paths() ), set( ['/I/J/K/L/M/N'] ) )

		self.assertEqual( len( A.setNames( includeDescendantSets = False ) ), 0 )

		self.assertEqual( set( A.readSet( "don", includeDescendantSets = False ).paths() ), set() )
		self.assertEqual( set( A.readSet( "john", includeDescendantSets = False ).paths() ), set() )
		self.assertEqual( set( H.readSet( "foo", includeDescendantSets = False ).paths() ), set() )

		self.assertEqual( len( C.setNames( includeDescendantSets = False ) ), 1 )
		self.assertEqual( set( C.setNames( includeDescendantSets = False ) ), set( ['don'] ) )
		self.assertEqual( set( C.readSet( "don", includeDescendantSets = False ).paths()  ), set( ['/O'] ) )

	def testSetHashes( self ):

		# A
		#   B

		# Note we don't need to write out any sets to test the hashing a
		# as we only use scene graph location, filename & set name for the hash

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		writeRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		A = writeRoot.createChild("A")
		B = A.createChild("B")

		del A, B, writeRoot

		anotherFileName = os.path.join( self.temporaryDirectory(), "testAnotherFile.usda" )
		shutil.copyfile( fileName, anotherFileName )

		readRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		readRoot2 = IECoreScene.SceneInterface.create( anotherFileName, IECore.IndexedIO.OpenMode.Read )

		readRoot3 = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		A = readRoot.child('A')
		Ap = readRoot.child('A')

		self.assertNotEqual( A.hashSet("dummySetA"), A.hashSet("dummySetB") )
		self.assertEqual( A.hashSet("dummySetA"), Ap.hashSet("dummySetA") )

		B = A.child("B")

		self.assertNotEqual( A.hashSet("dummySetA"), B.hashSet("dummySetA") )

		A2 = readRoot2.child('A')
		self.assertNotEqual( A.hashSet("dummySetA"), A2.hashSet("dummySetA") )

		A3 = readRoot3.child('A')
		self.assertEqual( A.hashSet("dummySetA"), A3.hashSet("dummySetA") )

	def testCameras( self ):

		fileName = os.path.join( self.temporaryDirectory(), "cameras.usda" )

		testCameras = []
		for projection in [ "orthographic", "perspective" ]:
			for horizontalAperture in [0.3, 1, 20, 50, 100]:
				for verticalAperture in [0.3, 1, 20, 50, 100]:
					for horizontalApertureOffset in [0, -0.4, 2.1]:
						for verticalApertureOffset in [0, -0.4, 2.1]:
							for focalLength in [1, 10, 60.5]:
								index = len( testCameras )
								c = IECoreScene.Camera()
								c.setProjection( projection )
								c.setAperture( imath.V2f( horizontalAperture, verticalAperture ) )
								c.setApertureOffset( imath.V2f( horizontalApertureOffset, verticalApertureOffset ) )
								c.setFocalLength( focalLength )
								testCameras.append( ( index, imath.M44d(), c ) )

		for near in [ 0.01, 0.1, 1.7 ]:
			for far in [ 10, 100.9, 10000000 ]:
				index = len( testCameras )
				c = IECoreScene.Camera()
				c.setClippingPlanes( imath.V2f( near, far ) )
				testCameras.append( ( index, imath.M44d( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 2, index, 1 ), c ) )

		for scale in [ 0.01, 0.1, 0.001 ]:
			index = len( testCameras )
			c = IECoreScene.Camera()
			c.setProjection( "perspective" )
			c.setAperture( imath.V2f( 36, 24 ) )
			c.setFocalLength( 35 )
			c.setFocalLengthWorldScale( scale )
			testCameras.append( ( index, imath.M44d(), c ) )

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root = sceneWrite.createChild( "root" )

		for i, matrix, camObj in testCameras:
			cam = root.createChild( "cam%i" %i  )
			cam.writeTransform( IECore.M44dData( matrix ), 0.0 )
			cam.writeObject( camObj, 0.0 )
			del cam

		del root
		del sceneWrite

		usdFile = pxr.Usd.Stage.Open( fileName )

		for i, matrix, cortexCam in testCameras:
			cG = pxr.UsdGeom.Camera.Get( usdFile, "/root/cam%i" % i )
			c = cG.GetCamera()
			usdMatrix = cG.MakeMatrixXform().GetOpTransform( 1.0 )
			for i in range( 16 ):
				self.assertAlmostEqual( usdMatrix[i//4][i%4], matrix[i//4][i%4] )

			self.assertEqual( c.projection.name.lower(), cortexCam.getProjection() )

			if cortexCam.getProjection() == "orthographic":
				self.assertAlmostEqual( c.horizontalAperture * 0.1, cortexCam.getAperture()[0], places = 6 )
				self.assertAlmostEqual( c.verticalAperture * 0.1, cortexCam.getAperture()[1], places = 6 )
				self.assertAlmostEqual( c.horizontalApertureOffset * 0.1, cortexCam.getApertureOffset()[0], places = 6 )
				self.assertAlmostEqual( c.verticalApertureOffset * 0.1, cortexCam.getApertureOffset()[1], places = 6 )
			else:
				scale = 0.1 / cortexCam.getFocalLengthWorldScale()
				self.assertAlmostEqual( c.horizontalAperture * scale, cortexCam.getAperture()[0], places = 5 )
				self.assertAlmostEqual( c.verticalAperture * scale, cortexCam.getAperture()[1], places = 5 )
				self.assertAlmostEqual( c.horizontalApertureOffset * scale, cortexCam.getApertureOffset()[0], places = 5 )
				self.assertAlmostEqual( c.verticalApertureOffset * scale, cortexCam.getApertureOffset()[1], places = 5 )
				self.assertAlmostEqual( c.focalLength * scale, cortexCam.getFocalLength(), places = 5 )


			self.assertEqual( c.clippingRange.min, cortexCam.getClippingPlanes()[0] )
			self.assertEqual( c.clippingRange.max, cortexCam.getClippingPlanes()[1] )
			self.assertEqual( c.fStop, cortexCam.getFStop() )
			self.assertEqual( c.focusDistance, cortexCam.getFocusDistance() )
			self.assertEqual( cG.GetShutterOpenAttr().Get(), cortexCam.getShutter()[0] )
			self.assertEqual( cG.GetShutterCloseAttr().Get(), cortexCam.getShutter()[1] )

			try :
				from pxr import CameraUtil
			except ImportError :
				# As far as I can tell, CameraUtil is a part of the Imaging
				# module, which we don't currently build in GafferHQ/dependencies.
				continue

			for usdFit, cortexFit in [
					(CameraUtil.MatchHorizontally, IECoreScene.Camera.FilmFit.Horizontal),
					(CameraUtil.MatchVertically, IECoreScene.Camera.FilmFit.Vertical),
					(CameraUtil.Fit, IECoreScene.Camera.FilmFit.Fit),
					(CameraUtil.Crop, IECoreScene.Camera.FilmFit.Fill)
				]:

				for aspect in [ 0.3, 1, 2.5 ]:
					usdWindow = CameraUtil.ConformedWindow( c.frustum.GetWindow(), usdFit, aspect )
					cortexWindow = cortexCam.frustum( cortexFit, aspect )
					for i in range( 2 ):
						self.assertAlmostEqual( usdWindow.min[i], cortexWindow.min()[i], delta = max( 1, math.fabs( cortexWindow.min()[i] ) ) * 0.000002 )
						self.assertAlmostEqual( usdWindow.max[i], cortexWindow.max()[i], delta = max( 1, math.fabs( cortexWindow.max()[i] ) ) * 0.000002 )

		del usdFile

	def testCornersAndCreases( self ) :

		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		mesh.setInterpolation( "catmullClark" )
		mesh.setCorners( IECore.IntVectorData( [ 3 ] ), IECore.FloatVectorData( [ 2 ] ) )
		mesh.setCreases( IECore.IntVectorData( [ 2 ] ), IECore.IntVectorData( [ 0, 1 ] ), IECore.FloatVectorData( [ 2.5 ] ) )

		fileName = os.path.join( self.temporaryDirectory(), "test.usdc" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "cube" )
		child.writeObject( mesh, 0 )
		del root, child

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		child = root.child( "cube" )

		mesh2 = child.readObject( 0 )
		self.assertEqual( mesh.cornerIds(), mesh2.cornerIds() )
		self.assertEqual( mesh.cornerSharpnesses(), mesh2.cornerSharpnesses() )
		self.assertEqual( mesh.creaseLengths(), mesh2.creaseLengths() )
		self.assertEqual( mesh.creaseIds(), mesh2.creaseIds() )
		self.assertEqual( mesh.creaseSharpnesses(), mesh2.creaseSharpnesses() )

	def testMissingBehaviourCreate( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		scene.child( "test", missingBehaviour = scene.MissingBehaviour.CreateIfMissing ).writeObject( IECoreScene.SpherePrimitive(), 0 )
		del scene

		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( scene.childNames(), [ "test"] )

	def testNameSanitisation( self ) :

		# USD is extremely restrictive about the names prims can have.
		# Test that we sanitise names appropriately when writing files.

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		scene.createChild( "0" ).writeObject( IECoreScene.SpherePrimitive(), 0 )
		scene.createChild( "wot:no:colons?" )
		scene.child( "fistFullOf$$$", missingBehaviour = scene.MissingBehaviour.CreateIfMissing ).writeObject( IECoreScene.SpherePrimitive(), 0 )
		del scene

		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( scene.childNames(), [ "_0", "wot_no_colons_", "fistFullOf___" ] )

	def testQuatConversion( self ) :

		# Imath and Gf quaternions do not share the same memory layout.
		# Check that we account for that when writing.

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData() )
		points["quatf"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Constant,
			imath.Quatf( 0, imath.V3f( 1, 2, 3 ) )
		)

		child.writeObject( points, 0 )
		del root, child

		stage = pxr.Usd.Stage.Open( fileName )
		primVarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/test" ) )
		quat = primVarsAPI.GetPrimvar( "quatf" ).Get( 0 )
		self.assertEqual( quat, pxr.Gf.Quatf( 0, pxr.Gf.Vec3f( 1, 2, 3 ) ) )

	def testRefCountAfterWrite( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )

		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData() )
		points["test1"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Constant,
			IECore.IntVectorData( [ 1 ] )
		)
		points["test2"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Constant,
			IECore.IntVectorData( [ 2 ] )
		)

		refCounts = { k : points[k].data.refCount() for k in points.keys() }

		child.writeObject( points, 0 )
		del root, child

		self.assertEqual(
			{ k : points[k].data.refCount() for k in points.keys() },
			refCounts
		)

	def testWriteUVs( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		child.writeObject( IECoreScene.MeshPrimitive.createSphere( 1 ), 0 )
		del root, child

		stage = pxr.Usd.Stage.Open( fileName )
		primvarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/test" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "uv" ) )
		self.assertTrue( primvarsAPI.GetPrimvar( "st" ) )
		self.assertEqual(
			primvarsAPI.GetPrimvar( "st" ).GetTypeName().role,
			"TextureCoordinate"
		)

	def testPointBasedPrimvars( self ) :

		mesh = IECoreScene.MeshPrimitive.createSphere( 1 )
		mesh["velocity"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData(
				[ imath.V3f( 1, 2, 3 ) ] * mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ),
				IECore.GeometricData.Interpretation.Vector
			)
		)
		mesh["acceleration"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData(
				[ imath.V3f( 4, 5, 6 ) ] * mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ),
				IECore.GeometricData.Interpretation.Vector
			)
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		child.writeObject( mesh, 0 )
		del root, child

		# Make sure we redirect Cortex primitive variables to the correct
		# attributes of UsdGeomPointBased instead of writing them to
		# arbitrary primvars.

		stage = pxr.Usd.Stage.Open( fileName )
		primvarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/test" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "P" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "N" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "velocity" ) )
		if pxr.Usd.GetVersion() >= ( 0, 19, 11 ) :
			self.assertFalse( primvarsAPI.GetPrimvar( "acceleration" ) )

		usdMesh = pxr.UsdGeom.Mesh( stage.GetPrimAtPath( "/test" ) )
		self.assertTrue( usdMesh.GetPointsAttr().HasAuthoredValue() )
		self.assertTrue( usdMesh.GetNormalsAttr().HasAuthoredValue() )
		self.assertTrue( usdMesh.GetVelocitiesAttr().HasAuthoredValue() )
		if pxr.Usd.GetVersion() >= ( 0, 19, 11 ) :
			self.assertTrue( usdMesh.GetAccelerationsAttr().HasAuthoredValue() )

		# And that we can load them back in successfully.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		mesh2 = root.child( "test" ).readObject( 0.0 )
		self.assertEqual( mesh2, mesh )

	def testPointWidthsAndIds( self ) :

		# Write USD file

		points = IECoreScene.PointsPrimitive(
			IECore.V3fVectorData(
				[ imath.V3f( x, 0, 0 ) for x in range( 0, 10 ) ]
			)
		)
		points["id"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.Int64VectorData( range( 0, 10 ) ),
		)
		points["width"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( range( 10, 20 ) ),
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create(fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		child.writeObject( points, 0 )
		del root, child

		# Check we wrote the correct attributes and not arbitrary primitive variables

		stage = pxr.Usd.Stage.Open( fileName )
		primvarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/test" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "id" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "width" ) )

		usdPoints = pxr.UsdGeom.Points( stage.GetPrimAtPath( "/test" ) )
		self.assertTrue( usdPoints.GetIdsAttr().HasAuthoredValue() )
		self.assertTrue( usdPoints.GetWidthsAttr().HasAuthoredValue() )

		# Read back and check we end up where we started

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		points2 = root.child( "test" ).readObject( 0.0 )
		self.assertEqual( points2, points )

	def testConstantPointWidths( self ) :

		# Write USD file

		points = IECoreScene.PointsPrimitive(
			IECore.V3fVectorData(
				[ imath.V3f( x, 0, 0 ) for x in range( 0, 10 ) ]
			)
		)
		points["width"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Constant,
			IECore.FloatData( 2 ),
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		child.writeObject( points, 0 )
		del root, child

		# Read back and check we end up where we started

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		points2 = root.child( "test" ).readObject( 0.0 )
		self.assertEqual( points2, points )

	def testCurvesWidths( self ) :

		# Write USD file

		curves = IECoreScene.CurvesPrimitive(
			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[ imath.V3f( x, 0, 0 ) for x in range( 0, 4 ) ]
			)
		)
		curves["width"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( range( 0, 4 ) ),
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		child.writeObject( curves, 0 )
		del root, child

		# Check we wrote the correct attributes and not arbitrary primitive variables

		stage = pxr.Usd.Stage.Open( fileName )
		primvarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/test" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "width" ) )

		usdCurves = pxr.UsdGeom.Curves( stage.GetPrimAtPath( "/test" ) )
		self.assertTrue( usdCurves.GetWidthsAttr().HasAuthoredValue() )

		# Read back and check we end up where we started

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		curves2 = root.child( "test" ).readObject( 0.0 )
		self.assertEqual( curves2, curves )

	def testConstantCurveWidths( self ) :

		# Write USD file

		curves = IECoreScene.CurvesPrimitive(
			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[ imath.V3f( x, 0, 0 ) for x in range( 0, 4 ) ]
			)
		)
		curves["width"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Constant,
			IECore.FloatData( 2 ),
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		child.writeObject( curves, 0 )
		del root, child

		# Read back and check we end up where we started

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		curves2 = root.child( "test" ).readObject( 0.0 )
		self.assertEqual( curves2, curves )

	def testCurveBasisAndWrap( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "curvesBasisAndWrap.usda" )

		for basis in [
			IECore.CubicBasisf.linear(),
			IECore.CubicBasisf.bezier(),
			IECore.CubicBasisf.bSpline(),
			IECore.CubicBasisf.catmullRom()
		] :

			for periodic in ( True, False ) :

				curves = IECoreScene.CurvesPrimitive(
					IECore.IntVectorData( [ 4 ] ),
					basis,
					periodic,
					IECore.V3fVectorData(
						[ imath.V3f( x, 0, 0 ) for x in range( 0, 4 ) ]
					)
				)

				root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
				root.createChild( "test" ).writeObject( curves, 0 )
				del root

				# Read back and check we end up where we started

				root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
				curves2 = root.child( "test" ).readObject( 0.0 )

				self.assertEqual( curves2.basis(), curves.basis() )

				self.assertEqual( curves2, curves )
				del root

	def testIndexedWidths( self ) :

		# Write USD file from points with indexed widths

		points = IECoreScene.PointsPrimitive(
			IECore.V3fVectorData(
				[ imath.V3f( x, 0, 0 ) for x in range( 0, 10 ) ]
			)
		)
		points["width"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( [ 1, 2 ] ),
			IECore.IntVectorData( [ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 ] )
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root.createChild( "test" ).writeObject( points, 0 )
		del root

		# Check that we wrote expanded data, since USD doesn't support
		# indices for widths.

		stage = pxr.Usd.Stage.Open( fileName )
		usdPoints = pxr.UsdGeom.Points( stage.GetPrimAtPath( "/test" ) )
		self.assertEqual( usdPoints.GetWidthsAttr().Get( 0 ), pxr.Vt.FloatArray( [ 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 ] ) )

if __name__ == "__main__":
	unittest.main()
