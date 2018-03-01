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
import imath

import IECore
import IECoreScene
import IECoreUSD

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
			print newPath
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

		uvData = IECore.V2fVectorData( [imath.V2f( 1.0, 2.0 )] )
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

		self.assertEqual(roundTripData, IECore.V2fVectorData( [ imath.V2f( 1.0, 2.0) ] ) )
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

		del root
		del sceneWrite

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


if __name__ == "__main__":
	unittest.main()