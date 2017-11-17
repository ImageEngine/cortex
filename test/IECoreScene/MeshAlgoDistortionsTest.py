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
import IECore
import IECoreScene

class MeshAlgoDistortionsTest( unittest.TestCase ) :

	def testSimple( self ) :
		""" Test MeshDistortionsOp """

		verticesPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [
			0,1,5,4,
			1,2,6,5,
			2,3,7,6,
			4,5,9,8,
			5,6,10,9,
			6,7,11,10
		] )
		pRef = IECore.V3fVectorData( [
			IECore.V3f( -1,-1, 0 ), IECore.V3f( 0,-1, 0 ), IECore.V3f( 1,-1, 0 ), IECore.V3f( 2,-1, 0 ),
			IECore.V3f( -1, 0, 0 ), IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 2, 0, 0 ),
			IECore.V3f( -1, 1, 0 ), IECore.V3f( 0, 1, 0 ), IECore.V3f( 1, 1, 0 ), IECore.V3f( 2, 1, 0 ),
		] )

		# p moves vertex from (0,0,0) to (0.5,0.5,0)
		p = pRef.copy()
		p[5] = IECore.V3f( 0.5, 0.5, 0 )

		uvs = IECore.V2fVectorData()
		for v in vertexIds :
			uvs.append( IECore.V2f( p[v][0], p[v][1] ) )

		m = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		m['Pref'] = IECoreScene.PrimitiveVariable( m['P'].interpolation, pRef )
		m['uv'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uvs )

		distortion, uvDistortion = IECoreScene.MeshAlgo.calculateDistortion( m )

		self.assertTrue( m.isPrimitiveVariableValid( distortion ) )
		self.assertTrue( m.isPrimitiveVariableValid( uvDistortion ) )

		expected = IECore.FloatVectorData( [ 0, 0.290569, 0, 0, 0.290569, 0.0834627, -0.103553, 0, 0, -0.207107, 0, 0 ] )
		for i in range( 0, len(expected) ) :
			self.assertAlmostEqual( distortion.data[i], expected[i], 6 )

		def vec2Err( vec ):
			acc = [ 0.0, 0.0 ]
			for v in vec:
				for i in ( 0, 1) :
					acc[i] += v[i]*v[i]
			return acc

		uvExpected = IECore.V2fVectorData( [
			IECore.V2f( 0, 0 ),
			IECore.V2f( 0.0918861, 0.275658 ),
			IECore.V2f( 0.0373256, 0.0373256 ),
			IECore.V2f( 0.275658, 0.0918861 ),
			IECore.V2f( 0.0918861, 0.275658 ),
			IECore.V2f( 0, 0 ),
			IECore.V2f( -0.0732233, -0.0732233 ),
			IECore.V2f( 0.0373256, 0.0373256 ),
			IECore.V2f( 0, 0 ),
			IECore.V2f( 0, 0 ),
			IECore.V2f( 0, 0 ),
			IECore.V2f( -0.0732233, -0.0732233 ),
			IECore.V2f( 0.275658, 0.091886 ),
			IECore.V2f( 0.0373256, 0.0373256 ),
			IECore.V2f( -0.146447, -0.146447 ),
			IECore.V2f( 0, 0 ),
			IECore.V2f( 0.0373256, 0.0373256 ),
			IECore.V2f( -0.0732233,-0.0732233 ),
			IECore.V2f( 0, 0 ),
			IECore.V2f( -0.146447, -0.146447 ),
			IECore.V2f( -0.0732233, -0.0732233 ),
			IECore.V2f( 0, 0 ),
			IECore.V2f( 0, 0 ),
			IECore.V2f( 0, 0 ),
		] )

		for i in range( 0, len(expected) ) :
			self.assertAlmostEqual( uvDistortion.data[i][0], uvExpected[i][0], 6 )
			self.assertAlmostEqual( uvDistortion.data[i][1], uvExpected[i][1], 6 )

if __name__ == "__main__":
    unittest.main()
