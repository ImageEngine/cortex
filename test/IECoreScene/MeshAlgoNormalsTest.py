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

import math
import random
import threading
import time
import unittest
import os

import IECore
import IECoreScene

import imath

class MeshAlgoNormalsTest( unittest.TestCase ) :

	def assertVectorListsAlmostEqual( self, vectors, expected, absError = 1e-7 ):
		self.assertEqual( len( vectors ), len( expected ) )
		for i in range( len( vectors ) ):
			if not vectors[i].equalWithAbsError( imath.V3f( expected[i] ), absError ):
				raise AssertionError( "Mismatch in vectors - first failing element %i, %s != %s" % ( i, repr( vectors[i] ), repr( expected[i] ) ) )

	def testPlane( self ) :

		p = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		del p["N"]

		normals = IECoreScene.MeshAlgo.calculateVertexNormals( p, IECoreScene.MeshAlgo.NormalWeighting.Equal )

		self.assertEqual( normals.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertTrue( normals.data.isInstanceOf( IECore.V3fVectorData.staticTypeId() ) )
		self.assertEqual( normals.data.size(), p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( normals.data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		for n in normals.data :
			self.assertEqual( n, imath.V3f( 0, 0, 1 ) )

	def testSphere( self ) :

		s = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pSphereShape1.cob" ) ).read()
		del s["N"]

		normals = IECoreScene.MeshAlgo.calculateVertexNormals( s, IECoreScene.MeshAlgo.NormalWeighting.Equal )

		self.assertEqual( normals.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertTrue( normals.data.isInstanceOf( IECore.V3fVectorData.staticTypeId() ) )
		self.assertEqual( normals.data.size(), s.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( normals.data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		points = s["P"].data
		for i in range( 0, normals.data.size() ) :

			p = points[i].normalize()
			with self.subTest( curNormal = normals.data[i], curPoint = p ):

				self.assertLess( math.fabs( normals.data[i].length() - 1 ), 0.001 )

				self.assertGreater( normals.data[i].dot( p ), 0.99 )
				self.assertLess( normals.data[i].dot( p ), 1.01 )

	def testUniformInterpolation( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 10 ) )
		del m["N"]

		normals = IECoreScene.MeshAlgo.calculateUniformNormals( m )
		self.assertEqual( normals.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( len( normals.data ), m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

		for n in normals.data :
			self.assertEqual( n, imath.V3f( 0, 0, 1 ) )

	def testHardSurfaceChamfers( self ):
		# The sort of case where using area weighting looks a lot nicer
		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ), imath.V2i( 16, 2 ) )
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ imath.V3f(
				( i // 8 ) * 4 + min(4, i % 8 ), y,
				( max( 0, i % 8 - 4) - 2 ) * math.copysign( 0.05, i % 16 - 8 ) )
				for y in [-1,0,1] for i in range( 17 )
			] )
		)

		normals = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal )

		# Unpleasantly rounded edges
		diag = imath.V3f(1, 0, 1 ) * 0.5 ** 0.5
		expected = (
			[ ( 0, 0, 1 ) ] * 4 + [ diag ] + [ ( 1, 0, 0 ) ] * 3 + [ diag ] +
			[ ( 0, 0, 1 ) ] * 3 + [ imath.V3f( -1, 0, 1 ) * diag ] + [ ( -1, 0, 0 ) ] * 4
		) * 3
		self.assertVectorListsAlmostEqual( normals.data, expected )

		# Using vertex angle weighting gives the same result ( since all the angles are 90 degrees )
		normals = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Angle )
		self.assertVectorListsAlmostEqual( normals.data, expected )

		normals = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Area )

		# The top face are fairly large, and the side faces are fairly small, so with area weighting, the edge
		# normals will stay fairly close to vertical
		expected = (
			[ ( 0, 0, 1 ) ] * 5 + [ ( 1, 0, 0 ) ] * 3 +
			[ ( 0, 0, 1 ) ] * 5 + [ ( -1, 0, 0 ) ] * 4
		) * 3
		self.assertVectorListsAlmostEqual( normals.data, expected, absError = 0.05 )

	def testConcaveQuads( self ):
		# Test a plane distorted into an arrow head shape, so that it is still perfectly flat, but contains
		# some concave polygons.
		for dir in [ (1, 1), (1, -1), (-1, -1), (-1, 1) ]:
			m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 4 ) )
			m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex,
				IECore.V3fVectorData( [ p + imath.V3f( dir[0], dir[1], 0 ) * abs( p[0] * dir[1] - p[1] * dir[0] ) for p in m["P"].data  ] )
			)
			normals = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal )
			self.assertVectorListsAlmostEqual( normals.data, [ (0,0,1) ] * len( normals.data ) )

			normals = IECoreScene.MeshAlgo.calculateUniformNormals( m )
			self.assertVectorListsAlmostEqual( normals.data, [ (0,0,1) ] * len( normals.data ) )

	def testTriangulatedCube( self ):
		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		mt = IECoreScene.MeshAlgo.triangulate( m )

		normals = IECoreScene.MeshAlgo.calculateVertexNormals( mt, IECoreScene.MeshAlgo.NormalWeighting.Equal )

		# Every vertex ends up touching both triangles on at least one face, so all the normals are at
		# some sort of weird off-kilter angle
		for n in normals.data:
			sortedVec = imath.V3f( *sorted( [ abs(i) for i in n ] ) )
			self.assertTrue(
				sortedVec.equalWithRelError( imath.V3f( 1, 1, 2 ).normalized(), 1e-7 ) or
				sortedVec.equalWithRelError( imath.V3f( 1, 2, 2 ).normalized(), 1e-7 )
			)

		# Using vertex angle weighting get everything lined up nicely
		normals = IECoreScene.MeshAlgo.calculateVertexNormals( mt, IECoreScene.MeshAlgo.NormalWeighting.Angle )

		for n in normals.data:
			self.assertAlmostEqual( abs( n[0] ), 1 / 3 ** 0.5, places = 6 )
			self.assertAlmostEqual( abs( n[1] ), 1 / 3 ** 0.5, places = 6 )
			self.assertAlmostEqual( abs( n[2] ), 1 / 3 ** 0.5, places = 6 )

	def testDiagonalBentPlane( self ):

		# A plane rotated 45 degrees and then bent 90 degrees in the middle will contain non-planar
		# faces along the edge - the edge faces and vertices should have diagonal normals, but
		# everything else should be flat
		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 4, 4 ) )

		rotP = [ p * imath.Eulerf( 0, 0, math.pi / 4 ).toMatrix33() for p in m["P"].data ]
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ p if p[0] > 0 else imath.V3f( 0, p[1], p[0] ) for p in rotP ] )
		)

		diag = imath.V3f( -1, 0, 1 ).normalized()
		up = imath.V3f( 0, 0, 1 )
		side = imath.V3f( -1, 0, 0 )
		normals = IECoreScene.MeshAlgo.calculateUniformNormals( m )
		self.assertVectorListsAlmostEqual( normals.data, [
			diag, up, up, up,
			side, diag, up, up,
			side, side, diag, up,
			side, side, side, diag
		], absError = 1e-6 )

		normals = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Angle )
		self.assertVectorListsAlmostEqual( normals.data, [
			imath.V3f( -1, 1, 1 ).normalized(), up, up, up, up,
			side, diag, up, up, up,
			side, side, diag, up, up,
			side, side, side, diag, up,
			side, side, side, side, imath.V3f( -1, -1, 1 ).normalized()
		], absError = 1e-6 )

	def testAlternatingHeightPlane( self ):

		# A plane with alterating vertices offset outward in Z should still have all the normals
		# average to flat, except for the vertex normals at the edge
		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 6, 6 ) )
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ m["P"].data[i] + 0.5 * imath.V3f( 0, 0, i % 2 )  for i in range( len( m["P"].data ) ) ] )
		)

		normals = IECoreScene.MeshAlgo.calculateUniformNormals( m )
		self.assertVectorListsAlmostEqual( normals.data, [ imath.V3f( 0, 0, 1 ) ] * len( normals.data ) )

		normals = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Angle )
		for x in range( 1, 6 ):
			for y in range( 1, 6 ):
				self.assertAlmostEqual( ( normals.data[ y * 7 + x ] - imath.V3f( 0, 0, 1 ) ).length(), 0, places = 6 )

	def testAngle( self ):

		angles = [ 0.01, 0.1, 0.15, 0.3, 1, 2, 3, 3.14 ]
		for a in angles:
			for b in angles:
				m = IECoreScene.MeshPrimitive(
					IECore.IntVectorData( [3, 3] ),
					IECore.IntVectorData( [ 0, 1, 2, 1, 0, 3] ),
					"linear",
					IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 0, 0, 1 ), imath.V3f( math.sin( a ), 0, math.cos( a ) ), imath.V3f( 0, math.sin( b ), math.cos( b ) ) ] )
				)
				normals = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Angle )

				with self.subTest( a = a, b = b, norm = normals.data[0] ):
					self.assertTrue( normals.data[0].equalWithAbsError( imath.V3f( b, a, 0 ).normalized(), 1e-5 ) )

	def testNonPlanarSide( self ):
		# Test that the normals we generate agree with the normals generated by IECore.polygonNormal for
		# all sorts of weird twisted shapes. In order to pass this test, we use IECore.polygonNormal inside
		# calculateNormals for faces with more than 4 verts. Note that our results for some of these
		# polygons are actually quite bad ( we basically fail on polygons with more than 180 degrees of bend
		# within one polygon ), but forcing the normals to point in the direction of polygonNormal is
		# consistent, and only breaks in cases we don't care about.
		random.seed( 42 )
		for num in [ 3, 4, 5, 6 ]:
			for t in range( 1000 ):
				p = IECore.V3fVectorData( [ imath.V3f( random.random(), random.random(), random.random() ) for i in range( num ) ] )
				m = IECoreScene.MeshPrimitive(
					IECore.IntVectorData( [ num ] ), IECore.IntVectorData( range( num ) ), "linear", p
				)

				normals = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal )

				faceNormal = IECore.polygonNormal( p )

				with self.subTest( num = num, t = t, p = p ):
					for n in normals.data:
						self.assertGreater( faceNormal.dot( n ), 0 )

	def testFaceVarying( self ):

		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )

		# An 89 degree angle threshold treats a cube as perfectly faceted
		n = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal, 89 )
		refN = IECoreScene.MeshAlgo.calculateUniformNormals( m )
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( m, refN, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertVectorListsAlmostEqual( n.data, refN.data, absError = 1e-6 )

		# A 91 degree angle threshold treats a cube like a sphere
		n = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal, 91 )
		refN = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal )
		IECoreScene.MeshAlgo.resamplePrimitiveVariable( m, refN, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertVectorListsAlmostEqual( n.data, refN.data, absError = 1e-6 )


		# Test with a plane that's been bent in half at a list of possible angles.
		# For every angle, a threshold lower than the angle results in faceted faces,
		# and higher than that angle produces a smooth result
		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 2, 1 ) )
		pData = list( m["P"].data )
		for angle in range( 1, 179 ):

			# Create a plane bent by exactly "angle"
			pData[0][0] = pData[3][0] = -math.cos( angle / 180 * math.pi )
			pData[0][2] = pData[3][2] = math.sin( angle / 180 * math.pi )
			m["P"] = IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( pData )
			)

			# With a threshold less than "angle, the result will match using uniform normals
			n = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal, angle - 0.5 )
			refN = IECoreScene.MeshAlgo.calculateUniformNormals( m )
			IECoreScene.MeshAlgo.resamplePrimitiveVariable( m, refN, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
			self.assertVectorListsAlmostEqual( n.data, refN.data, absError = 1e-6 )

			# With a threshold greater than "angle, the result will match using vertex normals
			n = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal, angle + 0.5 )
			refN = IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal )
			IECoreScene.MeshAlgo.resamplePrimitiveVariable( m, refN, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
			self.assertVectorListsAlmostEqual( n.data, refN.data, absError = 1e-6 )

	def testFaceVaryingOnSphere( self ):
		for divs in [ 10, 64, 10000 ]:
			for weighting in [ IECoreScene.MeshAlgo.NormalWeighting.Area, IECoreScene.MeshAlgo.NormalWeighting.Angle, IECoreScene.MeshAlgo.NormalWeighting.Equal ]:
				with self.subTest( divs = divs, weighting = weighting ):
					m = IECoreScene.MeshPrimitive.createSphere( 1, divisions = imath.V2i( 3, divs ) )
					n = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, weighting, 0 )

					refN = IECoreScene.MeshAlgo.calculateUniformNormals( m )
					IECoreScene.MeshAlgo.resamplePrimitiveVariable( m, refN, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
					self.assertVectorListsAlmostEqual( n.data, refN.data, absError = 1e-3 if divs == 10000 else 1e-5 )

					n = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, weighting, 180 )
					refN = IECoreScene.MeshAlgo.calculateVertexNormals( m, weighting )
					IECoreScene.MeshAlgo.resamplePrimitiveVariable( m, refN, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
					self.assertVectorListsAlmostEqual( n.data, refN.data, absError = 1e-5 )

					# 66 degrees is enough to merge all the faces, and have everything meet at the same normal
					# at the pole
					n = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, weighting, 66 )
					self.assertVectorListsAlmostEqual( n.data, refN.data, absError = 1e-5 )
					self.assertVectorListsAlmostEqual( list( n.data )[0:3 * divs:3], [ imath.V3f( 0, 0, -1 ) ] * divs, 1e-5 )

					# At 40 degrees, we don't merge across the pole, and every face-vertex normal at the pole
					# should be pointing in the direction of its face
					n = IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, weighting, 40 )

					zComp = -0.93073827
					if divs == 64:
						# As the tiny sides get closer together, they start to be averaged, changing the
						# normal slightly
						zComp = -0.934919655
					elif divs == 10000:
						# Once we have more than 64 faces touching the vert, we trip the special case where
						# the vert is treated as either all smooth ( like the above case where the threshold
						# is high ), or all sharp. This is actually a bit more accurate for a cone tip, without
						# averaging in the nearby cone faces that have similar normals and pushing the normal
						# towards the axis.
						zComp = -0.8660255
						
					planeComp = ( 1 - zComp * zComp ) ** 0.5
					angles = [ 2 * math.pi * ( i + 0.5 ) / divs for i in range( divs ) ]
					self.assertVectorListsAlmostEqual( list( n.data )[0:3 * divs:3], [ imath.V3f( planeComp * math.cos( a ), planeComp * math.sin( a ), zComp ) for a in angles ], absError = 1e-3 if divs == 10000 else 1e-5 )

	@unittest.skipIf( True, "Not running slow perf tests by default" )
	def testPerformance( self ) :
		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 1000 ) )

		startTime = time.time()
		IECoreScene.MeshAlgo.calculateNormals( m, IECoreScene.PrimitiveVariable.Interpolation.Uniform, "P" )
		elapsed = time.time() - startTime
		print( "\nTime for 1000000 faces, uniform deprecated: ", elapsed )

		startTime = time.time()
		IECoreScene.MeshAlgo.calculateNormals( m, IECoreScene.PrimitiveVariable.Interpolation.Uniform, "P" )
		elapsed = time.time() - startTime
		print( "\nTime for 1000000 faces, uniform: ", elapsed )

		startTime = time.time()
		IECoreScene.MeshAlgo.calculateNormals( m, IECoreScene.PrimitiveVariable.Interpolation.Vertex, "P" )
		elapsed = time.time() - startTime
		print( "\nTime for 1000000 faces, vertex deprecated: ", elapsed )

		startTime = time.time()
		IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Equal )
		elapsed = time.time() - startTime
		print( "Time for 1000000 faces, equal weighting: ", elapsed )

		startTime = time.time()
		IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Angle )
		elapsed = time.time() - startTime
		print( "Time for 1000000 faces, vertex angle: ", elapsed )

		startTime = time.time()
		IECoreScene.MeshAlgo.calculateVertexNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Area )
		elapsed = time.time() - startTime
		print( "Time for 1000000 faces, face area weighting: ", elapsed )

		m = IECoreScene.MeshPrimitive.createSphere( 1, divisions = imath.V2i( 20, 50000 ) )
		startTime = time.time()
		IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Area, 180 )
		elapsed = time.time() - startTime
		print( "Time for 1000000 face sphere with extreme verts, unthresholded faceVarying: ", elapsed )

		startTime = time.time()
		IECoreScene.MeshAlgo.calculateFaceVaryingNormals( m, IECoreScene.MeshAlgo.NormalWeighting.Area, 40 )
		elapsed = time.time() - startTime
		print( "Time for 1000000 face sphere with extreme verts, thresholded faceVarying: ", elapsed )

	@unittest.skipIf( ( IECore.TestUtil.inMacCI() or IECore.TestUtil.inWindowsCI() ), "Mac and Windows CI are too slow for reliable timing" )
	def testCancel( self ) :
		canceller = IECore.Canceller()
		cancelled = [False]

		strip = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 3000000, 1 ) ), imath.V2i( 3000000, 1 ) )
		def backgroundRun():

			try:
				IECoreScene.MeshAlgo.calculateVertexNormals( strip, IECoreScene.MeshAlgo.NormalWeighting.Equal, canceller = canceller )
			except IECore.Cancelled:
				cancelled[0] = True

		thread = threading.Thread(target=backgroundRun, args=())

		startTime = time.time()
		thread.start()

		time.sleep( 0.1 )
		canceller.cancel()
		thread.join()

		# This test should actually produce a time extremely close to the sleep duration ( within
		# 0.003 seconds whether the sleep duration is 0.01 seconds or 100 seconds ), but checking
		# that it terminates with 0.1 seconds is a minimal performance bar
		self.assertLess( time.time() - startTime, 0.2 )
		self.assertTrue( cancelled[0] )

if __name__ == "__main__":
	unittest.main()
