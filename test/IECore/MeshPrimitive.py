##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

from IECore import *

class TestMeshPrimitive( unittest.TestCase ) :

	def test( self ) :

		m = MeshPrimitive()
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Uniform ), 0 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Vertex ), 0 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Varying ), 0 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 0 )
		self.assertEqual( m.numFaces(), 0 )
		self.assertEqual( m.verticesPerFace, IntVectorData() )
		self.assertEqual( m.vertexIds, IntVectorData() )
		self.assertEqual( m.interpolation, "linear" )
		self.assertEqual( m, m.copy() )
		self.assertEqual( m.maxVerticesPerFace(), 0 )

		iface = IndexedIO.create( "test/IECore/mesh.fio", IndexedIO.OpenMode.Write )
		m.save( iface, "test" )
		mm = Object.load( iface, "test" )
		self.assertEqual( m, mm )

		vertsPerFace = IntVectorData( [ 3, 3 ] )
		vertexIds = IntVectorData( [ 0, 1, 2, 1, 2, 3 ] )

		m = MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Uniform ), 2 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 6 )
		self.assertEqual( m.numFaces(), 2 )
		self.assertEqual( m.verticesPerFace, vertsPerFace )
		self.assertEqual( m.maxVerticesPerFace(), 3 )
		self.assertEqual( m.minVerticesPerFace(), 3 )
		self.assert_( not m.verticesPerFace.isSame( vertsPerFace ) )
		self.assertEqual( m.vertexIds, vertexIds )
		self.assert_( not m.vertexIds.isSame( vertexIds ) )
		self.assertEqual( m.interpolation, "catmullClark" )
		self.assertEqual( m, m.copy() )
		m.save( iface, "test" )
		mm = Object.load( iface, "test" )
		self.assertEqual( m, mm )

		m.setTopology( m.verticesPerFace, m.vertexIds, "catmullClark" )

		mm = Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		self.assert_( mm.arePrimitiveVariablesValid() )

	def testUVsFromFile( self ) :

		# get the original values from a legacy cob file
		f = FileIndexedIO("test/IECore/data/cobFiles/pSphereShape1.cob", [], IndexedIO.OpenMode.Read )
		ff = f.directory( [ "object", "data", "Primitive" ] )
		# make sure its a legacy file
		self.assertEqual( ff.read( "ioVersion" ).value, 1 )
		ff = f.directory( [ "object", "data", "Primitive", "data", "variables", "t", "data", "data", "FloatVectorData", "data" ] )
		rawValues = ff.read( "value" )
		self.assertAlmostEqual( rawValues[0], 0.95 )
		self.assertEqual( rawValues[-1], 0 )

		# read legacy file and confirm values are packed into V2f and unflipped
		sphere = Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		self.assertTrue( sphere.arePrimitiveVariablesValid() )
		self.assertEqual( sphere["uv"].data[0][0], 0 )
		self.assertAlmostEqual( sphere["uv"].data[0][1], 0.05 )
		self.assertAlmostEqual( sphere["uv"].data[-1][0], 0.975 )
		self.assertEqual( sphere["uv"].data[-1][1], 1 )

		# write a new file and confirm values remain consistent
		Writer.create( sphere, "test/IECore/mesh.cob" ).write()
		newSphere = Reader.create( "test/IECore/mesh.cob" ).read()
		self.assertTrue( newSphere.arePrimitiveVariablesValid() )
		self.assertEqual( newSphere["uv"].data[0][0], 0 )
		self.assertAlmostEqual( newSphere["uv"].data[0][1], 0.05 )
		self.assertAlmostEqual( newSphere["uv"].data[-1][0], 0.975 )
		self.assertEqual( newSphere["uv"].data[-1][1], 1 )
		# make sure it matches the original
		self.assertEqual( newSphere, sphere )

		# get the original values from a legacy SceneCache file
		f = FileIndexedIO("test/IECore/data/sccFiles/animatedSpheres.scc", [], IndexedIO.OpenMode.Read )
		ff = f.directory( [ "root", "children", "A", "children", "a", "object", "0", "data", "Primitive" ] )
		# make sure its a legacy file
		self.assertEqual( ff.read( "ioVersion" ).value, 1 )
		# note this mesh has duplicate UVs pointing to the same
		# raw data, so we only need to verify the true values
		ff = f.directory( [ "root", "children", "A", "children", "a", "object", "0", "data", "Primitive", "data", "variables", "map1_t", "data", "data" ] )
		rawMap1Values = ff.read( "value" )
		self.assertAlmostEqual( rawMap1Values[0], 0.95 )
		self.assertEqual( rawMap1Values[-1], 0 )
		self.assertEqual( len(rawMap1Values), 1560 )
		# it also has indices
		ff = f.directory( [ "root", "children", "A", "children", "a", "object", "0", "data", "Primitive", "data", "variables", "map1Indices", "data", "data" ] )
		rawMap1Indices = ff.read( "value" )
		self.assertEqual( rawMap1Indices[0], 0 )
		self.assertEqual( rawMap1Indices[-1], 438 )
		self.assertEqual( max(rawMap1Indices), 438 )
		self.assertEqual( len(rawMap1Indices), 1560 )

		# read legacy file and confirm values are packed into V2f and unflipped
		s = SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IndexedIO.OpenMode.Read )
		ss = s.scene( [ "A", "a" ] )
		animSphere = ss.readObject( 0 )
		self.assertEqual( animSphere["uv"].data[0][0], 0 )
		self.assertAlmostEqual( animSphere["uv"].data[0][1], 0.05 )
		self.assertAlmostEqual( animSphere["uv"].data[-1][0], 0.975 )
		self.assertEqual( animSphere["uv"].data[-1][1], 1 )
		self.assertEqual( animSphere["uv"].indices[0], 0 )
		self.assertEqual( animSphere["uv"].indices[-1], 438 )
		self.assertEqual( len(animSphere["uv"].data), 439 )
		self.assertEqual( len(animSphere["uv"].indices), 1560 )
		# and the duplicate UV Set matches
		self.assertEqual( animSphere["map1"].data[0][0], 0 )
		self.assertAlmostEqual( animSphere["map1"].data[0][1], 0.05 )
		self.assertAlmostEqual( animSphere["map1"].data[-1][0], 0.975 )
		self.assertEqual( animSphere["map1"].data[-1][1], 1 )
		self.assertEqual( animSphere["map1"].indices[0], 0 )
		self.assertEqual( animSphere["map1"].indices[-1], 438 )
		self.assertEqual( len(animSphere["map1"].data), 439 )
		self.assertEqual( len(animSphere["map1"].indices), 1560 )

		# even for lower level loading from a SceneCache
		# note that the new uvSet names must be specified.
		justPrimVars = ss.readObjectPrimitiveVariables( [ "uv", "map1" ], 0 )
		self.assertEqual( animSphere["uv"], justPrimVars["uv"] )
		self.assertEqual( animSphere["map1"], justPrimVars["map1"] )
		self.assertEqual( animSphere["uv"], justPrimVars["map1"] )

	def testSetInterpolation( self ) :

		m = MeshPrimitive()
		self.assertEqual( m.interpolation, "linear" )
		m.interpolation = "catmullClark"
		self.assertEqual( m.interpolation, "catmullClark" )

	def testEmptyMeshConstructor( self ) :

		m = MeshPrimitive( IntVectorData(), IntVectorData(), "linear", V3fVectorData() )
		self.assertEqual( m["P"].data.getInterpretation(), GeometricData.Interpretation.Point )
		self.assert_( m.arePrimitiveVariablesValid() )

	def testEqualityOfEmptyMeshes( self ) :
	
		self.assertEqual( MeshPrimitive(), MeshPrimitive() )

	def testVerticesPerFace( self ) :

		vertexIds = IntVectorData( [ 0, 1, 2, 1, 2, 3, 4 ] )

		vertsPerFace = IntVectorData( [ 3, 4 ] )
		m = MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		self.assertEqual( m.minVerticesPerFace(), 3 )
		self.assertEqual( m.maxVerticesPerFace(), 4 )		

		vertsPerFace = IntVectorData( [ 4, 3 ] )
		m = MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		self.assertEqual( m.minVerticesPerFace(), 3 )
		self.assertEqual( m.maxVerticesPerFace(), 4 )		
		
	def testHash( self ) :
	
		m = MeshPrimitive( IntVectorData(), IntVectorData(), "linear", V3fVectorData() )
		h = m.hash()
		t = m.topologyHash()
		
		m2 = m.copy()
		self.assertEqual( m2.hash(), h )
		self.assertEqual( m2.topologyHash(), t )
		
		m.setTopology( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 1, 2 ] ), "linear" )
		self.assertNotEqual( m.hash(), h )
		self.assertNotEqual( m.topologyHash(), t )
		h = m.hash()
		t = m.topologyHash()
		
		m.setTopology( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 2, 1 ] ), "linear" )
		self.assertNotEqual( m.hash(), h )
		self.assertNotEqual( m.topologyHash(), t )
		h = m.hash()
		t = m.topologyHash()
		
		m.setTopology( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 2, 1 ] ), "catmullClark" )
		self.assertNotEqual( m.hash(), h )
		self.assertNotEqual( m.topologyHash(), t )
		h = m.hash()
		t = m.topologyHash()
		
		m["primVar"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, IntData( 10 ) )
		self.assertNotEqual( m.hash(), h )
		self.assertEqual( m.topologyHash(), t )
	
	def testBox( self ) :
		
		m = MeshPrimitive.createBox( Box3f( V3f( 0 ), V3f( 1 ) ) )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Uniform ), 6 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Varying ), 8 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 24 )
		self.assertEqual( m.numFaces(), 6 )
		self.assertEqual( m.verticesPerFace, IntVectorData( [ 4 ] * 6 ) )
		self.assertEqual( m.bound(), Box3f( V3f( 0 ), V3f( 1 ) ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
	
	def testPlane( self ) :
		
		m = MeshPrimitive.createPlane( Box2f( V2f( 0 ), V2f( 1 ) ) )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 4 )
		self.assertEqual( m.numFaces(), 1 )
		self.assertEqual( m.verticesPerFace, IntVectorData( [ 4 ] ) )
		self.assertEqual( m.vertexIds, IntVectorData( [ 0, 1, 3, 2 ] ) )
		self.assertEqual( m.bound(), Box3f( V3f( 0 ), V3f( 1, 1, 0 ) ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		
		# verify uvs
		e = MeshPrimitiveEvaluator( TriangulateOp()( input = m ) )
		r = e.createResult()
		self.assertTrue( e.pointAtUV( V2f( 0, 0 ), r ) )
		self.assertEqual( r.point(), m["P"].data[0] )
		self.assertEqual( r.point(), V3f( 0, 0, 0 ) )
		self.assertTrue( e.pointAtUV( V2f( 1, 0 ), r ) )
		self.assertEqual( r.point(), m["P"].data[1] )
		self.assertEqual( r.point(), V3f( 1, 0, 0 ) )
		self.assertTrue( e.pointAtUV( V2f( 1, 1 ), r ) )
		self.assertEqual( r.point(), m["P"].data[3] )
		self.assertEqual( r.point(), V3f( 1, 1, 0 ) )
		self.assertTrue( e.pointAtUV( V2f( 0, 1 ), r ) )
		self.assertEqual( r.point(), m["P"].data[2] )
		self.assertEqual( r.point(), V3f( 0, 1, 0 ) )
		
		# test divisions
		m = MeshPrimitive.createPlane( Box2f( V2f( 0 ), V2f( 1 ) ), divisions = V2i( 2, 3 ) )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Uniform ), 6 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Vertex ), 12 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Varying ), 12 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 24 )
		self.assertEqual( m.numFaces(), 6 )
		self.assertEqual( m.verticesPerFace, IntVectorData( [ 4 ] * 6 ) )
		self.assertEqual( m.vertexIds, IntVectorData( [ 0, 1, 4, 3, 1, 2, 5, 4, 3, 4, 7, 6, 4, 5, 8, 7, 6, 7, 10, 9, 7, 8, 11, 10 ] ) )
		self.assertEqual( m.bound(), Box3f( V3f( 0 ), V3f( 1, 1, 0 ) ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		
		# corners still have correct uvs
		e = MeshPrimitiveEvaluator( TriangulateOp()( input = m ) )
		r = e.createResult()
		self.assertTrue( e.pointAtUV( V2f( 0, 0 ), r ) )
		self.assertEqual( r.point(), m["P"].data[0] )
		self.assertEqual( r.point(), V3f( 0, 0, 0 ) )
		self.assertTrue( e.pointAtUV( V2f( 1, 0 ), r ) )
		self.assertEqual( r.point(), m["P"].data[2] )
		self.assertEqual( r.point(), V3f( 1, 0, 0 ) )
		self.assertTrue( e.pointAtUV( V2f( 1, 1 ), r ) )
		self.assertEqual( r.point(), m["P"].data[11] )
		self.assertEqual( r.point(), V3f( 1, 1, 0 ) )
		self.assertTrue( e.pointAtUV( V2f( 0, 1 ), r ) )
		self.assertEqual( r.point(), m["P"].data[9] )
		self.assertEqual( r.point(), V3f( 0, 1, 0 ) )

	def testSphere( self ) :
		
		m = MeshPrimitive.createSphere( radius = 1, divisions = V2i( 30, 40 ) )
		self.assertTrue( Box3f( V3f( -1 ), V3f( 1 ) ).contains( m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		me = PrimitiveEvaluator.create( m )
		mer = me.createResult()
		s = SpherePrimitive( 1 )
		se = PrimitiveEvaluator.create( s )
		ser = se.createResult()
		for s in range( 0, 100 ) :
			for t in range( 0, 100 ) :
				me.pointAtUV( V2f( s/100., t/100. ), mer )
				se.pointAtUV( V2f( s/100., t/100. ), ser )
				self.assertTrue( mer.point().equalWithAbsError( ser.point(), 1e-2 ) )
		
		# test divisions
		m = MeshPrimitive.createSphere( radius = 1, divisions = V2i( 300, 300 ) )
		self.assertTrue( Box3f( V3f( -1 ), V3f( 1 ) ).contains( m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		me = PrimitiveEvaluator.create( m )
		mer = me.createResult()
		for s in range( 0, 100 ) :
			for t in range( 0, 100 ) :
				me.pointAtUV( V2f( s/100., t/100. ), mer )
				se.pointAtUV( V2f( s/100., t/100. ), ser )
				# more divisions means points are closer to ground truth
				self.assertTrue( mer.point().equalWithAbsError( ser.point(), 1e-4 ) )
		
		# test radius
		m = MeshPrimitive.createSphere( radius = 2, divisions = V2i( 30, 40 ) )
		self.assertFalse( Box3f( V3f( -1 ), V3f( 1 ) ).contains( m.bound() ) )
		self.assertTrue( Box3f( V3f( -2 ), V3f( 2 ) ).contains( m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		me = PrimitiveEvaluator.create( m )
		mer = me.createResult()
		s = SpherePrimitive( 2 )
		se = PrimitiveEvaluator.create( s )
		ser = se.createResult()
		for s in range( 0, 100 ) :
			for t in range( 0, 100 ) :
				me.pointAtUV( V2f( s/100., t/100. ), mer )
				se.pointAtUV( V2f( s/100., t/100. ), ser )
				self.assertTrue( mer.point().equalWithAbsError( ser.point(), 1e-2 ) )
		
		# test zMin/zMax
		m = MeshPrimitive.createSphere( radius = 1, zMin = -0.75, zMax = 0.75 )
		self.assertTrue( Box3f( V3f( -1, -1, -0.75 ), V3f( 1, 1, 0.75 ) ).contains( m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		
		# test thetaMax
		m = MeshPrimitive.createSphere( radius = 1, thetaMax = 300 )
		self.assertTrue( Box3f( V3f( -1 ), V3f( 1 ) ).contains( m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		m2 = MeshPrimitive.createSphere( radius = 1 )
		self.assertTrue( m.numFaces() < m2.numFaces() )
	
	def testLegacyIndices( self ) :

		# load a legacy file that contains myString and myStringIndices
		m = Reader.create( "test/IECore/data/cobFiles/cube.cob" ).read()
		self.assertEqual( m.keys(), [ "P", "myString" ] )
		self.assertTrue( m.isPrimitiveVariableValid( m["myString"] ) )
		self.assertEqual( m["myString"].interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m["myString"].data, StringVectorData( [ "indexed", "my", "string" ] ) )
		self.assertEqual( m["myString"].indices, IntVectorData( [ 1, 0, 2, 1, 0, 2, 1, 0 ] ) )

	def tearDown( self ) :

		for f in (
			"test/IECore/mesh.fio",
			"test/IECore/mesh.cob",
		) :
			if os.path.isfile( f ) :
				os.remove( f )

if __name__ == "__main__":
    unittest.main()
