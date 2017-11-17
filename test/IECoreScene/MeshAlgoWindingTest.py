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
import IECoreScene

class MeshAlgoWindingTest( unittest.TestCase ) :

	def makeSingleTriangleMesh( self ):

		verticesPerFace = IECore.IntVectorData( [ 3 ] )
		vertexIds = IECore.IntVectorData( [ 0, 1, 2 ] )
		p = IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 0, 1, 0 ) ] )
		uv = IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 0 ), IECore.V2f( 0, 1 ) ] )

		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uv )

		mesh["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 0, 1 ), IECore.V2f( 1, 0 ) ] ) )

		prefData = IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 0, -1, 0 ), IECore.V3f( 1, 0, 0 ) ] )
		mesh["Pref"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, prefData )

		return mesh

	def testSingleTriangle( self ) :

		mesh = self.makeSingleTriangleMesh()
		mesh.blindData()["test"] = IECore.IntData( 10 )

		meshReversed = mesh.copy()
		IECoreScene.MeshAlgo.reverseWinding( meshReversed )

		# Meshes should be identical

		self.assertEqual( meshReversed.interpolation, mesh.interpolation )
		for interpolation in IECoreScene.PrimitiveVariable.Interpolation.values.values() :
			self.assertEqual( meshReversed.variableSize( interpolation ), mesh.variableSize( interpolation ) )
		self.assertEqual( mesh.keys(), meshReversed.keys() )
		self.assertEqual( mesh["P"], meshReversed["P"] )
		self.assertEqual( mesh["Pref"], meshReversed["Pref"] )
		self.assertEqual( mesh.blindData(), meshReversed.blindData() )

		# Except for vertex ids, and facevarying data

		self.assertEqual( list( meshReversed.vertexIds ), list( reversed( mesh.vertexIds ) ) )
		self.assertEqual( list( meshReversed["uv"].data ), list( reversed( mesh["uv"].data ) ) )

	def testPlane( self ) :

		mesh = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ), IECore.V2i( 10 ) )
		IECoreScene.TriangulateOp()( input = mesh, copyInput = False )

		meshReversed = mesh.copy()
		IECoreScene.MeshAlgo.reverseWinding( meshReversed )

		evaluator = IECoreScene.MeshPrimitiveEvaluator( mesh )
		evaluatorReversed = IECoreScene.MeshPrimitiveEvaluator( meshReversed )

		result = evaluator.createResult()
		resultReversed = evaluatorReversed.createResult()

		for i in range( 0, 1000 ) :

			p = IECore.V3f( random.uniform( -1.0, 1.0 ), random.uniform( -1.0, 1.0 ), 0 )
			evaluator.closestPoint( p, result )
			evaluatorReversed.closestPoint( p, resultReversed )

			self.assertEqual( resultReversed.normal(), -result.normal() )

			reversedUV = resultReversed.vec2PrimVar( meshReversed["uv"] )
			uv = result.vec2PrimVar( mesh["uv"] )
			self.assertAlmostEqual( reversedUV[0], uv[0], delta = 0.0001 )
			self.assertAlmostEqual( reversedUV[1], uv[1], delta = 0.0001 )

	def testRoundTrip( self ) :

		mesh = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ), IECore.V2i( 10 ) )
		meshReversed = mesh.copy()
		IECoreScene.MeshAlgo.reverseWinding( meshReversed )
		meshReversedAgain = meshReversed.copy()
		IECoreScene.MeshAlgo.reverseWinding( meshReversedAgain )

		self.assertEqual( mesh, meshReversedAgain )

	def testUVIndices( self ) :

		verticesPerFace = IECore.IntVectorData( [ 3 ] )
		vertexIds = IECore.IntVectorData( [ 0, 1, 2 ] )
		p = IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 0, 1, 0 ) ] )
		uv = IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 0 ), IECore.V2f( 0, 1 ) ] )
		uvIndices = IECore.IntVectorData( [ 0, 1, 2 ] )
		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uv, uvIndices )

		meshReversed = mesh.copy()
		IECoreScene.MeshAlgo.reverseWinding( meshReversed )

		# Meshes should be identical

		self.assertEqual( meshReversed.interpolation, mesh.interpolation )
		for interpolation in IECoreScene.PrimitiveVariable.Interpolation.values.values() :
			self.assertEqual( meshReversed.variableSize( interpolation ), mesh.variableSize( interpolation ) )
		self.assertEqual( mesh.keys(), meshReversed.keys() )
		self.assertEqual( mesh["P"], meshReversed["P"] )

		# UV indices should change, but UV data doesn't need to

		self.assertEqual( meshReversed["uv"].data, mesh["uv"].data )
		self.assertEqual( list( meshReversed["uv"].indices ), list( reversed( mesh["uv"].indices ) ) )

if __name__ == "__main__":
	unittest.main()
