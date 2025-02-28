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

import IECore

import importlib
import os
import math
import unittest
import pathlib
import shutil
import tempfile
import imath
import subprocess
import sys
import threading
import time

import IECore
import IECoreScene
import IECoreUSD

import pxr.Usd
import pxr.UsdGeom

if pxr.Usd.GetVersion() < ( 0, 19, 3 ) :
	pxr.Usd.Attribute.HasAuthoredValue = pxr.Usd.Attribute.HasAuthoredValueOpinion

haveVDB = importlib.util.find_spec( "IECoreVDB" ) is not None

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

	def assertSetNamesEqual( self, setNames1, setNames2 ) :

		self.assertEqual(
			{ str( x ) for x in setNames1 },
			{ str( x ) for x in setNames2 }
		)
		# Duplicates are not allowed
		self.assertEqual( len( set( setNames1 ) ), len( setNames1 ) )

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
		self.assertEqual( set( cubeMesh.keys() ), { "P", "uv" } )
		self.assertTrue( "render:displayColor" in cube.attributeNames() )
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
			'test_Float4_Array_constant' : IECore.Color4fVectorData( [imath.Color4f( 1.1, 1.2, 1.3, 1.4 ), imath.Color4f( 2.1, 2.2, 2.3, 2.4 ), imath.Color4f( 3.1, 3.2, 3.3, 3.4 )] ),
			'test_Float4_Scalar_constant' : IECore.Color4fData( imath.Color4f( 0.1, 0.2, 0.3, 0.4 ) ),
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
			'test_deprecated_Scalar_constant' : IECore.BoolData( 0 ),
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

	def testSpherePrimitiveAnimation( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "sphereTest.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "sphere" )
		child.writeObject( IECoreScene.SpherePrimitive( 1.0 ), 0 )
		child.writeObject( IECoreScene.SpherePrimitive( 2.0 ), 1 )
		del root, child

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		child = root.child( "sphere" )
		self.assertEqual( child.readObject( 0 ).radius(), 1 )
		self.assertEqual( child.readObject( 1 ).radius(), 2 )

	def testCubeRead( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )
		pxr.UsdGeom.Cube.Define( stage, "/defaultCube" )
		pxr.UsdGeom.Cube.Define( stage, "/littleCube" ).CreateSizeAttr().Set( 0.5 )
		animatedAttr = pxr.UsdGeom.Cube.Define( stage, "/animatedCube" ).CreateSizeAttr()
		animatedAttr.Set( 5, 0 )
		animatedAttr.Set( 10, 1 )
		stage.GetRootLayer().Save()
		del stage

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		defaultCube = root.child( "defaultCube" )
		defaultCubeObject = defaultCube.readObject( 0.0 )
		self.assertTrue( isinstance( defaultCubeObject, IECoreScene.MeshPrimitive ) )
		self.assertEqual( defaultCubeObject.bound(), imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		self.assertEqual( defaultCube.hash( root.HashType.ObjectHash, 0 ), defaultCube.hash( root.HashType.ObjectHash, 1 ) )

		littleCube = root.child( "littleCube" )
		littleCubeObject = littleCube.readObject( 0.0 )
		self.assertTrue( isinstance( littleCubeObject, IECoreScene.MeshPrimitive ) )
		self.assertEqual( littleCubeObject.bound(), imath.Box3f( imath.V3f( -0.25 ), imath.V3f( 0.25 ) ) )
		self.assertEqual( littleCube.hash( root.HashType.ObjectHash, 0 ), littleCube.hash( root.HashType.ObjectHash, 1 ) )

		animatedCube = root.child( "animatedCube" )
		animatedCubeObject = animatedCube.readObject( 0.0 )
		self.assertTrue( isinstance( animatedCubeObject, IECoreScene.MeshPrimitive ) )
		self.assertEqual( animatedCubeObject.bound(), imath.Box3f( imath.V3f( -2.5 ), imath.V3f( 2.5 ) ) )
		self.assertNotEqual( animatedCube.hash( root.HashType.ObjectHash, 0 ), animatedCube.hash( root.HashType.ObjectHash, 1 ) )
		animatedCubeObject = animatedCube.readObject( 1.0 )
		self.assertTrue( isinstance( animatedCubeObject, IECoreScene.MeshPrimitive ) )
		self.assertEqual( animatedCubeObject.bound(), imath.Box3f( imath.V3f( -5 ), imath.V3f( 5 ) ) )

	def testTraverseInstancedScene ( self ) :

		# Verify we can load a usd file which uses scene proxies

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/instances.usda", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( root.childNames(), [ "instance0", "instance1", "notInstance" ] )

		instance0Object = root.child( "instance0").child( "world" ).readObject( 0.0 )
		instance1Object = root.child( "instance1").child( "world" ).readObject( 0.0 )

		self.assertIsInstance( instance0Object, IECoreScene.SpherePrimitive )
		self.assertEqual( instance0Object, instance1Object )

	def testInstancesShareHashes( self ) :

		scene = IECoreScene.SceneInterface.create(
			os.path.dirname( __file__ ) + "/data/instances.usda",
			IECore.IndexedIO.OpenMode.Read
		)

		instance0 = scene.child( "instance0" )
		instance0Child = instance0.child( "world" )
		instance1 = scene.child( "instance1" )
		instance1Child = instance1.child( "world" )
		notInstance = scene.child( "notInstance" )
		notInstanceChild = notInstance.child( "world" )

		for hashType in scene.HashType.names.values() :
			# Instanced locations should share hashes
			self.assertEqual( instance0Child.hash( hashType, 0 ), instance1Child.hash( hashType, 0 ) )
			# Non-instanced locations should not share hashes with instanced locations,
			# even though in this case they reference the same file. The non-instanced
			# version may have overrides, and there is no cheap way of detecting that it
			# doesn't.
			if hashType != scene.HashType.AttributesHash :
				self.assertNotEqual( notInstanceChild.hash( hashType, 0 ), instance0Child.hash( hashType, 0 ) )
			else :
				# Attribute hashes are the same with or without instancing, simply because
				# we don't have any attributes in this example.
				self.assertEqual( notInstanceChild.hash( hashType, 0 ), instance0Child.hash( hashType, 0 ) )

			# The roots of the instanced locations should also not share hashes, because
			# overrides are allowed at this level. Attribute, bound and object hashes are
			# equal regardless in this case, because the locations lack those properties.
			if hashType not in ( scene.HashType.AttributesHash, scene.HashType.BoundHash, scene.HashType.ObjectHash ) :
				self.assertNotEqual( instance0.hash( hashType, 0 ), instance1.hash( hashType, 0 ) )
				self.assertNotEqual( notInstance.hash( hashType, 0 ), instance0.hash( hashType, 0 ) )
			else :
				self.assertEqual( instance0.hash( hashType, 0 ), instance1.hash( hashType, 0 ) )
				self.assertEqual( notInstance.hash( hashType, 0 ), instance0.hash( hashType, 0 ) )

	def testInstancePrototypeHashesNotReused( self ) :

		# The original intent of this test was to check that we assign consistent hashes to prototypes when
		# opening and closing the same file ( which triggers USD to randomly shuffle the prototype names ).

		# The solution we've ended up with is instead of assigning consistent hashes, each instance of the
		# file gets it's own unique hashes, and is basically treated separately. This allows us to just
		# use the prototype names in the hash, since we force the hash to be unique anyway.

		usedHashes = set()

		for i in range( 100 ):
			scene = IECoreScene.SceneInterface.create(
				os.path.dirname( __file__ ) + "/data/severalInstances.usda",
				IECore.IndexedIO.OpenMode.Read
			)

			instance0 = scene.child( "instance0" )
			instance0Child = instance0.child( "world" )
			instance10 = scene.child( "instance10" )
			instance10Child = instance10.child( "model" ).child( "assembly" )

			h1 = instance0Child.hash( scene.HashType.TransformHash, 1 )
			h2 = instance10Child.hash( scene.HashType.TransformHash, 1 )

			self.assertNotEqual( h1, h2 )

			self.assertNotIn( h1, usedHashes )
			for j in range( 1, 10 ):
				instanceJ = scene.child( "instance%i" % j )
				self.assertEqual( h1, instanceJ.child( "world" ).hash( scene.HashType.TransformHash, 1 ) )
				del instanceJ

			self.assertNotIn( h2, usedHashes )
			for j in range( 11, 20 ):
				instanceJ = scene.child( "instance%i" % j )
				self.assertEqual( h2, instanceJ.child( "model" ).child( "assembly" ).hash( scene.HashType.TransformHash, 1 ) )
				del instanceJ

			usedHashes.add( h1 )
			usedHashes.add( h2 )

			# We must carefully delete everything in order to reliably trigger USD randomly switching the
			# prototype names around ( I guess waiting for the garbage collector means we might not be
			# fully closing the file before we open it again? Weird, but seems reproducible ).
			# This is no longer really crucial to this test, since we just force every instance of the file
			# to get unique hashes rather than trying to keep prototypes hashing the same, but I'm trying to
			# document as much intent as possible here, in case we consider a different solution in the future.
			del instance0Child
			del instance0
			del instance10Child
			del instance10
			del scene

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
				for k in range( 4 ) :
					positionData.append( imath.V3f( [i, j, k] ) )

		curvesPrimitive = IECoreScene.CurvesPrimitive( IECore.IntVectorData( vertsPerCurveData ), IECore.CubicBasisf.linear(), False, IECore.V3fVectorData( positionData ) )
		self.assertTrue( curvesPrimitive.arePrimitiveVariablesValid() )

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

	def testSubdOptions( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		resaveFileName = os.path.join( self.temporaryDirectory(), "resave.usda" )

		# We need a list of all the values from USD we should support. There probably should be a
		# more direct way to get this, but I have already wasted far, far too much time trying to
		# understand which USD API to use.
		dummyStage = pxr.Usd.Stage.CreateInMemory()
		dummyMesh = pxr.UsdGeom.Mesh.Define( dummyStage, "/mesh" )
		allowedSubScheme = dummyMesh.GetSubdivisionSchemeAttr().GetMetadata( "allowedTokens" )
		allowedIB = dummyMesh.GetInterpolateBoundaryAttr().GetMetadata( "allowedTokens" )
		allowedFVLI = dummyMesh.GetFaceVaryingLinearInterpolationAttr().GetMetadata( "allowedTokens" )
		allowedTS = dummyMesh.GetTriangleSubdivisionRuleAttr().GetMetadata( "allowedTokens" )

		del dummyMesh
		del dummyStage

		for property, allowed in [
			( "subdivisionScheme", allowedSubScheme ),
			( "interpolateBoundary", allowedIB ),
			( "faceVaryingLinearInterpolation", allowedFVLI ),
			( "triangleSubdivisionRule", allowedTS ),

		]:
			for value in allowed:

				if property == "subdivisionScheme" and value == "bilinear":
					# We know we don't support this
					continue

				stage = pxr.Usd.Stage.CreateNew( fileName )
				mesh = pxr.UsdGeom.Mesh.Define( stage, "/mesh" )
				if property == "subdivisionScheme":
					mesh.CreateSubdivisionSchemeAttr().Set( value )
				else:
					mesh.CreateSubdivisionSchemeAttr().Set( "catmullClark" )

				if property == "interpolateBoundary":
					mesh.CreateInterpolateBoundaryAttr().Set( value, 0.0 )

				if property == "faceVaryingLinearInterpolation":
					mesh.CreateFaceVaryingLinearInterpolationAttr().Set( value, 0.0 )

				if property == "triangleSubdivisionRule":
					mesh.CreateTriangleSubdivisionRuleAttr().Set( value, 0.0 )

				stage.GetRootLayer().Save()
				del stage

				root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

				cortexMesh = root.child( "mesh" ).readObject( 0.0 )
				if property == "subdivisionScheme":
					if value == "none":
						self.assertEqual( cortexMesh.interpolation, "linear" )
					else:
						self.assertEqual( cortexMesh.interpolation, value )
				elif property == "interpolateBoundary":
					self.assertEqual( cortexMesh.getInterpolateBoundary(), value )
				elif property == "faceVaryingLinearInterpolation":
					self.assertEqual( cortexMesh.getFaceVaryingLinearInterpolation(), value )
				elif property == "triangleSubdivisionRule":
					self.assertEqual( cortexMesh.getTriangleSubdivisionRule(), value )

				sceneWrite = IECoreScene.SceneInterface.create( resaveFileName, IECore.IndexedIO.OpenMode.Write )
				root = sceneWrite.createChild( "root" )
				child = root.createChild( "mesh" )

				child.writeObject ( cortexMesh, 0.0 )

				del child
				del root
				del sceneWrite

				rereadFile = pxr.Usd.Stage.Open( resaveFileName )
				rereadMesh = pxr.UsdGeom.Mesh.Get( rereadFile, "/root/mesh" )

				if property == "subdivisionScheme":
					self.assertEqual( rereadMesh.GetSubdivisionSchemeAttr().Get( 0.0 ), value )
				elif property == "interpolateBoundary":
					self.assertEqual( rereadMesh.GetInterpolateBoundaryAttr().Get( 0.0 ), value )
				elif property == "faceVaryingLinearInterpolation":
					self.assertEqual( rereadMesh.GetFaceVaryingLinearInterpolationAttr().Get( 0.0 ), value )
				elif property == "triangleSubdivisionRule":
					self.assertEqual( rereadMesh.GetTriangleSubdivisionRuleAttr().Get( 0.0 ), value )

				del rereadMesh
				del rereadFile

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
		self.assertTrue( pointsPrimitive.arePrimitiveVariablesValid() )

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

		self.assertSetNamesEqual( a.readTags( IECoreScene.SceneInterface.EveryTag ), [ "b_set", "cd_set", "e_set" ] )
		self.assertEqual( a.childNames(), ["b", "c", "d"] )

		b = a.child( "b" )
		self.assertSetNamesEqual( b.readTags( IECoreScene.SceneInterface.LocalTag ), [ "b_set" ] )
		self.assertFalse( b.hasTag('b_set', IECoreScene.SceneInterface.AncestorTag) )
		self.assertFalse( b.hasTag('b_set', IECoreScene.SceneInterface.DescendantTag) )
		self.assertTrue( b.hasTag('b_set', IECoreScene.SceneInterface.LocalTag) )
		self.assertTrue( b.hasTag('b_set', IECoreScene.SceneInterface.EveryTag) )

		c = a.child( "c" )
		self.assertSetNamesEqual( c.readTags( IECoreScene.SceneInterface.LocalTag), [ "cd_set" ] )
		self.assertFalse( c.hasTag('cd_set', IECoreScene.SceneInterface.AncestorTag) )
		self.assertFalse( c.hasTag('cd_set', IECoreScene.SceneInterface.DescendantTag) )
		self.assertTrue( c.hasTag('cd_set', IECoreScene.SceneInterface.LocalTag) )
		self.assertTrue( c.hasTag('cd_set', IECoreScene.SceneInterface.EveryTag) )

		d = a.child( "d" )
		self.assertSetNamesEqual( d.readTags( IECoreScene.SceneInterface.LocalTag ), [ "cd_set" ] )

		self.assertEqual( d.childNames(), ["e"] )

		e = d.child("e")

		self.assertSetNamesEqual( e.readTags( IECoreScene.SceneInterface.AncestorTag ), [ "cd_set" ] )
		self.assertSetNamesEqual( e.readTags( IECoreScene.SceneInterface.DescendantTag ), [] )
		self.assertSetNamesEqual( e.readTags( IECoreScene.SceneInterface.LocalTag ), [ "e_set" ] )
		self.assertSetNamesEqual( e.readTags( IECoreScene.SceneInterface.EveryTag ), [ "e_set", "cd_set" ] )

		self.assertTrue( e.hasTag('cd_set', IECoreScene.SceneInterface.AncestorTag ) )
		self.assertFalse( e.hasTag('cd_set', IECoreScene.SceneInterface.DescendantTag ) )
		self.assertFalse( e.hasTag('cd_set', IECoreScene.SceneInterface.LocalTag ) )
		self.assertTrue( e.hasTag('cd_set', IECoreScene.SceneInterface.EveryTag ) )

		self.assertFalse( e.hasTag('e_set', IECoreScene.SceneInterface.AncestorTag ) )
		self.assertFalse( e.hasTag('e_set', IECoreScene.SceneInterface.DescendantTag ) )
		self.assertTrue( e.hasTag('e_set', IECoreScene.SceneInterface.LocalTag ) )
		self.assertTrue( e.hasTag('e_set', IECoreScene.SceneInterface.EveryTag ) )

		self.assertFalse( e.hasTag('not_found', IECoreScene.SceneInterface.AncestorTag ) )

		stage = pxr.Usd.Stage.Open( fileName )
		self.assertFalse( stage.GetDefaultPrim() )

	def testSets( self ) :

		# Based on IECoreScene/SceneCacheTest.py
		#
		# A
		#   B { 'don': ['/E'], 'john'; ['/F'] }
		#      E
		#      F
		#   C { 'don' : ['/O'] }
		#      O
		#   D { 'john' : ['/G] }
		#      G {'matti' : ['/'] }
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
		G = D.child('G')
		H = readRoot.child('H')

		self.assertEqual( set( B.childNames() ), set( ['E', 'F'] ) )
		self.assertEqual( D.childNames(), ['G'] )

		self.assertEqual( set(B.readSet("don").paths() ), set(['/E'] ) )
		self.assertEqual( set(B.readSet("john").paths() ), set(['/F'] ) )
		self.assertEqual( set(C.readSet("don").paths() ), set(['/O'] ) )
		self.assertEqual( set(D.readSet("john").paths() ), set(['/G'] ) )
		self.assertEqual( set(G.readSet("matti").paths() ), set(['/'] ) )

		self.assertEqual( set(E.readSet("don").paths() ), set([] ) )

		# Check the setNames returns all the sets in it's subtree
		self.assertSetNamesEqual( B.setNames(), [ "don", "john" ] )
		self.assertSetNamesEqual( C.setNames(), [ "don" ] )
		self.assertSetNamesEqual( D.setNames(), [ "john", "matti" ] )
		self.assertSetNamesEqual( E.setNames(), [] )
		self.assertSetNamesEqual( F.setNames(), [] )

		self.assertSetNamesEqual( A.setNames(), [ "don", "john", "matti" ] )
		self.assertEqual( set( A.readSet( "don" ).paths() ), set( ['/B/E', '/C/O'] ) )
		self.assertEqual( set( A.readSet( "john" ).paths() ), set( ['/B/F', '/D/G'] ) )
		self.assertEqual( set( A.readSet( "matti" ).paths() ), set( ['/D/G'] ) )

		self.assertEqual( set( H.readSet( "foo" ).paths() ), set( ['/I/J/K/L/M/N'] ) )

		self.assertSetNamesEqual( A.setNames( includeDescendantSets = False ), [] )

		self.assertEqual( set( A.readSet( "don", includeDescendantSets = False ).paths() ), set() )
		self.assertEqual( set( A.readSet( "john", includeDescendantSets = False ).paths() ), set() )
		self.assertEqual( set( H.readSet( "foo", includeDescendantSets = False ).paths() ), set() )

		self.assertSetNamesEqual( C.setNames( includeDescendantSets = False ), [ "don" ] )
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

		A = readRoot.child('A')
		Ap = readRoot.child('A')

		self.assertNotEqual( A.hashSet("dummySetA"), A.hashSet("dummySetB") )
		self.assertEqual( A.hashSet("dummySetA"), Ap.hashSet("dummySetA") )

		B = A.child("B")

		self.assertNotEqual( A.hashSet("dummySetA"), B.hashSet("dummySetA") )

		A2 = readRoot2.child('A')
		self.assertNotEqual( A.hashSet("dummySetA"), A2.hashSet("dummySetA") )

	def testSetsAtRoot( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "child" )
		grandChild = child.createChild( "grandChild" )
		root.writeSet( "test", IECore.PathMatcher( [ "/child/grandChild" ] ) )

		del root, child, grandChild

		# We want to be able to read the set from the same place we wrote it. We can,
		# but this relies on `includeDescendantSets = True` being the default.
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.readSet( "test" ), IECore.PathMatcher( [ "/child/grandChild" ] ) )
		# In fact, due to a USD limitation we will have authored the set onto
		# the child instead.
		self.assertEqual( root.readSet( "test", includeDescendantSets = False ), IECore.PathMatcher() )
		self.assertEqual( root.child( "child" ).readSet( "test", includeDescendantSets = False ), IECore.PathMatcher( [ "/grandChild" ] ) )

	def testCameras( self ):

		# Write a range of cameras from Cortex to USD

		fileName = os.path.join( self.temporaryDirectory(), "cameras.usda" )

		def formatCameraName( **kw ) :

			formatted = {
				k : str( v ).replace( ".", "_" ).replace( "-", "_" ) for k, v in kw.items()
			}
			return "_".join( [ k + "_" + v for k, v in formatted.items() ] )

		testCameras = {}
		for projection in [ "orthographic", "perspective" ]:
			for horizontalAperture in [0.3, 1, 20, 50, 100]:
				for verticalAperture in [0.3, 1, 20, 50, 100]:
					for horizontalApertureOffset in [0, -0.4, 2.1]:
						for verticalApertureOffset in [0, -0.4, 2.1]:
							for focalLength in [1, 10, 60.5]:
								c = IECoreScene.Camera()
								c.setProjection( projection )
								c.setAperture( imath.V2f( horizontalAperture, verticalAperture ) )
								c.setApertureOffset( imath.V2f( horizontalApertureOffset, verticalApertureOffset ) )
								c.setFocalLength( focalLength )
								name = formatCameraName(
									projection = projection,
									horizontalAperture = horizontalAperture,
									verticalAperture = verticalAperture,
									horizontalApertureOffset = horizontalApertureOffset,
									verticalApertureOffset = verticalApertureOffset,
									focalLength = focalLength,
								)
								testCameras[name] = c

		for near in [ 0.01, 0.1, 1.7 ]:
			for far in [ 10, 100.9, 10000000 ]:
				c = IECoreScene.Camera()
				c.setClippingPlanes( imath.V2f( near, far ) )
				name = formatCameraName( near = near, far = far )
				testCameras[name] = c

		for scale in [ 0.01, 0.1, 0.001 ]:
			c = IECoreScene.Camera()
			c.setProjection( "perspective" )
			c.setAperture( imath.V2f( 36, 24 ) )
			c.setFocalLength( 35 )
			c.setFocalLengthWorldScale( scale )
			name = formatCameraName( scale = scale )
			testCameras[name] = c

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		for name, c in testCameras.items() :
			root.createChild( name ).writeObject( c, 0.0 )

		del root

		# Load them via the USD API and check that they are as expected

		usdFile = pxr.Usd.Stage.Open( fileName )

		for name, cortexCam in testCameras.items() :

			cG = pxr.UsdGeom.Camera.Get( usdFile, "/" + name )
			c = cG.GetCamera()

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

		# Check that we can load them back into Cortex appropriately.
		# We do not check for perfect round tripping because USD doesn't
		# provide a variable focalLengthWorldScale.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		def assertVectorsAlmostEqual( a, b,**kw ) :

			for i in range( a.dimensions() ) :
				self.assertAlmostEqual( a[i], b[i], **kw )

		for name, c in testCameras.items() :

			c2 = root.child( name ).readObject( 0.0 )
			self.assertEqual( c2.getProjection(), c.getProjection() )

			if c.getProjection() == "perspective" :

				assertVectorsAlmostEqual(
					c2.getAperture()* c2.getFocalLengthWorldScale(),
					c.getAperture() * c.getFocalLengthWorldScale()
				)

				assertVectorsAlmostEqual(
					c2.getApertureOffset() * c2.getFocalLengthWorldScale(),
					c.getApertureOffset() * c.getFocalLengthWorldScale()
				)

				self.assertAlmostEqual(
					c2.getFocalLength() * c2.getFocalLengthWorldScale(),
					c.getFocalLength() * c.getFocalLengthWorldScale()
				)
				self.assertEqual( c2.getFocalLengthWorldScale(), IECore.FloatData( 0.1 ).value )

			elif c.getProjection() == "orthographic" :

				assertVectorsAlmostEqual( c2.getAperture(), c.getAperture() )
				assertVectorsAlmostEqual( c2.getApertureOffset(), c.getApertureOffset() )

			self.assertEqual( c2.getClippingPlanes(), c.getClippingPlanes() )
			self.assertEqual( c2.getFStop(), c.getFStop() )
			self.assertEqual( c2.getFocusDistance(), c.getFocusDistance() )
			self.assertEqual( c2.getShutter(), c.getShutter() )

			assertVectorsAlmostEqual( c2.frustum().min(), c.frustum().min(), places = 6 )

		# Now rewrite back to USD and reload. This should round-trip exactly because
		# the focalLengthWorldScale has now been hardcoded to the USD equivalent value.

		roundTripFileName = os.path.join( self.temporaryDirectory(), "roundTrippedCameras.usda" )
		roundTripRoot = IECoreScene.SceneInterface.create( roundTripFileName, IECore.IndexedIO.OpenMode.Write )

		for name in testCameras :

			camera = root.child( name ).readObject( 0.0 )
			roundTripRoot.createChild( name ).writeObject( camera, 0.0 )

		del roundTripRoot
		roundTripRoot = IECoreScene.SceneInterface.create( roundTripFileName, IECore.IndexedIO.OpenMode.Read )

		for name in testCameras :

			camera = root.child( name ).readObject( 0.0 )
			roundTripCamera = roundTripRoot.child( name ).readObject( 0.0 )

			self.assertEqual( camera, roundTripCamera )

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
		# arbitrary primvars. We don't use the `normals` attribute though,
		# since USD documents `primvars:normals` to take precedence, and
		# it is the only way we can preserve indexed normals.

		stage = pxr.Usd.Stage.Open( fileName )
		primvarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/test" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "P" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "N" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "velocity" ) )
		if pxr.Usd.GetVersion() >= ( 0, 19, 11 ) :
			self.assertFalse( primvarsAPI.GetPrimvar( "acceleration" ) )

		usdMesh = pxr.UsdGeom.Mesh( stage.GetPrimAtPath( "/test" ) )
		self.assertTrue( usdMesh.GetPointsAttr().HasAuthoredValue() )
		self.assertFalse( usdMesh.GetNormalsAttr().HasAuthoredValue() )
		self.assertTrue( primvarsAPI.GetPrimvar( "normals" ) )
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

	def testReadNurbsCurves( self ) :

		# Write USD file

		fileName = os.path.join( self.temporaryDirectory(), "nurbs.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )

		curves = pxr.UsdGeom.NurbsCurves.Define( stage, "/cubic" )
		curves.CreateCurveVertexCountsAttr().Set( [ 4 ] )
		curves.CreatePointsAttr().Set( [ ( x, x, x ) for x in range( 0, 4 ) ] )
		curves.CreateOrderAttr().Set( [ 4 ] )
		curves.CreateKnotsAttr().Set( [ 0, 0, 0, 0.333, 0.666, 1, 1, 1 ] )

		curves = pxr.UsdGeom.NurbsCurves.Define( stage, "/nonCubic" )
		curves.CreateCurveVertexCountsAttr().Set( [ 4 ] )
		curves.CreatePointsAttr().Set( [ ( x, x, x ) for x in range( 0, 4 ) ] )
		curves.CreateOrderAttr().Set( [ 3 ] )
		curves.CreateKnotsAttr().Set( [ 0, 0, 0, 0.5, 1, 1, 1 ] )

		stage.GetRootLayer().Save()
		del stage

		# Load and check

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		cubic = root.child( "cubic" ).readObject( 0.0 )
		self.assertIsInstance( cubic, IECoreScene.CurvesPrimitive )
		self.assertEqual( cubic.verticesPerCurve(), IECore.IntVectorData( [ 4 ] ) )
		self.assertEqual( cubic.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( cubic.periodic(), False )
		self.assertEqual(
			cubic["P"].data,
			IECore.V3fVectorData(
				[ imath.V3f( x ) for x in range( 0, 4 ) ],
				IECore.GeometricData.Interpretation.Point
			)
		)

		nonCubic = root.child( "nonCubic" ).readObject( 0.0 )
		self.assertIsInstance( nonCubic, IECoreScene.CurvesPrimitive )
		self.assertEqual( nonCubic.verticesPerCurve(), IECore.IntVectorData( [ 4 ] ) )
		self.assertEqual( nonCubic.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( nonCubic.periodic(), False )
		self.assertEqual(
			cubic["P"].data,
			IECore.V3fVectorData(
				[ imath.V3f( x ) for x in range( 0, 4 ) ],
				IECore.GeometricData.Interpretation.Point
			)
		)

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

	def testNormalsInterpolation( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "normalsInterpolation.usda" )
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 10 ) )

		for interpolation in [
			IECoreScene.PrimitiveVariable.Interpolation.Uniform,
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECoreScene.PrimitiveVariable.Interpolation.FaceVarying,
		] :

			resampledMesh = mesh.copy()
			n = resampledMesh["N"]
			IECoreScene.MeshAlgo.resamplePrimitiveVariable( resampledMesh, n, interpolation )
			resampledMesh["N"] = n
			self.assertEqual( resampledMesh["N"].interpolation, interpolation )

			root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
			root.createChild( "test" ).writeObject( resampledMesh, 0 )
			del root

			root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
			loadedMesh = root.child( "test" ).readObject( 0 )
			del root

			self.assertEqual( loadedMesh, resampledMesh )

	def testWriteUnsupportedObject( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( self.temporaryDirectory(), "unsupportedObject.usda" ),
			IECore.IndexedIO.OpenMode.Write
		)
		child = root.createChild( "test" )

		with IECore.CapturingMessageHandler() as mh :
			self.assertFalse( child.writeObject( IECore.CompoundObject(), 0 ) )

		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Warning )
		self.assertEqual( mh.messages[0].context, "USDScene::writeObject" )
		self.assertEqual( mh.messages[0].message, 'Unable to write CompoundObject to "/test" at time 0' )

	def testWriteUnsupportedProjection( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( self.temporaryDirectory(), "unsupportedProjection.usda" ),
			IECore.IndexedIO.OpenMode.Write
		)
		child = root.createChild( "test" )

		camera = IECoreScene.Camera()
		camera.setProjection( "fisheye" )

		with IECore.CapturingMessageHandler() as mh :
			self.assertFalse( child.writeObject( camera, 1 ) )

		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Warning )
		self.assertEqual( mh.messages[0].context, "IECoreUSD::CameraAlgo" )
		self.assertEqual( mh.messages[0].message, 'Unsupported projection "fisheye" writing "/test" at time 1' )

	def testVisibility( self ) :

		# Write via Cortex API

		fileName = os.path.join( self.temporaryDirectory(), "visibility.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		outerGroup = root.createChild( "outerGroup" )
		innerGroup = outerGroup.createChild( "innerGroup" )
		mesh = innerGroup.createChild( "mesh" )

		innerGroup.writeAttribute( root.visibilityName, IECore.BoolData( True ), 0.0 )
		mesh.writeAttribute( root.visibilityName, IECore.BoolData( False ), 0.0 )
		mesh.writeObject( IECoreScene.MeshPrimitive.createSphere( 1 ), 0.0 )

		del root, outerGroup, innerGroup, mesh

		# Check reading via USD API. We want to have converted to the USD visibility
		# attribute, not just done a generic conversion.

		stage = pxr.Usd.Stage.Open( fileName )

		outerGroup = pxr.UsdGeom.Imageable( stage.GetPrimAtPath( "/outerGroup" ) )
		self.assertFalse( outerGroup.GetVisibilityAttr().HasAuthoredValue() )

		innerGroup = pxr.UsdGeom.Imageable( stage.GetPrimAtPath( "/outerGroup/innerGroup" ) )
		self.assertTrue( innerGroup.GetVisibilityAttr().HasAuthoredValue() )
		self.assertEqual( innerGroup.GetVisibilityAttr().Get( 0.0 ), "inherited" )

		mesh = pxr.UsdGeom.Imageable( stage.GetPrimAtPath( "/outerGroup/innerGroup/mesh" ) )
		self.assertTrue( mesh.GetVisibilityAttr().HasAuthoredValue() )
		self.assertEqual( mesh.GetVisibilityAttr().Get( 0.0 ), "invisible" )

		# Check reading via Cortex API

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		outerGroup = root.child( "outerGroup" )
		innerGroup = outerGroup.child( "innerGroup" )
		mesh = innerGroup.child( "mesh" )

		self.assertFalse( root.hasAttribute( root.visibilityName ) )
		self.assertEqual( root.attributeNames(), [] )
		self.assertIsNone( root.readAttribute( root.visibilityName, 0.0 ) )

		self.assertFalse( outerGroup.hasAttribute( root.visibilityName ) )
		self.assertEqual( outerGroup.attributeNames(), [] )
		self.assertIsNone( outerGroup.readAttribute( root.visibilityName, 0.0 ) )

		self.assertTrue( innerGroup.hasAttribute( root.visibilityName ) )
		self.assertEqual( innerGroup.attributeNames(), [ root.visibilityName ] )
		self.assertEqual( innerGroup.readAttribute( root.visibilityName, 0.0 ), IECore.BoolData( True ) )

		self.assertTrue( mesh.hasAttribute( root.visibilityName ) )
		self.assertEqual( mesh.attributeNames(), [ root.visibilityName ] )
		self.assertEqual( mesh.readAttribute( root.visibilityName, 0.0 ), IECore.BoolData( False ) )

		self.assertEqual(
			root.hash( mesh.HashType.AttributesHash, 0.0 ),
			outerGroup.hash( mesh.HashType.AttributesHash, 0.0 )
		)

		self.assertNotEqual(
			innerGroup.hash( mesh.HashType.AttributesHash, 0.0 ),
			mesh.hash( mesh.HashType.AttributesHash, 0.0 )
		)

		self.assertEqual(
			mesh.hash( mesh.HashType.AttributesHash, 0.0 ),
			mesh.hash( mesh.HashType.AttributesHash, 1.0 )
		)

	def testAnimatedVisibility( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "animatedVisibility.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		mesh = root.createChild( "mesh" )
		mesh.writeAttribute( root.visibilityName, IECore.BoolData( False ), 0.0 )
		mesh.writeAttribute( root.visibilityName, IECore.BoolData( True ), 1.0 )
		del root, mesh

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		mesh = root.child( "mesh" )

		self.assertEqual( mesh.readAttribute( root.visibilityName, 0.0 ), IECore.BoolData( False ) )
		self.assertEqual( mesh.readAttribute( root.visibilityName, 1.0 ), IECore.BoolData( True ) )

		self.assertNotEqual(
			mesh.hash( mesh.HashType.AttributesHash, 0.0 ),
			mesh.hash( mesh.HashType.AttributesHash, 1.0 )
		)

	def testUntypedPrims( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "untypedParentPrim.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		self.assertEqual( root.childNames(), [ "untyped" ] )
		self.assertTrue( root.hasChild( "untyped" ) )
		self.assertFalse( root.hasChild( "undefined" ) )
		self.assertEqual(
			root.child( "untyped" ).child( "sphere" ).readObject( 0 ),
			IECoreScene.SpherePrimitive( 1 )
		)
		self.assertEqual(
			root.scene( [ "untyped", "sphere" ] ).readObject( 0 ),
			IECoreScene.SpherePrimitive( 1 )
		)

		self.assertIsNone( root.child( "undefined", root.MissingBehaviour.NullIfMissing ) )
		self.assertIsNone( root.scene( [ "undefined" ], root.MissingBehaviour.NullIfMissing ) )
		self.assertIsNone( root.scene( [ "undefined", "sphere" ], root.MissingBehaviour.NullIfMissing ) )

	def testScope( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "scope.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		self.assertEqual( root.childNames(), [ "scope" ] )
		scope = root.child( "scope" )
		self.assertEqual( scope.attributeNames(), [] )
		self.assertEqual( scope.readTransformAsMatrix( 0 ), imath.M44d() )
		self.assertFalse( scope.hasObject() )

		sphere = scope.child( "sphere" )
		self.assertEqual( sphere.readObject( 0.0 ), IECoreScene.SpherePrimitive( 1 ) )

	def testRenderSettings( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "renderSettings.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		self.assertEqual( root.childNames(), [] )
		self.assertIsNone( root.child( "renderSettings", root.MissingBehaviour.NullIfMissing ) )
		self.assertIsNone( root.scene( [ "renderSettings" ], root.MissingBehaviour.NullIfMissing ) )

	def testDisplayColor( self ) :

		# Create Cortex meshes with colours. Our convention for this is a primitive variable
		# called "Cs".

		constantPlane = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		constantPlane["Cs"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Constant,
			IECore.Color3fData( imath.Color3f( 1, 2, 3 ) )
		)

		uniformPlane = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		uniformPlane["Cs"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Uniform,
			IECore.Color3fVectorData( [ imath.Color3f( 1, 2, 3 ) ] )
		)

		vertexPlane = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		vertexPlane["Cs"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.Color3fVectorData(
				[ imath.Color3f( p.x + 1, p.y + 1, p.z + 1 ) for p in vertexPlane["P"].data ]
			)
		)

		# Write to USD.

		fileName = os.path.join( self.temporaryDirectory(), "displayColor.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		root.createChild( "constant" ).writeObject( constantPlane, 0 )
		root.createChild( "uniform" ).writeObject( uniformPlane, 0 )
		root.createChild( "vertex" ).writeObject( vertexPlane, 0 )
		del root

		# Read via USD API, and verify that "Cs" has been written to "displayColor",
		# which is the USD convention.

		stage = pxr.Usd.Stage.Open( fileName )

		usdConstant = pxr.UsdGeom.Mesh( stage.GetPrimAtPath( "/constant" ) ).GetDisplayColorPrimvar()
		self.assertEqual( usdConstant.GetInterpolation(), "constant" )
		self.assertEqual( usdConstant.Get( 0 ), pxr.Vt.Vec3fArray( [ pxr.Gf.Vec3f( 1, 2, 3 ) ] ) )

		usdUniform = pxr.UsdGeom.Mesh( stage.GetPrimAtPath( "/uniform" ) ).GetDisplayColorPrimvar()
		self.assertEqual( usdUniform.GetInterpolation(), "uniform" )
		self.assertEqual( usdUniform.Get( 0 ), pxr.Vt.Vec3fArray( [ pxr.Gf.Vec3f( 1, 2, 3 ) ] ) )

		usdVertex = pxr.UsdGeom.Mesh( stage.GetPrimAtPath( "/vertex" ) ).GetDisplayColorPrimvar()
		self.assertEqual( usdVertex.GetInterpolation(), "vertex" )
		self.assertEqual(
			usdVertex.Get( 0 ),
			pxr.Vt.Vec3fArray(
				[ pxr.Gf.Vec3f( c.x, c.y, c.z ) for c in vertexPlane["Cs"].data ]
			)
		)

		# Read via SceneInterface, and check that we've round-tripped successfully.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		self.assertEqual(
			root.child( "constant" ).readObject( 0 ),
			constantPlane
		)
		self.assertEqual(
			root.child( "uniform" ).readObject( 0 ),
			uniformPlane
		)
		self.assertEqual(
			root.child( "vertex" ).readObject( 0 ),
			vertexPlane
		)

	def testCanReferenceTags( self ) :

		# Write "tags.usda" via SceneInterface.

		tagsFileName = os.path.join( self.temporaryDirectory(), "tags.usda" )
		root = IECoreScene.SceneInterface.create( tagsFileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "child" )
		child.writeTags( [ "tagA", "tagB" ] )
		grandChild = child.createChild( "grandChild" )
		grandChild.writeTags( [ "tagB", "tagC" ] )
		del root, child, grandChild

		# Write "test.usda" via USD API, referencing "tags.usda".

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = pxr.Usd.Stage.CreateNew( fileName )
		root.OverridePrim( "/child" ).GetReferences().AddReference( tagsFileName, "/child" )
		root.GetRootLayer().Save()
		del root

		# Load "test.usda" via SceneInterface, and check that we can read the tags.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		child = root.child( "child" )
		self.assertSetNamesEqual( child.readTags(), [ "tagA", "tagB" ] )
		grandChild = child.child( "grandChild" )
		self.assertSetNamesEqual( grandChild.readTags(), [ "tagB", "tagC" ] )

	def __expectedLightSets( self ) :

		if pxr.Usd.GetVersion() >= ( 0, 21, 11 ) :
			return [ "__lights" ]
		else :
			return []

	def testTagSetEquivalence( self ) :

		# Location      Tags               Sets
		#
		# /a            tagOne             setOne : { /a, /a/b }
		#   /b          tagTwo
		# /c                               setTwo : { /c/d }
		#   /d          tagOne, tagThree

		fileName = os.path.join( self.temporaryDirectory(), "tagsAndSets.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		a = root.createChild( "a" )
		b = a.createChild( "b" )
		c = root.createChild( "c" )
		d = c.createChild( "d" )

		a.writeTags( [ "tagOne" ] )
		a.writeSet( "setOne", IECore.PathMatcher( [ "/", "/b" ] ) )
		b.writeTags( [ "tagTwo" ] )
		c.writeSet( "setTwo", IECore.PathMatcher( [ "/d" ] ) )
		d.writeTags( [ "tagOne", "tagThree" ] )

		del root, a, b, c, d

		# Read tags as sets

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual(
			root.readSet( "tagOne" ),
			IECore.PathMatcher( [
				"/a", "/c/d"
			] )
		)

		self.assertEqual(
			root.readSet( "tagTwo" ),
			IECore.PathMatcher( [
				"/a/b"
			] )
		)

		self.assertEqual(
			root.readSet( "tagThree" ),
			IECore.PathMatcher( [
				"/c/d"
			] )
		)

		# Read sets as tags

		allTags = [ "tagOne", "tagTwo", "tagThree", "setOne", "setTwo" ]

		def checkHasTag( scene ) :

			for f in ( scene.AncestorTag, scene.LocalTag, scene.DescendantTag ) :
				sceneTags = scene.readTags( f )
				for t in allTags :
					self.assertEqual(
						scene.hasTag( t, f ),
						t in sceneTags
					)

		self.assertSetNamesEqual( root.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( root.readTags( root.LocalTag ), [] )
		self.assertSetNamesEqual( root.readTags( root.DescendantTag ), allTags + [ "__cameras", "usd:pointInstancers" ] + self.__expectedLightSets() )
		checkHasTag( root )

		a = root.child( "a" )
		self.assertSetNamesEqual( a.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( a.readTags( root.LocalTag ), [ "tagOne", "setOne" ] )
		self.assertSetNamesEqual( a.readTags( root.DescendantTag ), [ "setOne", "tagTwo" ] )
		checkHasTag( a )

		b = a.child( "b" )
		self.assertSetNamesEqual( b.readTags( root.AncestorTag ), [ "tagOne", "setOne" ] )
		self.assertSetNamesEqual( b.readTags( root.LocalTag ), [ "setOne", "tagTwo" ] )
		self.assertSetNamesEqual( b.readTags( root.DescendantTag ), [] )
		checkHasTag( b )

		c = root.child( "c" )
		self.assertSetNamesEqual( c.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( c.readTags( root.LocalTag ), [] )
		self.assertSetNamesEqual( c.readTags( root.DescendantTag ), [ "setTwo", "tagOne", "tagThree" ] )
		checkHasTag( c )

		d = c.child( "d" )
		self.assertSetNamesEqual( d.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( d.readTags( root.LocalTag ), [ "setTwo", "tagOne", "tagThree" ] )
		self.assertSetNamesEqual( d.readTags( root.DescendantTag ), [] )
		checkHasTag( d )

	def testSchemaTypeSetsAndTags( self ) :

		# Write file containing camera via USD API.

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )
		pxr.UsdGeom.Xform.Define( stage, "/group" )
		pxr.UsdGeom.Camera.Define( stage, "/group/camera" )
		pxr.UsdGeom.Xform.Define( stage, "/group/instancerGroup" )
		pxr.UsdGeom.PointInstancer.Define( stage, "/group/instancerGroup/instancer" )

		stage.GetRootLayer().Save()

		# Check that we can load sets that identify the camera and point instancer.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		group = root.child( "group" )
		camera = group.child( "camera" )
		instancerGroup = group.child( "instancerGroup" )
		instancer = instancerGroup.child( "instancer" )

		self.assertSetNamesEqual( root.setNames(), [ "__cameras", "usd:pointInstancers" ] + self.__expectedLightSets() )
		self.assertSetNamesEqual( group.setNames(), [] )
		self.assertSetNamesEqual( camera.setNames(), [] )
		self.assertSetNamesEqual( instancerGroup.setNames(), [] )
		self.assertSetNamesEqual( instancer.setNames(), [] )

		self.assertEqual( root.readSet( "__cameras" ), IECore.PathMatcher( [ "/group/camera" ] ) )
		self.assertEqual( group.readSet( "__cameras" ), IECore.PathMatcher() )
		self.assertEqual( camera.readSet( "__cameras" ), IECore.PathMatcher() )
		self.assertEqual( instancerGroup.readSet( "__cameras" ), IECore.PathMatcher() )
		self.assertEqual( instancer.readSet( "__cameras" ), IECore.PathMatcher() )

		self.assertEqual( root.readSet( "usd:pointInstancers" ), IECore.PathMatcher( [ "/group/instancerGroup/instancer" ] ) )
		self.assertEqual( group.readSet( "usd:pointInstancers" ), IECore.PathMatcher() )
		self.assertEqual( camera.readSet( "usd:pointInstancers" ), IECore.PathMatcher() )
		self.assertEqual( instancerGroup.readSet( "usd:pointInstancers" ), IECore.PathMatcher() )
		self.assertEqual( instancer.readSet( "usd:pointInstancers" ), IECore.PathMatcher() )

		# Check that we can load tags that identify the camera and point instancer.

		self.assertSetNamesEqual( root.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( root.readTags( root.LocalTag ), [] )
		self.assertSetNamesEqual( root.readTags( root.DescendantTag ), [ "__cameras", "usd:pointInstancers" ] + self.__expectedLightSets() )

		self.assertSetNamesEqual( group.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( group.readTags( root.LocalTag ), [] )
		self.assertSetNamesEqual( group.readTags( root.DescendantTag ), [ "__cameras", "usd:pointInstancers" ] )

		self.assertSetNamesEqual( camera.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( camera.readTags( root.LocalTag ), [  "__cameras" ] )
		self.assertSetNamesEqual( camera.readTags( root.DescendantTag ), [] )

		self.assertSetNamesEqual( instancerGroup.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( instancerGroup.readTags( root.LocalTag ), [] )
		self.assertSetNamesEqual( instancerGroup.readTags( root.DescendantTag ), [ "usd:pointInstancers" ] )

		self.assertSetNamesEqual( instancer.readTags( root.AncestorTag ), [] )
		self.assertSetNamesEqual( instancer.readTags( root.LocalTag ), [ "usd:pointInstancers" ] )
		self.assertSetNamesEqual( instancer.readTags( root.DescendantTag ), [] )

	def testNonSelfContainedCollection( self ) :

		scene = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "nonSelfContainedCollection.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		collections = scene.child( "collections" )

		with IECore.CapturingMessageHandler() as mh :
			self.assertEqual( collections.readSet( "elsewhere" ), IECore.PathMatcher() )

		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Warning )
		self.assertEqual( mh.messages[0].context, "USDScene" )
		self.assertEqual( mh.messages[0].message, 'Ignoring path "/sphere1" in collection "elsewhere" because it is not beneath the collection root "/collections"' )

	def testMeshOrientation( self ) :

		# Contains cube with 24 distinct vertices (no sharing), and hard vertex
		# normals, in left-handed orientation. The vertex normals are pointed in
		# the same direction as the geometric normals as long as orientation is
		# considered.

		scene = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "leftHandedCube.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		# Cortex requires meshes to be in right-handed orientation, so winding
		# must be reversed during loading. We can test that this has worked by
		# recalculating the vertex normals and checking they match the
		# originals.

		cube = scene.child( "cube" ).readObject( 0 )
		self.assertEqual(
			IECoreScene.MeshAlgo.calculateNormals( cube ),
			cube["N"]
		)

	def testPurposeAttribute( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		purposes = ( None, "default", "render", "proxy", "guide" )

		# Use SceneInterface to write values for purpose.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		for purpose in purposes :
			child = root.createChild( purpose or "none" )
			if purpose is not None :
				child.writeAttribute( "usd:purpose", IECore.StringData( purpose ), 0 )
			del child
		del root

		# Verify by reading via USD API.

		stage = pxr.Usd.Stage.Open( fileName )
		for purpose in purposes :
			child = pxr.UsdGeom.Xform( stage.GetPrimAtPath( "/{}".format( purpose or "none" ) ) )
			if purpose is None :
				self.assertFalse( child.GetPurposeAttr().HasAuthoredValue() )
			else :
				self.assertEqual( child.GetPurposeAttr().Get(), purpose )

		# Verify by reading via SceneInterface.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		for purpose in purposes :
			child = root.child( purpose or "none" )
			if purpose is not None :
				self.assertTrue( child.hasAttribute( "usd:purpose" ) )
				self.assertIn( "usd:purpose", child.attributeNames() )
				self.assertIsInstance( child.readAttribute( "usd:purpose", 0 ), IECore.StringData )
				self.assertEqual( child.readAttribute( "usd:purpose", 0 ).value, purpose )
			else :
				self.assertFalse( child.hasAttribute( "usd:purpose" ) )
				self.assertNotIn( "usd:purpose", child.attributeNames() )
				self.assertEqual( child.readAttribute( "usd:purpose", 0 ), None )

	def testNonImageablePurpose( self ) :

		scene = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "nonImageablePurpose.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		for path, purpose in {
			"/Root" : None,
			"/Root/RenderXform" : "render",
			"/Root/RenderXform/Prim" : None,
			"/Root/RenderXform/Prim/InheritXform" : None,
			"/Root/RenderXform/Prim/GuideXform" : "guide",
			"/Root/Xform" : None,

		}.items() :

			location = scene.scene( scene.stringToPath( path ) )
			if purpose is not None :
				self.assertTrue( location.hasAttribute( "usd:purpose" ) )
				self.assertIn( "usd:purpose", location.attributeNames() )
				self.assertIsInstance( location.readAttribute( "usd:purpose", 0 ), IECore.StringData )
				self.assertEqual( location.readAttribute( "usd:purpose", 0 ).value, purpose )
			else :
				self.assertFalse( location.hasAttribute( "usd:purpose" ) )
				self.assertNotIn( "usd:purpose", location.attributeNames() )
				self.assertEqual( location.readAttribute( "usd:purpose", 0 ), None )

	def testKind( self ) :

		# Test read, including non-xform prims

		scene = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "kind.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		for path, kind in {
			"/group" : "group",
			"/group/model" : "model",
			"/group/model/assembly" : "assembly",
			"/group/model/assembly/spheres" : None,
			"/group/model/assembly/spheres/sphere1" : None,
			"/group/model/component" : "component",
			"/group/model/component/sphere" : None,
			"/group/model/component/subcomponent" : "subcomponent",
			"/group/model/component/subcomponent/sphere1" : None

		}.items() :

			location = scene.scene( scene.stringToPath( path ) )
			if kind is not None :
				self.assertTrue( location.hasAttribute( "usd:kind" ) )
				self.assertIn( "usd:kind", location.attributeNames() )
				self.assertIsInstance( location.readAttribute( "usd:kind", 0 ), IECore.StringData )
				self.assertEqual( location.readAttribute( "usd:kind", 0 ).value, kind )
			else :
				self.assertFalse( location.hasAttribute( "usd:kind" ) )
				self.assertNotIn( "usd:kind", location.attributeNames() )
				self.assertEqual( location.readAttribute( "usd:kind", 0 ), None )

		# Test write via SceneInterface

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		kinds = ( None, "model", "assembly", "component", "subcomponent" )

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		for kind in kinds :
			child = root.createChild( kind or "none" )
			if kind is not None :
				child.writeAttribute( "usd:kind", IECore.StringData( kind ), 0 )
			del child
		del root

		# Verify by reading via USD API.

		stage = pxr.Usd.Stage.Open( fileName )
		for kind in kinds :
			child = pxr.Usd.ModelAPI( stage.GetPrimAtPath( "/{}".format( kind or "none" ) ) )
			self.assertEqual( pxr.Usd.ModelAPI( child ).GetKind(), kind if kind is not None else '' )

	def testSkelSkinning( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/arm.usda", IECore.IndexedIO.OpenMode.Read )
		arm = root.scene( [ "Model", "Arm" ] )
		self.assertTrue( arm.hasObject() )

		# arm is animated from frames 1-10
		arm_1 = arm.readObject( 1.0 / 24 )
		self.assertEqual( arm_1, arm.readObject( 0 ) )
		arm_5 = arm.readObject( 5.0 / 24 )
		self.assertNotEqual( arm_1, arm_5 )
		arm_10 = arm.readObject( 10.0 / 24 )
		self.assertNotEqual( arm_5, arm_10 )
		self.assertEqual( arm_10, arm.readObject( 11.0 / 24 ) )

		self.assertTrue( arm_1.arePrimitiveVariablesValid() )
		self.assertTrue( arm_5.arePrimitiveVariablesValid() )
		self.assertTrue( arm_10.arePrimitiveVariablesValid() )

		self.assertEqual( set( arm_1.keys() ), { "P" } )
		self.assertEqual( set( arm_5.keys() ), { "P" } )
		self.assertEqual( set( arm_10.keys() ), { "P" } )

		self.assertEqual( arm_1.topologyHash(), arm_5.topologyHash() )
		self.assertEqual( arm_1.topologyHash(), arm_10.topologyHash() )

		self.assertEqual(
			arm_1["P"].data,
			IECore.V3fVectorData(
				[
					imath.V3f(0.5, -0.5, 4), imath.V3f(-0.5, -0.5, 4), imath.V3f(0.5, 0.5, 4), imath.V3f(-0.5, 0.5, 4),
					imath.V3f(-0.5, -0.5, 0), imath.V3f(0.5, -0.5, 0), imath.V3f(-0.5, 0.5, 0), imath.V3f(0.5, 0.5, 0),
					imath.V3f(-0.5, 0.5, 2), imath.V3f(0.5, 0.5, 2), imath.V3f(0.5, -0.5, 2), imath.V3f(-0.5, -0.5, 2)
				],
				IECore.GeometricData.Interpretation.Point
			)
		)

		expected_5 = [
			imath.V3f( 0.5, -1.66859, 3.210705 ), imath.V3f( -0.5, -1.66859, 3.210705 ), imath.V3f( 0.5, -0.90254, 3.85349 ), imath.V3f( -0.5, -0.90254, 3.85349 ),
			imath.V3f( -0.5, -0.5, 0 ), imath.V3f( 0.5, -0.5, 0 ), imath.V3f( -0.5, 0.5, 0 ), imath.V3f( 0.5, 0.5, 0 ),
			imath.V3f( -0.5, 0.38302, 2.32139 ), imath.V3f( 0.5, 0.38302, 2.32139 ), imath.V3f( 0.5, -0.38302, 1.67861 ), imath.V3f( -0.5, -0.38302, 1.67861 )
		]
		self.assertEqual( arm_5["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		for i in range( 0, arm_5.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) ) :
			self.assertAlmostEqual( arm_5["P"].data[i].x, expected_5[i].x, 5 )
			self.assertAlmostEqual( arm_5["P"].data[i].y, expected_5[i].y, 5 )
			self.assertAlmostEqual( arm_5["P"].data[i].z, expected_5[i].z, 5 )

		expected_10 = [
			imath.V3f( 0.5, -1.99997, 1.50005 ), imath.V3f( -0.5, -1.99997, 1.50005 ), imath.V3f( 0.5, -1.99995, 2.50003 ), imath.V3f( -0.5, -1.99995, 2.50003 ),
			imath.V3f( -0.5, -0.5, 0 ), imath.V3f( 0.5, -0.5, 0 ), imath.V3f( -0.5, 0.5, 0 ), imath.V3f( 0.5, 0.5, 0 ),
			imath.V3f( -0.5, 0.00001, 2.49999 ), imath.V3f( 0.5, 0.00001, 2.49999 ), imath.V3f( 0.5, -0.00001, 1.50001 ), imath.V3f( -0.5, -0.00001, 1.50001 )
		]
		self.assertEqual( arm_10["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		for i in range( 0, arm_10.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) ) :
			self.assertAlmostEqual( arm_10["P"].data[i].x, expected_10[i].x, 5 )
			self.assertAlmostEqual( arm_10["P"].data[i].y, expected_10[i].y, 5 )
			self.assertAlmostEqual( arm_10["P"].data[i].z, expected_10[i].z, 5 )

	def testSkelBlendShapes( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/arm.blendshapes.usda", IECore.IndexedIO.OpenMode.Read )
		arm = root.scene( [ "Model", "Arm" ] )
		self.assertTrue( arm.hasObject() )

		# arm is animated from frames 1-10
		arm_1 = arm.readObject( 1.0 / 24 )
		self.assertEqual( arm_1, arm.readObject( 0 ) )
		arm_5 = arm.readObject( 5.0 / 24 )
		self.assertNotEqual( arm_1, arm_5 )
		arm_10 = arm.readObject( 10.0 / 24 )
		self.assertNotEqual( arm_5, arm_10 )
		self.assertEqual( arm_10, arm.readObject( 11.0 / 24 ) )

		self.assertTrue( arm_1.arePrimitiveVariablesValid() )
		self.assertTrue( arm_5.arePrimitiveVariablesValid() )
		self.assertTrue( arm_10.arePrimitiveVariablesValid() )

		self.assertEqual( set( arm_1.keys() ), { "P" } )
		self.assertEqual( set( arm_5.keys() ), { "P" } )
		self.assertEqual( set( arm_10.keys() ), { "P" } )

		self.assertEqual( arm_1.topologyHash(), arm_5.topologyHash() )
		self.assertEqual( arm_1.topologyHash(), arm_10.topologyHash() )

		self.assertEqual(
			arm_1["P"].data,
			IECore.V3fVectorData(
				[
					imath.V3f(0.5, -0.5, 4), imath.V3f(-0.5, -0.5, 4), imath.V3f(0.5, 0.5, 4), imath.V3f(-0.5, 0.5, 4),
					imath.V3f(-0.5, -0.5, 0), imath.V3f(0.5, -0.5, 0), imath.V3f(-0.5, 0.5, 0), imath.V3f(0.5, 0.5, 0),
					imath.V3f(-0.5, 0.5, 2), imath.V3f(0.5, 0.5, 2), imath.V3f(0.5, -0.5, 2), imath.V3f(-0.5, -0.5, 2)
				],
				IECore.GeometricData.Interpretation.Point
			)
		)

		expected_5 = [
			imath.V3f( 0.5, -1.66859, 3.210705 ), imath.V3f( -0.5, -1.66859, 3.210705 ), imath.V3f( 0.5, -0.90254, 3.85349 ), imath.V3f( -0.5, -0.90254, 3.85349 ),
			imath.V3f( -0.55, -0.625, 0 ), imath.V3f( 0.55, -0.625, 0 ), imath.V3f( -0.5, 0.5, 0 ), imath.V3f( 0.5, 0.5, 0 ),
			imath.V3f( -0.5, 0.38302, 2.32139 ), imath.V3f( 0.5, 0.38302, 2.32139 ), imath.V3f( 0.55, -0.719823, 1.88553 ), imath.V3f( -0.55, -0.719823, 1.88553 )
		]
		self.assertEqual( arm_5["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		for i in range( 0, arm_5.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) ) :
			self.assertAlmostEqual( arm_5["P"].data[i].x, expected_5[i].x, 5 )
			self.assertAlmostEqual( arm_5["P"].data[i].y, expected_5[i].y, 5 )
			self.assertAlmostEqual( arm_5["P"].data[i].z, expected_5[i].z, 5 )

		expected_10 = [
			imath.V3f( 0.5, -1.99997, 1.50005 ), imath.V3f( -0.5, -1.99997, 1.50005 ), imath.V3f( 0.5, -1.99995, 2.50003 ), imath.V3f( -0.5, -1.99995, 2.50003 ),
			imath.V3f( -0.6, -0.75, 0 ), imath.V3f( 0.6, -0.75, 0 ), imath.V3f( -0.5, 0.5, 0 ), imath.V3f( 0.5, 0.5, 0 ),
			imath.V3f( -0.5, 0.00001, 2.49999 ), imath.V3f( 0.5, 0.00001, 2.49999 ), imath.V3f( 0.6, -0.75, 1.25003 ), imath.V3f( -0.6, -0.75, 1.25003 )
		]
		self.assertEqual( arm_10["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		for i in range( 0, arm_10.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) ) :
			self.assertAlmostEqual( arm_10["P"].data[i].x, expected_10[i].x, 5 )
			self.assertAlmostEqual( arm_10["P"].data[i].y, expected_10[i].y, 5 )
			self.assertAlmostEqual( arm_10["P"].data[i].z, expected_10[i].z, 5 )

	def testSkinnedFaceVaryingNormals( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/skinnedFaceVaryingNormals.usda", IECore.IndexedIO.OpenMode.Read )
		cubeLocation = root.scene( [ "main", "pCube1" ] )
		for timeSample in range( 1, 25 ) :

			cubeMesh = cubeLocation.readObject( timeSample / 24.0 )
			self.assertIn( "N", cubeMesh )
			self.assertTrue( cubeMesh.isPrimitiveVariableValid( cubeMesh["N"] ) )
			self.assertEqual( cubeMesh["N"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

			referenceNormals = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( cubeMesh, thresholdAngle = 5 )
			for referenceNormal, normal in zip( referenceNormals.data, cubeMesh["N"].data ) :
				self.assertTrue( normal.equalWithAbsError( referenceNormal, 0.000001 ) )

	@unittest.skipIf( ( IECore.TestUtil.inMacCI() or IECore.TestUtil.inWindowsCI() ), "Mac and Windows CI are too slow for reliable timing" )
	def testCancel ( self ) :

		strip = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 100000, 1 ) ), imath.V2i( 100000, 1 ) )
		for i in range( 30 ):
			strip["var%i" % i] = strip["P"]

		fileName = os.path.join( self.temporaryDirectory(), "cancelTestTemp.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "sphere" )
		child.writeObject( strip, 0 )

		del root, child

		readRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		canceller = IECore.Canceller()
		cancelled = [False]

		def backgroundRun():
			try:
				readRoot.child( "sphere" ).readObject( 0.0, canceller )
			except IECore.Cancelled:
				cancelled[0] = True

		thread = threading.Thread(target=backgroundRun, args=())

		startTime = time.time()
		thread.start()

		time.sleep( 0.01 )
		canceller.cancel()
		thread.join()

		self.assertLess( time.time() - startTime, 0.03 )
		self.assertTrue( cancelled[0] )

	def testCustomAttributes( self ) :

		# Load a file with a variety of attributes, and check we only load the
		# ones we want.

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/customAttribute.usda", IECore.IndexedIO.OpenMode.Read )
		sphere = root.child( "sphere" )

		self.assertNotEqual( sphere.hash( sphere.HashType.AttributesHash, 0 ), sphere.hash( sphere.HashType.AttributesHash, 1 ) )

		def assertExpectedAttributes( sphere ) :
			# test expected attributes names
			self.assertEqual(
				set( sphere.attributeNames() ),
				{
					"user:notAConstantPrimVar", "user:notAConstantPrimVarDeprecated", "user:test",
					"studio:foo", "customNamespaced:testAnimated", "user:bongo", "render:test",
					"ai:disp_height", "ai:poly_mesh:subdiv_iterations"
				}
			)

			# test incompatible primvars hasAttribute/readAttribute
			for name in [
				"customNotNamespaced",
				"notNamespaced",
				"namespaced:test",
				"nonexistent",
				"radius",
				"test:noAuthoredValue",
			] :
				self.assertFalse( sphere.hasAttribute( name ) )
				self.assertIsNone( sphere.readAttribute( name, 0 ) )

		def assertAttributesValues( sphere ):
			# make sure no attributes are loaded as PrimitiveVariable, since this is a SpherePrimitive, it has no PrimitiveVariable
			# not even P which is normal.
			self.assertFalse( sphere.readObject( 0 ).keys() )
			self.assertEqual( sphere.readAttribute( "user:test", 0 ), IECore.StringData( "green" ) )
			self.assertEqual( sphere.readAttribute( "render:test", 0 ), IECore.StringData( "cyan" ) )
			self.assertEqual( sphere.readAttribute( "studio:foo", 0 ), IECore.StringData( "brown" ) )
			self.assertEqual( sphere.readAttribute( "user:bongo", 0 ), IECore.StringData( "cyan" ) )
			# Animation not yet supported
			self.assertEqual( sphere.readAttribute( "customNamespaced:testAnimated", 0 ), IECore.DoubleData( 0 ) )
			self.assertEqual( sphere.readAttribute( "user:notAConstantPrimVar", 0 ), IECore.StringData( "pink" ) )
			self.assertEqual( sphere.readAttribute( "user:notAConstantPrimVarDeprecated", 0 ), IECore.StringData( "pink" ) )
			self.assertEqual( sphere.readAttribute( "ai:disp_height", 0 ), IECore.FloatData( 0.5 ) )
			self.assertEqual( sphere.readAttribute( "ai:poly_mesh:subdiv_iterations", 0 ), IECore.IntData( 3 ) )

		assertExpectedAttributes( sphere )
		assertAttributesValues( sphere )

		a = root.child( "a" )
		self.assertEqual( a.attributeNames(), ["user:foo"] )
		for n in a.attributeNames():
			self.assertTrue( a.hasAttribute( n ) )
		self.assertFalse( a.hasAttribute( "primvars:displayColor" ) ) # Make sure unauthored primvars not loaded
		self.assertEqual( a.readAttribute( "user:foo", 0 ), IECore.StringData( "yellow" ) )

		b = a.child( "b" )
		self.assertEqual( set( b.attributeNames() ), { "render:notUserPrefixAttribute", "user:baz" } )
		for n in b.attributeNames():
			self.assertTrue( b.hasAttribute( n ) )
		# Make sure primvars and indices not loaded as attributes
		self.assertFalse( b.hasAttribute( "primvars:withIndices" ) )
		self.assertFalse( b.hasAttribute( "primvars:withIndices:indices" ) )
		self.assertFalse( b.hasAttribute( "render:bar" ) )
		self.assertFalse( b.hasAttribute( "render:barDeprecated" ) )
		self.assertFalse( b.hasAttribute( "user:foo" ) )
		self.assertTrue( b.hasAttribute( "user:baz" ) )
		self.assertEqual( b.readAttribute( "user:baz", 0 ), IECore.StringData( "white" ) )
		self.assertEqual( b.readAttribute( "render:notUserPrefixAttribute", 0 ), IECore.StringData( "orange" ) )

		# constant primvar non attribute
		bObj = b.readObject( 0 )
		self.assertTrue( bObj["bar"] )
		self.assertTrue( bObj["barDeprecated"] )
		# make sure no other PrimitiveVariables are creeping in
		# Having a primvar with indices here makes sure we don't read the indices themselves as a primvar or attribute
		self.assertEqual( bObj.keys(), ["bar", "barDeprecated", "withIndices"] )
		self.assertEqual( bObj["bar"].data, IECore.StringData( "black" ) )
		self.assertEqual( bObj["barDeprecated"].data, IECore.StringData( "black" ) )
		self.assertEqual(
			bObj["withIndices"],
			IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Vertex,
				IECore.FloatVectorData( [ 1, 2 ] ),
				IECore.IntVectorData( [ 0, 1, 0, 1 ] )
			)
		)

		# check primvars from USD API
		stage = pxr.Usd.Stage.Open(  os.path.dirname( __file__ ) + "/data/customAttribute.usda" )
		cPrim = stage.GetPrimAtPath( "/a/b/c" )
		primVarAPI = pxr.UsdGeom.PrimvarsAPI( cPrim )
		expectedValue = {
			"bar" : "black",
			"user:foo" : "yellow",
			"user:baz" : "white",
			"notUserPrefixAttribute" : "orange",
		}
		for primVarName in expectedValue.keys():
			primVar = primVarAPI.FindPrimvarWithInheritance( "primvars:{}".format( primVarName ) )
			self.assertTrue( primVar )
			self.assertEqual( primVar.Get( 0 ), expectedValue[primVarName] )
			self.assertEqual( bool( primVar.GetAttr().GetMetadata( "cortex_isConstantPrimitiveVariable" ) ), primVarName == "bar" )

		# Check that we can round-trip the supported attributes.

		fileName = os.path.join( self.temporaryDirectory(), "customAttribute.usda" )
		writerRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		writerSphere = writerRoot.createChild( "sphere" )
		writerSphere.writeObject( IECoreScene.SpherePrimitive( 10 ), 0  )
		writerAb = writerRoot.createChild( "ab" )
		writerAb.writeObject( bObj, 0  )
		for attribute in sphere.attributeNames() :
			writerSphere.writeAttribute( attribute, sphere.readAttribute( attribute, 0 ), 0 )
		for attribute in b.attributeNames() :
			writerAb.writeAttribute( attribute, b.readAttribute( attribute, 0 ), 0 )
		del writerRoot, writerSphere, writerAb

		# Check that the USD file written looks as expected
		stage = pxr.Usd.Stage.Open( fileName )
		sphereUsd = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/sphere" ) )
		self.assertEqual( sphereUsd.GetPrim().GetAttribute( "customNamespaced:testAnimated" ).Get( 0 ), 0 )
		self.assertEqual( sphereUsd.GetPrimvar( "test" ).Get( 0 ), "cyan" )
		self.assertEqual( sphereUsd.GetPrimvar( "user:bongo" ).Get( 0 ), "cyan" )
		self.assertEqual( sphereUsd.GetPrimvar( "user:notAConstantPrimVar" ).Get( 0 ), "pink" )
		self.assertEqual( sphereUsd.GetPrimvar( "user:test" ).Get( 0 ), "green" )
		self.assertEqual( sphereUsd.GetPrim().GetAttribute( "radius" ).Get( 0 ), 10 )
		self.assertEqual( sphereUsd.GetPrim().GetAttribute( "studio:foo" ).Get( 0 ), "brown" )

		abUsd = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/ab" ) )
		self.assertEqual( abUsd.GetPrimvar( "bar" ).Get( 0 ), "black" )
		self.assertEqual( abUsd.GetPrimvar( "bar" ).GetAttr().GetMetadata( "cortex_isConstantPrimitiveVariable" ), True )
		self.assertEqual( abUsd.GetPrimvar( "notUserPrefixAttribute" ).Get( 0 ), "orange" )
		self.assertEqual( abUsd.GetPrimvar( "user:baz" ).Get( 0 ), "white" )
		self.assertEqual( abUsd.GetPrim().GetAttribute( "radius" ).Get( 0 ), 1 )

		# Check that things read OK
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		assertExpectedAttributes( root.child( "sphere" ) )
		assertAttributesValues( sphere )

		readAb = root.child( "ab" )
		self.assertEqual( readAb.readObject( 0 ), bObj )
		self.assertEqual( readAb.readAttribute( "user:baz", 0 ), IECore.StringData( "white" ) )
		self.assertEqual( readAb.readAttribute( "render:notUserPrefixAttribute", 0 ), IECore.StringData( "orange" ) )

	def testAttributeBadPrefix( self ):
		# For Arnold attributes, we currently use a prefix of "ai:" in Cortex, but "arnold:" in USD.
		# Mixing these up can produce odd results.

		# Using a prefix of "ai:" in USD is completely broken, but shouldn't happen in the future.
		# The one place it could arise is when reading USD's written with an old Gaffer version.
		# To the best of our knowledge, USD has not yet been used heavily in Gaffer, so this
		# shouldn't be a big concern.  These old USDs will need to be re-exported
		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/attributeBadPrefix.usda", IECore.IndexedIO.OpenMode.Read )

		# If we have one of these old bad attributes, it will show up in attributeNames,
		# but not in hasAttribute or readAttribute.  We rely on Gaffer's SceneReader
		# to note that the attribute value is null, and prune the attribute while printing
		# a warning.  If we wanted to do better, we would have to add support for checking
		# in two places to find the source of the attribute, or map the ai prefix to some
		# other special prefix for deprecated ai attributes.  It seems like the warning in
		# Gaffer should be fine though.
		self.assertEqual( set( root.child( "loc" ).attributeNames() ), set( ['ai:foo' ] ) )
		self.assertEqual( root.child( "loc" ).hasAttribute( "ai:foo" ), False )
		self.assertEqual( root.child( "loc" ).hasAttribute( "arnold:foo" ), False )
		self.assertEqual( root.child( "loc" ).readAttribute( "ai:foo", 0 ), None )
		self.assertEqual( root.child( "loc" ).readAttribute( "arnold:foo", 0 ), None )

		# Using a prefix of "arnold:" in Cortex may become the standard in the future.  It currently is
		# basically OK, but comes back as "ai:" instead of round-tripping
		fileName = os.path.join( self.temporaryDirectory(), "arnoldPrefix.usda" )

		writerRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		loc = writerRoot.createChild( "loc" )
		loc.writeAttribute( "arnold:testAttribute", IECore.FloatData( 9 ), 0 )

		del writerRoot, loc

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set( root.child( "loc" ).attributeNames() ), set( ['ai:testAttribute' ] ) )
		self.assertEqual( root.child( "loc" ).readAttribute( 'ai:testAttribute', 0 ), IECore.FloatData( 9 ) )

	def testWriteAnimatedAttribute( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "child" )

		for t in ( 0.0, 0.5, 1.0 ) :
			child.writeAttribute( "user:test", IECore.FloatData( t ), t )

		del child, root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		child = root.child( "child" )

		for t in ( 0.0, 0.5, 1.0 ) :
			self.assertEqual( child.readAttribute( "user:test", t ), IECore.FloatData( t ) )

	def testWriteAnimatedBound( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "child" )

		for t in ( 1.0, 1.5, 2.0 ) :
			child.writeObject( IECoreScene.SpherePrimitive( t ), t )
			child.writeBound( imath.Box3d( imath.V3d( -t ), imath.V3d( t ) ), t )

		del child, root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		child = root.child( "child" )

		for t in ( 1.0, 1.5, 2.0 ) :
			self.assertEqual( child.readBound( t ), imath.Box3d( imath.V3d( -t ), imath.V3d( t ) ) )

	def testShaders( self ) :

		# Write shaders

		fileName = os.path.join( self.temporaryDirectory(), "shaders.usda" )

		# /todo : Note that if our output shader was not of type "surface", it would be
		# automatically labelled as a "surface" by the import from USD, so it would not technically
		# round trip correctly.  The difference between a "surface" and "shader" isn't really significant,
		# and is likely to go away in the future, so we're not currently worrying about this.
		surface = IECoreScene.Shader( "standardsurface", "ai:surface" )
		surface.parameters["a"] = IECore.FloatData( 42.0 )
		surface.parameters["b"] = IECore.IntData( 42 )
		surface.parameters["c"] = IECore.StringData( "42" )
		surface.parameters["d"] = IECore.Color3fData( imath.Color3f( 3 ) )
		surface.parameters["e"] = IECore.V3fVectorData( [ imath.V3f( 7 ) ] )
		surface.parameters["f"] = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.bSpline(),
			( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ) ) )
		)
		surface.parameters["g"] = IECore.SplinefColor3fData( IECore.SplinefColor3f( IECore.CubicBasisf.linear(),
			( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ), ( 20, imath.Color3f(0) ) ) )
		)

		add1 = IECoreScene.Shader( "add", "ai:shader" )
		add1.parameters["b"] = IECore.FloatData( 3.0 )

		add2 = IECoreScene.Shader( "add", "osl:shader" )
		add2.parameters["b"] = IECore.FloatData( 7.0 )

		texture = IECoreScene.Shader( "texture", "ai:shader" )
		texture.parameters["filename"] = IECore.StringData( "sometexture.tx" )
		texture.parameters["filename_colorspace"] = IECore.InternedStringData( "ACEScg" )

		oneShaderNetwork = IECoreScene.ShaderNetwork()
		oneShaderNetwork.addShader( "foo", surface )
		oneShaderNetwork.setOutput( IECoreScene.ShaderNetwork.Parameter( "foo", "" ) )

		# A network with no output can be written out, but not read back in, because
		# it will not have been connected to a material output.
		noOutputNetwork = IECoreScene.ShaderNetwork()
		noOutputNetwork.addShader( "foo", surface )

		# Test picking a specific output
		pickOutputNetwork = IECoreScene.ShaderNetwork()
		pickOutputNetwork.addShader( "foo", surface )
		pickOutputNetwork.setOutput( IECoreScene.ShaderNetwork.Parameter( "foo", "test" ) )

		# A more complicated example.  Try some different kinds of connections, and make sure that we only
		# output one shader when it's referenced multiple times
		complexNetwork = IECoreScene.ShaderNetwork()
		complexNetwork.addShader( "foo", surface )
		complexNetwork.addShader( "add1", add1 )
		complexNetwork.addShader( "add2", add2 )
		complexNetwork.addShader( "texture", texture )
		complexNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "add1", "" ),
			IECoreScene.ShaderNetwork.Parameter( "foo", "a" )
		) )
		complexNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "add2", "sum" ),
			IECoreScene.ShaderNetwork.Parameter( "foo", "new" )
		) )
		complexNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "texture", "" ),
			IECoreScene.ShaderNetwork.Parameter( "add1", "a" )
		) )
		complexNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "texture", "" ),
			IECoreScene.ShaderNetwork.Parameter( "add2", "a" )
		) )
		complexNetwork.setOutput( IECoreScene.ShaderNetwork.Parameter( "foo", "" ) )

		# Test component connections

		dest = IECoreScene.Shader( "dest", "ai:surface" )
		dest.parameters["a"] = IECore.Color3fData( imath.Color3f( 0.0 ) )
		dest.parameters["b"] = IECore.Color3fData( imath.Color3f( 0.0 ) )
		dest.parameters["c"] = IECore.FloatData( 0.0 )
		dest.parameters["sf"] = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.catmullRom(),
			( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 30, 1 ) )
		) )
		dest.parameters["sc"] = IECore.SplinefColor3fData( IECore.SplinefColor3f( IECore.CubicBasisf.linear(),
			( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ), ( 20, imath.Color3f(0) ) )
		) )

		componentConnectionNetwork = IECoreScene.ShaderNetwork()
		componentConnectionNetwork.addShader( "source1", add1 )
		componentConnectionNetwork.addShader( "source2", add1 )
		componentConnectionNetwork.addShader( "source3", add1 )
		componentConnectionNetwork.addShader( "dest", dest )

		# Float to color connection
		componentConnectionNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source1", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "a.r" )
		) )
		# Swizzled color connection
		componentConnectionNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source2", "out.r" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "b.g" )
		) )
		componentConnectionNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source2", "out.g" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "b.b" )
		) )
		componentConnectionNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source2", "out.b" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "b.r" )
		) )
		# Color to float connection
		componentConnectionNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source3", "out.r" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "c" )
		) )
		componentConnectionNetwork.setOutput( IECoreScene.ShaderNetwork.Parameter( "dest", "" ) )

		# Float to spline element connection
		componentConnectionNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source1", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "sf[3].y" )
		) )

		# Color to spline element connection
		componentConnectionNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source3", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "sc[2].y" )
		) )

		# Float to spline element component connection
		componentConnectionNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source1", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "sc[0].y.g" )
		) )

		# If we manually create the shaders that are used as adapters for component connections,
		# they should not be automatically removed on import.  ( This is implemented using
		# a label for automatically created adapters, stored as blindData in Cortex that is
		# translated to metadata in USD )
		manualSwizzle = IECoreScene.Shader( "MaterialX/mx_swizzle_color_float", "osl:shader" )
		manualPack = IECoreScene.Shader( "MaterialX/mx_pack_color", "osl:shader" )

		manualComponentNetwork = IECoreScene.ShaderNetwork()
		manualComponentNetwork.addShader( "swizzle", manualSwizzle )
		manualComponentNetwork.addShader( "pack", manualPack )
		manualComponentNetwork.addShader( "dest", dest )
		manualComponentNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "pack", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "a" )
		) )
		manualComponentNetwork.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "swizzle", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "c" )
		) )
		manualComponentNetwork.setOutput( IECoreScene.ShaderNetwork.Parameter( "dest", "" ) )

		writerRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		shaderLocation = writerRoot.createChild( "shaderLocation" )
		shaderLocation.writeAttribute( "ai:surface", oneShaderNetwork, 0 )
		shaderLocation.writeAttribute( "ai:disp_map", pickOutputNetwork, 0 )
		shaderLocation.writeAttribute( "testBad:surface", noOutputNetwork, 0 )
		shaderLocation.writeAttribute( "complex:surface", complexNetwork, 0 )
		shaderLocation.writeAttribute( "componentConnection:surface", componentConnectionNetwork, 0 )
		shaderLocation.writeAttribute( "manualComponent:surface", manualComponentNetwork, 0 )

		shaderLocation.writeAttribute( "volume", oneShaderNetwork, 0 ) # USD supports shaders without a prefix

		del writerRoot, shaderLocation

		# Read via USD API

		stage = pxr.Usd.Stage.Open( fileName )

		mat = pxr.UsdShade.MaterialBindingAPI.ComputeBoundMaterials( [ stage.GetPrimAtPath( "/shaderLocation" ) ] )[0][0]

		oneShaderSource = mat.GetOutput( "arnold:surface" ).GetConnectedSource()
		self.assertEqual( oneShaderSource[1], "DEFAULT_OUTPUT" )
		oneShaderUsd = pxr.UsdShade.Shader( oneShaderSource[0].GetPrim() )
		self.assertEqual( oneShaderUsd.GetShaderId(), "arnold:standardsurface" )
		self.assertEqual( oneShaderUsd.GetInput( "a" ).Get(), 42.0 )
		self.assertEqual( oneShaderUsd.GetInput( "b" ).Get(), 42 )
		self.assertEqual( oneShaderUsd.GetInput( "c" ).Get(), "42" )
		self.assertEqual( oneShaderUsd.GetInput( "d" ).Get(), pxr.Gf.Vec3f( 3 ) )
		self.assertEqual( oneShaderUsd.GetInput( "e" ).Get(), pxr.Vt.Vec3fArray( [ pxr.Gf.Vec3f( 7 ) ] ) )

		pickOutputSource = mat.GetOutput( "arnold:displacement" ).GetConnectedSource()
		self.assertEqual( pickOutputSource[1], "test" )
		pickOutputUsd = pxr.UsdShade.Shader( pickOutputSource[0].GetPrim() )
		self.assertEqual( pickOutputUsd.GetShaderId(), "arnold:standardsurface" )
		self.assertEqual( pickOutputUsd.GetInput( "c" ).Get(), "42" )

		self.assertEqual( mat.GetOutput( "testBad:surface" ).GetConnectedSource(), None )

		complexShaderSource = mat.GetOutput( "complex:surface" ).GetConnectedSource()
		self.assertEqual( complexShaderSource[1], "DEFAULT_OUTPUT" )
		complexShaderUsd = pxr.UsdShade.Shader( complexShaderSource[0].GetPrim() )
		self.assertEqual( complexShaderUsd.GetShaderId(), "arnold:standardsurface" )
		self.assertEqual( complexShaderUsd.GetInput( "c" ).Get(), "42" )
		self.assertEqual( complexShaderUsd.GetInput( "a" ).Get(), 42.0 )

		aSource = complexShaderUsd.GetInput( "a" ).GetConnectedSource()
		self.assertEqual( aSource[1], "DEFAULT_OUTPUT" )
		add1Usd = pxr.UsdShade.Shader( aSource[0].GetPrim() )
		self.assertEqual( add1Usd.GetShaderId(), "arnold:add" )
		self.assertEqual( add1Usd.GetInput( "b" ).Get(), 3.0 )

		newSource = complexShaderUsd.GetInput( "new" ).GetConnectedSource()
		self.assertEqual( newSource[1], "sum" )
		add2Usd = pxr.UsdShade.Shader( newSource[0].GetPrim() )
		self.assertEqual( add2Usd.GetShaderId(), "osl:add" )
		self.assertEqual( add2Usd.GetInput( "b" ).Get(), 7.0 )

		add1Source = add1Usd.GetInput( "a" ).GetConnectedSource()
		add2Source = add2Usd.GetInput( "a" ).GetConnectedSource()

		self.assertEqual( add1Source[0].GetPrim(), add2Source[0].GetPrim() )
		self.assertEqual( add1Source[1], "DEFAULT_OUTPUT" )
		textureUsd = pxr.UsdShade.Shader( add1Source[0].GetPrim() )
		self.assertEqual( textureUsd.GetShaderId(), "arnold:texture" )
		self.assertEqual( textureUsd.GetInput( "filename" ).Get(), "sometexture.tx" )
		self.assertEqual( add1Source[0].GetPrim().GetAttribute( "inputs:filename" ).GetColorSpace(), "ACEScg" )
		self.assertEqual( add1Source[0].GetPrim().HasAttribute( "inputs:filename_colorspace" ), False )


		# Read via SceneInterface, and check that we've round-tripped successfully.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set( root.child( "shaderLocation" ).attributeNames() ), set( ['ai:disp_map', 'ai:surface', 'complex:surface', 'volume', 'componentConnection:surface', 'manualComponent:surface' ] ) )

		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "ai:surface", 0 ).outputShader().parameters, oneShaderNetwork.outputShader().parameters )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "ai:surface", 0 ).outputShader(), oneShaderNetwork.outputShader() )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "ai:surface", 0 ), oneShaderNetwork )
		self.assertFalse( root.child( "shaderLocation" ).hasAttribute( "testBad:surface" ) )

		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "testBad:surface", 0 ), None )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "ai:disp_map", 0 ), pickOutputNetwork )
		self.assertEqual( root.child( "shaderLocation" ).hasAttribute( "ai:volume" ), False )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "volume", 0 ), oneShaderNetwork )
		self.assertEqual( root.child( "shaderLocation" ).hasAttribute( "volume" ), True )
		self.assertEqual( root.child( "shaderLocation" ).hasAttribute( "surface" ), False )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "surface", 0 ), None )
		self.assertEqual( root.child( "shaderLocation" ).hasAttribute( "displacement" ), False )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "displacement", 0 ), None )

		# Reading a shader type that we didn't write
		self.assertFalse( root.child( "shaderLocation" ).hasAttribute( "ai:volume" ) )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "ai:volume", 0 ), None )

		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "complex:surface", 0 ), complexNetwork )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "componentConnection:surface", 0 ), componentConnectionNetwork )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "manualComponent:surface", 0 ), manualComponentNetwork )

	def testManyShaders( self ) :

		# Write shaders

		fileName = os.path.join( self.temporaryDirectory(), "manyShaders.usda" )

		surfaceA = IECoreScene.Shader( "standardsurface", "ai:surface" )
		surfaceA.parameters["a"] = IECore.FloatData( 42.0 )

		surfaceB = IECoreScene.Shader( "standardsurface", "ai:surface" )
		surfaceB.parameters["a"] = IECore.FloatData( 43.0 )

		texture = IECoreScene.Shader( "texture", "ai:shader" )

		networkA = IECoreScene.ShaderNetwork()
		networkA.addShader( "foo", surfaceA )
		networkA.setOutput( IECoreScene.ShaderNetwork.Parameter( "foo", "" ) )

		networkB = IECoreScene.ShaderNetwork()
		networkB.addShader( "foo", surfaceB )
		networkB.setOutput( IECoreScene.ShaderNetwork.Parameter( "foo", "" ) )

		networkDisp = IECoreScene.ShaderNetwork()
		networkDisp.addShader( "foo", texture )
		networkDisp.setOutput( IECoreScene.ShaderNetwork.Parameter( "foo", "" ) )

		writerRoot = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		topLevel = writerRoot.createChild( "topLevel" )

		for i in range( 20 ):
			shaderLocation = topLevel.createChild( "shaderLocation%i" % i )
			if i >= 10:
				shaderLocation.writeAttribute( "surface", networkB, 0 )
			else:
				shaderLocation.writeAttribute( "surface", networkA, 0 )

			if i == 9 or i == 18 or i == 19:
				shaderLocation.writeAttribute( "displacement", networkDisp, 0 )

			del shaderLocation

		del topLevel, writerRoot

		# Read via USD API

		stage = pxr.Usd.Stage.Open( fileName )

		materials = [ pxr.UsdShade.MaterialBindingAPI.ComputeBoundMaterials( [ stage.GetPrimAtPath( "/topLevel/shaderLocation%i" % i ) ] )[0][0] for i in range( 20 ) ]

		m1 = materials[0]
		m2 = materials[9]
		m3 = materials[10]
		m4 = materials[18]
		self.assertEqual(
			[ i.GetPrim() for i in materials ],
			[ i.GetPrim() for i in
				[ m1, m1, m1, m1, m1, m1, m1, m1, m1, m2, m3, m3, m3, m3, m3, m3, m3, m3, m4, m4 ]
			]
		)

		for m in [ m1, m2, m3, m4 ]:
			o = set( [i.GetName() for i in m.GetPrim().GetAuthoredAttributes() ] )
			if m == m1 or m == m3:
				self.assertEqual( o, set( ["outputs:surface"] ) )
			else:
				self.assertEqual( o, set( ["outputs:surface", "outputs:displacement"] ) )

		m1Source = m1.GetOutput( "surface" ).GetConnectedSource()
		self.assertEqual( pxr.UsdShade.Shader( m1Source[0].GetPrim() ).GetInput( "a" ).Get(), 42.0 )
		m2Source = m2.GetOutput( "surface" ).GetConnectedSource()
		self.assertEqual( pxr.UsdShade.Shader( m2Source[0].GetPrim() ).GetInput( "a" ).Get(), 42.0 )
		m3Source = m3.GetOutput( "surface" ).GetConnectedSource()
		self.assertEqual( pxr.UsdShade.Shader( m3Source[0].GetPrim() ).GetInput( "a" ).Get(), 43.0 )
		m4Source = m4.GetOutput( "surface" ).GetConnectedSource()
		self.assertEqual( pxr.UsdShade.Shader( m4Source[0].GetPrim() ).GetInput( "a" ).Get(), 43.0 )

		# Read via SceneInterface, and check that we've round-tripped successfully.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.childNames(), ["topLevel"] )
		topLevel = root.child( "topLevel" )
		self.assertEqual( set( topLevel.childNames() ), set( [ "shaderLocation%i"%i for i in range( 20 ) ] ) )

		for i in range( 20 ):
			a = set( topLevel.child( "shaderLocation%i" % i ).attributeNames() )
			if i == 9 or i == 18 or i == 19:
				self.assertEqual( a, set( ['surface', 'displacement'] ) )
			else:
				self.assertEqual( a, set( ['surface' ] ) )

			p = topLevel.child( "shaderLocation%i" % i ).readAttribute( "surface", 0 ).outputShader().parameters["a"]
			self.assertEqual( p, IECore.FloatData( 43.0 if i >= 10 else 42.0 ) )

	def testShaderNameConflict( self ):
		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/shaderNameConflict.usda", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set( root.child( "shaderLocation" ).attributeNames() ), set( ['ai:surface' ] ) )

		# The attribute "arnold:surface" has won out over the actual surface shader.  This behaviour is probably
		# unimportant, since it should never happen, but it feels worth having a test for
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "ai:surface", 0 ), IECore.FloatData( 7 ) )

	def testShadersAcrossDifferentScopes( self ):
		# This test case serves to test two different things:
		# A) Shaders with connections to shaders within scopes
		# B) Shaders with connections to shaders outside their scope
		# Case A is definitely valid.  It's unclear if Case B can actually occur, but we should probably do
		# something reasonable here anyway
		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/shaderParentLoc.usda", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set( root.child( "shaderLocation" ).attributeNames() ), set( ['surface' ] ) )
		shaderFoo = IECoreScene.Shader( "standardsurface", "ai:surface" )
		shaderFoo.parameters["a"] = IECore.FloatData( 42 )
		shaderBar = IECoreScene.Shader( "image", "ai:shader" )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "surface", 0 ).getShader( "foo" ), shaderFoo )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "surface", 0 ).getShader( "../scopeB/bar" ), shaderBar )
		self.assertEqual( root.child( "shaderLocation" ).readAttribute( "surface", 0 ).inputConnections( "foo" ),
			[ IECoreScene.ShaderNetwork.Connection(
					IECoreScene.ShaderNetwork.Parameter( "../scopeB/bar", "" ),
					IECoreScene.ShaderNetwork.Parameter( "foo", "b" )
			) ]
		)

		rewriteFileName = os.path.join( self.temporaryDirectory(), "shaders.usda" )
		writerRoot = IECoreScene.SceneInterface.create( rewriteFileName, IECore.IndexedIO.OpenMode.Write )
		writeLocation = writerRoot.createChild( "shaderLocation" )
		writeLocation.writeAttribute( "surface", root.child( "shaderLocation" ).readAttribute( "surface", 0 ), 0 )

		del writerRoot, writeLocation

		rereadRoot = IECoreScene.SceneInterface.create( rewriteFileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( rereadRoot.child( "shaderLocation" ).readAttribute( "surface", 0 ).getShader( "foo" ), shaderFoo )
		self.assertEqual( rereadRoot.child( "shaderLocation" ).readAttribute( "surface", 0 ).getShader( "___scopeB_bar" ), shaderBar )
		self.assertEqual( rereadRoot.child( "shaderLocation" ).readAttribute( "surface", 0 ).inputConnections( "foo" ),
			[ IECoreScene.ShaderNetwork.Connection(
					IECoreScene.ShaderNetwork.Parameter( "___scopeB_bar", "" ),
					IECoreScene.ShaderNetwork.Parameter( "foo", "b" )
			) ]
		)

	def testUnsupportedShaderParameterTypes( self ) :

		sourceNetwork = IECoreScene.ShaderNetwork(
			shaders = {
				"test" : IECoreScene.Shader(
					"test",
					parameters = {
						"unsupported1" : IECore.CompoundData(),
						"unsupported2" : IECore.PathMatcherData(),
						"supported" : "abc"
					}
				)
			},
			output = "test"
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		with IECore.CapturingMessageHandler() as mh :
			root.createChild( "test" ).writeAttribute( "surface", sourceNetwork, 0 )

		self.assertEqual(
			{ m.message for m in mh.messages },
			{
				"Shader parameter `test.unsupported1` has unsupported type `CompoundData`",
				"Shader parameter `test.unsupported2` has unsupported type `PathMatcherData`",
			}
		)

		del root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		network = root.child( "test" ).readAttribute( "surface", 0 )
		self.assertEqual( network.outputShader().parameters, IECore.CompoundData( { "supported" : "abc" } ) )

	def testHoudiniVaryingLengthArrayPrimVar( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "houdiniVaryingLengthArrayPrimVar.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		points = root.child( "plane" )
		self.assertEqual( points.attributeNames(), [] )
		self.assertFalse( points.hasAttribute( "varyingLengthArray" ) )

		primitive = points.readObject( 1.0 )
		self.assertEqual(
			set( primitive.keys() ),
			{ "P", "varyingLengthArray", "varyingLengthArray:lengths" }
		)
		self.assertEqual(
			primitive["varyingLengthArray"],
			IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Constant,
				IECore.FloatVectorData( [ 1, 1, 1, 2, 3, 3, 3, 3, 4 ] )
			)
		)
		self.assertEqual(
			primitive["varyingLengthArray:lengths"],
			IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Vertex,
				IECore.IntVectorData( [ 3, 1, 4, 1 ] )
			)
		)

	def testAssetAttributes( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "assetPathAttribute.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		xform = root.child( "xform" )

		self.assertEqual( xform.attributeNames(), [ "render:testAsset" ] )
		self.assertEqual(
			os.path.normcase( os.path.normpath( xform.readAttribute( "render:testAsset", 0 ).value ) ),
			os.path.normcase( os.path.join( os.path.dirname( __file__ ), "data", "cube.usda" ) )
		)

	def testTextureParameters( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "textureParameters.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		sphere = root.child( "model" ).child( "sphere" )

		self.assertEqual( sphere.attributeNames(), [ "surface" ] )
		network = sphere.readAttribute( "surface", 0 )

		self.assertEqual( network.size(), 4 )
		self.assertEqual(
			os.path.normcase( os.path.normpath( network.getShader( "relativeTexture" ).parameters["file"].value ) ),
			os.path.normcase( os.path.join( os.path.dirname( __file__ ), "myTexture.tx" ) ),
		)
		self.assertEqual(
			os.path.normcase( os.path.normpath( network.getShader( "relativeUDIMTexture" ).parameters["file"].value ) ),
			os.path.normcase( os.path.normpath( os.path.join( os.path.join( os.path.dirname( __file__ ) ), "myTexture.<UDIM>.tx" ) ) ),
		)
		self.assertEqual(
			os.path.normcase( os.path.normpath( network.getShader( "udimTexture" ).parameters["file"].value ) ),
			os.path.normcase( os.path.normpath( "/full/path/to/myTexture.<UDIM>.tx" ) )
		)
		self.assertEqual(
			network.getShader( "relativeTexture" ).parameters["file_colorspace"].value, "ACEScg"
		)
		self.assertEqual(
			network.getShader( "relativeUDIMTexture" ).parameters["file_colorspace"].value, "lin_rec709_scene"
		)
		self.assertEqual(
			network.getShader( "udimTexture" ).parameters["file_colorspace"].value, "srgb_rec709_scene"
		)

	def testExposedShaderInput( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "exposedShaderInput.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		sphere = root.child( "model" ).child( "sphere" )

		self.assertEqual( sphere.attributeNames(), [ "surface" ] )
		network = sphere.readAttribute( "surface", 0 )

		self.assertEqual( network.size(), 1 )
		self.assertEqual( network.getOutput(), "surface" )
		self.assertEqual( network.getShader( "surface" ).parameters["diffuse_roughness"].value, 0.75 )

	@unittest.skipIf( pxr.Usd.GetVersion() < ( 0, 21, 11 ), "UsdLuxLightAPI not available" )
	def testLightsSet( self ) :

		scene = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "sphereLight.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		self.assertIn( "__lights", scene.setNames() )
		self.assertEqual( scene.readSet( "__lights" ), IECore.PathMatcher( [ "/SpotLight23" ] ) )

	@unittest.skipIf( pxr.Usd.GetVersion() < ( 0, 21, 11 ), "UsdLuxLightAPI not available" )
	def testLightAttribute( self ) :

		scene = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "sphereLight.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		light = scene.child( "SpotLight23" )
		self.assertIn( "light", light.attributeNames() )
		self.assertTrue( light.hasAttribute( "light" ) )

		shader = light.readAttribute( "light", 0 )
		self.assertIsInstance( shader, IECoreScene.ShaderNetwork )
		self.assertEqual( shader.size(), 1 )
		self.assertEqual( shader.getOutput(), "SpotLight23" )

		self.assertEqual( shader.getShader( "SpotLight23" ).name, "SphereLight" )
		self.assertEqual( shader.getShader( "SpotLight23" ).type, "light" )

		self.assertEqual(
			shader.getShader( "SpotLight23" ).parameters,
			IECore.CompoundData( {
				"color" : imath.Color3f( 1, 1, 1 ),
				"colorTemperature" : 6500.0,
				"enableColorTemperature" : False,
				"exposure" : 0.0,
				"intensity" : 30000.0,
				"radius" : 0.0,
				"treatAsPoint" : True,
				"shaping:cone:angle" : 66.0,
				"shaping:cone:softness" : 1.0
			} )
		)

	@unittest.skipIf( pxr.Usd.GetVersion() < ( 0, 21, 11 ), "UsdLuxLightAPI not available" )
	def testLightAndShadowLinkCollections( self ) :

		# We ignore `lightLink` and `shadowLink` on lights, because they have
		# a specific meaning in USD that doesn't translate to our definition of a set.

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "sphereLight.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		self.assertNotIn( "lightLink", root.setNames() )
		self.assertNotIn( "shadowLink", root.setNames() )
		self.assertEqual( root.readSet( "lightLink" ), IECore.PathMatcher() )
		self.assertEqual( root.readSet( "shadowLink" ), IECore.PathMatcher() )

		# But that doesn't mean folks can't use those names for sets elsewhere if they
		# want to.

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		root.writeSet( "lightLink", IECore.PathMatcher( [ "/test1" ] ) )
		root.writeSet( "shadowLink", IECore.PathMatcher( [ "/test2" ] ) )
		del root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertIn( "lightLink", root.setNames() )
		self.assertIn( "shadowLink", root.setNames() )
		self.assertEqual( root.readSet( "lightLink" ), IECore.PathMatcher( [ "/test1" ] ) )
		self.assertEqual( root.readSet( "shadowLink" ), IECore.PathMatcher( [ "/test2" ] ) )

	def testReadDoubleSidedAttribute( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "doubleSidedAttribute.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		for name, doubleSided in {
			"sphere" : None,
			"singleSidedSphere" : False,
			"doubleSidedSphere" : True
		}.items() :
			object = root.child( name )
			if doubleSided is None :
				self.assertFalse( object.hasAttribute( "doubleSided" ) )
				self.assertNotIn( "doubleSided", object.attributeNames() )
			else :
				self.assertTrue( object.hasAttribute( "doubleSided" ) )
				self.assertIn( "doubleSided", object.attributeNames() )
				self.assertEqual( object.readAttribute( "doubleSided", 1 ), IECore.BoolData( doubleSided ) )

	def testWriteDoubleSidedAttribute( self ) :

		# Write via SceneInterface

		fileName = os.path.join( self.temporaryDirectory(), "doubleSidedAttribute.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		toWrite = (
			( "singleSidedSphere", "before", False ),
			( "doubleSidedSphere", "before", True ),
			( "doubleSidedSphereWrittenAfter", "after", True ),
			( "doubleSidedNoObject", "never", True ),
			( "sphere", "before", None ),
		)

		for name, writeObject, doubleSided in toWrite :

			child = root.createChild( name )
			if writeObject == "before" :
				child.writeObject( IECoreScene.SpherePrimitive(), 1 )

			if doubleSided is not None :

				with IECore.CapturingMessageHandler() as mh :
					child.writeAttribute( "doubleSided", IECore.BoolData( doubleSided ), 1 )

				if writeObject != "before" :
					self.assertEqual( len( mh.messages ), 1 )
					self.assertEqual(
						mh.messages[0].message,
						'Unable to write attribute "doubleSided" to "/{}", because it is not a Gprim'.format(
							name
						)
					)

			if writeObject == "after" :
				child.writeObject( IECoreScene.SpherePrimitive(), 1 )

		del root, child

		# Verify via USD API

		stage = pxr.Usd.Stage.Open( fileName )

		for name, writeObject, doubleSided in toWrite :

			if writeObject != "before" :
				doubleSided = None

			if doubleSided is None :
				self.assertFalse(
					pxr.UsdGeom.Gprim( stage.GetPrimAtPath( "/" + name ) ).GetDoubleSidedAttr().HasAuthoredValue(),
				)
			else :
				self.assertEqual(
					pxr.UsdGeom.Gprim( stage.GetPrimAtPath( "/" + name ) ).GetDoubleSidedAttr().Get( 1 ),
					doubleSided
				)

	def testColor4fShaderParameter( self ) :

		network = IECoreScene.ShaderNetwork(
			shaders = {
				"output" : IECoreScene.Shader(
					"mySurface", "surface",
					{
						"color4fParameter" : imath.Color4f( 1, 2, 3, 4 ),
						"color4fArrayParameter" : IECore.Color4fVectorData( [
							imath.Color4f( 1, 2, 3, 4 ),
							imath.Color4f( 5, 6, 7, 8 ),
						] ),
					}
				),
			},
			output = "output"
		)

		fileName = os.path.join( self.temporaryDirectory(), "shader.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "child" )
		child.writeAttribute( "surface", network, 0 )

		del child, root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		child = root.child( "child" )
		self.assertEqual( child.readAttribute( "surface", 0 ), network )

	def testColor4fShaderParameterComponentConnections( self ) :

		network = IECoreScene.ShaderNetwork(
			shaders = {
				"source" : IECoreScene.Shader( "noise" ),
				"output" : IECoreScene.Shader(
					"color_correct",
					parameters = {
						"input" : imath.Color4f( 1 ),
					}
				),
			},
			connections = [
				( ( "source", "r" ), ( "output", "input.g" ) ),
				( ( "source", "g" ), ( "output", "input.b" ) ),
				( ( "source", "b" ), ( "output", "input.r" ) ),
				( ( "source", "r" ), ( "output", "input.a" ) ),
			],
			output = "output",
		)

		fileName = os.path.join( self.temporaryDirectory(), "color4fComponentConnections.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		object = root.createChild( "object" )
		object.writeAttribute( "ai:surface", network, 0.0 )
		del object, root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.child( "object" ).readAttribute( "ai:surface", 0 ), network )

	def testLegacyComponentConnections( self ) :

		expectedNetwork = IECoreScene.ShaderNetwork(
			shaders = {
				"source" : IECoreScene.Shader( "noise" ),
				"output" : IECoreScene.Shader(
					"color_correct",
					parameters = {
						"input" : imath.Color4f( 1 ),
					}
				),
			},
			connections = [
				( ( "source", "r" ), ( "output", "input.g" ) ),
				( ( "source", "g" ), ( "output", "input.b" ) ),
				( ( "source", "b" ), ( "output", "input.r" ) ),
				( ( "source", "r" ), ( "output", "input.a" ) ),
			],
			output = "output",
		)

		root = IECoreScene.SceneInterface.create( os.path.join( os.path.dirname( __file__ ), "data", "legacyComponentConnections.usda" ), IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.child( "object" ).readAttribute( "ai:surface", 0 ), expectedNetwork )

	def testShaderBlindData( self ) :

		shader = IECoreScene.Shader( "test" )
		shader.blindData()["testInt"] = IECore.IntData( 10 )
		shader.blindData()["testFloatVector"] = IECore.FloatVectorData( [ 1, 2, 3, ] )
		shader.blindData()["test:colon"] = IECore.BoolData( True )
		shader.blindData()["testCompound"] = IECore.CompoundData( {
			"testString" : "test",
			"testStringVector" : IECore.StringVectorData( [ "one", "two" ] )
		} )

		network = IECoreScene.ShaderNetwork(
			shaders = { "test" : shader },
			output = ( "test", "out" )
		)

		fileName = os.path.join( self.temporaryDirectory(), "testShaderBlindData.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		object = root.createChild( "object" )
		object.writeAttribute( "surface", network, 0.0 )
		del object, root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.child( "object" ).readAttribute( "surface", 0 ), network )

	def testMaterialPurpose( self ) :

		def assertExpected( root ) :

			sphere = root.child( "model" ).child( "sphere" )

			self.assertEqual( set( sphere.attributeNames() ), { "surface", "surface:full", "surface:preview" } )
			for n in ( "surface", "surface:full", "surface:preview" ) :
				self.assertTrue( sphere.hasAttribute( n ) )

			self.assertEqual(
				sphere.readAttribute( "surface", 0 ).getShader( "surface" ).parameters["base"],
				IECore.FloatData( 0 )
			)

			self.assertEqual(
				sphere.readAttribute( "surface:full", 0 ).getShader( "surface" ).parameters["base"],
				IECore.FloatData( 0.5 )
			)

			self.assertEqual(
				sphere.readAttribute( "surface:preview", 0 ).getShader( "surface" ).parameters["base"],
				IECore.FloatData( 1 )
			)

		inRoot = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/materialPurpose.usda", IECore.IndexedIO.OpenMode.Read )
		assertExpected( inRoot )

		roundTripFileName = os.path.join( self.temporaryDirectory(), "materialPurpose.usda" )
		outRoot = IECoreScene.SceneInterface.create( roundTripFileName, IECore.IndexedIO.OpenMode.Write )

		IECoreScene.SceneAlgo.copy( inRoot, outRoot, 0, 0, 24, IECoreScene.SceneAlgo.ProcessFlags.All )

		roundTripRoot = IECoreScene.SceneInterface.create( roundTripFileName, IECore.IndexedIO.OpenMode.Read )
		assertExpected( roundTripRoot )

	def testMultipleLights( self ) :

		scene = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "twoLights.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		self.assertIn( "__lights", scene.setNames() )
		self.assertEqual( scene.readSet( "__lights" ), IECore.PathMatcher( [ "/Light1", "/Light2" ] ) )

		hashes = {
			scene.child( n ).hash( scene.HashType.AttributesHash, 0 )
			for n in [ "NoLight", "Light1", "Light2" ]
		}
		self.assertEqual( len( hashes ), 3 )

		for light, exposure in [
			( "Light1", 1 ),
			( "Light2", 2 ),
		] :
			self.assertEqual( scene.child( light ).attributeNames(), [ "light" ] )
			attribute = scene.child( light ).readAttribute( "light", 0 )
			self.assertIsInstance( attribute, IECoreScene.ShaderNetwork )
			self.assertEqual( attribute.outputShader().parameters["exposure"], IECore.FloatData( exposure ) )

	def testWriteLight( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "pointInstancePrimvars.usda" )
		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		light = scene.createChild( "light" )
		light.writeAttribute(
			"light",
			IECoreScene.ShaderNetwork(
				shaders = {
					"output" : IECoreScene.Shader(
						"SphereLight", "light",
						{
							"exposure" : 3.0,
							"radius" : 1.5,
							"color" : imath.Color3f( 1, 0.5, 0.25 ),
						}
					)
				},
				output = "output",
			),
			0
		)

		del light, scene

		stage = pxr.Usd.Stage.Open( fileName )
		light = pxr.UsdLux.SphereLight( stage.GetPrimAtPath( "/light" ) )
		self.assertTrue( light )

		self.assertEqual( light.GetExposureAttr().Get(), 3.0 )
		self.assertEqual( light.GetRadiusAttr().Get(), 1.5 )
		self.assertEqual( light.GetColorAttr().Get(), pxr.Gf.Vec3f( 1.0, 0.5, 0.25 ) )
		self.assertEqual( light.GetPrim().GetChildren(), [] )

	def testWriteLightWithInputNetwork( self ) :

		# Write to USD

		fileName = os.path.join( self.temporaryDirectory(), "pointInstancePrimvars.usda" )
		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		lightNetwork = IECoreScene.ShaderNetwork(
			shaders = {
				"output" : IECoreScene.Shader( "RectLight", "light" ),
				"texture" : IECoreScene.Shader(
					"UsdUVTexture", "shader",
					{
						"file" : "test.exr",
					}
				),
			},
			connections = [
				( ( "texture", "rgb" ), ( "output", "color" ) )
			],
			output = "output",
		)

		light = scene.createChild( "light" )
		light.writeAttribute( "light", lightNetwork, 0 )

		del light, scene

		# Verify via USD API

		stage = pxr.Usd.Stage.Open( fileName )
		self.assertTrue( pxr.UsdLux.RectLight( stage.GetPrimAtPath( "/light" ) ) )

		light = pxr.UsdLux.LightAPI( stage.GetPrimAtPath( "/light" ) )
		source, sourceName, sourceType = light.GetInput( "color" ).GetConnectedSource()
		self.assertEqual( source.GetPrim().GetName(), "texture" )
		self.assertEqual( source.GetPrim().GetParent(), light.GetPrim() )
		self.assertEqual( sourceName, "rgb" )
		self.assertEqual( sourceType, pxr.UsdShade.AttributeType.Output )

		self.assertEqual( light.GetPrim().GetChildren(), [ source.GetPrim() ] )

	def testWriteDomeLightFile( self ) :

		# Write to USD

		fileName = os.path.join( self.temporaryDirectory(), "pointInstancePrimvars.usda" )
		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		lightNetwork = IECoreScene.ShaderNetwork(
			shaders = {
				"output" : IECoreScene.Shader( "DomeLight", "light", { "texture:file" : "test.exr", "texture:format" : "latlong" } ),
			},
			output = "output",
		)

		light = scene.createChild( "light" )
		light.writeAttribute( "light", lightNetwork, 0 )

		del light, scene

		# Verify via USD API

		stage = pxr.Usd.Stage.Open( fileName )
		light = pxr.UsdLux.DomeLight( stage.GetPrimAtPath( "/light" ) )
		self.assertEqual( light.GetTextureFileAttr().Get(), "test.exr" )
		self.assertEqual( light.GetTextureFormatAttr().Get(), "latlong" )

	def testPointInstancerPrimvars( self ) :

		# Use the USD API to author a point instancer with primvars on it.

		fileName = os.path.join( self.temporaryDirectory(), "pointInstancePrimvars.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )
		pointInstancer = pxr.UsdGeom.PointInstancer.Define( stage, "/points" )
		pointInstancer.CreatePositionsAttr( [ ( v, v, v ) for v in range( 0, 5 ) ] )

		primvars = pxr.UsdGeom.PrimvarsAPI( pointInstancer )
		primvar = primvars.CreatePrimvar( "myColor", pxr.Sdf.ValueTypeNames.Color3fArray, "vertex" )
		primvar.Set(
			[ ( c, c, c ) for c in range( 1, 6 ) ]
		)

		stage.GetRootLayer().Save()

		# Check we can load the primvar via a SceneInterface.

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		points = root.child( "points" ).readObject( 0 )

		self.assertEqual( points.keys(), ['P', 'myColor', 'prototypeRoots'] )

		self.assertIsInstance( points, IECoreScene.PointsPrimitive )
		self.assertIn( "myColor", points )
		self.assertEqual(
			points["myColor"].data,
			IECore.Color3fVectorData( [ imath.Color3f( c ) for c in range( 1, 6 ) ] )
		)
		self.assertEqual( points["myColor"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( points["myColor"].indices, None )

		# Now try deactivating some ids

		pointInstancer.DeactivateIds( [ 0, 2 ] )
		pointInstancer.InvisIds( [ 1, 4 ], 0 )

		stage.GetRootLayer().Save()

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		points = root.child( "points" ).readObject( 0 )

		self.assertEqual( points.keys(), ['P', 'inactiveIds', 'invisibleIds', 'myColor', 'prototypeRoots'] )

		self.assertEqual( points["inactiveIds"], IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.Int64VectorData( [ 0, 2 ] ) ) )
		self.assertEqual( points["invisibleIds"], IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.Int64VectorData( [ 1, 4 ] ) ) )

	def testArnoldArrayInputs( self ) :

		def assertExpectedArrayInputs( network ) :

			inputs = network.inputConnections( "rampRGB" )
			self.assertEqual( len( inputs ), 2 )
			self.assertEqual( inputs[0], ( ( "noise", "out" ), ( "rampRGB", "color[0]" ) ) )
			self.assertEqual( inputs[1], ( ( "flat", "out" ), ( "rampRGB", "color[1]" ) ) )

		# Load original USD out of USD-Arnold.

		scene = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "arnoldArrayInputs.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		network = scene.child( "sphere" ).readAttribute( "ai:surface", 0 )

		assertExpectedArrayInputs( network )

		# Write our own USD from that data, to check we can round-trip it.

		fileName = os.path.join( self.temporaryDirectory(), "arnoldArrayInputsRewritten.usda" )
		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		scene.createChild( "sphere" ).writeAttribute( "ai:surface", network, 0 )

		del scene
		scene = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		assertExpectedArrayInputs( scene.child( "sphere" ).readAttribute( "ai:surface", 0 ) )

	def testInvalidPrimitiveVariables( self ) :

		goodRoot = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.usda", IECore.IndexedIO.OpenMode.Read )
		badRoot = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/invalidCube.usda", IECore.IndexedIO.OpenMode.Read )

		goodCube = goodRoot.child( "pCube1" ).readObject( 0 )
		self.assertIn( "uv", goodCube )
		self.assertTrue( goodCube.arePrimitiveVariablesValid() )

		with IECore.CapturingMessageHandler() as mh :
			badCube = badRoot.child( "pCube1" ).readObject( 0 )

		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Warning )
		self.assertEqual( mh.messages[0].message, "Ignoring invalid primitive variable \"/pCube1.primvars:st\"" )

		self.assertNotIn( "uv", badCube )
		self.assertTrue( badCube.arePrimitiveVariablesValid() )

		self.assertNotEqual( goodCube, badCube )
		del goodCube["uv"]
		self.assertEqual( goodCube, badCube )

	def testInvalidPointBasedAttributes( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "pointBased.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )

		curves = pxr.UsdGeom.BasisCurves.Define( stage, "/curves" )
		curves.CreateCurveVertexCountsAttr().Set( [ 4 ] )
		curves.CreatePointsAttr().Set( [ ( 1, 1, 1 ), ( 2, 2, 2 ) ] )
		curves.CreateVelocitiesAttr().Set( [ ( 1, 1, 1 ) ] )
		curves.CreateAccelerationsAttr().Set( [ ( 0, 0, 0 ) ] )
		curves.CreateNormalsAttr().Set( [ ( 1, 0, 0 ) ] )

		stage.GetRootLayer().Save()

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		with IECore.CapturingMessageHandler() as mh :
			curves = root.child( "curves" ).readObject( 0 )

		# No valid primitive variables.
		self.assertEqual( curves.keys(), [] )
		# And a warning message for every one we tried to load.
		messages = { m.message for m in mh.messages }
		for attribute in [
			"/curves.points",
			"/curves.velocities",
			"/curves.accelerations",
			"/curves.normals",
		] :
			self.assertIn(
				f"Ignoring invalid primitive variable \"{attribute}\"",
				messages
			)

	def testInvalidCurvesAttributes( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "curves.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )

		curves = pxr.UsdGeom.BasisCurves.Define( stage, "/curves" )
		curves.CreateCurveVertexCountsAttr().Set( [ 4 ] )
		curves.CreateWidthsAttr().Set( [ 1, 2 ] )

		stage.GetRootLayer().Save()

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		with IECore.CapturingMessageHandler() as mh :
			curves = root.child( "curves" ).readObject( 0 )

		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual(
			mh.messages[0].message, f"Ignoring invalid primitive variable \"/curves.widths\"",
		)
		self.assertNotIn( "width", curves )

	def testInvalidPointsAttributes( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "points.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )

		points = pxr.UsdGeom.Points.Define( stage, "/points" )
		points.CreatePointsAttr().Set( [ ( 1, 1, 1 ), ( 2, 2, 2 ), ( 3, 3, 3 ) ] )
		points.CreateWidthsAttr().Set( [ 1, 2 ] )
		points.CreateIdsAttr().Set( [ 1, 2 ] )

		stage.GetRootLayer().Save()

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		with IECore.CapturingMessageHandler() as mh :
			points = root.child( "points" ).readObject( 0 )

		self.assertEqual( len( mh.messages ), 2 )
		self.assertEqual( mh.messages[0].message, f"Ignoring invalid primitive variable \"/points.ids\"" )
		self.assertEqual( mh.messages[1].message, f"Ignoring invalid primitive variable \"/points.widths\"" )
		self.assertNotIn( "id", points )
		self.assertNotIn( "width", points )

	def testNormalsPrimVar( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/normalsPrimVar.usda", IECore.IndexedIO.OpenMode.Read )
		mesh = root.child( "mesh" ).readObject( 0 )

		self.assertNotIn( "normals", mesh )
		self.assertIn( "N", mesh )
		self.assertEqual( mesh["N"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertIsNotNone( mesh["N"].indices )

	def testNormalsPrimVarBeatsNormalsAttribute( self ) :

		root = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/normalsAttributeAndPrimVar.usda", IECore.IndexedIO.OpenMode.Read )
		mesh = root.child( "mesh" ).readObject( 0 )

		self.assertNotIn( "normals", mesh )
		self.assertIn( "N", mesh )
		self.assertEqual( mesh["N"].data, IECore.V3fVectorData( [ imath.V3f( 0, 0, 1 ) ] * 3, IECore.GeometricData.Interpretation.Normal ) )
		self.assertEqual( mesh["N"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertIsNone( mesh["N"].indices )

	def testRoundTripIndexedNormals( self ) :

		mesh = IECoreScene.MeshPrimitive(
			IECore.IntVectorData( [ 3, 3 ] ),
			IECore.IntVectorData( [ 0, 2, 1, 2, 3, 1 ] ),
			"linear",
			IECore.V3fVectorData( [
				imath.V3f( 0, 1, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 )
			] )
		)

		mesh["N"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.FaceVarying,
			IECore.V3fVectorData( [ imath.V3f( 0, 0, 1 ) ], IECore.GeometricData.Interpretation.Normal ),
			IECore.IntVectorData( [ 0, 0, 0, 0, 0, 0 ] )
		)

		fileName = os.path.join( self.temporaryDirectory(), "indexedNormals.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root.createChild( "mesh" ).writeObject( mesh, 0.0 )
		del root

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.child( "mesh").readObject( 0.0 ), mesh )

	def testResetXFormStack( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "resetXFormStack.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )

		g1 = pxr.UsdGeom.Xform.Define( stage, "/g1" )
		g1.AddScaleOp().Set( pxr.Gf.Vec3f( 1, 2, 3 ) )

		g2 = pxr.UsdGeom.Xform.Define( stage, "/g1/g2" )
		g2.AddTranslateOp().Set( pxr.Gf.Vec3f( 4, 5, 6 ) )

		g3 = pxr.UsdGeom.Xform.Define( stage, "/g1/g2/g3" )
		g3.AddTranslateOp().Set( pxr.Gf.Vec3f( 1, 0, 1 ) )
		g3.SetResetXformStack( True )

		pxr.UsdGeom.Sphere.Define( stage, "/g1/g2/g3/s" )

		stage.GetRootLayer().Save()
		del stage

		worldTransform = imath.M44d()
		location = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		for name in [ "g1", "g2", "g3", "s" ] :
			location = location.child( name )
			worldTransform = location.readTransformAsMatrix( 0 ) * worldTransform
			self.assertEqual(
				location.hash( location.HashType.TransformHash, 1 ), # There is no animation,
				location.hash( location.HashType.TransformHash, 2 ), # so hashes should be identical
			)

		self.assertEqual( worldTransform, imath.M44d().translate( imath.V3f( 1, 0, 1 ) ) )

	def testResetXFormStackHash( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "resetXFormStackHash.usda" )
		stage = pxr.Usd.Stage.CreateNew( fileName )

		# /g1 (animated transform)
		#  /g2a (reset transform stack)
		#  /g2b

		g1 = pxr.UsdGeom.Xform.Define( stage, "/g1" )
		scaleOp = g1.AddScaleOp()
		scaleOp.Set( pxr.Gf.Vec3f( 2 ), 1 )
		scaleOp.Set( pxr.Gf.Vec3f( 4 ), 2 )

		g2a = pxr.UsdGeom.Xform.Define( stage, "/g1/g2a" )
		g2a.SetResetXformStack( True )
		pxr.UsdGeom.Xform.Define( stage, "/g1/g2b" )

		stage.GetRootLayer().Save()
		del stage

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		g2a = root.scene( [ "g1", "g2a" ] )
		g2b = root.scene( [ "g1", "g2b" ] )

		# Hash should be animated because the reset needs to account
		# for animation on parent.

		self.assertNotEqual(
			g2a.hash( g2a.HashType.TransformHash, 1 ),
			g2a.hash( g2a.HashType.TransformHash, 2 ),
		)

		# Hash should be static, because there is no reset.

		self.assertEqual(
			g2b.hash( g2a.HashType.TransformHash, 1 ),
			g2b.hash( g2a.HashType.TransformHash, 2 ),
		)

	def testChildExceptions( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "untypedParentPrim.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		with self.assertRaisesRegex( RuntimeError, 'USDScene::child : Name "!" is not a valid identifier' ) :
			root.child( "!", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )

		with self.assertRaisesRegex( RuntimeError, 'USDScene::child : UsdPrim "/" has no child named "notHere"' ) :
			root.child( "notHere", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )

		with self.assertRaisesRegex( RuntimeError, 'USDScene::child : UsdPrim "/undefined" does not contribute to the scene hierarchy' ) :
			root.child( "undefined", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )

	def __createInstancedComposition( self, numInstances, numInstanceChildren ) :

		instanceFileName = os.path.join( self.temporaryDirectory(), "instance.usd" )
		stage = pxr.Usd.Stage.CreateNew( instanceFileName )
		pxr.UsdGeom.Xform.Define( stage, "/root/group" )
		spherePaths = []
		for i in range( 0, numInstanceChildren ) :
			sphere = pxr.UsdGeom.Sphere.Define( stage, f"/root/group/sphere{i}" )
			spherePaths.append( sphere.GetPath() )

		collection = pxr.Usd.CollectionAPI.Apply( stage.GetPrimAtPath( "/root/group" ), "testCollection" )
		collection.CreateIncludesRel().SetTargets( spherePaths )

		stage.GetRootLayer().Save()
		del stage

		compositionFileName = os.path.join( self.temporaryDirectory(), "composition.usd" )
		stage = pxr.Usd.Stage.CreateNew( compositionFileName )
		for i in range( 0, numInstances ) :
			instance = stage.DefinePrim( f"/instance{i}" )
			instance.GetReferences().AddReference( str( instanceFileName ), "/root" )
			instance.SetInstanceable( True )
		stage.GetRootLayer().Save()

		return compositionFileName

	@unittest.skipIf( IECore.TestUtil.inCI(), "Performance test, not useful to run during CI" )
	def testInstancedSetNames( self ) :

		fileName = self.__createInstancedComposition( 10000, 200 )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		t = time.perf_counter()
		self.assertIn( "testCollection", root.setNames( includeDescendantSets = True ) )
		print( time.perf_counter() - t )

	@unittest.skipIf( IECore.TestUtil.inCI(), "Performance test, not useful to run during CI" )
	def testInstancedSet( self ) :

		fileName = self.__createInstancedComposition( 10000, 200 )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		t = time.perf_counter()
		s = root.readSet( "testCollection", includeDescendantSets = True )
		self.assertEqual( s.match( "/instance0/group/sphere0" ), s.Result.ExactMatch )
		print( time.perf_counter() - t )

	def testInstancedCameraSet( self ) :

		instanceFileName = os.path.join( self.temporaryDirectory(), "instance.usd" )
		stage = pxr.Usd.Stage.CreateNew( instanceFileName )
		pxr.UsdGeom.Xform.Define( stage, "/root" )
		pxr.UsdGeom.Camera.Define( stage, f"/root/camera" )
		stage.GetRootLayer().Save()
		del stage

		compositionFileName = os.path.join( self.temporaryDirectory(), "composition.usd" )
		stage = pxr.Usd.Stage.CreateNew( compositionFileName )
		for i in range( 0, 2 ) :
			instance = stage.DefinePrim( f"/instance{i}" )
			instance.GetReferences().AddReference( str( instanceFileName ), "/root" )
			instance.SetInstanceable( True )
		stage.GetRootLayer().Save()
		del stage

		root = IECoreScene.SceneInterface.create( compositionFileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.readSet( "__cameras" ), IECore.PathMatcher( [ "/instance0/camera", "/instance1/camera" ] ) )

	def testMaterialBindingInsideInstance( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "materialBindingInsideInstance.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		instance1 = root.scene( [ "instance1", "cube" ] )
		instance2 = root.scene( [ "instance2", "cube" ] )

		self.assertEqual(
			instance1.hash( IECoreScene.SceneInterface.HashType.AttributesHash, 0.0 ),
			instance2.hash( IECoreScene.SceneInterface.HashType.AttributesHash, 0.0 ),
		)

		shader1 = instance1.readAttribute( "surface", 0.0, _copy = False )
		shader2 = instance2.readAttribute( "surface", 0.0, _copy = False )
		self.assertTrue( shader1.isSame( shader2 ) )

	@unittest.skipIf( not haveVDB, "No IECoreVDB" )
	def testReadUsdVolVolume( self ) :

		import IECoreVDB

		fileName = os.path.dirname( __file__ ) + "/data/volume.usda"
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		child = root.child( "volume" )
		self.assertEqual( child.childNames(), [] )

		vdbObject = child.readObject( 0 )
		self.assertIsInstance( vdbObject, IECoreVDB.VDBObject )
		self.assertTrue( pathlib.Path( vdbObject.fileName() ).samefile( "test/IECoreVDB/data/smoke.vdb" ) )
		self.assertTrue( vdbObject.unmodifiedFromFile() )

	@unittest.skipIf( not haveVDB, "No IECoreVDB" )
	def testWriteVDBObject( self ) :

		import IECoreVDB

		vdbFileName = "./test/IECoreVDB/data/smoke.vdb"
		vdbObject = IECoreVDB.VDBObject( vdbFileName )

		# Write via SceneInterface

		fileName = os.path.join( self.temporaryDirectory(), "volume.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "smoke" )
		child.writeObject( vdbObject, 0 )

		del root, child

		# Verify via USD API

		stage = pxr.Usd.Stage.Open( fileName )
		volume = pxr.UsdVol.Volume( stage.GetPrimAtPath( "/smoke" ) )
		self.assertTrue( volume )
		self.assertTrue( volume.HasFieldRelationship( "density" ) )
		self.assertEqual( volume.GetFieldPath( "density" ), "/smoke/density" )
		self.assertEqual( volume.GetPrim().GetChildrenNames(), [ "density" ] )

		field = pxr.UsdVol.OpenVDBAsset( stage.GetPrimAtPath( "/smoke/density" ) )
		self.assertTrue( field )
		self.assertEqual( field.GetFieldNameAttr().Get( 0 ), "density" )
		self.assertEqual( field.GetFilePathAttr().Get( 0 ), vdbFileName )
		self.assertEqual( field.GetFieldClassAttr().Get( 0 ), "GRID_FOG_VOLUME" )

	def testUnconnectedMaterialOutput( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "unconnectedMaterialOutput.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)

		sphere = root.child( "sphere" )
		self.assertFalse( sphere.hasAttribute( "cycles:surface" ) )
		self.assertNotIn( "cycles:surface", sphere.attributeNames() )
		self.assertIsNone( sphere.readAttribute( "cycles:surface", 0 ) )

	def testReadFromStageCache( self ) :

		stage = pxr.Usd.Stage.CreateInMemory()
		pxr.UsdGeom.Sphere.Define( stage, "/sphere" )
		id = pxr.UsdUtils.StageCache.Get().Insert( stage )

		root = IECoreScene.SceneInterface.create( "stageCache:{}.usd".format( id.ToString() ), IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.childNames(), [ "sphere" ] )
		self.assertIsInstance( root.child( "sphere" ).readObject( 0 ), IECoreScene.SpherePrimitive )

	def testRoundTripArnoldLight( self ) :

		lightShader = IECoreScene.ShaderNetwork(
			shaders = {
				"light" : IECoreScene.Shader( "distant_light", "ai:light", parameters = { "exposure" : 2.0 } )
			},
			output = "light",
		)

		root = IECoreScene.SceneInterface.create(
			os.path.join( self.temporaryDirectory(), "test.usda" ),
			IECore.IndexedIO.OpenMode.Write
		)
		light = root.createChild( "light" )
		light.writeAttribute( "ai:light", lightShader, 0 )
		root.writeSet( "__lights", IECore.PathMatcher( [ "/light" ] ) )
		del root, light

		root = IECoreScene.SceneInterface.create(
			os.path.join( self.temporaryDirectory(), "test.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		light = root.child( "light" )
		self.assertIn( "ai:light", light.attributeNames() )
		self.assertEqual( light.readAttribute( "ai:light", 0 ), lightShader )
		self.assertIn( "__lights", root.setNames() )
		self.assertEqual( root.readSet( "__lights" ), IECore.PathMatcher( [ "/light" ] ) )

	def testArnoldSpecificLightInputs( self ) :

		# The `arnold-usd` project doesn't represent Arnold-specific UsdLux
		# extensions as `inputs:arnold:*` attributes as it logically should :
		# instead it uses `primvars:arnold:*` attributes. In Cortex/Gaffer we
		# wish to use regular `arnold:*` shader parameters rather than primvars,
		# so must convert to and from the less logical form in USDScene.

		lightShader = IECoreScene.ShaderNetwork(
			shaders = {
				"light" : IECoreScene.Shader(
					"RectLight", "light",
					parameters = {
						"exposure" : 1.0,
						"arnold:roundness" : 2.0,
					}
				)
			},
			output = "light",
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		light = root.createChild( "light" )
		light.writeAttribute( "light", lightShader, 0 )
		del root, light

		stage = pxr.Usd.Stage.Open( fileName )
		shadeAPI = pxr.UsdShade.ConnectableAPI( stage.GetPrimAtPath( "/light" ) )
		self.assertTrue( shadeAPI.GetInput( "exposure" ) )
		self.assertFalse( shadeAPI.GetInput( "arnold:roundness" ) )
		primvarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/light" ) )
		self.assertTrue( primvarsAPI.HasPrimvar( "arnold:roundness" ) )
		self.assertEqual( primvarsAPI.GetPrimvar( "arnold:roundness" ).Get(), 2.0 )
		del stage, shadeAPI, primvarsAPI

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.child( "light" ).readAttribute( "light", 0 ), lightShader )
		self.assertEqual( root.child( "light" ).attributeNames(), [ "light" ] )

	def testTreatLightAsPointOrLine( self ) :

		# `treatAsPoint` and `treatAsLine` aren't defined as UsdShade inputs but we store
		# them as regular shader parameter, so they need special handling when writing to USD.

		sphereLightShader = IECoreScene.ShaderNetwork(
			shaders = {
				"sphereLight" : IECoreScene.Shader(
					"SphereLight", "light",
					parameters = {
						"treatAsPoint" : True,
					}
				)
			},
			output = "sphereLight",
		)

		cylinderLightShader = IECoreScene.ShaderNetwork(
			shaders = {
				"cylinderLight" : IECoreScene.Shader(
					"CylinderLight", "light",
					parameters = {
						"treatAsLine" : True,
					}
				)
			},
			output = "cylinderLight",
		)

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )
		root.createChild( "sphereLight" ).writeAttribute( "light", sphereLightShader, 0 )
		root.createChild( "cylinderLight" ).writeAttribute( "light", cylinderLightShader, 0 )
		del root

		stage = pxr.Usd.Stage.Open( fileName )
		self.assertEqual( pxr.UsdLux.SphereLight( stage.GetPrimAtPath( "/sphereLight" ) ).GetTreatAsPointAttr().Get(), True )
		self.assertEqual( pxr.UsdLux.CylinderLight( stage.GetPrimAtPath( "/cylinderLight" ) ).GetTreatAsLineAttr().Get(), True )
		del stage

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( root.child( "sphereLight" ).readAttribute( "light", 0 ), sphereLightShader )
		self.assertEqual( root.child( "cylinderLight" ).readAttribute( "light", 0 ), cylinderLightShader )

	def testModelBound( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "modelBound.usda" )

		stage = pxr.Usd.Stage.CreateNew( fileName )
		pxr.UsdGeom.Xform.Define( stage, "/withoutModelAPI" )
		pxr.UsdGeom.Xform.Define( stage, "/withModelAPI" )
		pxr.UsdGeom.Xform.Define( stage, "/withModelAPIAndExtent" )

		pxr.UsdGeom.ModelAPI.Apply( stage.GetPrimAtPath( "/withModelAPI" ) )
		modelAPI = pxr.UsdGeom.ModelAPI.Apply( stage.GetPrimAtPath( "/withModelAPIAndExtent" ) )
		modelAPI.SetExtentsHint( [ ( 1, 2, 3 ), ( 4, 5, 6 ) ] )

		stage.GetRootLayer().Save()
		del stage

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertFalse( root.hasBound() )

		self.assertFalse( root.child( "withoutModelAPI" ).hasBound() )
		self.assertFalse( root.child( "withModelAPI" ).hasBound() )
		self.assertTrue( root.child( "withModelAPIAndExtent" ).hasBound() )
		self.assertEqual( root.child( "withModelAPIAndExtent" ).readBound( 0 ), imath.Box3d( imath.V3d( 1, 2, 3 ), imath.V3d( 4, 5, 6 ) ) )

	def testAnimatedModelBound( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "modelBound.usda" )

		stage = pxr.Usd.Stage.CreateNew( fileName )
		pxr.UsdGeom.Xform.Define( stage, "/model" )

		pxr.UsdGeom.ModelAPI.Apply( stage.GetPrimAtPath( "/model" ) )
		modelAPI = pxr.UsdGeom.ModelAPI.Apply( stage.GetPrimAtPath( "/model" ) )
		modelAPI.SetExtentsHint( [ ( 1, 2, 3 ), ( 4, 5, 6 ) ], 0 )
		modelAPI.SetExtentsHint( [ ( 2, 3, 4 ), ( 5, 6, 7 ) ], 24 )

		stage.GetRootLayer().Save()
		del stage

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertTrue( root.child( "model" ).hasBound() )
		self.assertEqual( root.child( "model" ).readBound( 0 ), imath.Box3d( imath.V3d( 1, 2, 3 ), imath.V3d( 4, 5, 6 ) ) )
		self.assertEqual( root.child( "model" ).readBound( 1 ), imath.Box3d( imath.V3d( 2, 3, 4 ), imath.V3d( 5, 6, 7 ) ) )

		self.assertNotEqual(
			root.child( "model" ).hash( root.HashType.BoundHash, 0 ),
			root.child( "model" ).hash( root.HashType.BoundHash, 1 )
		)

	def testPerPurposeModelBound( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "testPerPurposeModelBound.usda" )

		stage = pxr.Usd.Stage.CreateNew( fileName )
		pxr.UsdGeom.Xform.Define( stage, "/group" )
		cube = pxr.UsdGeom.Cube.Define( stage, "/group/proxy" )
		cube.CreatePurposeAttr().Set( "proxy" )

		bboxCache = pxr.UsdGeom.BBoxCache( pxr.Usd.TimeCode( 0 ), [ "default", "render", "proxy", "guide" ] )
		modelAPI = pxr.UsdGeom.ModelAPI.Apply( stage.GetPrimAtPath( "/group" ) )
		modelAPI.SetExtentsHint( modelAPI.ComputeExtentsHint( bboxCache ) )

		stage.GetRootLayer().Save()
		del stage

		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertFalse( root.hasBound() )

		self.assertTrue( root.child( "group" ).hasBound() )
		self.assertEqual( root.child( "group" ).readBound( 0 ), imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) ) )

	def testSetNameValidation( self ) :

		fileName = os.path.join( self.temporaryDirectory(), "test.usda" )
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		expectedSetNames = {
			"a" : "a",
			"foo" : "foo",
			"foo:includes" : "foo:includes",
			"render:test" : "render:test",
			"render:test:foo" : "render:test:foo",
			"1" : "_1",
			"render:2": "render:_2",
			"" : "_",
		}

		for setIndex, setName in enumerate( expectedSetNames.keys() ) :
			root.writeSet( setName, IECore.PathMatcher( [ f"/set{setIndex}Member" ] ) )

		del root
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		self.assertEqual(
			set( root.setNames() ),
			set( expectedSetNames.values() ) | { "__lights", "usd:pointInstancers", "__cameras" }
		)

		for setIndex, setName in enumerate( expectedSetNames.values() ) :
			with self.subTest( setName = setName ) :
				self.assertEqual( root.readSet( setName ), IECore.PathMatcher( [ f"/set{setIndex}Member" ] ) )

	def testWriteToOpenScene( self ) :

		# Using posix-format filename, because Windows backslashes don't play nicely
		# with `assertRaisesRegex()`.
		fileName = ( pathlib.Path( self.temporaryDirectory() ) / "test.usda" ).as_posix()
		IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		reader = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )

		with self.assertRaisesRegex( RuntimeError, f"USDScene : Failed to open USD stage : '{fileName}'" ) :
			IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

	def testAssetPathSlashes ( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "assetPathAttribute.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		xform = root.child( "xform" )

		self.assertEqual( xform.attributeNames(), [ "render:testAsset" ] )
		self.assertNotIn( "\\", xform.readAttribute( "render:testAsset", 0 ).value )
		self.assertTrue( pathlib.Path( xform.readAttribute( "render:testAsset", 0 ).value ).is_file() )

	def _testPointInstancerRelativePrototypes( self ) :

		root = IECoreScene.SceneInterface.create(
			os.path.join( os.path.dirname( __file__ ), "data", "pointInstancerWeirdPrototypes.usda" ),
			IECore.IndexedIO.OpenMode.Read
		)
		pointInstancer = root.child( "inst" )
		obj = pointInstancer.readObject(0.0)

		if os.environ.get( "IECOREUSD_POINTINSTANCER_RELATIVE_PROTOTYPES", "0" ) != "0" :
			self.assertEqual( obj["prototypeRoots"].data, IECore.StringVectorData( [ './Prototypes/sphere', '/cube' ] ) )
		else :
			self.assertEqual( obj["prototypeRoots"].data, IECore.StringVectorData( [ '/inst/Prototypes/sphere', '/cube' ] ) )

	def testPointInstancerRelativePrototypes( self ) :

		for relative in [ "0", "1", None ] :

			with self.subTest( relative = relative ) :

				env = os.environ.copy()
				if relative is not None :
					env["IECOREUSD_POINTINSTANCER_RELATIVE_PROTOTYPES"] = relative
				else :
					env.pop( "IECOREUSD_POINTINSTANCER_RELATIVE_PROTOTYPES", None )

				try :
					subprocess.check_output(
						[ sys.executable, __file__, "USDSceneTest._testPointInstancerRelativePrototypes" ],
						env = env, stderr = subprocess.STDOUT
					)
				except subprocess.CalledProcessError as e :
					self.fail( e.output )

	@unittest.skipIf( not haveVDB, "No IECoreVDB" )
	def testUsdVolVolumeSlashes( self ) :

		import IECoreVDB

		fileName = os.path.dirname( __file__ ) + "/data/volume.usda"
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		child = root.child( "volume" )

		vdbObject = child.readObject( 0 )
		self.assertNotIn( "\\", vdbObject.fileName() )
		self.assertTrue( pathlib.Path( vdbObject.fileName() ).is_file() )

	@unittest.skipIf( not haveVDB, "No IECoreVDB" )
	def testUsdVolVolumeWithEmptyField( self ) :

		fileName = os.path.dirname( __file__ ) + "/data/volumeWithEmptyField.usda"
		root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertIsNone( root.child( "volume" ).readObject( 0 ) )

if __name__ == "__main__":
	unittest.main()
