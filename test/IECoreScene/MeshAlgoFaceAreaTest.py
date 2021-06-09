##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
import imath
import IECore
import IECoreScene
import math
import threading
import time

class MeshAlgoFaceAreaTest( unittest.TestCase ) :

	def test( self ) :

		p = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -2, -1 ), imath.V2f( 2, 1 ) ) )

		faceArea = IECoreScene.MeshAlgo.calculateFaceArea( p )

		self.assertTrue( p.isPrimitiveVariableValid( faceArea ) )

		self.assertEqual( faceArea.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( faceArea.data[0], 8 )

	def testRandomTriangles( self ) :

		r = imath.Rand32()
		for i in range( 0, 1000 ) :

			p = IECore.V3fVectorData( [
				imath.V3f( r.nextf(), r.nextf(), r.nextf() ),
				imath.V3f( r.nextf(), r.nextf(), r.nextf() ),
				imath.V3f( r.nextf(), r.nextf(), r.nextf() ),
			] )
			m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ), "linear", p )

			uv = IECore.V2fVectorData( [ imath.V2f( r.nextf(), r.nextf() ), imath.V2f( r.nextf(), r.nextf() ), imath.V2f( r.nextf(), r.nextf() ) ] )
			m["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, uv )

			faceArea = IECoreScene.MeshAlgo.calculateFaceArea( m )
			textureArea = IECoreScene.MeshAlgo.calculateFaceTextureArea( m, "uv" )

			self.assertAlmostEqual( faceArea.data[0], IECore.triangleArea( p[0], p[1], p[2] ), 4 )
			self.assertAlmostEqual( textureArea.data[0], IECore.triangleArea( imath.V3f( uv[0][0], uv[0][1], 0 ), imath.V3f( uv[1][0], uv[1][1], 0 ), imath.V3f( uv[2][0], uv[2][1], 0 ) ), 4 )

	def testTwoFaces( self ) :

		v = imath.V3f

		# P
		#  _ _
		# |   |\
		# |_ _|_\
		#
		# uv
		#
		#  _ _ _
		# |     |  |\
		# |_ _ _|  |_\
		#

		p = IECore.V3fVectorData(
			[
				v( 0, 0, 0 ),
				v( 2, 0, 0 ),
				v( 2, 2, 0 ),
				v( 0, 2, 0 ),
				v( 3, 0, 0 ),
			]
		)

		uvs = IECore.V2fVectorData(
			[
				imath.V2f( 0, 0 ),
				imath.V2f( 3, 0 ),
				imath.V2f( 3, 2 ),
				imath.V2f( 0, 2 ),
				imath.V2f( 5, 0 ),
				imath.V2f( 6, 0 ),
				imath.V2f( 5, 2 ),
			]
		)

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 4, 3 ] ), IECore.IntVectorData( [ 0, 1, 2, 3, 1, 4, 2 ] ), "linear", p )
		m["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, uvs )

		faceArea = IECoreScene.MeshAlgo.calculateFaceArea( m )
		textureArea = IECoreScene.MeshAlgo.calculateFaceTextureArea( m, "uv" )

		faceAreas = faceArea.data
		self.assertEqual( len( faceAreas ), 2 )
		self.assertEqual( faceAreas[0], 4 )
		self.assertEqual( faceAreas[1], 1 )

		textureAreas = textureArea.data
		self.assertEqual( len( textureAreas ), 2 )
		self.assertEqual( textureAreas[0], 6 )
		self.assertEqual( textureAreas[1], 1 )

	def testCancel( self ) :
		canceller = IECore.Canceller()
		cancelled = [False]

		# Basic large mesh
		strip = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1000000, 1 ) ), imath.V2i( 1000000, 1 ) )

		def backgroundRun( texture ):
			try:
				if texture:
					IECoreScene.MeshAlgo.calculateFaceTextureArea( strip, "uv", "P", canceller )
				else:
					IECoreScene.MeshAlgo.calculateFaceArea( strip, "P", canceller )
			except IECore.Cancelled:
				cancelled[0] = True

		for texture in [ False, True ]:
			cancelled[0] = False
			thread = threading.Thread(target=backgroundRun, args=(texture,))

			startTime = time.time()
			thread.start()

			time.sleep( 0.01 )
			canceller.cancel()
			thread.join()

			self.assertLess( time.time() - startTime, 0.03 )
			self.assertTrue( cancelled[0] )

if __name__ == "__main__":
    unittest.main()
