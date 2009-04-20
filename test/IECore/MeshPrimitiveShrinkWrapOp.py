##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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
import random
from IECore import *



class TestMeshPrimitiveShrinkWrapOp( unittest.TestCase ) :

	def testSimple( self ) :
		""" Test MeshPrimitiveShrinkWrapOp """
		
		random.seed( 1011 )
	
		# Load poly sphere of radius 1
		m = Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		radius = 1.0
				
		# Duplicate and scale to radius 3, jitter slightly
		targetRadius = 3.0
		target = m.copy()
		
		pData = m["P"].data
		
		target["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData() )
		for p in pData:
			target["P"].data.append( p * targetRadius + 0.01 * V3f( random.random(), random.random(), random.random() ) ) 
			
		self.assertEqual( len( target["P"].data ), len( m["P"].data ) )
	
		# Shrink wrap smaller mesh to larger mesh
		op = MeshPrimitiveShrinkWrapOp()
		res = op(
			target = target,			
			input = m,
			
			method =  MeshPrimitiveShrinkWrapOp.Method.Normal,
			direction = MeshPrimitiveShrinkWrapOp.Direction.Both
		)

		pData = res["P"].data
		for p in pData:
			self.assert_( math.fabs( p.length() - targetRadius ) < 0.1 )
			
			
	

if __name__ == "__main__":
    unittest.main()   
