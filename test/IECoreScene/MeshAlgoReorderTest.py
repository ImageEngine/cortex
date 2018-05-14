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

		m = IECore.Reader.create( "test/IECore/data/cobFiles/polySphereQuads.cob" ).read()

		IECoreScene.MeshAlgo.reorderVertices( m, 0, 1, 21 )

		self.assertTrue( m.arePrimitiveVariablesValid() )

		expected = IECore.Reader.create( "test/IECore/data/expectedResults/meshVertexReorderQuadSphere.cob" ).read()

		self.assertEqual( m, expected )

	def testSphere( self ) :

		m = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()

		IECoreScene.MeshAlgo.reorderVertices( m, 20, 1, 21 )

		self.assertTrue( m.arePrimitiveVariablesValid() )

		expected = IECore.Reader.create( "test/IECore/data/expectedResults/meshVertexReorderSphere.cob" ).read()

		self.assertEqual( m, expected )

	def testCube( self ) :

		m = IECore.Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()

		IECoreScene.MeshAlgo.reorderVertices( m, 0, 1, 3 )

		self.assertTrue( m.arePrimitiveVariablesValid() )

if __name__ == "__main__":
    unittest.main()