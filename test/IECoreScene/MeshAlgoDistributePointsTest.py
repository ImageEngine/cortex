##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

import os
import re
import sys
import unittest
import imath
import time
import threading

import IECore
import IECoreScene

class MeshAlgoDistributePointsTest( unittest.TestCase ) :

	def pointTest( self, mesh, points, density, error=0.05 ) :

		self.assertTrue( "P" in points )
		self.assertEqual( points.numPoints, points['P'].data.size() )
		self.assertTrue( points.arePrimitiveVariablesValid() )
		self.assertEqual( points['P'].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		mesh = IECoreScene.MeshAlgo.triangulate( mesh )
		meshEvaluator = IECoreScene.MeshPrimitiveEvaluator( mesh )
		result = meshEvaluator.createResult()
		pointsPerFace = [ 0 ] * mesh.verticesPerFace.size()
		positions = points["P"].data

		## test that the points are on the mesh
		for p in positions :
			self.assertAlmostEqual( meshEvaluator.signedDistance( p, result ), 0.0, 3 )
			pointsPerFace[result.triangleIndex()] += 1

		## test that we have roughly the expected density per face
		origDensity = density
		mesh["faceArea"] = IECoreScene.MeshAlgo.calculateFaceArea( mesh )
		for f in range( 0, mesh.verticesPerFace.size() ) :
			if "density" in mesh :
				if mesh["density"].interpolation == IECoreScene.PrimitiveVariable.Interpolation.Constant:
					# For consistency, no primitive variable may increase the density, only decrease it.
					# At least currently, we could just fix this now.
					density = min( 1, mesh["density"].data.value ) * origDensity
				else:
					density = min( 1, mesh["density"].data[f] ) * origDensity
			self.assertLessEqual( abs( pointsPerFace[f] - density * mesh['faceArea'].data[f] ), density * error + len(positions) * error * error )

	def testSimple( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 100 )
		self.pointTest( m, p, 100 )


	def testRaisesExceptionIfInvalidUVs( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()

		del m['uv']

		with self.assertRaisesRegex( RuntimeError, re.escape('MeshAlgo::distributePoints : MeshPrimitive has no uv primitive variable named "uv" of type FaceVarying or Vertex.') ) :
			p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 100 )

	def testHighDensity( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 50000, offset = imath.V2f( 0.0001, 0.0001 ) )

		self.pointTest( m, p, 50000 )

	def testDensityMaskPrimVar( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()
		m = IECoreScene.MeshAlgo.triangulate( m )
		numFaces = m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		m['density'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ float(x)/numFaces for x in range( 0, numFaces ) ] ) )
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 100 )
		self.pointTest( m, p, 100, error=0.1 )
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1000 )
		self.pointTest( m, p, 1000, error=0.1 )

		m['density'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ float(x) * 4.0 / numFaces for x in range( 0, numFaces ) ] ) )
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1000 )
		self.pointTest( m, p, 1000, error=0.1 )

		m['density'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ -1 for x in range( 0, numFaces ) ] ) )
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1000 )
		self.assertEqual( p.numPoints, 0 )

		m['density'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.3 ) )
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1000 )
		self.pointTest( m, p, 1000, error=0.1 )

		m['density'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 2 ) )
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1000 )
		self.pointTest( m, p, 1000, error=0.1 )

		m['density'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( -0.001 ) )
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1000 )
		self.assertEqual( p.numPoints, 0 )

	def testOffsetParameter( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()
		density = 500
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = density, offset = imath.V2f( 0, 0 ) )
		pOffset = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = density, offset = imath.V2f( 0.5, 0.75 ) )
		self.pointTest( m, p, density )
		self.pointTest( m, pOffset, density )
		self.assertNotEqual( p.numPoints, pOffset.numPoints )

		pos = p["P"].data
		posOffset = pOffset["P"].data
		for i in range( 0, min(p.numPoints, pOffset.numPoints) ) :
			self.assertNotEqual( pos[i], posOffset[i] )

	def testRefPosition( self ):
		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 1 ) )
		m["altP"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ i * 2 + imath.V3f( 1, 7, 10 ) for i in m["P"].data ] ) )

		p1 = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 40 )
		p2 = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 10, refPosition = "altP" )

		# Using a reference position with a scale affects the density ( which is compensated by changing the
		# density argument, but otherwise does not affect anything )

		self.assertEqual( p1, p2 )

		# Holding the reference position constant while scaling up the position results in points spread
		# across a much larger area, but the point counts are kept constant
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ i * 100 for i in m["P"].data ] ) )

		p3 = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 10, refPosition = "altP" )
		self.assertLess( p3.bound().min()[0], -100 )
		self.assertGreater( p3.bound().max()[0], 100 )
		self.assertEqual( p3.numPoints, p2.numPoints )

	def testDistanceBetweenPoints( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()

		density = 300
		points = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = density )
		positions = points["P"].data

		tree = IECore.V3fTree( points["P"].data )
		for i in range( 0, positions.size() ) :
			neighbours = list(tree.nearestNNeighbours( positions[i], 6 ))
			self.assertTrue( i in neighbours )
			neighbours.remove( i )
			for n in neighbours :
				self.assertGreater( ( positions[i] - positions[n] ).length(), 1.0 / density )

	def testPointOrder( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()
		m2 = m.copy()
		m2['P'].data += imath.V3f( 0, 5, 0 )
		pos = m["P"].data
		pos2 = m2["P"].data
		for i in range( 0, pos.size() ) :
			self.assertNotEqual( pos[i], pos2[i] )

		density = 500
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = density )
		p2 = IECoreScene.MeshAlgo.distributePoints( mesh = m2, density = density )
		self.pointTest( m, p, density )
		self.pointTest( m2, p2, density )
		self.assertEqual( p.numPoints, p2.numPoints )

		pos = p["P"].data
		pos2 = p2["P"].data
		for i in range( 0, p.numPoints ) :
			self.assertTrue( pos2[i].equalWithRelError( pos[i] + imath.V3f( 0, 5, 0 ), 1e-6 ) )

	def testDensityRange( self ) :

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()
		self.assertRaises( RuntimeError, IECoreScene.MeshAlgo.distributePoints, m, -1.0 )

	def testVertexUVs( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 4 ) )

		# We know that createPlane creates FaceVarying uvs, but the data is actually per vertex, and just indexed
		# to be FaceVarying, so we can copy the data as Vertex uvs
		m2 = m.copy()
		m2["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, m["uv"].data )

		density = 500
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = density )
		p2 = IECoreScene.MeshAlgo.distributePoints( mesh = m2, density = density )

		self.assertEqual( p, p2 )

	def testZeroAreaPolygons( self ):

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 1 ) )
		m['uv'] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.V2fVectorData( [ imath.V2f( i ) for i in [ 0, 0, 1, 1 ] ] ) )
		p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1 )
		self.assertEqual( len( p["P"].data ), 0 )

	@unittest.skipIf( ( IECore.TestUtil.inMacCI() or IECore.TestUtil.inWindowsCI() ), "Mac and Windows CI are too slow for reliable timing" )
	def testCancel( self ) :
		# Initializing the points distribution is slow and not cancellable
		# Pre-initialize it so it doesn't mess with our timing
		IECore.PointDistribution.defaultInstance()

		canceller = IECore.Canceller()
		cancelled = [False]

		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pCubeShape1.cob" ) ).read()

		def backgroundRun():
			try:
				p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 10000000, offset = imath.V2f( 0.0001, 0.0001 ), canceller = canceller )
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

	@unittest.skipIf( True, "Not running slow perf tests by default" )
	def testScaledUVsPerf( self ):

		def uvScaledPlane( scale ):
			# The diagonal axes we are using - we'll be applying a scale along b
			a = imath.V2f( 1, 1 ).normalized()
			b = imath.V2f( 1, -1 ).normalized()

			m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ), imath.V2i( 1 ) )
			m['uv'] = IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.FaceVarying,
				IECore.V2fVectorData(
					[ i.dot( a ) * a + i.dot( b ) * b * scale for i in m['uv'].data ]
				),
				m['uv'].indices
			)
			return m

		# As we hold the object space size of the plane constant, and scaled down the UV size on a diagonal
		# axis, the algorithm becomes steadily less efficient. Even though we are just trying to generate
		# 4000 points on a default plane ( and mostly succeeding, though in practice the resulting distribution
		# becomes unusably bad due to lattice artifacts and/or precision issues before it gets slow ), it
		# takes longer and longer to generate those 4000 points. We are keeping the same goal for number of
		# of points, but trying to acheive it by throwing darts in UV space at a smaller and smaller target,
		# so we have to throw more and more darts.

		print( "\n" )
		for scale in [ 1, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000004 ]:
			m = uvScaledPlane( scale )
			startTime = time.time()
			p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1000 )
			elapsed = time.time() - startTime
			print( "UV scale %f: Generated %i of 4000 points in %f seconds" % ( scale, len( p["P"].data ), elapsed ) )

		# Eventually, it is better to throw an exception than to consume unbounded amounts of time and memory
		m = uvScaledPlane( 0.000003 )
		with self.assertRaisesRegex( RuntimeError, "MeshAlgo::distributePoints : Cannot generate more than 1000000000 candidate points per polygon. Trying to generate 1342177152." ) :

			p = IECoreScene.MeshAlgo.distributePoints( mesh = m, density = 1000 )

	def setUp( self ) :

		os.environ["CORTEX_POINTDISTRIBUTION_TILESET"] = os.path.join( "test", "IECore", "data", "pointDistributions", "pointDistributionTileSet2048.dat" )

if sys.platform == "darwin" :

	# These fail because MacOS uses libc++, and libc++ has a
	# different `std::random_shuffle()` than libstdc++.

	MeshAlgoDistributePointsTest.testDensityMaskPrimVar = unittest.expectedFailure( MeshAlgoDistributePointsTest.testDensityMaskPrimVar )
	MeshAlgoDistributePointsTest.testDistanceBetweenPoints = unittest.expectedFailure( MeshAlgoDistributePointsTest.testDistanceBetweenPoints )

if __name__ == "__main__":
	unittest.main()

