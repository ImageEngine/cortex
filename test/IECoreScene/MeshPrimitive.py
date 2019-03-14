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
import imath

import IECore
import IECoreScene

class TestMeshPrimitive( unittest.TestCase ) :

	def test( self ) :

		m = IECoreScene.MeshPrimitive()
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 0 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 0 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 0 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 0 )
		self.assertEqual( m.numFaces(), 0 )
		self.assertEqual( m.verticesPerFace, IECore.IntVectorData() )
		self.assertEqual( m.vertexIds, IECore.IntVectorData() )
		self.assertEqual( m.interpolation, "linear" )
		self.assertEqual( m, m.copy() )
		self.assertEqual( m.maxVerticesPerFace(), 0 )

		iface = IECore.IndexedIO.create( "test/IECore/mesh.fio", IECore.IndexedIO.OpenMode.Write )
		m.save( iface, "test" )
		mm = IECore.Object.load( iface, "test" )
		self.assertEqual( m, mm )

		vertsPerFace = IECore.IntVectorData( [ 3, 3 ] )
		vertexIds = IECore.IntVectorData( [ 0, 1, 2, 1, 2, 3 ] )

		m = IECoreScene.MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 2 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 6 )
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
		mm = IECore.Object.load( iface, "test" )
		self.assertEqual( m, mm )

		m.setTopology( m.verticesPerFace, m.vertexIds, "catmullClark" )

		mm = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		self.assert_( mm.arePrimitiveVariablesValid() )

	def testUVsFromFile( self ) :

		# get the original values from a legacy cob file
		f = IECore.FileIndexedIO("test/IECore/data/cobFiles/pSphereShape1.cob", [], IECore.IndexedIO.OpenMode.Read )
		ff = f.directory( [ "object", "data", "Primitive" ] )
		# make sure its a legacy file
		self.assertEqual( ff.read( "ioVersion" ).value, 1 )
		ff = f.directory( [ "object", "data", "Primitive", "data", "variables", "t", "data", "data", "FloatVectorData", "data" ] )
		rawValues = ff.read( "value" )
		self.assertAlmostEqual( rawValues[0], 0.95 )
		self.assertEqual( rawValues[-1], 0 )

		# read legacy file and confirm values are packed into V2f and unflipped
		sphere = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		self.assertTrue( sphere.arePrimitiveVariablesValid() )
		self.assertEqual( sphere["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		self.assertEqual( sphere["uv"].data[0][0], 0 )
		self.assertAlmostEqual( sphere["uv"].data[0][1], 0.05 )
		self.assertAlmostEqual( sphere["uv"].data[-1][0], 0.975 )
		self.assertEqual( sphere["uv"].data[-1][1], 1 )

		# write a new file and confirm values remain consistent
		IECore.Writer.create( sphere, "test/IECore/mesh.cob" ).write()
		newSphere = IECore.Reader.create( "test/IECore/mesh.cob" ).read()
		self.assertTrue( newSphere.arePrimitiveVariablesValid() )
		self.assertEqual( newSphere["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		self.assertEqual( newSphere["uv"].data[0][0], 0 )
		self.assertAlmostEqual( newSphere["uv"].data[0][1], 0.05 )
		self.assertAlmostEqual( newSphere["uv"].data[-1][0], 0.975 )
		self.assertEqual( newSphere["uv"].data[-1][1], 1 )
		# make sure it matches the original
		self.assertEqual( newSphere, sphere )

		# get the original values from a legacy SceneCache file
		f = IECore.FileIndexedIO("test/IECore/data/sccFiles/animatedSpheres.scc", [], IECore.IndexedIO.OpenMode.Read )
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
		s = IECoreScene.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
		ss = s.scene( [ "A", "a" ] )
		animSphere = ss.readObject( 0 )
		self.assertEqual( animSphere["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		self.assertEqual( animSphere["uv"].data[0][0], 0 )
		self.assertAlmostEqual( animSphere["uv"].data[0][1], 0.05 )
		self.assertAlmostEqual( animSphere["uv"].data[-1][0], 0.975 )
		self.assertEqual( animSphere["uv"].data[-1][1], 1 )
		self.assertEqual( animSphere["uv"].indices[0], 0 )
		self.assertEqual( animSphere["uv"].indices[-1], 438 )
		self.assertEqual( len(animSphere["uv"].data), 439 )
		self.assertEqual( len(animSphere["uv"].indices), 1560 )
		# and the duplicate UV Set matches
		self.assertEqual( animSphere["map1"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
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

		m = IECoreScene.MeshPrimitive()
		self.assertEqual( m.interpolation, "linear" )
		m.interpolation = "catmullClark"
		self.assertEqual( m.interpolation, "catmullClark" )

	def testEmptyMeshConstructor( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData(), IECore.IntVectorData(), "linear", IECore.V3fVectorData() )
		self.assertEqual( m["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assert_( m.arePrimitiveVariablesValid() )

	def testEqualityOfEmptyMeshes( self ) :

		self.assertEqual( IECoreScene.MeshPrimitive(), IECoreScene.MeshPrimitive() )

	def testVerticesPerFace( self ) :

		vertexIds = IECore.IntVectorData( [ 0, 1, 2, 1, 2, 3, 4 ] )

		vertsPerFace = IECore.IntVectorData( [ 3, 4 ] )
		m = IECoreScene.MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		self.assertEqual( m.minVerticesPerFace(), 3 )
		self.assertEqual( m.maxVerticesPerFace(), 4 )

		vertsPerFace = IECore.IntVectorData( [ 4, 3 ] )
		m = IECoreScene.MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		self.assertEqual( m.minVerticesPerFace(), 3 )
		self.assertEqual( m.maxVerticesPerFace(), 4 )

	def testHash( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData(), IECore.IntVectorData(), "linear", IECore.V3fVectorData() )
		h = m.hash()
		t = m.topologyHash()

		m2 = m.copy()
		self.assertEqual( m2.hash(), h )
		self.assertEqual( m2.topologyHash(), t )

		m.setTopology( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ), "linear" )
		self.assertNotEqual( m.hash(), h )
		self.assertNotEqual( m.topologyHash(), t )
		h = m.hash()
		t = m.topologyHash()

		m.setTopology( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 2, 1 ] ), "linear" )
		self.assertNotEqual( m.hash(), h )
		self.assertNotEqual( m.topologyHash(), t )
		h = m.hash()
		t = m.topologyHash()

		m.setTopology( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 2, 1 ] ), "catmullClark" )
		self.assertNotEqual( m.hash(), h )
		self.assertNotEqual( m.topologyHash(), t )
		h = m.hash()
		t = m.topologyHash()

		m["primVar"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.IntData( 10 ) )
		self.assertNotEqual( m.hash(), h )
		self.assertEqual( m.topologyHash(), t )

	def testBox( self ) :

		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 6 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 8 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 24 )
		self.assertEqual( m.numFaces(), 6 )
		self.assertEqual( m.verticesPerFace, IECore.IntVectorData( [ 4 ] * 6 ) )
		self.assertEqual( m.bound(), imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )

		# verify uvs
		self.assertEqual( m["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertGreater( len( m["uv"].data ), len( m["P"].data ) )
		self.assertLess( len( m["uv"].data ), m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) )
		self.assertEqual( len( m["uv"].indices ), m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) )

		# verify normals
		self.assertEqual( m["N"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( len( m["N"].data ), 6 )
		self.assertEqual( len( m["uv"].indices ), m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) )


	def testPlane( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 4 )
		self.assertEqual( m.numFaces(), 1 )
		self.assertEqual( m.verticesPerFace, IECore.IntVectorData( [ 4 ] ) )
		self.assertEqual( m.vertexIds, IECore.IntVectorData( [ 0, 1, 3, 2 ] ) )
		self.assertEqual( m.bound(), imath.Box3f( imath.V3f( 0 ), imath.V3f( 1, 1, 0 ) ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )

		# verify uvs
		self.assertEqual( m["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( len( m["uv"].data ), len( m["P"].data ) )
		self.assertEqual( m["uv"].indices, m.vertexIds )

		e = IECoreScene.MeshPrimitiveEvaluator( IECoreScene.TriangulateOp()( input = m ) )

		r = e.createResult()
		self.assertTrue( e.pointAtUV( imath.V2f( 0, 0 ), r ) )
		self.assertEqual( r.point(), m["P"].data[0] )
		self.assertEqual( r.point(), imath.V3f( 0, 0, 0 ) )
		self.assertTrue( e.pointAtUV( imath.V2f( 1, 0 ), r ) )
		self.assertEqual( r.point(), m["P"].data[1] )
		self.assertEqual( r.point(), imath.V3f( 1, 0, 0 ) )
		self.assertTrue( e.pointAtUV( imath.V2f( 1, 1 ), r ) )
		self.assertEqual( r.point(), m["P"].data[3] )
		self.assertEqual( r.point(), imath.V3f( 1, 1, 0 ) )
		self.assertTrue( e.pointAtUV( imath.V2f( 0, 1 ), r ) )
		self.assertEqual( r.point(), m["P"].data[2] )
		self.assertEqual( r.point(), imath.V3f( 0, 1, 0 ) )

		# test divisions
		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ), divisions = imath.V2i( 2, 3 ) )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 6 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 12 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 12 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 24 )
		self.assertEqual( m.numFaces(), 6 )
		self.assertEqual( m.verticesPerFace, IECore.IntVectorData( [ 4 ] * 6 ) )
		self.assertEqual( m.vertexIds, IECore.IntVectorData( [ 0, 1, 4, 3, 1, 2, 5, 4, 3, 4, 7, 6, 4, 5, 8, 7, 6, 7, 10, 9, 7, 8, 11, 10 ] ) )
		self.assertEqual( m.bound(), imath.Box3f( imath.V3f( 0 ), imath.V3f( 1, 1, 0 ) ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )

		# corners still have correct uvs
		e = IECoreScene.MeshPrimitiveEvaluator( IECoreScene.TriangulateOp()( input = m ) )
		r = e.createResult()
		self.assertTrue( e.pointAtUV( imath.V2f( 0, 0 ), r ) )
		self.assertEqual( r.point(), m["P"].data[0] )
		self.assertEqual( r.point(), imath.V3f( 0, 0, 0 ) )
		self.assertTrue( e.pointAtUV( imath.V2f( 1, 0 ), r ) )
		self.assertEqual( r.point(), m["P"].data[2] )
		self.assertEqual( r.point(), imath.V3f( 1, 0, 0 ) )
		self.assertTrue( e.pointAtUV( imath.V2f( 1, 1 ), r ) )
		self.assertEqual( r.point(), m["P"].data[11] )
		self.assertEqual( r.point(), imath.V3f( 1, 1, 0 ) )
		self.assertTrue( e.pointAtUV( imath.V2f( 0, 1 ), r ) )
		self.assertEqual( r.point(), m["P"].data[9] )
		self.assertEqual( r.point(), imath.V3f( 0, 1, 0 ) )

	def testSphere( self ) :

		m = IECoreScene.MeshPrimitive.createSphere( radius = 1, divisions = imath.V2i( 30, 40 ) )
		self.assertTrue( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ), m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		me = IECoreScene.PrimitiveEvaluator.create( IECoreScene.TriangulateOp()( input = m ) )
		mer = me.createResult()
		s = IECoreScene.SpherePrimitive( 1 )
		se = IECoreScene.PrimitiveEvaluator.create( s )
		ser = se.createResult()
		for s in range( 0, 100 ) :
			for t in range( 0, 100 ) :
				uv = imath.V2f( s / 100.0, t / 100.0 )
				if not me.pointAtUV( uv, mer ) :
					# Not all UV coordinates are covered,
					# due to the row of triangles at the
					# poles.
					continue
				self.assertTrue( se.pointAtUV( uv, ser ) )
				self.assertTrue( mer.point().equalWithAbsError( ser.point(), 1e-2 ) )

		# test divisions
		m = IECoreScene.MeshPrimitive.createSphere( radius = 1, divisions = imath.V2i( 300, 300 ) )
		self.assertTrue( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ), m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		me = IECoreScene.PrimitiveEvaluator.create( IECoreScene.TriangulateOp()( input = m ) )
		mer = me.createResult()
		for s in range( 0, 100 ) :
			for t in range( 0, 100 ) :
				uv = imath.V2f( s / 100.0, t / 100.0 )
				if not me.pointAtUV( uv, mer ) :
					# Not all UV coordinates are covered,
					# due to the row of triangles at the
					# poles.
					continue
				self.assertTrue( se.pointAtUV( uv, ser ) )
				# more divisions means points are closer to ground truth
				self.assertTrue( mer.point().equalWithAbsError( ser.point(), 1e-4 ) )

		# test radius
		m = IECoreScene.MeshPrimitive.createSphere( radius = 2, divisions = imath.V2i( 30, 40 ) )
		self.assertFalse( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ), m.bound() ) )
		self.assertTrue( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -2 ), imath.V3f( 2 ) ), m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		me = IECoreScene.PrimitiveEvaluator.create( IECoreScene.TriangulateOp()( input = m ) )
		mer = me.createResult()
		s = IECoreScene.SpherePrimitive( 2 )
		se = IECoreScene.PrimitiveEvaluator.create( s )
		ser = se.createResult()
		for s in range( 0, 100 ) :
			for t in range( 0, 100 ) :
				uv = imath.V2f( s / 100.0, t / 100.0 )
				if not me.pointAtUV( uv, mer ) :
					# Not all UV coordinates are covered,
					# due to the row of triangles at the
					# poles.
					continue
				self.assertTrue( se.pointAtUV( uv, ser ) )
				self.assertTrue( mer.point().equalWithAbsError( ser.point(), 1e-2 ) )

		# test zMin/zMax
		m = IECoreScene.MeshPrimitive.createSphere( radius = 1, zMin = -0.75, zMax = 0.75 )
		self.assertTrue( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -1, -1, -0.75 ), imath.V3f( 1, 1, 0.75 ) ), m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )

		# test thetaMax
		m = IECoreScene.MeshPrimitive.createSphere( radius = 1, thetaMax = 300 )
		self.assertTrue( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ), m.bound() ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )
		m2 = IECoreScene.MeshPrimitive.createSphere( radius = 1 )
		self.assertTrue( m.numFaces() < m2.numFaces() )

	def testLegacyIndices( self ) :

		# load a legacy file that contains myString and myStringIndices
		m = IECore.Reader.create( "test/IECore/data/cobFiles/cube.cob" ).read()
		self.assertEqual( m.keys(), [ "P", "myString" ] )
		self.assertTrue( m.isPrimitiveVariableValid( m["myString"] ) )
		self.assertEqual( m["myString"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m["myString"].data, IECore.StringVectorData( [ "indexed", "my", "string" ] ) )
		self.assertEqual( m["myString"].indices, IECore.IntVectorData( [ 1, 0, 2, 1, 0, 2, 1, 0 ] ) )

	def testSphereIndexing( self ) :

		m = IECoreScene.MeshPrimitive.createSphere( radius = 1, divisions = imath.V2i( 3, 6 ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )

		# We expect 18 faces. A row of 6 triangles at each cap, and a row of 6 quads
		# in the middle.

		self.assertEqual(
			m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ),
			18
		)

		self.assertEqual( m.verticesPerFace.count( 3 ), 12 )
		self.assertEqual( m.verticesPerFace.count( 4 ), 6 )

		# We expect 14 vertices. Two rings of 6 in the middle, and one
		# for each of the poles.

		self.assertEqual( len( m["P"].data ), 14 )
		self.assertEqual(
			m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ),
			14
		)

		# UVs are slightly different because there's a seam.
		# We expect 6 uvs at each pole (for different values of u),
		# and 7 for each ring in between.

		self.assertEqual( m["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( len( m["uv"].data ), 6 * 2 + 7 * 2 )

	def testDefaultCornersAndCreases( self ) :

		m = IECoreScene.MeshPrimitive()

		self.assertEqual( m.cornerIds(), IECore.IntVectorData() )
		self.assertEqual( m.cornerSharpnesses(), IECore.FloatVectorData() )

		self.assertEqual( m.creaseLengths(), IECore.IntVectorData() )
		self.assertEqual( m.creaseIds(), IECore.IntVectorData() )
		self.assertEqual( m.creaseSharpnesses(), IECore.FloatVectorData() )

	def testSetCorners( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )

		with self.assertRaisesRegexp( Exception, r"Bad corners : id \(-1\) is out of expected range \(0-3\)" ) :
			m.setCorners( IECore.IntVectorData( [ -1 ] ), IECore.FloatVectorData( 2 ) )

		with self.assertRaisesRegexp( Exception, r"Bad corners : number of sharpnesses \(2\) does not match number of ids \(3\)" ) :
			m.setCorners( IECore.IntVectorData( [ 0, 1, 2 ] ), IECore.FloatVectorData( [ 1, 2 ] ) )

		ids = IECore.IntVectorData( [ 0 ] )
		sharpnesses = IECore.FloatVectorData( [ 2 ]  )

		m.setCorners( ids, sharpnesses )
		self.assertEqual( m.cornerIds(), ids )
		self.assertEqual( m.cornerSharpnesses(), sharpnesses )
		self.assertFalse( m.cornerIds().isSame( ids ) )
		self.assertFalse( m.cornerSharpnesses().isSame( sharpnesses ) )

		m2 = m.copy()
		self.assertEqual( m2, m )
		self.assertEqual( m2.hash(), m.hash() )

		m2.removeCorners()
		self.assertNotEqual( m2, m )
		self.assertNotEqual( m2.hash(), m.hash() )

	def testSetCreases( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )

		with self.assertRaisesRegexp( Exception, r"Bad creases : length \(1\) is less than 2" ) :
			m.setCreases( IECore.IntVectorData( [ 1 ] ), IECore.IntVectorData( [ 1 ] ), IECore.FloatVectorData( 2 ) )

		with self.assertRaisesRegexp( Exception, r"Bad creases : id \(-1\) is out of expected range \(0-3\)" ) :
			m.setCreases( IECore.IntVectorData( [ 2 ] ), IECore.IntVectorData( [ -1, 2 ] ), IECore.FloatVectorData( 2 ) )

		with self.assertRaisesRegexp( Exception, r"Bad creases : expected 3 ids but given 2" ) :
			m.setCreases( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 1, 2 ] ), IECore.FloatVectorData( 2 ) )

		with self.assertRaisesRegexp( Exception, r"Bad creases : number of sharpnesses \(2\) does not match number of lengths \(1\)" ) :
			m.setCreases( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ), IECore.FloatVectorData( [ 2, 4 ] ) )

		lengths = IECore.IntVectorData( [ 3 ] )
		ids = IECore.IntVectorData( [ 0, 1, 2 ] )
		sharpnesses = IECore.FloatVectorData( [ 4 ] )

		m.setCreases( lengths, ids, sharpnesses )
		self.assertEqual( m.creaseLengths(), lengths )
		self.assertEqual( m.creaseIds(), ids )
		self.assertEqual( m.creaseSharpnesses(), sharpnesses )
		self.assertFalse( m.creaseLengths().isSame( lengths ) )
		self.assertFalse( m.creaseIds().isSame( ids ) )
		self.assertFalse( m.creaseSharpnesses().isSame( sharpnesses ) )

		m2 = m.copy()
		self.assertEqual( m2, m )
		self.assertEqual( m2.hash(), m.hash() )

		m2.removeCreases()
		self.assertNotEqual( m2, m )
		self.assertNotEqual( m2.hash(), m.hash() )

	def testSaveAndLoadCorners( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		m.setCreases( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ), IECore.FloatVectorData( [ 4 ] ) )
		m.setCorners( IECore.IntVectorData( [ 3 ] ), IECore.FloatVectorData( [ 5 ] ) )

		io = IECore.MemoryIndexedIO( IECore.CharVectorData(), [], IECore.IndexedIO.OpenMode.Append )

		m.save( io, "test" )
		m2 = IECore.Object.load( io, "test" )
		self.assertEqual( m, m2 )

	def tearDown( self ) :

		for f in (
			"test/IECore/mesh.fio",
			"test/IECore/mesh.cob",
		) :
			if os.path.isfile( f ) :
				os.remove( f )

if __name__ == "__main__":
    unittest.main()
