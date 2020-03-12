##########################################################################
#
#  Copyright (c) 2019, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreScene

import imath

class MeshAlgoNormalsTest( unittest.TestCase ) :

	def testPlane( self ) :

		p = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		del p["N"]

		normals = IECoreScene.MeshAlgo.calculateNormals( p )

		self.assertEqual( normals.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertTrue( normals.data.isInstanceOf( IECore.V3fVectorData.staticTypeId() ) )
		self.assertEqual( normals.data.size(), p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( normals.data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		for n in normals.data :
			self.assertEqual( n, imath.V3f( 0, 0, 1 ) )

	def testSphere( self ) :

		s = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		del s["N"]

		normals = IECoreScene.MeshAlgo.calculateNormals( s )

		self.assertEqual( normals.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertTrue( normals.data.isInstanceOf( IECore.V3fVectorData.staticTypeId() ) )
		self.assertEqual( normals.data.size(), s.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( normals.data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		points = s["P"].data
		for i in range( 0, normals.data.size() ) :

			self.assertTrue( math.fabs( normals.data[i].length() - 1 ) < 0.001 )

			p = points[i].normalize()
			self.assertTrue( normals.data[i].dot( p ) > 0.99 )
			self.assertTrue( normals.data[i].dot( p ) < 1.01 )

	def testUniformInterpolation( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 10 ) )
		del m["N"]

		normals = IECoreScene.MeshAlgo.calculateNormals( m, interpolation = IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( normals.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( len( normals.data ), m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

		for n in normals.data :
			self.assertEqual( n, imath.V3f( 0, 0, 1 ) )

if __name__ == "__main__":
	unittest.main()
