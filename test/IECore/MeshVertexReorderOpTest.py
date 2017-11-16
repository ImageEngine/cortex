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

import unittest
import IECore

class MeshVertexReorderOpTest( unittest.TestCase ) :

	def testPlane( self ) :

		verticesPerFace = IECore.IntVectorData()

		vertexIds = IECore.IntVectorData()

		verticesPerFace.append( 3 )
		vertexIds.append( 0 )
		vertexIds.append( 1 )
		vertexIds.append( 2 )

		verticesPerFace.append( 3 )
		vertexIds.append( 1 )
		vertexIds.append( 3 )
		vertexIds.append( 2 )

		m = IECore.MeshPrimitive( verticesPerFace, vertexIds )

		p = IECore.V3fVectorData()
		p.append( IECore.V3f( -1, -1, 0 ) )
		p.append( IECore.V3f(  1, -1, 0 ) )
		p.append( IECore.V3f( -1,  1, 0 ) )
		p.append( IECore.V3f(  1,  1, 0 ) )

		s = IECore.FloatVectorData()
		s.append( 0 )
		s.append( 1 )
		s.append( 0 )

		s.append( 1 )
		s.append( 1 )
		s.append( 0 )


		t = IECore.FloatVectorData()
		t.append( 0 )
		t.append( 0 )
		t.append( 1 )

		t.append( 0 )
		t.append( 1 )
		t.append( 1 )

		uni = IECore.IntVectorData()
		uni.append( 0 )
		uni.append( 1 )

		m["P"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, p )
		m["s"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, s )
		m["t"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, t )
		m["uni"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, uni )

		self.assert_( m.arePrimitiveVariablesValid() )

		op = IECore.MeshVertexReorderOp()

		result = op(
			input = m,
			startingVertices = IECore.V3i( 2, 3, 1 )
		)

		expectedVerticesPerFace = IECore.IntVectorData()
		expectedVerticesPerFace.append( 3 )
		expectedVerticesPerFace.append( 3 )

		self.assertEqual( result.verticesPerFace, expectedVerticesPerFace )

		expectedVertexIds = IECore.IntVectorData()
		expectedVertexIds.append( 0 )
		expectedVertexIds.append( 1 )
		expectedVertexIds.append( 2 )
		expectedVertexIds.append( 0 )
		expectedVertexIds.append( 2 )
		expectedVertexIds.append( 3 )

		self.assertEqual( result.vertexIds, expectedVertexIds )

		expectedP = IECore.V3fVectorData()
		expectedP.append( IECore.V3f( -1,  1, 0 ) )
		expectedP.append( IECore.V3f(  1,  1, 0 ) )
		expectedP.append( IECore.V3f(  1, -1, 0 ) )
		expectedP.append( IECore.V3f( -1, -1, 0 ) )

		self.assertEqual( result["P"].data, expectedP )

		expectedS = IECore.FloatVectorData()
		expectedS.append( 0 )
		expectedS.append( 1 )
		expectedS.append( 1 )

		expectedS.append( 0 )
		expectedS.append( 1 )
		expectedS.append( 0 )

		self.assertEqual( result["s"].data, expectedS )

		expectedT = IECore.FloatVectorData()
		expectedT.append( 1 )
		expectedT.append( 1 )
		expectedT.append( 0 )

		expectedT.append( 1 )
		expectedT.append( 0 )
		expectedT.append( 0 )

		self.assertEqual( result["t"].data, expectedT )

		expectedUni = IECore.IntVectorData()
		expectedUni.append( 1 )
		expectedUni.append( 0 )

		self.assertEqual( result["uni"].data, expectedUni )

		self.assert_( result.arePrimitiveVariablesValid() )

	def testPlaneOppositeWinding( self ) :

		verticesPerFace = IECore.IntVectorData()

		vertexIds = IECore.IntVectorData()

		verticesPerFace.append( 3 )
		vertexIds.append( 2 )
		vertexIds.append( 1 )
		vertexIds.append( 0 )

		verticesPerFace.append( 3 )
		vertexIds.append( 2 )
		vertexIds.append( 3 )
		vertexIds.append( 1 )

		m = IECore.MeshPrimitive( verticesPerFace, vertexIds )

		p = IECore.V3fVectorData()
		p.append( IECore.V3f( -1, -1, 0 ) )
		p.append( IECore.V3f(  1, -1, 0 ) )
		p.append( IECore.V3f( -1,  1, 0 ) )
		p.append( IECore.V3f(  1,  1, 0 ) )

		s = IECore.FloatVectorData()
		s.append( 0 )
		s.append( 1 )
		s.append( 0 )

		s.append( 0 )
		s.append( 1 )
		s.append( 1 )


		t = IECore.FloatVectorData()
		t.append( 1 )
		t.append( 0 )
		t.append( 0 )

		t.append( 1 )
		t.append( 1 )
		t.append( 0 )

		uni = IECore.IntVectorData()
		uni.append( 1 )
		uni.append( 0 )

		m["P"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, p )
		m["s"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, s )
		m["t"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, t )
		m["uni"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, uni )

		self.assert_( m.arePrimitiveVariablesValid() )

		op = IECore.MeshVertexReorderOp()

		result = op(
			input = m,
			startingVertices = IECore.V3i( 2, 3, 1 )
		)

		expectedVerticesPerFace = IECore.IntVectorData()
		expectedVerticesPerFace.append( 3 )
		expectedVerticesPerFace.append( 3 )

		self.assertEqual( result.verticesPerFace, expectedVerticesPerFace )

		expectedVertexIds = IECore.IntVectorData()
		expectedVertexIds.append( 0 )
		expectedVertexIds.append( 1 )
		expectedVertexIds.append( 2 )
		expectedVertexIds.append( 0 )
		expectedVertexIds.append( 2 )
		expectedVertexIds.append( 3 )

		self.assertEqual( result.vertexIds, expectedVertexIds )

		expectedP = IECore.V3fVectorData()
		expectedP.append( IECore.V3f( -1,  1, 0 ) )
		expectedP.append( IECore.V3f(  1,  1, 0 ) )
		expectedP.append( IECore.V3f(  1, -1, 0 ) )
		expectedP.append( IECore.V3f( -1, -1, 0 ) )

		self.assertEqual( result["P"].data, expectedP )

		expectedS = IECore.FloatVectorData()
		expectedS.append( 0 )
		expectedS.append( 1 )
		expectedS.append( 1 )

		expectedS.append( 0 )
		expectedS.append( 1 )
		expectedS.append( 0 )

		self.assertEqual( result["s"].data, expectedS )

		expectedT = IECore.FloatVectorData()
		expectedT.append( 1 )
		expectedT.append( 1 )
		expectedT.append( 0 )

		expectedT.append( 1 )
		expectedT.append( 0 )
		expectedT.append( 0 )

		self.assertEqual( result["t"].data, expectedT )

		expectedUni = IECore.IntVectorData()
		expectedUni.append( 0 )
		expectedUni.append( 1 )

		self.assertEqual( result["uni"].data, expectedUni )

		self.assert_( result.arePrimitiveVariablesValid() )

	def testQuadSphere( self ) :

		m = IECore.Reader.create( "test/IECore/data/cobFiles/polySphereQuads.cob" ).read()

		op = IECore.MeshVertexReorderOp()

		result = op(
			input = m,
			startingVertices = IECore.V3i( 0, 1, 21 )
		)

		self.assert_( result.arePrimitiveVariablesValid() )

		expected = IECore.Reader.create( "test/IECore/data/expectedResults/meshVertexReorderQuadSphere.cob" ).read()

		self.assertEqual( result, expected )

	def testSphere( self ) :

		m = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()

		op = IECore.MeshVertexReorderOp()

		result = op(
			input = m,
			startingVertices = IECore.V3i( 20, 1, 21 )
		)

		self.assert_( result.arePrimitiveVariablesValid() )

		expected = IECore.Reader.create( "test/IECore/data/expectedResults/meshVertexReorderSphere.cob" ).read()

		self.assertEqual( result, expected )

	def testCube( self ) :

		m = IECore.Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()

		op = IECore.MeshVertexReorderOp()

		result = op(
			input = m,
			startingVertices = IECore.V3i( 0, 1, 3 )
		)

		self.assert_( result.arePrimitiveVariablesValid() )



if __name__ == "__main__":
    unittest.main()
