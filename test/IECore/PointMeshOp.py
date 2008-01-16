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

class TestPointMeshOp( unittest.TestCase ) :

	def testFloat( self ) :
		
		o = PointMeshOp()
		
		points = V3fVectorData()
		radius = DoubleVectorData()
		strength = DoubleVectorData()
		
		bound = Box3f( V3f( -1, -1, -1 ), V3f( 1, 15, 1) )
		resolution = V3i( 10, 80, 10 )
		threshold = 0.1
		
		for i in range( 0, 10):
			points.append( V3f(0, i, 0 ) )
			radius.append( 0.8 )
			strength.append( 1 )
			
		self.assertEqual( len(points), 10 )
		self.assertEqual( len(radius), 10 )
		self.assertEqual( len(strength), 10 )				
		
		m = o(
			points = points,
			radius = radius,
			strength = strength,
			threshold = threshold,
			bound = bound,
			resolution = resolution
		)	

		# Verified by eye
		self.assert_( len(m.vertexIds) > 9000)	
		self.assert_( len(m.vertexIds) < 11000)			
				
		
	def testDouble( self ) :
		
		o = PointMeshOp()
		
		points = V3dVectorData()
		radius = DoubleVectorData()
		strength = DoubleVectorData()
		
		bound = Box3f( V3f( -1, -1, -1 ), V3f( 1, 15, 1) )
		resolution = V3i( 10, 80, 10 )
		threshold = 0.1
		
		for i in range( 0, 10):
			points.append( V3d(0, i, 0 ) )
			radius.append( 0.8 )
			strength.append( 1 )
			
		self.assertEqual( len(points), 10 )
		self.assertEqual( len(radius), 10 )
		self.assertEqual( len(strength), 10 )				
		
		m = o(
			points = points,
			radius = radius,
			strength = strength,
			threshold = threshold,
			bound = bound,
			resolution = resolution
		)	
		
		# Verified by eye
		self.assert_( len(m.vertexIds) > 9000)	
		self.assert_( len(m.vertexIds) < 11000)			
		
				

if __name__ == "__main__":
	unittest.main()
	
