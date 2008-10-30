##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
import random
import math
import os
from IECore import *

class GammaOp( ColorTransformOp ) :

	def __init__( self, gamma = 1.0 ) :
	
		ColorTransformOp.__init__( self, "gamma", "applies gamma" )

		self.gamma = gamma
		
	def begin( self, operands ) :
	
		pass	
	
	def transform( self, color ) :			
	
		return Color3f(
			math.pow( color.r, 1.0 / self.gamma ),
			math.pow( color.g, 1.0 / self.gamma ),
			math.pow( color.b, 1.0 / self.gamma )
		)	
		
	def end( self ) :
	
		pass	


class CubeColorLookupTest( unittest.TestCase ) :

	def testOpConstruction( self ) :
	
		gammaOp = GammaOp( 2.0 )
		
		dim = V3i( 48, 66, 101 )
		cubeLookup = CubeColorLookupf( dim, gammaOp )
		
		random.seed( 23 )
		
		# Perform 100 random comparisons with the LUT against the original function
		for i in range( 0, 100 ) :
		
			c = Color3f( random.random(), random.random(), random.random() )
			
			c1 = cubeLookup( c )
			c2 = gammaOp.transform( c )
									
			self.assertAlmostEqual( c1.r, c2.r, 1 )
			self.assertAlmostEqual( c1.g, c2.g, 1 )
			self.assertAlmostEqual( c1.b, c2.b, 1 )
			
	
		
if __name__ == "__main__":
	unittest.main()
	
