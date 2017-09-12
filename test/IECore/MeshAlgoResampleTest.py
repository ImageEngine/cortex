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

class MeshAlgoResampleTest( unittest.TestCase ) :

	@classmethod
	def makeMesh( cls ) :
		testObject = MeshPrimitive.createPlane( Box2f( V2f( 0 ), V2f( 10 ) ), V2i( 2 ) )

		testObject["a"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatData( 0.5 ) )
		testObject["b"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, FloatVectorData( range( 0, 9 ) ) )
		testObject["c"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( range( 0, 4 ) ) )
		testObject["d"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Varying, FloatVectorData( range( 0, 9 ) ) )
		testObject["e"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, FloatVectorData( range( 0, 16 ) ) )

		return testObject

	@classmethod
	def setUpClass(cls):
		cls.mesh = cls.makeMesh()

	def testMeshConstantToVertex( self ) :

		p = self.mesh["a"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p, PrimitiveVariable.Interpolation.Vertex );
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, FloatVectorData( [ 0.5 ] * 9 ) )

	def testMeshConstantToUniform( self ) :

		p = self.mesh["a"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, FloatVectorData( [ 0.5 ] * 4 ) )

	def testMeshConstantToVarying( self ) :

		p = self.mesh["a"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, FloatVectorData( [ 0.5 ] * 9 ) )

	def testMeshConstantToFaceVarying( self ) :

		p = self.mesh["a"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, FloatVectorData( [ 0.5 ] * 16 ) )


	def testMeshVertexToConstant( self ) :
		p = self.mesh["b"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, FloatData( sum(range(0,9))/9. ) )

	def testMeshVertexToUniform( self ) :
		p = self.mesh["b"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, FloatVectorData( [ 2, 3, 5, 6 ] ) )

	def testMeshVertexToVarying( self ) :
		p = self.mesh["b"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, FloatVectorData( range( 0, 9 ) ) )

	def testMeshVertexToFaceVarying( self ) :
		p = self.mesh["b"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, FloatVectorData( [ orig[x] for x in self.mesh.vertexIds ] ) )

	def testMeshUniformToConstant( self ) :
		p = self.mesh["c"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, FloatData( sum(range(0,4))/4. ) )

	def testMeshUniformToVertex( self ) :
		p = self.mesh["c"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, FloatVectorData( [ 0, 0.5, 1, 1, 1.5, 2, 2, 2.5, 3 ] ) )

	def testMeshUniformToVarying( self ) :
		p = self.mesh["c"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, FloatVectorData( [ 0, 0.5, 1, 1, 1.5, 2, 2, 2.5, 3 ] ) )

	def testMeshUniformToFaceVarying( self ) :
		p = self.mesh["c"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, FloatVectorData( ( [ 0 ] * 4 ) + ( [ 1 ] * 4 ) + ( [ 2 ] * 4 ) + ( [ 3 ] * 4 ) ) )

	def testMeshVaryingToConstant( self ) :
		p = self.mesh["d"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, FloatData( sum(range(0,9))/9. ) )

	def testMeshVaryingToVertex( self ) :
		p = self.mesh["d"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, FloatVectorData( range( 0, 9 ) ) )

	def testMeshVaryingToUniform( self ) :
		p = self.mesh["d"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, FloatVectorData( [ 2, 3, 5, 6 ] ) )

	def testMeshVaryingToFaceVarying( self ) :
		p = self.mesh["d"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, FloatVectorData( [ orig[x] for x in self.mesh.vertexIds ] ) )

	def testMeshFaceVaryingToConstant( self ) :
		p = self.mesh["e"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, FloatData( sum(range(0,16))/16. ) )

	def testMeshFaceVaryingToVertex( self ) :
		p = self.mesh["e"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, FloatVectorData( [ 0, 2.5, 5, 5.5, 7.5, 9.5, 11, 12.5, 14 ] ) )

	def testMeshFaceVaryingToUniform( self ) :
		p = self.mesh["e"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, FloatVectorData( [ 1.5, 5.5, 9.5, 13.5 ] ) )

	def testMeshFaceVaryingToVarying( self ) :
		p = self.mesh["e"]
		MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, FloatVectorData( [ 0, 2.5, 5, 5.5, 7.5, 9.5, 11, 12.5, 14 ] ) )

if __name__ == "__main__":
	unittest.main()
