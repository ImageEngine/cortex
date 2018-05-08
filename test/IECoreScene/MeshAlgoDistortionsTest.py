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
import imath
import IECore
import IECoreScene

class MeshAlgoDistortionsTest( unittest.TestCase ) :

	def testSimple( self ) :

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
			imath.V3f( -1,-1, 0 ), imath.V3f( 0,-1, 0 ), imath.V3f( 1,-1, 0 ), imath.V3f( 2,-1, 0 ),
			imath.V3f( -1, 0, 0 ), imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ), imath.V3f( 2, 0, 0 ),
			imath.V3f( -1, 1, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 2, 1, 0 ),
		] )

		# p moves vertex from (0,0,0) to (0.5,0.5,0)
		p = pRef.copy()
		p[5] = imath.V3f( 0.5, 0.5, 0 )

		uvs = IECore.V2fVectorData()
		for v in vertexIds :
			uvs.append( imath.V2f( p[v][0], p[v][1] ) )

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
			imath.V2f( 0, 0 ),
			imath.V2f( 0.0918861, 0.275658 ),
			imath.V2f( 0.367544, 0.367544 ),
			imath.V2f( 0.275658, 0.0918861 ),
			imath.V2f( 0.0918861, 0.275658 ),
			imath.V2f( 0, 0 ),
			imath.V2f( -0.146447, -0.146447 ),
			imath.V2f( -0.0545605, 0.129212 ),
			imath.V2f( 0, 0 ),
			imath.V2f( 0, 0 ),
			imath.V2f( 0, 0 ),
			imath.V2f( 0, 0 ),
			imath.V2f( 0.275658, 0.0918861 ),
			imath.V2f( 0.129212, -0.0545605 ),
			imath.V2f( -0.146447, -0.146447 ),
			imath.V2f( 0, 0 ),
			imath.V2f( -0.292893, -0.292893 ),
			imath.V2f( -0.146447, -0.146447 ),
			imath.V2f( 0, 0 ),
			imath.V2f( -0.146447, -0.146447 ),
			imath.V2f( 0, 0 ),
			imath.V2f( 0, 0 ),
			imath.V2f( 0, 0 ),
			imath.V2f( 0, 0 ),
		] )

		self.assertEqual( len( uvExpected ), len( uvDistortion.data ) )

		for i in range( 0, len( uvExpected ) ) :
			self.assertAlmostEqual( uvDistortion.data[i][0], uvExpected[i][0], 6 )
			self.assertAlmostEqual( uvDistortion.data[i][1], uvExpected[i][1], 6 )

	def testIndexedUVs( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 2 ) )
		self.assertEqual( m["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m["uv"].indices, m.vertexIds )

		m["Pref"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			m["P"].data.copy()
		)

		m["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ p * imath.V3f( 0.5, 1, 1 ) for p in m["P"].data ] )
		)

		distortion, uvDistortion = IECoreScene.MeshAlgo.calculateDistortion( m )

		self.assertEqual( uvDistortion.interpolation, m["uv"].interpolation )
		self.assertEqual( len( uvDistortion.data ), len( m["uv"].data ) )
		self.assertEqual( uvDistortion.indices, m["uv"].indices )

		for d in uvDistortion.data :
			self.assertEqual( d, imath.V2f( -0.5, 0 ) )

if __name__ == "__main__":
    unittest.main()
