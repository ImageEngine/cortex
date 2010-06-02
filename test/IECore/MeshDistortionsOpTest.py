##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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



class TestMeshDistortionsOp( unittest.TestCase ) :

	def testSimple( self ) :
		""" Test MeshDistortionsOp """

		verticesPerFace = IntVectorData( [ 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IntVectorData( [ 0,1,5,4, 1,2,6,5, 2,3,7,6,
								    4,5,9,8, 5,6,10,9, 6,7,11,10 ] )
		pRef = V3fVectorData( [ V3f(-1,-1,0), V3f(0,-1,0), V3f(1,-1,0), V3f(2,-1,0),
							 V3f(-1, 0,0), V3f(0, 0,0), V3f(1, 0,0), V3f(2, 0,0),
							 V3f(-1, 1,0), V3f(0, 1,0), V3f(1, 1,0), V3f(2, 1,0) ] )

		# p moves vertex from (0,0,0) to (0.5,0.5,0)
		p = pRef.copy()
		p[5] = V3f(0.5,0.5,0)

		s = FloatVectorData()
		t = FloatVectorData()
		for v in vertexIds :
			s.append( p[v][0] )
			t.append( p[v][1] )

		m = MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		m['Pref'] = PrimitiveVariable( m['P'].interpolation, pRef )
		m['s'] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, s )
		m['t'] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, t )

		op = MeshDistortionsOp()
		res = op(
			input = m,
			uvIndicesPrimVarName = "",
		)

		def vecErr( vec ):
			acc = 0.0
			for v in vec:
				acc += v*v
			return acc

		self.assert_( vecErr( res['distortion'].data - FloatVectorData( [ 0, 0.290569, 0, 0, 0.290569, 0.0834627, -0.103553, 0, 0, -0.207107, 0, 0 ] ) ) < 0.001 )

		self.assert_( vecErr( res['uDistortion'].data - FloatVectorData( [ 0, 0.0918861, 0.0373256, 0.275658, 0.0918861, 0, -0.0732233, 0.0373256, 0, 0, 0, -0.0732233, 0.275658, 0.0373256, -0.146447, 0, 0.0373256, -0.0732233, 0, -0.146447, -0.0732233, 0, 0, 0 ] ) ) < 0.001 )

		self.assert_( vecErr( res['vDistortion'].data - FloatVectorData( [ 0, 0.275658, 0.0373256, 0.0918861, 0.275658, 0, -0.0732233, 0.0373256, 0, 0, 0, -0.0732233, 0.0918861, 0.0373256, -0.146447, 0, 0.0373256, -0.0732233, 0, -0.146447, -0.0732233, 0, 0, 0 ] ) ) < 0.001 )

if __name__ == "__main__":
    unittest.main()
