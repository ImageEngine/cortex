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

import random
import unittest
import os
from IECore import *

class TestInverseDistanceWeightedInterpolation(unittest.TestCase):

	def testSimple( self ):
	
		p = V3fVectorData()
		v = FloatVectorData()
		
		p.append( V3f( -1,  1, 0 ) )
		p.append( V3f( -1, -1, 0 ) )		
		p.append( V3f(  1,  1, 0 ) )		
		p.append( V3f(  1, -1, 0 ) )						
		
		v.append( 1 )
		v.append( 2 )
		v.append( 3 )
		v.append( 4 )						
	
		idw = InverseDistanceWeightedInterpolationV3ff( p, v, 1 )
		
		for i in range( 0, 4 ):
		
			res = idw( p[i] )
			
			self.assertAlmostEqual( res, v[i] )
			
	def testRandomPoints( self ):
	
		random.seed( 1 )
	
		p = V2fVectorData()
		v = FloatVectorData()
		
		size = 256
		numPoints = 100
								
		for i in range( 0, numPoints ):
		
			p.append( V2f( random.uniform( 0, size ), random.uniform( 0, size ) ) )
			v.append( random.uniform( 0, 1 ) )
		
		idw = InverseDistanceWeightedInterpolationV2ff( p, v, 10 )	
			
		b = Box2i( V2i(0, 0), V2i( size-1, size-1 ) )
				
		o = 0
		f = FloatVectorData( size * size )
		for i in range( 0, size ):
		
			for j in range( 0, size ) :
			
				r =  idw( V2f( i, j ) )

				f[o] = r
				o = o + 1
			
		i = ImagePrimitive( b, b )					
		i["r"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, f )
		i["g"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, f )
		i["b"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, f )
		
		Writer.create( i, "test/InverseDistanceWeightedInterpolationV2ff.exr" ).write()
		
		# \todo Verify this output with known "good" image
		
	def tearDown( self ) :
				
		if os.path.isfile( 'test/InverseDistanceWeightedInterpolationV2ff.exr' ):
			os.remove( 'test/InverseDistanceWeightedInterpolationV2ff.exr' )
		
	
if __name__ == "__main__":
	unittest.main()
