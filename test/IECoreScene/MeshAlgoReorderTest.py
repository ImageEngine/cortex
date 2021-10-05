##########################################################################
#
#  Copyright (c) 2008-2018, Image Engine Design Inc. All rights reserved.
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

import IECoreScene
import IECore

import imath
import os

import unittest

class MeshAlgoReorderTest( unittest.TestCase ) :

	def testPlane( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3, 3 ] ), IECore.IntVectorData( [ 0, 1, 2, 1, 3, 2 ] ) )

		p = IECore.V3fVectorData( [
			imath.V3f( -1, -1, 0 ),
			imath.V3f(  1, -1, 0 ),
			imath.V3f( -1,  1, 0 ),
			imath.V3f(  1,  1, 0 ),
		] )

		uvs = IECore.V2fVectorData( [
			imath.V2f( 0, 0 ),
			imath.V2f( 1, 0 ),
			imath.V2f( 0, 1 ),
			imath.V2f( 1, 0 ),
			imath.V2f( 1, 1 ),
			imath.V2f( 0, 1 ),
		] )

		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, p )
		m["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uvs )
		m["uni"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [ 0, 1 ] ) )

		self.assertTrue( m.arePrimitiveVariablesValid() )

		IECoreScene.MeshAlgo.reorderVertices( m, 2, 3, 1 )

		self.assertEqual( m.verticesPerFace, IECore.IntVectorData( [ 3, 3 ] ) )
		self.assertEqual( m.vertexIds, IECore.IntVectorData( [ 0, 1, 2, 0, 2, 3 ] ) )

		self.assertEqual(
			m["P"].data,
			IECore.V3fVectorData( [
				imath.V3f( -1, 1, 0 ),
				imath.V3f( 1, 1, 0 ),
				imath.V3f( 1, -1, 0 ),
				imath.V3f(-1, -1, 0 ),
			] )
		)

		self.assertEqual(
			m["uv"].data,
			IECore.V2fVectorData( [
				imath.V2f( 0, 1 ),
				imath.V2f( 1, 1 ),
				imath.V2f( 1, 0 ),
				imath.V2f( 0, 1 ),
				imath.V2f( 1, 0 ),
				imath.V2f( 0, 0 ),
			])
		)

		self.assertEqual( m["uni"].data, IECore.IntVectorData( [ 1, 0 ] ) )

		self.assertTrue( m.arePrimitiveVariablesValid() )

	def testPlaneOppositeWinding( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3, 3 ] ), IECore.IntVectorData( [ 2, 1, 0, 2, 3, 1 ] ) )

		p = IECore.V3fVectorData( [
			imath.V3f( -1, -1, 0 ),
			imath.V3f(  1, -1, 0 ),
			imath.V3f( -1,  1, 0 ),
			imath.V3f(  1,  1, 0 ),
		] )

		uvs = IECore.V2fVectorData( [
			imath.V2f( 0, 1 ),
			imath.V2f( 1, 0 ),
			imath.V2f( 0, 0 ),
			imath.V2f( 0, 1 ),
			imath.V2f( 1, 1 ),
			imath.V2f( 1, 0 ),
		] )

		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, p )
		m["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uvs )
		m["uni"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [ 1, 0 ] ) )

		self.assertTrue( m.arePrimitiveVariablesValid() )

		IECoreScene.MeshAlgo.reorderVertices( m, 2, 3, 1 )

		self.assertEqual( m.verticesPerFace, IECore.IntVectorData( [ 3, 3 ] ) )
		self.assertEqual( m.vertexIds, IECore.IntVectorData( [ 0, 1, 2, 0, 2, 3 ] ) )

		self.assertEqual(
			m["P"].data,
			IECore.V3fVectorData( [
				imath.V3f( -1, 1, 0 ),
				imath.V3f( 1, 1, 0 ),
				imath.V3f( 1, -1, 0 ),
				imath.V3f(-1, -1, 0 ),
			] )
		)

		self.assertEqual(
			m["uv"].data,
			IECore.V2fVectorData( [
				imath.V2f( 0, 1 ),
				imath.V2f( 1, 1 ),
				imath.V2f( 1, 0 ),
				imath.V2f( 0, 1 ),
				imath.V2f( 1, 0 ),
				imath.V2f( 0, 0 ),
			])
		)

		self.assertEqual( m["uni"].data, IECore.IntVectorData( [ 0, 1 ] ) )

		self.assertTrue( m.arePrimitiveVariablesValid() )

	def testQuadSphere( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "polySphereQuads.cob" ) ).read()

		IECoreScene.MeshAlgo.reorderVertices( m, 0, 1, 21 )

		self.assertTrue( m.arePrimitiveVariablesValid() )

		expected = IECore.Reader.create( os.path.join( "test", "IECore", "data", "expectedResults", "meshVertexReorderQuadSphere.cob" ) ).read()

		self.assertEqual( m, expected )

	def testSphere( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pSphereShape1.cob" ) ).read()

		IECoreScene.MeshAlgo.reorderVertices( m, 20, 1, 21 )

		self.assertTrue( m.arePrimitiveVariablesValid() )

		expected = IECore.Reader.create( os.path.join( "test", "IECore", "data", "expectedResults", "meshVertexReorderSphere.cob" ) ).read()

		self.assertEqual( m, expected )

	def testCube( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()

		IECoreScene.MeshAlgo.reorderVertices( m, 0, 1, 3 )

		self.assertTrue( m.arePrimitiveVariablesValid() )

	def testIndexedPrimitiveVariables( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ), imath.V2i( 2 ) )
		m["otherUVs"] = IECoreScene.PrimitiveVariable( m["uv"].interpolation, m["uv"].data, m["uv"].indices )

		self.assertEqual( m.verticesPerFace, IECore.IntVectorData( [ 4, 4, 4, 4  ] ) )
		self.assertEqual( m.vertexIds, IECore.IntVectorData( [ 0, 1, 4, 3, 1, 2, 5, 4, 3, 4, 7, 6, 4, 5, 8, 7 ] ) )

		self.assertEqual( m["P"].indices, None )
		expectedP = IECore.V3fVectorData( [
			imath.V3f( 0, 0, 0 ),
			imath.V3f( 0.5, 0, 0 ),
			imath.V3f( 1, 0, 0 ),
			imath.V3f( 0, 0.5, 0 ),
			imath.V3f( 0.5, 0.5, 0 ),
			imath.V3f( 1, 0.5, 0 ),
			imath.V3f( 0, 1, 0 ),
			imath.V3f( 0.5, 1, 0 ),
			imath.V3f( 1, 1, 0 ),
		] )
		for i in range( 0, len(expectedP) ) :
			self.assertEqual( m["P"].data[i], expectedP[i] )

		expectedUVs = IECore.V2fVectorData( [
			imath.V2f( 0, 0 ),
			imath.V2f( 0.5, 0 ),
			imath.V2f( 1, 0 ),
			imath.V2f( 0, 0.5 ),
			imath.V2f( 0.5, 0.5 ),
			imath.V2f( 1, 0.5 ),
			imath.V2f( 0, 1 ),
			imath.V2f( 0.5, 1 ),
			imath.V2f( 1, 1 ),
		] )
		self.assertEqual( m["uv"].indices, IECore.IntVectorData( [ 0, 1, 4, 3, 1, 2, 5, 4, 3, 4, 7, 6, 4, 5, 8, 7 ] ) )
		self.assertEqual( m["otherUVs"].indices, IECore.IntVectorData( [ 0, 1, 4, 3, 1, 2, 5, 4, 3, 4, 7, 6, 4, 5, 8, 7 ] ) )
		for i in range( 0, len(expectedUVs) ) :
			self.assertEqual( m["uv"].data[i], expectedUVs[i] )
			self.assertEqual( m["otherUVs"].data[i], expectedUVs[i] )

		self.assertTrue( m.arePrimitiveVariablesValid() )

		IECoreScene.MeshAlgo.reorderVertices( m, 1, 2, 5 )

		self.assertEqual( m.verticesPerFace, IECore.IntVectorData( [ 4, 4, 4, 4  ] ) )
		self.assertEqual( m.vertexIds, IECore.IntVectorData( [ 0, 1, 2, 3, 3, 2, 4, 5, 3, 5, 6, 7, 3, 7, 8, 0 ] ) )

		self.assertEqual( m["P"].indices, None )
		expectedP = IECore.V3fVectorData( [
			imath.V3f( 0.5, 0, 0 ),
			imath.V3f( 1, 0, 0 ),
			imath.V3f( 1, 0.5, 0 ),
			imath.V3f( 0.5, 0.5, 0 ),
			imath.V3f( 1, 1, 0 ),
			imath.V3f( 0.5, 1, 0 ),
			imath.V3f( 0, 1, 0 ),
			imath.V3f( 0, 0.5, 0 ),
			imath.V3f( 0, 0, 0 ),
		] )
		for i in range( 0, len(expectedP) ) :
			self.assertEqual( m["P"].data[i], expectedP[i] )

		# the index order should have changed
		self.assertEqual( m["uv"].indices, IECore.IntVectorData( [ 1, 2, 5, 4, 4, 5, 8, 7, 4, 7, 6, 3, 4, 3, 0, 1 ] ) )
		self.assertEqual( m["otherUVs"].indices, IECore.IntVectorData( [ 1, 2, 5, 4, 4, 5, 8, 7, 4, 7, 6, 3, 4, 3, 0, 1 ] ) )
		# the data order should be the same as before
		for i in range( 0, len(expectedUVs) ) :
			self.assertEqual( m["uv"].data[i], expectedUVs[i] )
			self.assertEqual( m["otherUVs"].data[i], expectedUVs[i] )

		self.assertTrue( m.arePrimitiveVariablesValid() )

	def testCornersAndCreases( self ) :

		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		cornerIds = [ 5 ]
		cornerSharpnesses = [ 10.0 ]
		m.setCorners( IECore.IntVectorData( cornerIds ), IECore.FloatVectorData( cornerSharpnesses ) )
		creaseLengths = [ 3, 2 ]
		creaseIds = [ 1, 2, 3, 4, 5 ]  # note that these are vertex ids
		creaseSharpnesses = [ 1, 5 ]
		m.setCreases( IECore.IntVectorData( creaseLengths ), IECore.IntVectorData( creaseIds ), IECore.FloatVectorData( creaseSharpnesses ) )

		originalP = m["P"].data.copy()

		IECoreScene.MeshAlgo.reorderVertices( m, 0, 1, 3 )

		# verify the points reordered as expected
		self.assertEqual( m["P"].data[1], originalP[1] )
		self.assertEqual( m["P"].data[2], originalP[2] )
		self.assertEqual( m["P"].data[3], originalP[3] )
		self.assertEqual( m["P"].data[5], originalP[4] )
		self.assertEqual( m["P"].data[7], originalP[5] )
		self.assertEqual( m["P"].data[4], originalP[6] )
		self.assertEqual( m["P"].data[6], originalP[7] )

		# verify the corner and crease ids have been updated to match
		self.assertTrue( m.arePrimitiveVariablesValid() )
		self.assertEqual( m.cornerIds(), IECore.IntVectorData( [ 7 ] ) )
		self.assertEqual( m.cornerSharpnesses(), IECore.FloatVectorData( cornerSharpnesses ) )
		self.assertEqual( m.creaseLengths(), IECore.IntVectorData( creaseLengths ) )
		self.assertEqual( m.creaseIds(), IECore.IntVectorData( [ 1, 2, 3, 5, 7 ] ) )
		self.assertEqual( m.creaseSharpnesses(), IECore.FloatVectorData( creaseSharpnesses ) )

if __name__ == "__main__":
    unittest.main()
