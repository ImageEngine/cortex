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

class TestCSGImplicitSurfaceFunction( unittest.TestCase ) :
	class SphereFunctionV3ff( ImplicitSurfaceFunctionV3ff ):
		
		def __init__( self, centre, radius ) :	

			ImplicitSurfaceFunctionV3ff.__init__( self )

			self.centre = centre
			self.radius = radius

		def getValue( self, p ):

			return -(1.0 - ( p - self.centre ).length()  / self.radius)

	def testUnion( self ) :
		""" Test implicit surface CSG union """
		sphere1 = TestCSGImplicitSurfaceFunction.SphereFunctionV3ff( V3f(0,0,0), 1 )
		sphere2 = TestCSGImplicitSurfaceFunction.SphereFunctionV3ff( V3f(0,1,0), 1 )
		
		csgFn = CSGImplicitSurfaceFunctionV3ff( sphere1, sphere2, CSGImplicitSurfaceFunctionV3ff.Mode.Union )
		builder = MeshPrimitiveBuilderf()       
		marcher = MarchingCubesf( csgFn, builder )
                                
		marchMin = V3f(-2.5, -2.5, -2.5)
		marchMax = V3f( 2.5,  2.5,  2.5)
		marchBound = Box3f( marchMin, marchMax )
		marchResolution = V3i( 30, 30, 30 )
		marcher.march( marchBound, marchResolution )
                                
		m = builder.mesh()
		
		# Verified visually
		self.assertEqual( len( m.vertexIds ), 5760 )

	def testIntersection( self ):
		""" Test implicit surface CSG intersection """
		sphere1 = TestCSGImplicitSurfaceFunction.SphereFunctionV3ff( V3f(0,0,0), 1 )
		sphere2 = TestCSGImplicitSurfaceFunction.SphereFunctionV3ff( V3f(0,1,0), 1 )
				
		csgFn = CSGImplicitSurfaceFunctionV3ff( sphere1, sphere2, CSGImplicitSurfaceFunctionV3ff.Mode.Intersection )				
		builder = MeshPrimitiveBuilderf()       				
		marcher = MarchingCubesf( csgFn, builder )
		
		            
		marchMin = V3f(-2.5, -2.5, -2.5)
		marchMax = V3f( 2.5,  2.5,  2.5)
		marchBound = Box3f( marchMin, marchMax )
		marchResolution = V3i( 30, 30, 30 )
		marcher.march( marchBound, marchResolution )
		                 
		m = builder.mesh()
		
		# Verified visually
		self.assertEqual( len( m.vertexIds ), 2088 )
	
	def testDifference( self ):	
	
		""" Test implicit surface CSG difference """
		sphere1 = TestCSGImplicitSurfaceFunction.SphereFunctionV3ff( V3f(0,0,0), 1 )
		sphere2 = TestCSGImplicitSurfaceFunction.SphereFunctionV3ff( V3f(0,1,0), 1 )
				
		csgFn = CSGImplicitSurfaceFunctionV3ff( sphere1, sphere2, CSGImplicitSurfaceFunctionV3ff.Mode.Difference )				
		builder = MeshPrimitiveBuilderf()       				
		marcher = MarchingCubesf( csgFn, builder )
		
		            
		marchMin = V3f(-2.5, -2.5, -2.5)
		marchMax = V3f( 2.5,  2.5,  2.5)
		marchBound = Box3f( marchMin, marchMax )
		marchResolution = V3i( 30, 30, 30 )
		marcher.march( marchBound, marchResolution )
		                 
		m = builder.mesh()
		
		# Verified visually
		self.assertEqual( len( m.vertexIds ), 3672 )
			
				
	

if __name__ == "__main__":
    unittest.main()   
