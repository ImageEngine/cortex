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
from IECore import *

class MeshAlgoDeleteFacesTest( unittest.TestCase ) :

	def makeQuadTriangleMesh( self ):

		verticesPerFace = IntVectorData( [ 3, 3 ] )
		vertexIds = IntVectorData( [ 0, 1, 2, 0, 2, 3 ] )
		p = V3fVectorData( [ V3f( 0, 0, 0 ), V3f( 1, 0, 0 ), V3f( 1, 1, 0 ), V3f( 0, 1, 0 ) ] )
		uv = V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] )

		mesh = MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["uv"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, uv )

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

		self.assertEqual( len( facesDeletedMesh["uv"].data ), 0 )
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

		self.assertEqual( facesDeletedMesh["uv"].data, V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] ) )

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

		self.assertEqual( facesDeletedMesh["uv"].data, V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] ) )

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

		self.assertEqual( facesDeletedMesh["uv"].data, V2fVectorData( [ IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ), IECore.V2f( 0, 1 ) ] ) )

		self.assertEqual( facesDeletedMesh["P"].data, V3fVectorData( [V3f( 0, 0, 0 ), V3f( 1, 1, 0 ), V3f( 0, 1, 0 )] ) )
		self.assertEqual( facesDeletedMesh["delete"].data, FloatVectorData( [0.0] ) )

if __name__ == "__main__":
	unittest.main()
