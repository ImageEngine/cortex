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

import time
import threading
import unittest

import IECore
import IECoreScene

import imath

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
		tangentPrimVar, bitangentPrimVar = IECoreScene.MeshAlgo.calculateTangentsFromUV( triangle )

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
		self.assertTrue( mesh.arePrimitiveVariablesValid() )

		tangentPrimVar, bitangentPrimVar = IECoreScene.MeshAlgo.calculateTangentsFromUV( mesh )

		self.assertEqual( tangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( bitangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		for v in tangentPrimVar.data :
			self.assertTrue( v.equalWithAbsError( imath.V3f( 1, 0, 0 ), 0.000001 ) )
		for v in bitangentPrimVar.data :
			self.assertTrue( v.equalWithAbsError( imath.V3f( 0, 0, -1 ), 0.000001 ) )

	def testSplitAndOpposedUVEdges( self ) :

		mesh = IECore.ObjectReader( "test/IECore/data/cobFiles/twoTrianglesWithSplitAndOpposedUVs.cob" ).read()

		tangentPrimVar, bitangentPrimVar = IECoreScene.MeshAlgo.calculateTangentsFromUV( mesh )

		self.assertEqual( tangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( bitangentPrimVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		for i in tangentPrimVar.indices[:3] :
			v = tangentPrimVar.data[i]
			self.assertTrue( v.equalWithAbsError( imath.V3f( -1, 0, 0 ), 0.000001 ) )
		for i in tangentPrimVar.indices[3:] :
			v = tangentPrimVar.data[i]
			self.assertTrue( v.equalWithAbsError( imath.V3f( 1, 0, 0 ), 0.000001 ) )

		for i in bitangentPrimVar.indices[:3] :
			v = bitangentPrimVar.data[i]
			self.assertTrue( v.equalWithAbsError( imath.V3f( 0, 0, 1 ), 0.000001 ) )
		for i in bitangentPrimVar.indices[3:] :
			v = bitangentPrimVar.data[i]
			self.assertTrue( v.equalWithAbsError( imath.V3f( 0, 0, -1 ), 0.000001 ) )

	def testInvalidPositionPrimVarRaisesException( self ) :
		triangle = self.makeSingleTriangleMesh()
		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.calculateTangentsFromUV( triangle, position = "foo" ) )

	def testMissingUVsetPrimVarsRaisesException ( self ):
		triangle = self.makeSingleTriangleMesh()
		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.calculateTangentsFromUV( triangle, uvSet = "bar") )

	def testIncorrectUVPrimVarInterpolationRaisesException ( self ):
		triangle = self.makeSingleBadUVTriangleMesh()
		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.calculateTangentsFromUV( triangle ) )

	def testCanUseSecondUVSet( self ) :

		triangle = self.makeSingleTriangleMesh()
		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangentsFromUV( triangle , uvSet = "foo" )

		self.assertEqual( len( uTangent.data ), 3 )
		self.assertEqual( len( vTangent.data ), 3 )

		for v in uTangent.data :
			self.assertTrue( v.equalWithAbsError( imath.V3f( 0, 1, 0 ), 0.000001 ) )

		# really I'd expect the naive answer to the vTangent to be V3f( 1, 0, 0 )
		# but the code forces the triple of n, uT, vT to flip the direction of vT if we don't have a correctly handed set of basis vectors
		for v in vTangent.data :
			self.assertTrue( v.equalWithAbsError( imath.V3f( -1, 0, 0 ), 0.000001 ) )

	def testCanUsePref( self ) :

		triangle = self.makeSingleTriangleMesh()
		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangentsFromUV( triangle , position = "Pref")

		self.assertEqual( len( uTangent.data ), 3 )
		self.assertEqual( len( vTangent.data ), 3 )

		for v in uTangent.data :
			self.assertTrue( v.equalWithAbsError( imath.V3f( 0, -1, 0 ), 0.000001 ) )

		for v in vTangent.data :
			self.assertTrue( v.equalWithAbsError( imath.V3f( 1, 0, 0 ), 0.000001 ) )

	def testQuadMesh( self ):

		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ), imath.V2i( 1 ) )

		self.assertEqual( len( mesh["P"].data ), 4 )
		self.assertEqual( mesh.numFaces(), 1 )

		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangentsFromUV( mesh )

		self.assertEqual( len( uTangent.data ), 4)
		self.assertEqual( len( vTangent.data ), 4)

		for v in uTangent.data :
			self.assertTrue( v.equalWithAbsError( imath.V3f( 1, 0, 0 ), 0.000001 ) )

		for v in vTangent.data :
			self.assertTrue( v.equalWithAbsError( imath.V3f( 0, 1, 0 ), 0.000001 ) )

	def assertArrayEqual( self, a, b ) :

		self.assertEqual( len( a ), len( b ) )
		for c in zip( a, b ) :
			self.assertTrue( c[0].equalWithAbsError( c[1], 0.0001 ) )

	def testVertexInterpolatedUVs( self ) :

		verticesPerFace = IECore.IntVectorData( [3, 3] )
		vertexIds = IECore.IntVectorData( [0, 1, 2, 1, 3, 4] )
		interpolation = "linear"
		positions = IECore.V3fVectorData(
			[imath.V3f( 0.0, -0.5, 0.0 ), imath.V3f( 1.0, 0.0, 0.0 ), imath.V3f( 0.0, 0.5, 0.0 ), imath.V3f( 2.0, -0.5, 0.0 ), imath.V3f( 2.0, 0.5, 0.0 )] )
		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, interpolation, positions )

		uvData = IECore.V2fVectorData( [imath.V2f( 0, -0.5 ), imath.V2f( 1, 0 ), imath.V2f( 0, 0.5 ), imath.V2f( 1.5, 1 ), imath.V2f( 0.5, 1 )] )
		mesh["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, uvData )

		uTangent, vTangent = IECoreScene.MeshAlgo.calculateTangentsFromUV( mesh, orthoTangents = False )

		u0 = imath.V3f( 1, 0, 0 )
		u2 = imath.V3f( 0, -1, 0 )

		u1 = (u0 + u2).normalize()

		v0 = imath.V3f( 0, 1, 0 )
		v2 = imath.V3f( 1, 0, 0 )

		v1 = (v0 + v2).normalize()

		self.assertArrayEqual( IECore.V3fVectorData( [u0, u1, u0, u2, u2] ), uTangent.data )
		self.assertArrayEqual( IECore.V3fVectorData( [v0, v1, v0, v2, v2] ), vTangent.data )

	def testComputeTangentsFromFirstEdge( self ):

		verticesPerFace = IECore.IntVectorData( [3, 3] )
		vertexIds = IECore.IntVectorData( [0, 1, 2, 2, 1, 3] )
		interpolation = "linear"
		positions = IECore.V3fVectorData(
			[imath.V3f( -1.0, 0.0, 0.0 ), imath.V3f( 0.0, 1.0, 0.0 ), imath.V3f( 0.0, 0.0, 1.0 ), imath.V3f( 1.0, 0.0, 0.0 )] )

		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, interpolation, positions )
		normals = IECore.V3fVectorData( [ imath.V3f( -1, 1, 1 ).normalize(), imath.V3f( 0, 1, 1 ).normalize(),imath.V3f( 0, 1, 1 ).normalize(), imath.V3f( 1, 1, 1 ).normalize() ], IECore.GeometricData.Interpretation.Normal)
		mesh['N'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, normals )

		vecFirstEdge = IECore.V3fVectorData( [ imath.V3f( 1, 1, 0 ).normalize(), imath.V3f( -1, -1, 0 ).normalize(),imath.V3f( -1, 0, -1).normalize(), imath.V3f( -1, 1, 0 ).normalize() ], IECore.GeometricData.Interpretation.Normal )

		# non orthogonal, non left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromFirstEdge( mesh, orthoTangents=False, leftHanded=False )
		tRes = vecFirstEdge
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

		# orthogonal, non left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromFirstEdge( mesh, orthoTangents=True, leftHanded=False )
		tRes = vecFirstEdge
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )
		tRes = IECore.V3fVectorData( [vt.cross(n).normalize() for vt, n in zip(btRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

		# orthogonal, left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromFirstEdge( mesh, orthoTangents=True, leftHanded=True )
		tRes = vecFirstEdge
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )
		tRes = IECore.V3fVectorData( [n.cross(vt).normalize() for vt, n in zip(btRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

	def testComputeTangentsFromTwoEdges( self ):

		verticesPerFace = IECore.IntVectorData( [3, 3] )
		vertexIds = IECore.IntVectorData( [0, 1, 2, 2, 1, 3] )
		interpolation = "linear"
		p = IECore.V3fVectorData(
			[imath.V3f( -1.0, 0.0, 0.0 ), imath.V3f( 0.0, 1.0, 0.0 ), imath.V3f( 0.0, 0.0, 1.0 ), imath.V3f( 1.0, 0.0, 0.0 )] )

		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, interpolation, p )
		normals = IECore.V3fVectorData( [ imath.V3f( -1, 1, 1 ).normalize(), imath.V3f( 0, 1, 1 ).normalize(),imath.V3f( 0, 1, 1 ).normalize(), imath.V3f( 1, 1, 1 ).normalize() ], IECore.GeometricData.Interpretation.Normal)
		mesh['N'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, normals )

		vecBetweenTwoEdges = IECore.V3fVectorData( [x.normalized() for x in [ (p[1]+p[2])*0.5-p[0], (p[0]+p[2])*0.5-p[1], (p[0]+p[1])*0.5-p[2], (p[1]+p[2])*0.5-p[3] ] ], IECore.GeometricData.Interpretation.Normal )

		# non orthogonal, non left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromTwoEdges( mesh, orthoTangents=False, leftHanded=False )
		tRes =  vecBetweenTwoEdges
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

		# orthogonal, non left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromTwoEdges( mesh, orthoTangents=True, leftHanded=False )
		tRes =  vecBetweenTwoEdges
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )
		tRes = IECore.V3fVectorData( [vt.cross(n).normalize() for vt, n in zip(btRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

		# orthogonal, left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromTwoEdges( mesh, orthoTangents=True, leftHanded=True )
		tRes =  vecBetweenTwoEdges
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )
		tRes = IECore.V3fVectorData( [n.cross(vt).normalize() for vt, n in zip(btRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

	def testComputeTangentsFromPrimitiveCentroid( self ):

		verticesPerFace = IECore.IntVectorData( [3, 3] )
		vertexIds = IECore.IntVectorData( [0, 1, 2, 2, 1, 3] )
		interpolation = "linear"
		p = IECore.V3fVectorData(
			[imath.V3f( -1.0, 0.0, 0.0 ), imath.V3f( 0.0, 1.0, 0.0 ), imath.V3f( 0.0, 0.0, 1.0 ), imath.V3f( 1.0, 0.0, 0.0 )] )

		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, interpolation, p )
		normals = IECore.V3fVectorData( [ imath.V3f( -1, 1, 1 ).normalize(), imath.V3f( 0, 1, 1 ).normalize(),imath.V3f( 0, 1, 1 ).normalize(), imath.V3f( 1, 1, 1 ).normalize() ], IECore.GeometricData.Interpretation.Normal)
		mesh['N'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, normals )

		vecToCentroid = IECore.V3fVectorData( [x.normalized() for x in [ (p[0]+p[1]+p[2])/3.0-p[0], (p[3]+p[1]+p[2])/3.0-p[1],  (p[3]+p[1]+p[2])/3.0-p[2],  (p[3]+p[1]+p[2])/3.0-p[3] ] ], IECore.GeometricData.Interpretation.Normal )

		# non orthogonal, non left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromPrimitiveCentroid( mesh, orthoTangents=False, leftHanded=False )
		tRes =  vecToCentroid
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

		# orthogonal, non left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromPrimitiveCentroid( mesh, orthoTangents=True, leftHanded=False )
		tRes =  vecToCentroid
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )
		tRes = IECore.V3fVectorData( [vt.cross(n).normalize() for vt, n in zip(btRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

		# orthogonal, left handed
		tangent, biTangent = IECoreScene.MeshAlgo.calculateTangentsFromPrimitiveCentroid( mesh, orthoTangents=True, leftHanded=True )
		tRes =  vecToCentroid
		btRes = IECore.V3fVectorData( [n.cross(ut).normalize() for ut, n in zip(tRes, normals)] )
		tRes = IECore.V3fVectorData( [n.cross(vt).normalize() for vt, n in zip(btRes, normals)] )

		self.assertArrayEqual( tRes, tangent.data )
		self.assertArrayEqual( btRes, biTangent.data )

	def testCancel( self ) :
		canceller = IECore.Canceller()

		strip = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 3000000, 1 ) ), imath.V2i( 3000000, 1 ) )
		variant = 0
		def backgroundRun():

			try:
				if variant == 0:
					IECoreScene.MeshAlgo.calculateTangentsFromUV( strip, canceller = canceller )
				elif variant == 1:
					IECoreScene.MeshAlgo.calculateTangentsFromFirstEdge( strip, canceller = canceller )
				elif variant == 2:
					IECoreScene.MeshAlgo.calculateTangentsFromTwoEdges( strip, canceller = canceller )
				elif variant == 3:
					IECoreScene.MeshAlgo.calculateTangentsFromPrimitiveCentroid( strip, canceller = canceller )
			except IECore.Cancelled:
				cancelled[0] = True

		for i in range( 4 ):
			cancelled = [False]
			variant = i
			thread = threading.Thread(target=backgroundRun, args=())

			startTime = time.time()
			thread.start()

			time.sleep( 0.1 )
			canceller.cancel()
			thread.join()

			# This test should actually produce a time extremely close to the sleep duration ( within
			# 0.003 seconds whether the sleep duration is 0.01 seconds or 100 seconds ), but checking
			# that it terminates with 0.1 seconds is a minimal performance bar
			self.assertLess( time.time() - startTime, 0.2 )
			self.assertTrue( cancelled[0] )

if __name__ == "__main__":
	unittest.main()
