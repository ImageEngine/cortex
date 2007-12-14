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
		builder = MeshPrimitiveBuilderf()	
		marcher = MarchingCubesf( noiseFn, builder )
				
		marchMin = V3f(-1, -1, -1)
		marchMax = V3f( 1,  1,  1)
		marchBound = Box3f( marchMin, marchMax )
		marchResolution = V3i( 30, 30, 30 )
		marcher.march( marchBound, marchResolution )
				
		m = builder.mesh()

		self.assertEqual( len( m.verticesPerFace ), 7491 )
		self.assertEqual( len( m.vertexIds ), 22473 )
		
		# \todo Verify that vertex positions are close to original implicit surface function
		
		
class TestMarchingCubesd( unittest.TestCase ) :

	def test( self ) :
		""" Test MarchingCubesd """
	
		class NoiseFunction( ImplicitSurfaceFunctionV3dd ):
		
			def __init__( self ) :	
			
				ImplicitSurfaceFunctionV3dd.__init__( self )
				
				self.n = PerlinNoiseV3ff()
				
			def getValue( self, p ):
			
				return self.n.noise( V3f(p.x, p.y, p.z) )
		
		noiseFn = NoiseFunction()	
		builder = MeshPrimitiveBuilderf()	
		marcher = MarchingCubesd( noiseFn, builder )
				
		marchMin = V3d(-1, -1, -1)
		marchMax = V3d( 1,  1,  1)
		marchBound = Box3d( marchMin, marchMax )
		marchResolution = V3i( 30, 30, 30 )
		marcher.march( marchBound, marchResolution, 0.0 )
				
		m = builder.mesh()

		self.assertEqual( len( m.verticesPerFace ), 7491 )
		self.assertEqual( len( m.vertexIds ), 22473 )
		
		# \todo Verify that vertex positions are close to original implicit surface function		


if __name__ == "__main__":
    unittest.main()   
