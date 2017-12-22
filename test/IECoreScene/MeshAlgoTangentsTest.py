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

import unittest
import imath

import IECore
import IECoreScene

class MeshAlgoTangentsTest( unittest.TestCase ) :

	def makeSingleTriangleMesh( self ):

		verticesPerFace = IECore.IntVectorData( [ 3 ] )
		vertexIds = IECore.IntVectorData( [ 0, 1, 2 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 0, 1, 0 ) ] )
		uv = IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 0 ), imath.V2f( 0, 1 ) ] )

		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uv )

		mesh["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 0, 1 ), imath.V2f( 1, 0 ) ] ) )

		prefData = IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 0, -1, 0 ), imath.V3f( 1, 0, 0 ) ] )
		mesh["Pref"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, prefData )

		return mesh

	def makeSingleBadUVTriangleMesh( self ) :

		verticesPerFace = IECore.IntVectorData( [3] )
		vertexIds = IECore.IntVectorData( [0, 1, 2] )
		p = IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 0, 1, 0 )] )
		uv = IECore.V2fVectorData( [ imath.V2f( 0 ) ] )

		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, uv )

		return mesh

	def testSingleTriangleGeneratesCorrectTangents( self ) :
		triangle = self.makeSingleTriangleMesh()
		tangentPrimVar, bitangentPrimVar = IECoreScene.MeshAlgo.calculateTangents( triangle )

		self.assertEqual(tangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)
		self.assertEqual(bitangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		tangents = IECore.V3fVectorData( tangentPrimVar.data )
		bitangent = IECore.V3fVectorData( bitangentPrimVar.data )

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

		mesh = IECore.ObjectReader( "test/IECore/data/cobFiles/twoTrianglesWithSharedUVs.cob" ).read()
		self.assert_( mesh.arePrimitiveVariablesValid() )

		tangentPrimVar, bitangentPrimVar = IECoreScene.MeshAlgo.calculateTangents( mesh )

		self.assertEqual( tangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( bitangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		for v in tangentPrimVar.data :
			self.failUnless( v.equalWithAbsError( imath.V3f( 1, 0, 0 ), 0.000001 ) )
		for v in bitangentPrimVar.data :
			self.failUnless( v.equalWithAbsError( imath.V3f( 0, 0, -1 ), 0.000001 ) )

	def testSplitAndOpposedUVEdges( self ) :

		mesh = IECore.ObjectReader( "test/IECore/data/cobFiles/twoTrianglesWithSplitAndOpposedUVs.cob" ).read()

		tangentPrimVar, bitangentPrimVar = IECoreScene.MeshAlgo.calculateTangents( mesh )

		self.assertEqual( tangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( bitangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		for v in tangentPrimVar.data[:3] :
			self.failUnless( v.equalWithAbsError( imath.V3f( -1, 0, 0 ), 0.000001 ) )
		for v in tangentPrimVar.data[3:] :
			self.failUnless( v.equalWithAbsError( imath.V3f( 1, 0, 0 ), 0.000001 ) )

		for v in bitangentPrimVar.data[:3] :
			self.failUnless( v.equalWithAbsError( imath.V3f( 0, 0, 1 ), 0.000001 ) )
		for v in bitangentPrimVar.data[3:] :
			self.failUnless( v.equalWithAbsError( imath.V3f( 0, 0, -1 ), 0.000001 ) )

	def testNonTriangulatedMeshRaisesException( self ):
		plane = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -0.1 ), imath.V2f( 0.1 ) ) )
		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.calculateTangents( plane ) )

	def testInvalidPositionPrimVarRaisesException( self ) :
		triangle = self.makeSingleTriangleMesh()
		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.calculateTangents( triangle, position = "foo" ) )

	def testMissingUVsetPrimVarsRaisesException ( self ):
		triangle = self.makeSingleTriangleMesh()
		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.calculateTangents( triangle, uvSet = "bar") )

	def testIncorrectUVPrimVarInterpolationRaisesException ( self ):
		triangle = self.makeSingleBadUVTriangleMesh()
		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.calculateTangents( triangle ) )

	def testCanUseSecondUVSet( self ) :

		triangle = self.makeSingleTriangleMesh()
		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangents( triangle , uvSet = "foo" )

		self.assertEqual( len( uTangent.data ), 3 )
		self.assertEqual( len( vTangent.data ), 3 )

		for v in uTangent.data :
			self.failUnless( v.equalWithAbsError( imath.V3f( 0, 1, 0 ), 0.000001 ) )

		# really I'd expect the naive answer to the vTangent to be V3f( 1, 0, 0 )
		# but the code forces the triple of n, uT, vT to flip the direction of vT if we don't have a correctly handed set of basis vectors
		for v in vTangent.data :
			self.failUnless( v.equalWithAbsError( imath.V3f( -1, 0, 0 ), 0.000001 ) )

	def testCanUsePref( self ) :

		triangle = self.makeSingleTriangleMesh()
		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangents( triangle , position = "Pref")

		self.assertEqual( len( uTangent.data ), 3 )
		self.assertEqual( len( vTangent.data ), 3 )

		for v in uTangent.data :
			self.failUnless( v.equalWithAbsError( imath.V3f( 0, -1, 0 ), 0.000001 ) )

		for v in vTangent.data :
			self.failUnless( v.equalWithAbsError( imath.V3f( 1, 0, 0 ), 0.000001 ) )

if __name__ == "__main__":
	unittest.main()
