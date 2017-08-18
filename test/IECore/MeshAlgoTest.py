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

import random
import unittest

import IECore
from IECore import *

class MeshAlgoTangentTest( unittest.TestCase ) :

	@classmethod
	def makeSingleTriangleMesh( self ):

		verticesPerFace = IntVectorData( [ 3 ] )
		vertexIds = IntVectorData( [ 0, 1, 2 ] )
		p = V3fVectorData( [ V3f( 0, 0, 0 ), V3f( 1, 0, 0 ), V3f( 0, 1, 0 ) ] )
		s = FloatVectorData( [ 0, 1, 0 ] )
		t = FloatVectorData( [ 0, 0, 1 ] )

		mesh = MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["s"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, s )
		mesh["t"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, t )

		mesh["foo_s"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, FloatVectorData( [0, 0, 1] ) )
		mesh["foo_t"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, FloatVectorData( [0, 1, 0] ) )

		prefData = V3fVectorData( [V3f( 0, 0, 0 ), V3f( 0, -1, 0 ), V3f( 1, 0, 0 )] )
		mesh["Pref"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, prefData )

		return mesh

	def makeSingleBadUVTriangleMesh( self ) :

		verticesPerFace = IntVectorData( [3] )
		vertexIds = IntVectorData( [0, 1, 2] )
		p = V3fVectorData( [V3f( 0, 0, 0 ), V3f( 1, 0, 0 ), V3f( 0, 1, 0 )] )
		s = FloatVectorData( [0] )
		t = FloatVectorData( [0] )

		mesh = MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["s"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, s )
		mesh["t"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, t )

		return mesh

	def testSingleTriangleGeneratesCorrectTangents( self ) :
		triangle = self.makeSingleTriangleMesh()
		tangentPrimVar, bitangentPrimVar = MeshAlgo.calculateTangents( triangle )

		self.assertEqual(tangentPrimVar.interpolation, PrimitiveVariable.Interpolation.FaceVarying)
		self.assertEqual(bitangentPrimVar.interpolation, PrimitiveVariable.Interpolation.FaceVarying)

		tangents = V3fVectorData( tangentPrimVar.data )
		bitangent = V3fVectorData( bitangentPrimVar.data )

		self.assertEqual( len( tangents ), 3 )
		self.assertEqual( len( bitangent ), 3 )

		for t in tangents :
			self.assertAlmostEqual( t[0], 1.0 )
			self.assertAlmostEqual( t[1], 0.0 )
			self.assertAlmostEqual( t[2], 0.0 )

		for b in bitangent :
			self.assertAlmostEqual( b[0], 0.0 )
			self.assertAlmostEqual( b[1], 1.0 )
			self.assertAlmostEqual( b[2], 0.0 )


	def testJoinedUVEdges( self ) :

		mesh = ObjectReader( "test/IECore/data/cobFiles/twoTrianglesWithSharedUVs.cob" ).read()
		self.assert_( mesh.arePrimitiveVariablesValid() )

		tangentPrimVar, bitangentPrimVar = MeshAlgo.calculateTangents( mesh )

		self.assertEqual( tangentPrimVar.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( bitangentPrimVar.interpolation, PrimitiveVariable.Interpolation.FaceVarying )

		for v in tangentPrimVar.data :
			self.failUnless( v.equalWithAbsError( V3f( 1, 0, 0 ), 0.000001 ) )
		for v in bitangentPrimVar.data :
			self.failUnless( v.equalWithAbsError( V3f( 0, 0, -1 ), 0.000001 ) )

	def testSplitAndOpposedUVEdges( self ) :

		mesh = ObjectReader( "test/IECore/data/cobFiles/twoTrianglesWithSplitAndOpposedUVs.cob" ).read()

		tangentPrimVar, bitangentPrimVar = MeshAlgo.calculateTangents( mesh )

		self.assertEqual( tangentPrimVar.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( bitangentPrimVar.interpolation, PrimitiveVariable.Interpolation.FaceVarying )

		for v in tangentPrimVar.data[:3] :
			self.failUnless( v.equalWithAbsError( V3f( -1, 0, 0 ), 0.000001 ) )
		for v in tangentPrimVar.data[3:] :
			self.failUnless( v.equalWithAbsError( V3f( 1, 0, 0 ), 0.000001 ) )

		for v in bitangentPrimVar.data[:3] :
			self.failUnless( v.equalWithAbsError( V3f( 0, 0, 1 ), 0.000001 ) )
		for v in bitangentPrimVar.data[3:] :
			self.failUnless( v.equalWithAbsError( V3f( 0, 0, -1 ), 0.000001 ) )

	def testNonTriangulatedMeshRaisesException( self ):
		plane = MeshPrimitive.createPlane( Box2f( V2f( -0.1 ), V2f( 0.1 ) ) )
		self.assertRaises( RuntimeError, lambda : MeshAlgo.calculateTangents( plane ) )

	def testInvalidPositionPrimVarRaisesException( self ) :
		triangle = self.makeSingleTriangleMesh()
		self.assertRaises( RuntimeError, lambda : MeshAlgo.calculateTangents( triangle, position = "foo" ) )

	def testMissingUVsetPrimVarsRaisesException ( self ):
		triangle = self.makeSingleTriangleMesh()
		self.assertRaises( RuntimeError, lambda : MeshAlgo.calculateTangents( triangle, uvSet = "bar") )

	def testIncorrectUVPrimVarInterpolationRaisesException ( self ):
		triangle = self.makeSingleBadUVTriangleMesh()
		self.assertRaises( RuntimeError, lambda : MeshAlgo.calculateTangents( triangle ) )

	def testCanUseSecondUVSet( self ) :

		triangle = self.makeSingleTriangleMesh()
		uTangent, vTangent = MeshAlgo.calculateTangents( triangle , uvSet = "foo" )

		self.assertEqual( len( uTangent.data ), 3 )
		self.assertEqual( len( vTangent.data ), 3 )

		for v in uTangent.data :
			self.failUnless( v.equalWithAbsError( V3f( 0, 1, 0 ), 0.000001 ) )

		# really I'd expect the naive answer to the vTangent to be V3f( 1, 0, 0 )
		# but the code forces the triple of n, uT, vT to flip the direction of vT if we don't have a correctly handed set of basis vectors
		for v in vTangent.data :
			self.failUnless( v.equalWithAbsError( V3f( -1, 0, 0 ), 0.000001 ) )

	def testCanUsePref( self ) :

		triangle = self.makeSingleTriangleMesh()
		uTangent, vTangent = MeshAlgo.calculateTangents( triangle , position = "Pref")

		self.assertEqual( len( uTangent.data ), 3 )
		self.assertEqual( len( vTangent.data ), 3 )

		for v in uTangent.data :
			self.failUnless( v.equalWithAbsError( V3f( 0, -1, 0 ), 0.000001 ) )

		for v in vTangent.data :
			self.failUnless( v.equalWithAbsError( V3f( 1, 0, 0 ), 0.000001 ) )

class MeshAlgoPrimitiveVariableTest( unittest.TestCase ) :

	@classmethod
	def makeMesh( cls ) :
		testObject = MeshPrimitive.createPlane( Box2f( V2f( 0 ), V2f( 10 ) ), V2i( 2 ) )

		testObject["a"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatData( 0.5 ) )
		testObject["b"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, FloatVectorData( range( 0, 9 ) ) )
		testObject["c"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( range( 0, 4 ) ) )
		testObject["d"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Varying, FloatVectorData( range( 0, 9 ) ) )
		testObject["e"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, FloatVectorData( range( 0, 16 ) ) )

		return testObject

	@classmethod
	def setUpClass(cls):
		cls.mesh = cls.makeMesh()

	def testMeshConstantToVertex( self ) :

		p = self.mesh["a"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p, PrimitiveVariable.Interpolation.Vertex );
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, FloatVectorData( [ 0.5 ] * 9 ) )

	def testMeshConstantToUniform( self ) :

		p = self.mesh["a"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, FloatVectorData( [ 0.5 ] * 4 ) )

	def testMeshConstantToVarying( self ) :

		p = self.mesh["a"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, FloatVectorData( [ 0.5 ] * 9 ) )

	def testMeshConstantToFaceVarying( self ) :

		p = self.mesh["a"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, FloatVectorData( [ 0.5 ] * 16 ) )


	def testMeshVertexToConstant( self ) :
		p = self.mesh["b"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, FloatData( sum(range(0,9))/9. ) )

	def testMeshVertexToUniform( self ) :
		p = self.mesh["b"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, FloatVectorData( [ 2, 3, 5, 6 ] ) )

	def testMeshVertexToVarying( self ) :
		p = self.mesh["b"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, FloatVectorData( range( 0, 9 ) ) )

	def testMeshVertexToFaceVarying( self ) :
		p = self.mesh["b"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, FloatVectorData( [ orig[x] for x in self.mesh.vertexIds ] ) )

	def testMeshUniformToConstant( self ) :
		p = self.mesh["c"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, FloatData( sum(range(0,4))/4. ) )

	def testMeshUniformToVertex( self ) :
		p = self.mesh["c"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, FloatVectorData( [ 0, 0.5, 1, 1, 1.5, 2, 2, 2.5, 3 ] ) )

	def testMeshUniformToVarying( self ) :
		p = self.mesh["c"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, FloatVectorData( [ 0, 0.5, 1, 1, 1.5, 2, 2, 2.5, 3 ] ) )

	def testMeshUniformToFaceVarying( self ) :
		p = self.mesh["c"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, FloatVectorData( ( [ 0 ] * 4 ) + ( [ 1 ] * 4 ) + ( [ 2 ] * 4 ) + ( [ 3 ] * 4 ) ) )

	def testMeshVaryingToConstant( self ) :
		p = self.mesh["d"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, FloatData( sum(range(0,9))/9. ) )

	def testMeshVaryingToVertex( self ) :
		p = self.mesh["d"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, FloatVectorData( range( 0, 9 ) ) )

	def testMeshVaryingToUniform( self ) :
		p = self.mesh["d"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, FloatVectorData( [ 2, 3, 5, 6 ] ) )

	def testMeshVaryingToFaceVarying( self ) :
		p = self.mesh["d"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, FloatVectorData( [ orig[x] for x in self.mesh.vertexIds ] ) )

	def testMeshFaceVaryingToConstant( self ) :
		p = self.mesh["e"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, FloatData( sum(range(0,16))/16. ) )

	def testMeshFaceVaryingToVertex( self ) :
		p = self.mesh["e"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, FloatVectorData( [ 0, 2.5, 5, 5.5, 7.5, 9.5, 11, 12.5, 14 ] ) )

	def testMeshFaceVaryingToUniform( self ) :
		p = self.mesh["e"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, FloatVectorData( [ 1.5, 5.5, 9.5, 13.5 ] ) )

	def testMeshFaceVaryingToVarying( self ) :
		p = self.mesh["e"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, FloatVectorData( [ 0, 2.5, 5, 5.5, 7.5, 9.5, 11, 12.5, 14 ] ) )


class MeshAlgoDeleteFacesTest( unittest.TestCase ) :

	def makeQuadTriangleMesh( self ):

		verticesPerFace = IntVectorData( [ 3, 3 ] )
		vertexIds = IntVectorData( [ 0, 1, 2, 0, 2, 3 ] )
		p = V3fVectorData( [ V3f( 0, 0, 0 ), V3f( 1, 0, 0 ), V3f( 1, 1, 0 ), V3f( 0, 1, 0 ) ] )
		s = FloatVectorData( [ 0, 1, 1, 0, 1, 0] )
		t = FloatVectorData( [ 0, 0, 1, 0, 1, 1] )

		mesh = MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["s"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, s )
		mesh["t"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, t )

		return mesh

	def testHandlesInvalidPrimvarType( self ) :
		deleteAttributeData = V3fVectorData( [ V3f( 0, 0, 0 ), V3f( 1, 0, 0 ), V3f( 1, 1, 0 ), V3f( 0, 1, 0 ) ]  )
		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		self.assertRaises( RuntimeError, lambda : MeshAlgo.deleteFaces( mesh, mesh["delete"] ) )

	def testHandlesInvalidPrimvarInterpolation( self ) :
		deleteAttributeData = IntVectorData( [0, 0, 0, 0, 0, 0] )
		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, deleteAttributeData )

		self.assertRaises( RuntimeError, lambda : MeshAlgo.deleteFaces( mesh, mesh["delete"] ) )

	def testCanRemoveAllFaces( self ) :
		deleteAttributeData = IntVectorData( [1, 1] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 0 )
		self.assertEqual( len( facesDeletedMesh.verticesPerFace ), 0 )
		self.assertEqual( len( facesDeletedMesh.vertexIds ), 0 )

		self.assertEqual( len( facesDeletedMesh["s"].data ), 0 )
		self.assertEqual( len( facesDeletedMesh["t"].data ), 0 )
		self.assertEqual( len( facesDeletedMesh["P"].data ), 0 )
		self.assertEqual( len( facesDeletedMesh["delete"].data ), 0 )

	def testCanRemoveFirstFace( self ) :
		deleteAttributeData = IntVectorData( [1, 0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IntVectorData( [3] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["s"].data, FloatVectorData( [0, 1, 0] ) )
		self.assertEqual( facesDeletedMesh["t"].data, FloatVectorData( [0, 1, 1] ) )

		self.assertEqual( facesDeletedMesh["P"].data, V3fVectorData( [V3f( 0, 0, 0 ), V3f( 1, 1, 0 ), V3f( 0, 1, 0 )] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IntVectorData( [0] ) )


	def testCanRemoveFirstFaceBool( self ) :
		deleteAttributeData = BoolVectorData( [True, False] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IntVectorData( [3] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["s"].data, FloatVectorData( [0, 1, 0] ) )
		self.assertEqual( facesDeletedMesh["t"].data, FloatVectorData( [0, 1, 1] ) )

		self.assertEqual( facesDeletedMesh["P"].data, V3fVectorData( [V3f( 0, 0, 0 ), V3f( 1, 1, 0 ), V3f( 0, 1, 0 )] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, BoolVectorData( [False] ) )

	def testCanRemoveFirstFaceFloat( self ) :
		deleteAttributeData = FloatVectorData( [1.0, 0.0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IntVectorData( [3] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["s"].data, FloatVectorData( [0, 1, 0] ) )
		self.assertEqual( facesDeletedMesh["t"].data, FloatVectorData( [0, 1, 1] ) )

		self.assertEqual( facesDeletedMesh["P"].data, V3fVectorData( [V3f( 0, 0, 0 ), V3f( 1, 1, 0 ), V3f( 0, 1, 0 )] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, FloatVectorData( [0.0] ) )

class MeshAlgoReverseWindingTest( unittest.TestCase ) :

	def testSingleTriangle( self ) :

		mesh = MeshAlgoTangentTest.makeSingleTriangleMesh()
		mesh.blindData()["test"] = IECore.IntData( 10 )

		meshReversed = mesh.copy()
		IECore.MeshAlgo.reverseWinding( meshReversed )

		# Meshes should be identical

		self.assertEqual( meshReversed.interpolation, mesh.interpolation )
		for interpolation in IECore.PrimitiveVariable.Interpolation.values.values() :
			self.assertEqual( meshReversed.variableSize( interpolation ), mesh.variableSize( interpolation ) )
		self.assertEqual( mesh.keys(), meshReversed.keys() )
		self.assertEqual( mesh["P"], meshReversed["P"] )
		self.assertEqual( mesh["Pref"], meshReversed["Pref"] )
		self.assertEqual( mesh.blindData(), meshReversed.blindData() )

		# Except for vertex ids, and facevarying data

		self.assertEqual( list( meshReversed.vertexIds ), list( reversed( mesh.vertexIds ) ) )
		self.assertEqual( list( meshReversed["s"].data ), list( reversed( mesh["s"].data ) ) )

	def testPlane( self ) :

		mesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ), IECore.V2i( 10 ) )
		IECore.TriangulateOp()( input = mesh, copyInput = False )

		meshReversed = mesh.copy()
		IECore.MeshAlgo.reverseWinding( meshReversed )

		evaluator = IECore.MeshPrimitiveEvaluator( mesh )
		evaluatorReversed = IECore.MeshPrimitiveEvaluator( meshReversed )

		result = evaluator.createResult()
		resultReversed = evaluatorReversed.createResult()

		for i in range( 0, 1000 ) :

			p = IECore.V3f( random.uniform( -1.0, 1.0 ), random.uniform( -1.0, 1.0 ), 0 )
			evaluator.closestPoint( p, result )
			evaluatorReversed.closestPoint( p, resultReversed )

			self.assertEqual( resultReversed.normal(), -result.normal() )
			for n in ( "s", "t" ) :
				self.assertAlmostEqual(
					resultReversed.floatPrimVar( meshReversed[n] ),
					result.floatPrimVar( mesh[n] ),
					delta = 0.0001
				)

	def testRoundTrip( self ) :

		mesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ), IECore.V2i( 10 ) )
		meshReversed = mesh.copy()
		IECore.MeshAlgo.reverseWinding( meshReversed )
		meshReversedAgain = meshReversed.copy()
		IECore.MeshAlgo.reverseWinding( meshReversedAgain )

		self.assertEqual( mesh, meshReversedAgain )

if __name__ == "__main__":
	unittest.main()
