##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import math
import unittest
from IECore import *



class TestTriangulateOp( unittest.TestCase ) :

	def testSimple( self ) :
		""" Test TriangulateOp with a single polygon"""

		verticesPerFace = IntVectorData()
		verticesPerFace.append( 4 )

		vertexIds = IntVectorData()
		vertexIds.append( 0 )
		vertexIds.append( 1 )
		vertexIds.append( 2 )
		vertexIds.append( 3 )

		P = V3fVectorData()
		P.append( V3f( -1, 0, -1 ) )
		P.append( V3f( -1, 0,  1 ) )
		P.append( V3f(  1, 0,  1 ) )
		P.append( V3f(  1, 0, -1 ) )

		m = MeshPrimitive( verticesPerFace, vertexIds )
		m["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, P )

		fv = IntVectorData()
		fv.append( 5 )
		fv.append( 6 )
		fv.append( 7 )
		fv.append( 8 )
		m["fv"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, fv )

		u = FloatVectorData()
		u.append( 1.0 )
		m["u"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, u )

		op = TriangulateOp()

		result = op(
			input = m
		)

		self.assert_( "P" in result )

		self.assert_( result.arePrimitiveVariablesValid() )

		resultP = result["P"].data

		self.assertEqual( len(resultP), 4 )
		for i in range(0, 4) :
			self.assert_( ( resultP[i] - P[i] ).length() < 0.001 )

		self.assertEqual( len(result.vertexIds), 6 )

		for faceVertexCount in result.verticesPerFace :
			self.assertEqual( faceVertexCount, 3 )

		for vId in result.vertexIds:
			self.assert_( vId < len(resultP) )

		self.assert_( "fv" in result )
		fv = result["fv"]
		self.assertEqual( len(fv.data), len(result.vertexIds) )

		for i in fv.data:
			self.assert_( i >= 5 and i <= 8 )


	def testQuadrangulatedSphere( self ) :
		""" Test TriangulateOp with a quadrangulated poly sphere"""

		m = Reader.create( "test/IECore/data/cobFiles/polySphereQuads.cob").read()
		P = m["P"].data

		self.assertEqual ( len( m.vertexIds ), 1560 )

		op = TriangulateOp()

		result = op(
			input = m
		)

		self.assert_( result.arePrimitiveVariablesValid() )

		self.assert_( "P" in result )

		resultP = result["P"].data

		self.assertEqual( len( resultP ), len( P ) )
		for i in range(0, len( resultP ) ) :
			self.assert_( ( resultP[i] - P[i] ).length() < 0.001 )

		for faceVertexCount in result.verticesPerFace :
			self.assertEqual( faceVertexCount, 3 )

		for vId in result.vertexIds:
			self.assert_( vId < len(resultP) )

		self.assertEqual ( len( result.vertexIds ), 2280 )


	def testTriangulatedSphere( self ) :
		""" Test TriangulateOp with a triangulated poly sphere"""

		m = Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob").read()

		op = TriangulateOp()

		result = op(
			input = m
		)

		self.assert_( result.arePrimitiveVariablesValid() )

		# As input was already triangulated, the result should be exactly the same
		self.assertEqual( m, result )

	def testNonPlanar( self ) :
		""" Test TriangulateOp with a nonplanar polygon"""


		verticesPerFace = IntVectorData()
		verticesPerFace.append( 4 )

		vertexIds = IntVectorData()
		vertexIds.append( 0 )
		vertexIds.append( 1 )
		vertexIds.append( 2 )
		vertexIds.append( 3 )

		P = V3dVectorData()
		P.append( V3d( -1, 0, -1 ) )
		P.append( V3d( -1, 0,  1 ) )
		P.append( V3d(  1, 0,  1 ) )
		P.append( V3d(  1, 1, -1 ) )

		m = MeshPrimitive( verticesPerFace, vertexIds )
		m["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, P )

		op = TriangulateOp()

		op.parameters().input = m

		# Non-planar faces not supported by default
		self.assertRaises( RuntimeError, op )

		op.parameters().throwExceptions = False
		result = op()


	def testConcave( self ) :
		""" Test TriangulateOp with a concave polygon"""

		verticesPerFace = IntVectorData()
		verticesPerFace.append( 4 )

		vertexIds = IntVectorData()
		vertexIds.append( 0 )
		vertexIds.append( 1 )
		vertexIds.append( 2 )
		vertexIds.append( 3 )

		P = V3dVectorData()
		P.append( V3d( -1, 0, -1 ) )
		P.append( V3d( -1, 0,  1 ) )
		P.append( V3d(  1, 0,  1 ) )
		P.append( V3d(  -0.9, 0, -0.9 ) )

		m = MeshPrimitive( verticesPerFace, vertexIds )
		m["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, P )

		op = TriangulateOp()

		op.parameters().input = m

		# Concave faces not supported by default
		self.assertRaises( RuntimeError, op )

		op.parameters().throwExceptions = False
		result = op()

	def testErrors( self ):
		""" Test TriangulateOp with invalid P data """

		verticesPerFace = IntVectorData()
		verticesPerFace.append( 4 )

		vertexIds = IntVectorData()
		vertexIds.append( 0 )
		vertexIds.append( 1 )
		vertexIds.append( 2 )
		vertexIds.append( 3 )

		P = FloatVectorData()
		P.append( 1 )
		P.append( 2 )
		P.append( 3 )
		P.append( 4 )

		m = MeshPrimitive( verticesPerFace, vertexIds )
		m["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, P )

		op = TriangulateOp()

		op.parameters().input = m

		# FloatVectorData not valid for "P"
		self.assertRaises( RuntimeError, op )

	def testConstantPrimVars( self ) :

		m = Reader.create( "test/IECore/data/cobFiles/polySphereQuads.cob").read()

		m["constantScalar"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatData( 1 ) )
		m["constantArray"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, StringVectorData( [ "one", "two" ] ) )

		result = TriangulateOp()( input = m )
		self.assert_( result.arePrimitiveVariablesValid() )


if __name__ == "__main__":
    unittest.main()
