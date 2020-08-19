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
import unittest
import tempfile
import math
import imath
import shutil

import IECore
import IECoreScene
import IECoreUSD
import pxr.Usd
import pxr.UsdGeom

class USDSceneWriterTest( unittest.TestCase ) :

	def setUp( self ):
		self.tmpFiles = []

	def tearDown( self ):
		for p, cleanUp in self.tmpFiles:
			if cleanUp:
		 		os.remove( p )

	def getOutputPath( self, filename, cleanUp = True ):
		root, extension = os.path.splitext( filename )
		f, newPath = tempfile.mkstemp( prefix = root + '_', suffix = extension)
		os.close(f)
		if not cleanUp:
			print( newPath )
		self.tmpFiles.append( (newPath, cleanUp, ) )

		return newPath

	def testCanWriteEmptyUSDFile( self ) :

		fileName = self.getOutputPath("usd_empty.usda")

		sceneWrite = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

		del sceneWrite

		sceneRead = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sceneRead.childNames(), [] )

	def testCanWriteTransformHeirarchy ( self ) :

		fileName = self.getOutputPath("usd_transform.usda")

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

		fileName = self.getOutputPath("usd_simple_mesh.usda")

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

		fileName = self.getOutputPath("usd_primvars.usda")

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

		fileName = self.getOutputPath("usd_attributes.usda")

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

		fileName = self.getOutputPath("usd_points.usda")

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

		fileName = self.getOutputPath("usd_curves.usda")

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

		fileName = self.getOutputPath("usd_animated_transforms.usda")

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

		fileName = self.getOutputPath("usd_animated_positions.usda")

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

		fileName = self.getOutputPath("usd_subd.usda")

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

		fileName = self.getOutputPath("usd_animated_primvar.usda")

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

		fileName = self.getOutputPath("usd_various_primvars.usdc")

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

		fileName = self.getOutputPath( "usd_sets.usda" )

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

	def testSets( self ):
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

		writeRoot = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Write )

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

		readRoot = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Read )

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

		writeRoot = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Write )

		A = writeRoot.createChild("A")
		B = A.createChild("B")

		del A, B, writeRoot

		shutil.copyfile('/tmp/test.usda', '/tmp/testAnotherFile.usda')

		readRoot = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Read )
		readRoot2 = IECoreScene.SceneInterface.create( "/tmp/testAnotherFile.usda", IECore.IndexedIO.OpenMode.Read )

		readRoot3 = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Read )

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

		fileName = self.getOutputPath("cameras.usda", cleanUp = False )

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

		root = IECoreScene.SceneInterface.create( "/tmp/test.usdc", IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "cube" )
		child.writeObject( mesh, 0 )
		del root, child

		root = IECoreScene.SceneInterface.create( "/tmp/test.usdc", IECore.IndexedIO.OpenMode.Read )
		child = root.child( "cube" )

		mesh2 = child.readObject( 0 )
		self.assertEqual( mesh.cornerIds(), mesh2.cornerIds() )
		self.assertEqual( mesh.cornerSharpnesses(), mesh2.cornerSharpnesses() )
		self.assertEqual( mesh.creaseLengths(), mesh2.creaseLengths() )
		self.assertEqual( mesh.creaseIds(), mesh2.creaseIds() )
		self.assertEqual( mesh.creaseSharpnesses(), mesh2.creaseSharpnesses() )

	def testMissingBehaviourCreate( self ) :

		scene = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Write )
		scene.child( "test", missingBehaviour = scene.MissingBehaviour.CreateIfMissing ).writeObject( IECoreScene.SpherePrimitive(), 0 )
		del scene

		scene = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( scene.childNames(), [ "test"] )

	def testNameSanitisation( self ) :

		# USD is extremely restrictive about the names prims can have.
		# Test that we sanitise names appropriately when writing files.

		scene = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Write )
		scene.createChild( "0" ).writeObject( IECoreScene.SpherePrimitive(), 0 )
		scene.createChild( "wot:no:colons?" )
		scene.child( "fistFullOf$$$", missingBehaviour = scene.MissingBehaviour.CreateIfMissing ).writeObject( IECoreScene.SpherePrimitive(), 0 )
		del scene

		scene = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( scene.childNames(), [ "_0", "wot_no_colons_", "fistFullOf___" ] )

	def testQuatConversion( self ) :

		# Imath and Gf quaternions do not share the same memory layout.
		# Check that we account for that when writing.

		root = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData() )
		points["quatf"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Constant,
			imath.Quatf( 0, imath.V3f( 1, 2, 3 ) )
		)

		child.writeObject( points, 0 )
		del root, child

		stage = pxr.Usd.Stage.Open( "/tmp/test.usda" )
		primVarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/test" ) )
		quat = primVarsAPI.GetPrimvar( "quatf" ).Get( 0 )
		self.assertEqual( quat, pxr.Gf.Quatf( 0, pxr.Gf.Vec3f( 1, 2, 3 ) ) )

	def testRefCountAfterWrite( self ) :

		root = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Write )
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

	def testUVs( self ) :

		root = IECoreScene.SceneInterface.create( "/tmp/test.usda", IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "test" )
		child.writeObject( IECoreScene.MeshPrimitive.createSphere( 1 ), 0 )
		del root, child

		stage = pxr.Usd.Stage.Open( "/tmp/test.usda" )
		primvarsAPI = pxr.UsdGeom.PrimvarsAPI( stage.GetPrimAtPath( "/test" ) )
		self.assertFalse( primvarsAPI.GetPrimvar( "uv" ) )
		self.assertTrue( primvarsAPI.GetPrimvar( "st" ) )

if __name__ == "__main__":
	unittest.main()
