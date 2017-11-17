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
import IECoreScene

class MeshAlgoResampleTest( unittest.TestCase ) :

	@classmethod
	def makeMesh( cls ) :
		testObject = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 10 ) ), IECore.V2i( 2 ) )

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 9 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 4 ) ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 9 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 16 ) ) )

		# indexed
		testObject["f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ) )
		testObject["g"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0 ] ) )
		testObject["h"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ) )
		testObject["i"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 ] ) )

		return testObject

	@classmethod
	def setUpClass(cls):
		cls.mesh = cls.makeMesh()

	def testMeshConstantToVertex( self ) :

		p = self.mesh["a"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex );
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 9 ) )

	def testMeshConstantToUniform( self ) :

		p = self.mesh["a"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 4 ) )

	def testMeshConstantToVarying( self ) :

		p = self.mesh["a"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p,  IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 9 ) )

	def testMeshConstantToFaceVarying( self ) :

		p = self.mesh["a"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 16 ) )


	def testMeshVertexToConstant( self ) :
		p = self.mesh["b"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,9))/9. ) )

	def testMeshVertexToUniform( self ) :
		p = self.mesh["b"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 2, 3, 5, 6 ] ) )

	def testMeshVertexToVarying( self ) :
		p = self.mesh["b"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 9 ) ) )

	def testMeshVertexToFaceVarying( self ) :
		p = self.mesh["b"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, IECore.FloatVectorData( [ orig[x] for x in self.mesh.vertexIds ] ) )

	def testMeshUniformToConstant( self ) :
		p = self.mesh["c"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,4))/4. ) )

	def testMeshUniformToVertex( self ) :
		p = self.mesh["c"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 0.5, 1, 1, 1.5, 2, 2, 2.5, 3 ] ) )

	def testMeshUniformToVarying( self ) :
		p = self.mesh["c"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 0.5, 1, 1, 1.5, 2, 2, 2.5, 3 ] ) )

	def testMeshUniformToFaceVarying( self ) :
		p = self.mesh["c"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 4 ) + ( [ 1 ] * 4 ) + ( [ 2 ] * 4 ) + ( [ 3 ] * 4 ) ) )

	def testMeshVaryingToConstant( self ) :
		p = self.mesh["d"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,9))/9. ) )

	def testMeshVaryingToVertex( self ) :
		p = self.mesh["d"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 9 ) ) )

	def testMeshVaryingToUniform( self ) :
		p = self.mesh["d"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 2, 3, 5, 6 ] ) )

	def testMeshVaryingToFaceVarying( self ) :
		p = self.mesh["d"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, IECore.FloatVectorData( [ orig[x] for x in self.mesh.vertexIds ] ) )

	def testMeshFaceVaryingToConstant( self ) :
		p = self.mesh["e"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,16))/16. ) )

	def testMeshFaceVaryingToVertex( self ) :
		p = self.mesh["e"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 2.5, 5, 5.5, 7.5, 9.5, 11, 12.5, 14 ] ) )

	def testMeshFaceVaryingToUniform( self ) :
		p = self.mesh["e"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 1.5, 5.5, 9.5, 13.5 ] ) )

	def testMeshFaceVaryingToVarying( self ) :
		p = self.mesh["e"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 2.5, 5, 5.5, 7.5, 9.5, 11, 12.5, 14 ] ) )

	def testMeshIndexedVertexToUniform( self ) :
		p = self.mesh["f"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5, 1.5, 0.5, 1.5 ] ) )
		self.assertEqual( p.indices, None )

	def testMeshIndexedVertexToVarying( self ) :
		p = self.mesh["f"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ) )

	def testMeshIndexedVertexToFaceVarying( self ) :
		p = self.mesh["f"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 1, 1, 0, 1, 2, 2, 1, 0, 1, 1, 0, 1, 2, 2, 1 ] ) )

	def testMeshIndexedUniformToVertex( self ) :
		p = self.mesh["g"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 0, 1, 1, 0, 0, 2, 1, 0 ] ) )

	def testMeshIndexedUniformToVarying( self ) :
		p = self.mesh["g"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 0, 1, 1, 0, 0, 2, 1, 0 ] ) )

	def testMeshIndexedUniformToFaceVarying( self ) :
		p = self.mesh["g"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0 ] ) )

	def testMeshIndexedVaryingToVertex( self ) :
		p = self.mesh["h"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ) )
		self.assertEqual( p.indices, None )

	def testMeshIndexedVaryingToUniform( self ) :
		p = self.mesh["h"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5, 1.5, 0.5, 1.5 ] ) )
		self.assertEqual( p.indices, None )

	def testMeshIndexedVaryingToFaceVarying( self ) :
		p = self.mesh["h"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 1, 1, 0, 1, 2, 2, 1, 0, 1, 1, 0, 1, 2, 2, 1 ] ) )

	def testMeshIndexedFaceVaryingToVertex( self ) :
		p = self.mesh["i"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2, 1, 0.75, 0.5, 2, 0.5, 2 ] ) )
		self.assertEqual( p.indices, None )

	def testMeshIndexedFaceVaryingToUniform( self ) :
		p = self.mesh["i"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.75, 1, 1.25, 0.75 ] ) )
		self.assertEqual( p.indices, None )

	def testMeshIndexedFaceVaryingToVarying( self ) :
		p = self.mesh["i"]
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( self.mesh, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 1, 2, 1, 0.75, 0.5, 2, 0.5, 2 ] ) )
		self.assertEqual( p.indices, None )

	def testInitialisationOfVectorData( self ) :

		# This exercises a bug whereby the resampling methods that use averaging
		# were not initialising the results to zero before accumulating.

		m = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ), IECore.V2i( 100 ) )

		for i in range( 0, 10 ) :

			for interpolation in ( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) :

				pv = IECoreScene.PrimitiveVariable(
					interpolation,
					IECore.V2fVectorData( [ IECore.V2f( 0 ) ] * m.variableSize( interpolation ) )
				)

				IECoreScene.MeshAlgo.resamplePrimitiveVariable( m, pv, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
				self.assertEqual( len( pv.data ), m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) )
				for v in pv.data :
					self.assertEqual( v, IECore.V2f( 0 ) )

if __name__ == "__main__":
	unittest.main()
