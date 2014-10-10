##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
from IECore import *



class TestMarchingCubesf( unittest.TestCase ) :

	def test( self ) :
		""" Test MarchingCubesf """

		class NoiseFunction( ImplicitSurfaceFunctionV3ff ):

			def __init__( self ) :

				ImplicitSurfaceFunctionV3ff.__init__( self )

				self.n = PerlinNoiseV3ff()

			def getValue( self, p ):

				return self.n.noise( p )

		noiseFn = NoiseFunction()
		builder = MeshPrimitiveBuilder()
		marcher = MarchingCubesf( noiseFn, builder )

		marchMin = V3f(-1, -1, -1)
		marchMax = V3f( 1,  1,  1)
		marchBound = Box3f( marchMin, marchMax )
		marchResolution = V3i( 30, 30, 30 )
		marcher.march( marchBound, marchResolution )

		m = builder.mesh()

		self.assertEqual( len( m.verticesPerFace ), 6967 ) # Tested against OpenEXR 1.6.1
		self.assertEqual( len( m.vertexIds ), 20901 ) # Tested against OpenEXR 1.6.1

		# \todo Verify that vertex positions are close to original implicit surface function

	def testWindingOrder( self ) :

		sphereFn = SphereImplicitSurfaceFunctionV3ff( V3f( 0 ), 1 )
		builder = MeshPrimitiveBuilder()
		marcher = MarchingCubesf( sphereFn, builder )

		marcher.march( Box3f( V3f( -2 ), V3f( 2 ) ), V3i( 100 ) )

		m = builder.mesh()

		Nimplicit = m["N"].data
		MeshNormalsOp()( input=m, copyInput=False )
		N = m["N"].data
		P = m["P"].data

		for ni, n, p in zip( Nimplicit, N, P ) :

			self.assert_( ni.dot( n ) > 0 )
			self.assert_( ni.dot( p ) > 0 )
			self.assert_( n.dot( p ) > 0 )

class TestMarchingCubesd( unittest.TestCase ) :

	def test( self ) :
		""" Test MarchingCubesd """

		class NoiseFunction( ImplicitSurfaceFunctionV3dd ):

			def __init__( self ) :

				ImplicitSurfaceFunctionV3dd.__init__( self )

				# \todo We'd like to use a PerlinNoiseV3dd here, but we don't have one yet!
				self.n = PerlinNoiseV3ff()

			def getValue( self, p ):

				# \todo No need to copy to a V3f once we have a PerlinNoiseV3dd
				return self.n.noise( V3f(p.x, p.y, p.z) )

		noiseFn = NoiseFunction()
		builder = MeshPrimitiveBuilder()
		marcher = MarchingCubesd( noiseFn, builder )

		marchMin = V3d(-1, -1, -1)
		marchMax = V3d( 1,  1,  1)
		marchBound = Box3d( marchMin, marchMax )
		marchResolution = V3i( 30, 30, 30 )
		marcher.march( marchBound, marchResolution, 0.0 )

		m = builder.mesh()

		self.assertEqual( len( m.verticesPerFace ), 6967 ) # Tested against OpenEXR 1.6.1
		self.assertEqual( len( m.vertexIds ), 20901 ) # Tested against OpenEXR 1.6.1

		# \todo Verify that vertex positions are close to original implicit surface function


if __name__ == "__main__":
    unittest.main()
