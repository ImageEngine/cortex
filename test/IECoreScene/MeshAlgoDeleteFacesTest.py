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

import IECore

import IECoreScene

import unittest
import imath

class MeshAlgoDeleteFacesTest( unittest.TestCase ) :

	def makeQuadTriangleMesh( self ):

		verticesPerFace = IECore.IntVectorData( [ 3, 3 ] )
		vertexIds = IECore.IntVectorData( [ 0, 1, 2, 0, 2, 3 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 0, 1, 0 ) ] )
		uv = IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 0 ), imath.V2f( 1, 1 ), imath.V2f( 0, 0 ), imath.V2f( 1, 1 ), imath.V2f( 0, 1 ) ] )

		mesh = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uv )

		return mesh

	def testHandlesInvalidPrimvarType( self ) :
		deleteAttributeData = IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 0, 1, 0 ) ]  )
		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"] ) )

	def testHandlesInvalidPrimvarInterpolation( self ) :
		deleteAttributeData = IECore.IntVectorData( [0, 0, 0, 0, 0, 0] )
		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, deleteAttributeData )

		self.assertRaises( RuntimeError, lambda : IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"] ) )

	def testCanRemoveAllFaces( self ) :
		deleteAttributeData = IECore.IntVectorData( [1, 1] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 0 )
		self.assertEqual( len( facesDeletedMesh.verticesPerFace ), 0 )
		self.assertEqual( len( facesDeletedMesh.vertexIds ), 0 )

		self.assertEqual( len( facesDeletedMesh["uv"].data ), 0 )
		self.assertEqual( len( facesDeletedMesh["P"].data ), 0 )
		self.assertEqual( len( facesDeletedMesh["delete"].data ), 0 )

	def testNoFacesRemovedIfFlagIsTrueButIfInverted( self ) :
		deleteAttributeData = IECore.IntVectorData( [1, 1] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"], invert = True )

		self.assertEqual( facesDeletedMesh.numFaces(), 2 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3, 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [ 0, 1, 2, 0, 2, 3 ] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 0 ), imath.V2f( 1, 1 ), imath.V2f( 0, 0 ), imath.V2f( 1, 1 ), imath.V2f( 0, 1 ) ] ) )
		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 0, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.IntVectorData( [1, 1] ) )

	def testCanRemoveFirstFace( self ) :
		deleteAttributeData = IECore.IntVectorData( [1, 0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [3] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 1 ), imath.V2f( 0, 1 ) ] ) )

		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 0, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.IntVectorData( [0] ) )

	def testRemovesAllButFirstIfInverted( self ) :
		deleteAttributeData = IECore.IntVectorData( [1, 0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"], invert = True )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [ 0, 1, 2 ] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 0 ), imath.V2f( 1, 1 ) ] ) )
		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 1, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.IntVectorData( [1] ) )

	def testCanRemoveFirstFaceBool( self ) :
		deleteAttributeData = IECore.BoolVectorData( [True, False] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 1 ), imath.V2f( 0, 1 ) ] ) )

		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 0, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.BoolVectorData( [False] ) )

	def testRemovesAllButFirstIfInvertedBool( self ) :
		deleteAttributeData = IECore.BoolVectorData( [True, False] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"], invert = True)

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [ 0, 1, 2 ] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 0 ), imath.V2f( 1, 1 ) ] ) )
		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 1, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.BoolVectorData( [True] ) )

	def testCanRemoveFirstFaceFloat( self ) :
		deleteAttributeData = IECore.FloatVectorData( [1.0, 0.0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [3] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 1 ), imath.V2f( 0, 1 ) ] ) )

		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 0, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.FloatVectorData( [0.0] ) )

	def testRemovesAllButFirstIfInvertedFloat( self ) :
		deleteAttributeData = IECore.FloatVectorData( [1.0, 0.0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( mesh, mesh["delete"], invert = True  )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [ 0, 1, 2 ] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 0 ), imath.V2f( 1, 1 ) ] ) )
		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 1, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.FloatVectorData( [1.0] ) )

	def testIndexPrimitiveVariables( self ):

		#                                     +-----------------+
		#                                     |     |XXXXX|     |    invert = True (3 faces remain)
		#                +------------------> |     |XXXXX|     |
		#                |                    |     |XXXXX|     |
		#                |                    +-----------------+
		#                +                    |     |XXXXX|     |
		#       12    13    14    15          |     |XXXXX|     |
		#       +-----+-----+-----+           |     |XXXXX|     |
		#       |     |     |     |           +-----------------+
		#       |     |     |     |           |     |XXXXX|     |
		#       |8    |9    |10   |11         |     |XXXXX|     |
		#       +-----------------+           |     |XXXXX|     |
		#       |     |     |     |           +-----------------+
		#       |     |     |     |
		#       |4    |5    |6    |7          +-----------------+
		#       +-----------------+           |XXXXX|     |XXXXX|    invert = False (6 faces remain)
		#       |     |     |     |           |XXXXX|     |XXXXX|
		#       |     |     |     |           |XXXXX|     |XXXXX|
		#       |0    |1    |2    |3          +-----------------+
		#       +-----+-----+-----+           |XXXXX|     |XXXXX|
		#                                     |XXXXX|     |XXXXX|
		#                +                    |XXXXX|     |XXXXX|
		#                |                    +-----------------+
		#                +------------------> |XXXXX|     |XXXXX|
		#                                     |XXXXX|     |XXXXX|
		#                                     |XXXXX|     |XXXXX|
		#                                     +-----------------+

		planeMesh3x3 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 3, 3 ) )

		primvarUniformNonIndexed = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform,
			IECore.IntVectorData( [0, 1, 0, 0, 1, 0, 0, 1, 0] ) )

		primvarUniformIndexed = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [0, 1, 0] ),
			IECore.IntVectorData( [0, 1, 2, 0, 1, 2, 0, 1, 2] ) )

		primvarVertexNonIndexed = IECoreScene.PrimitiveVariable (
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.IntVectorData(
				[
					0, 0, 0, 0,
					1, 1, 1, 1,
					2, 2, 2, 2,
					3, 3, 3, 3
				]
			)
		)

		primvarVertexIndexed = IECoreScene.PrimitiveVariable (
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.IntVectorData(
				[ 4, 3, 2, 1 ]
			),
			IECore.IntVectorData( [0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3] )
		)

		primvarFaceVaryingIndexed = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.FaceVarying,
			IECore.IntVectorData( [3, 2, 1] ),
			IECore.IntVectorData( [
				0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
				0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
				0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2
			] )
		)

		planeMesh3x3["primvarUniformNonIndexed"] = primvarUniformNonIndexed
		planeMesh3x3["primvarUniformIndexed"] = primvarUniformIndexed
		planeMesh3x3["primvarVertexNonIndexed"] = primvarVertexNonIndexed
		planeMesh3x3["primvarVertexIndexed"] = primvarVertexIndexed
		planeMesh3x3["primvarFaceVaryingIndexed"] = primvarFaceVaryingIndexed

		self.assertTrue( planeMesh3x3.arePrimitiveVariablesValid() )

		# delete an indexed uniform primitive variable using a non indexed uniform primitive variable to flag which faces to delete

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( planeMesh3x3, planeMesh3x3["primvarUniformNonIndexed"], invert = False )
		self.assertTrue( facesDeletedMesh.arePrimitiveVariablesValid() )

		self.assertEqual( facesDeletedMesh.numFaces(), 6 )

		self.assertEqual( facesDeletedMesh["primvarUniformIndexed"].data, IECore.IntVectorData( [0, 0] ) )
		self.assertEqual( facesDeletedMesh["primvarUniformIndexed"].indices, IECore.IntVectorData( [0, 1, 0, 1, 0, 1] ) )

		# delete an indexed vertex primitive variable using an indexed uniform primitive variable to flag which faces to delete

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( planeMesh3x3, planeMesh3x3["primvarUniformNonIndexed"], invert = True )
		self.assertTrue( facesDeletedMesh.arePrimitiveVariablesValid() )

		self.assertEqual( facesDeletedMesh.numFaces(), 3 )
		self.assertEqual( facesDeletedMesh["primvarVertexIndexed"].data, IECore.IntVectorData( [4, 3, 2, 1] ) )
		self.assertEqual( facesDeletedMesh["primvarVertexIndexed"].indices, IECore.IntVectorData( [0, 0, 1, 1, 2, 2, 3, 3] ) )

		# delete an indexed uniform primitive variable using an indexed uniform primitive variable to flag which faces to delete

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( planeMesh3x3, planeMesh3x3["primvarUniformIndexed"], invert = False )
		self.assertTrue( facesDeletedMesh.arePrimitiveVariablesValid() )

		self.assertEqual( facesDeletedMesh.numFaces(), 6 )

		self.assertEqual( facesDeletedMesh["primvarUniformIndexed"].data, IECore.IntVectorData( [0, 0] ) )
		self.assertEqual( facesDeletedMesh["primvarUniformIndexed"].indices, IECore.IntVectorData( [0, 1, 0, 1, 0, 1] ) )

		# delete an indexed FaceVarying primitive variable using an indexed uniform primitive variable to flag which faces to delete

		facesDeletedMesh = IECoreScene.MeshAlgo.deleteFaces( planeMesh3x3, planeMesh3x3["primvarUniformIndexed"], invert = True)
		self.assertTrue( facesDeletedMesh.arePrimitiveVariablesValid() )

		self.assertEqual( facesDeletedMesh.numFaces(), 3 )

		self.assertEqual( facesDeletedMesh["primvarFaceVaryingIndexed"].data, IECore.IntVectorData( [ 2 ] ) )
		self.assertEqual( facesDeletedMesh["primvarFaceVaryingIndexed"].indices, IECore.IntVectorData( [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] ) )


if __name__ == "__main__":
	unittest.main()
