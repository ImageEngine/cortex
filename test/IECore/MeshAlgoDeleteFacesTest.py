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

import IECore

class MeshAlgoDeleteFacesTest( unittest.TestCase ) :

	def makeQuadTriangleMesh( self ):

		verticesPerFace = IECore.IntVectorData( [ 3, 3 ] )
		vertexIds = IECore.IntVectorData( [ 0, 1, 2, 0, 2, 3 ] )
		p = IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 1, 0 ), IECore.V3f( 0, 1, 0 ) ] )
		uv = IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] )

		mesh = IECore.MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["uv"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, uv )

		return mesh

	def testHandlesInvalidPrimvarType( self ) :
		deleteAttributeData = IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 1, 0 ), IECore.V3f( 0, 1, 0 ) ]  )
		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		self.assertRaises( RuntimeError, lambda : IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"] ) )

	def testHandlesInvalidPrimvarInterpolation( self ) :
		deleteAttributeData = IECore.IntVectorData( [0, 0, 0, 0, 0, 0] )
		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, deleteAttributeData )

		self.assertRaises( RuntimeError, lambda : IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"] ) )

	def testCanRemoveAllFaces( self ) :
		deleteAttributeData = IECore.IntVectorData( [1, 1] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 0 )
		self.assertEqual( len( facesDeletedMesh.verticesPerFace ), 0 )
		self.assertEqual( len( facesDeletedMesh.vertexIds ), 0 )

		self.assertEqual( len( facesDeletedMesh["uv"].data ), 0 )
		self.assertEqual( len( facesDeletedMesh["P"].data ), 0 )
		self.assertEqual( len( facesDeletedMesh["delete"].data ), 0 )

	
	def testNoFacesRemovedIfFlagIsTrueButIfInverted( self ) :
		deleteAttributeData = IECore.IntVectorData( [1, 1] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"], invert = True )

		self.assertEqual( facesDeletedMesh.numFaces(), 2 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3, 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [ 0, 1, 2, 0, 2, 3 ] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] ) )
		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 1, 0 ), IECore.V3f( 0, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.IntVectorData( [1, 1] ) )


	def testCanRemoveFirstFace( self ) :
		deleteAttributeData = IECore.IntVectorData( [1, 0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [3] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] ) )

		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 1, 0 ), IECore.V3f( 0, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.IntVectorData( [0] ) )

	def testRemovesAllButFirstIfInverted( self ) :
		deleteAttributeData = IECore.IntVectorData( [1, 0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"], invert = True )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [ 0, 1, 2 ] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 0 ), IECore.V2f( 1, 1 ) ] ) )
		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.IntVectorData( [1] ) )

	def testCanRemoveFirstFaceBool( self ) :
		deleteAttributeData = IECore.BoolVectorData( [True, False] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] ) )

		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 1, 0 ), IECore.V3f( 0, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.BoolVectorData( [False] ) )
	
	def testRemovesAllButFirstIfInvertedBool( self ) :
		deleteAttributeData = IECore.BoolVectorData( [True, False] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"], invert = True)

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [ 0, 1, 2 ] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 0 ), IECore.V2f( 1, 1 ) ] ) )
		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.BoolVectorData( [True] ) )

	def testCanRemoveFirstFaceFloat( self ) :
		deleteAttributeData = IECore.FloatVectorData( [1.0, 0.0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"] )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [3] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [0, 1, 2] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] ) )

		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 1, 0 ), IECore.V3f( 0, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.FloatVectorData( [0.0] ) )

	
	def testRemovesAllButFirstIfInvertedFloat( self ) :
		deleteAttributeData = IECore.FloatVectorData( [1.0, 0.0] )

		mesh = self.makeQuadTriangleMesh()
		mesh["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, deleteAttributeData )

		facesDeletedMesh = IECore.MeshAlgo.deleteFaces( mesh, mesh["delete"], invert = True  )

		self.assertEqual( facesDeletedMesh.numFaces(), 1 )
		self.assertEqual( facesDeletedMesh.verticesPerFace, IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( facesDeletedMesh.vertexIds, IECore.IntVectorData( [ 0, 1, 2 ] ) )

		self.assertEqual( facesDeletedMesh["uv"].data, IECore.V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 0 ), IECore.V2f( 1, 1 ) ] ) )
		self.assertEqual( facesDeletedMesh["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 1, 0 ) ] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, IECore.FloatVectorData( [1.0] ) )

if __name__ == "__main__":
	unittest.main()
