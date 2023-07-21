##########################################################################
#
#  Copyright (c) 2019, Image Engine Design Inc. All rights reserved.
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
import time
import IECore
import IECoreScene

class MeshAlgoConnectedVerticesTest( unittest.TestCase ) :


	def generateTestMesh( self ):
		v = imath.V3f

		# p
		#  3_ _2 _5
		#  |   |\ |
		#  |_ _|_\|
		#  0   1  4

		p = IECore.V3fVectorData(
			[
				v( 0, 0, 0 ),
				v( 2, 0, 0 ),
				v( 2, 2, 0 ),
				v( 0, 2, 0 ),
				v( 3, 0, 0 ),
				v( 3, 2, 0 ),
			]
		)

		return IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 4, 3, 3 ] ), IECore.IntVectorData( [ 0, 1, 2, 3, 1, 4, 2, 4, 5, 2 ] ), "linear", p )

	def testConnectedVertices( self ) :

		m = self.generateTestMesh()

		neighborList, offsets = IECoreScene.MeshAlgo.connectedVertices( m )
		neighbors = [ [] for i in offsets ]
		offsets.insert( 0,0 )

		for i in range( len( offsets ) - 1 ):
			neighbors[ i ] = set( neighborList[ offsets[ i ] : offsets[ i + 1 ] ] )

		result = [
			set([1, 3]),
			set([0, 2, 4]),
			set([1, 3, 4, 5]),
			set([0, 2]),
			set([1, 2, 5]),
			set([2, 4])]

		self.assertEqual( neighbors, result)

	def assertCorrespondingFaceVerticesValid( self, faceVertexList, offsets, m ):
		# Check that all face vertices are included
		self.assertEqual(
			sorted( faceVertexList),
			list( range( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) ) )
		)

		# Check that the selected face vertices point back to the vertices that point to them
		refVertList = []
		curOffset = 0
		for i in range( len( offsets ) ):
			refVertList += [ i ] * ( offsets[i] - curOffset )
			curOffset = offsets[i]

		vertIds = m.vertexIds
		self.assertEqual( [ vertIds[i] for i in faceVertexList ], refVertList )

	def testCorrespondingFaceVertices( self ) :

		m = self.generateTestMesh()
		faceVertList, offsets = IECoreScene.MeshAlgo.correspondingFaceVertices( m )
		self.assertCorrespondingFaceVerticesValid( faceVertList, offsets, m )

		m = IECoreScene.MeshPrimitive.createSphere( 1.0, divisions = imath.V2i( 7, 7 ) )
		faceVertList, offsets = IECoreScene.MeshAlgo.correspondingFaceVertices( m )
		self.assertCorrespondingFaceVerticesValid( faceVertList, offsets, m )

	@unittest.skipIf( True, "Not running slow perf tests by default" )
	def testPerformance( self ) :
		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 1000 ) )

		startTime = time.time()
		IECoreScene.MeshAlgo.connectedVertices( m )
		elapsed = time.time() - startTime
		print( "\nTime for 1000000 faces: ", elapsed )

		m = IECoreScene.MeshPrimitive.createSphere( 1.0, divisions = imath.V2i( 10, 10000 ) )

		startTime = time.time()
		IECoreScene.MeshAlgo.connectedVertices( m )
		elapsed = time.time() - startTime
		print( "Time for sphere with very extreme vertices: ", elapsed )

if __name__ == "__main__":
	unittest.main()
