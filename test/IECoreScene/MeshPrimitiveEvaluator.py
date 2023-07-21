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

import math
import unittest
import random
import imath
import os
import IECore
import IECoreScene

class TestMeshPrimitiveEvaluator( unittest.TestCase ) :

	def testConstructor( self ) :

		m = IECoreScene.MeshPrimitive()
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData() )
		e = IECoreScene.MeshPrimitiveEvaluator( m )

	def testResultTypeValidation( self ) :

		m = IECoreScene.MeshPrimitive()
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData() )
		e = IECoreScene.PrimitiveEvaluator.create( m )

		wrongResultType = IECoreScene.PrimitiveEvaluator.create( IECoreScene.SpherePrimitive() ).createResult()

		self.assertRaises( Exception, e.closestPoint, imath.V3f( 0 ), wrongResultType )

	def testEmptyMesh( self ) :
		""" Testing MeshPrimitiveEvaluator with empty mesh"""

		m = IECoreScene.MeshPrimitive()
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData() )

		mpe = IECoreScene.PrimitiveEvaluator.create( m )

		self.assertTrue( mpe.isInstanceOf( "MeshPrimitiveEvaluator" ) )

		r = mpe.createResult()

		foundClosest = mpe.closestPoint( imath.V3f( 0, 10, 0 ), r )

		self.assertFalse( foundClosest )

	def testTangents( self ) :

		reader = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pSphereShape1.cob" ) )
		m = reader.read()

		self.assertTrue( m.isInstanceOf( "MeshPrimitive" ) )

		numTriangles = len( m.verticesPerFace )

		mpe = IECoreScene.PrimitiveEvaluator.create( m )

		r = mpe.createResult()

		# Currently unimplemented
		self.assertRaises( RuntimeError, r.uTangent )
		self.assertRaises( RuntimeError, r.vTangent )


	def testSimpleMesh( self ) :
		""" Testing MeshPrimitiveEvaluator with mesh containing single triangle"""
		verticesPerFace = IECore.IntVectorData()
		verticesPerFace.append( 3 )

		vertexIds = IECore.IntVectorData()
		vertexIds.append( 0 )
		vertexIds.append( 1 )
		vertexIds.append( 2 )

		translation = imath.V3f( 3, 3, 3 )

		P = IECore.V3fVectorData()
		P.append( imath.V3f( -1, 0,  0 ) + translation )
		P.append( imath.V3f(  0, 0, -1 ) + translation )
		P.append( imath.V3f( -1, 0, -1 ) + translation )

		uOffset = 7
		uv = IECore.V2fVectorData()
		vOffset = 12

		for p in P :
			uv.append( imath.V2f( p.x + uOffset, p.z + vOffset ) )

		self.assertEqual( len( P ), len( uv ) )

		m = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds )
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, P )

		# We use Varying interpolation here because the tests which use pSphereShape1.cob exercise FaceVarying
		m["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, uv )

		mpe = IECoreScene.PrimitiveEvaluator.create( m )
		r = mpe.createResult()

		# For each point verify that the closest point to it is itself
		for p in P:
			foundClosest = mpe.closestPoint( p , r )
			self.assertTrue( foundClosest )
			self.assertAlmostEqual( ( p - r.point() ).length(), 0 )


		foundClosest = mpe.closestPoint( imath.V3f( 0, 10, 0 ) + translation , r )

		self.assertTrue( foundClosest )

		self.assertAlmostEqual( ( imath.V3f( -0.5, 0, -0.5 ) + translation - r.point()).length(), 0 )
		self.assertAlmostEqual( math.fabs( r.normal().dot( imath.V3f(0, 1, 0 ) ) ) , 1, places = 3  )

		# For each point verify that the UV data is exactly what we specified at those vertices
		for p in P:
			foundClosest = mpe.closestPoint( p , r )
			self.assertTrue( foundClosest )
			testUV = imath.V2f( p.x + uOffset, p.z + vOffset )
			self.assertAlmostEqual( ( testUV - r.uv() ).length(), 0 )

			# Now when we looking up that UV in reverse we should get back the point again!
			found = mpe.pointAtUV( testUV, r )
			self.assertTrue( found )

			self.assertAlmostEqual( ( p - r.point()).length(), 0 )

		# test the uvBound method
		uvb = imath.Box2f()
		for i in range( 0, len( uv ) ) :
			uvb.extendBy( uv[i] )

		self.assertEqual( mpe.uvBound(), uvb )

	def testSphereMesh( self ) :
		""" Testing MeshPrimitiveEvaluator with sphere mesh"""

		# File represents a sphere of radius 1.0 at the origin
		reader = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "pSphereShape1.cob" ) )
		m = reader.read()

		self.assertTrue( m.isInstanceOf( "MeshPrimitive" ) )

		numTriangles = len( m.verticesPerFace )

		mpe = IECoreScene.PrimitiveEvaluator.create( m )

		maxAbsError = 0.2

		# Test volume against (theoretical) 4/3 * pi * r^3
		self.assertTrue( math.fabs ( 4.0 / 3.0 * math.pi * ( 1.0 * 1.0 * 1.0 ) - mpe.volume()  ) < maxAbsError )

		# Center of gravity should be at origin
		self.assertTrue( mpe.centerOfGravity().length() < maxAbsError )

		# Test surface area against (theoretical) 4 * pi * r^20
		self.assertTrue( math.fabs ( 4.0 * math.pi * ( 1.0 * 1.0  ) - mpe.surfaceArea()  ) < maxAbsError )

		r = mpe.createResult()

		random.seed( 1 )

		# Perform 100 closest point queries
		for i in range(0, 100):

			# Pick a random point outside the sphere
			testPt = None
			while not testPt or testPt.length() < 1.5:
				testPt = 3 * imath.V3f( random.uniform(-1, 1), random.uniform(-1, 1), random.uniform(-1, 1) )

			foundClosest = mpe.closestPoint( testPt, r )

			self.assertTrue( foundClosest )

			# Closest point should lie on unit sphere
			self.assertTrue( math.fabs( r.point().length() - 1.0 ) < maxAbsError  )

			# Distance to closest point should be approximately distance to origin minus sphere radius - allow some error
			# because our source mesh does not represent a perfect sphere.
			absError = math.fabs( ( testPt  - r.point() ).length() - ( testPt.length() - 1.0 ) )

			self.assertTrue( absError < maxAbsError )

			self.assertTrue( r.triangleIndex() >= 0 )
			self.assertTrue( r.triangleIndex() < numTriangles )

			# Origin->Closest point should be roughly same direction as Origin->Test point, for a sphere
			self.assertTrue( r.point().normalized().dot( testPt.normalized() ) > 0.5 )

			geometricNormal = r.normal().normalized()
			shadingNormal = r.vectorPrimVar( m["N"] ).normalized()

			# Geometric and shading normals should be facing the same way, roughly
			self.assertTrue( geometricNormal.dot( shadingNormal ) > 0.5 )

			# Shading normal should be pointing away from the origin at the closest point
			self.assertTrue( shadingNormal.dot( r.point().normalized() ) > 0.5 )

			# Vector from closest point to test point should be roughly the same direction as the normal
			self.assertTrue( shadingNormal.dot( ( testPt - r.point() ).normalized() ) > 0.5 )

		rand = imath.Rand48()

		# Perform 100 ray intersection queries from inside the sphere, in random directions
		for i in range(0, 100):

			origin = rand.nextSolidSphere( imath.V3f() ) * 0.5
			direction = rand.nextHollowSphere( imath.V3f() )
			hit = mpe.intersectionPoint( origin, direction, r )
			self.assertTrue( hit )
			self.assertTrue( math.fabs( r.point().length() -1 ) < 0.1 )

			hits = mpe.intersectionPoints( origin, direction )
			self.assertEqual( len(hits), 1 )

			for hit in hits:
				self.assertTrue( math.fabs( hit.point().length() -1 ) < 0.1 )

		# Perform 100 nearest ray intersection queries from outside the sphere, going outwards
		for i in range(0, 100):

			direction = rand.nextHollowSphere( imath.V3f() )
			origin = direction * 2

			hit = mpe.intersectionPoint( origin, direction, r )
			self.assertFalse( hit )

			hits = mpe.intersectionPoints( origin, direction )
			self.assertFalse( hits )

		# Perform 100 nearest ray intersection queries from outside the sphere, going inwards
		for i in range(0, 100):

			direction = -rand.nextHollowSphere( imath.V3f() )
			origin = -direction * 2

			hit = mpe.intersectionPoint( origin, direction, r )
			self.assertTrue( hit )
			self.assertTrue( math.fabs( r.point().length() -1 ) < 0.1 )

			# Make sure we get the nearest point, not the furthest
			self.assertTrue( ( origin - r.point() ).length() < 1.1 )

			hits = mpe.intersectionPoints( origin, direction )

			# There should be 0, 1, or 2 intersections
			self.assertTrue( len(hits) >= 0 )
			self.assertTrue( len(hits) <= 2 )

			for hit in hits:
				self.assertTrue( math.fabs( hit.point().length() - 1 ) < 0.1 )

	def testCylinderMesh( self ) :
		"""Testing special case of intersection query."""
		m = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "cylinder3Mesh.cob" ) ) ()
		e = IECoreScene.MeshPrimitiveEvaluator( m )
		res = e.createResult()
		self.assertFalse( e.intersectionPoint( imath.V3f(0.5,0,0.5), imath.V3f(1,0,0), res ) )
		self.assertTrue( e.intersectionPoint( imath.V3f(0.5,0,0.5), imath.V3f(-1,0,0), res ) )

		self.assertFalse( e.intersectionPoints( imath.V3f(0.5,0,0.5), imath.V3f(1,0,0) ) )
		self.assertTrue( e.intersectionPoints( imath.V3f(0.5,0,0.5), imath.V3f(-1,0,0) ) )


	def testRandomTriangles( self ) :
		""" Testing MeshPrimitiveEvaluator with random triangles"""

		random.seed( 100 )
		rand = imath.Rand48( 100 )

		numConfigurations = 100
		numTests = 50
		numTriangles = 250

		for config in range( 0, numConfigurations ) :

			P = IECore.V3fVectorData()
			verticesPerFace = IECore.IntVectorData()
			vertexIds = IECore.IntVectorData()

			vertexId = 0

			for tri in range( 0, numTriangles ) :

				verticesPerFace.append( 3 )

				P.append( imath.V3f( random.uniform(-10, 10), random.uniform(-10, 10), random.uniform(-10, 10) ) )
				P.append( imath.V3f( random.uniform(-10, 10), random.uniform(-10, 10), random.uniform(-10, 10) ) )
				P.append( imath.V3f( random.uniform(-10, 10), random.uniform(-10, 10), random.uniform(-10, 10) ) )

				vertexIds.append( vertexId + 0 )
				vertexIds.append( vertexId + 1 )
				vertexIds.append( vertexId + 2 )

				vertexId = vertexId + 3

			m = IECoreScene.MeshPrimitive( verticesPerFace, vertexIds )
			m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, P )
			mpe = IECoreScene.PrimitiveEvaluator.create( m )
			r = mpe.createResult()
			r2 = mpe.createResult()

			# Make sure that the closest hit point found with intersectionPoint() is actually the closest, by
			# comparing long-hand with the list of all intersections.
			for test in range( 0, numTests ) :

				origin = imath.V3f( 0, 0, 0 )
				direction = rand.nextHollowSphere( imath.V3f() )

				hit = mpe.intersectionPoint( origin, direction, r )

				if hit:

					hits = mpe.intersectionPoints( origin, direction )
					self.assertTrue( hits )

					closestHitDist = 100000

					closestHit = None

					for hit in hits :

						hitDist = ( origin - hit.point() ).length()
						if hitDist < closestHitDist:

							closestHitDist = hitDist
							closestHit = hit


					self.assertTrue( (r.point() - closestHit.point() ).length() < 1.e-4 )

					barycentricQuerySucceeded = mpe.barycentricPosition( r.triangleIndex(), r.barycentricCoordinates(), r2 )
					self.assertTrue( barycentricQuerySucceeded )
					self.assertTrue( r.point().equalWithAbsError( r2.point(), 0.00001 ) )
					self.assertTrue( r.normal().equalWithAbsError( r2.normal(), 0.00001 ) )
					self.assertTrue( r.barycentricCoordinates().equalWithAbsError( r2.barycentricCoordinates(), 0.00001 ) )
					self.assertEqual( r.triangleIndex(), r2.triangleIndex() )


				else:

					hits = mpe.intersectionPoints( origin, direction )
					self.assertFalse( hits )

	def testEvaluateIndexedPrimitiveVariables( self ) :

		m = IECoreScene.MeshPrimitive(
			# Two triangles
			IECore.IntVectorData( [ 3, 3 ] ),
			# Winding counter clockwise
			IECore.IntVectorData( [ 0, 1, 2, 0, 2, 3 ] ),
			# Linear interpolation
			"linear",
			# Points in shape of a square
			IECore.V3fVectorData( [
				imath.V3f( 0, 0, 0 ),
				imath.V3f( 1, 0, 0 ),
				imath.V3f( 1, 1, 0 ),
				imath.V3f( 0, 1, 0 )
			] )
		)

		m["uniform"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Uniform,
			IECore.IntVectorData( [ 123, 321 ] ),
			IECore.IntVectorData( [ 1, 0 ] ),
		)

		m["vertex"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( [ 314, 271 ] ),
			IECore.IntVectorData( [ 0, 1, 1, 0] ),
		)

		m["faceVarying"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.FaceVarying,
			IECore.FloatVectorData( [ 10, 20, 30 ] ),
			IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2 ] ),
		)

		evaluator = IECoreScene.PrimitiveEvaluator.create( m )
		result = evaluator.createResult()

		# Test each corner of each triangle

		for triangleIndex in ( 0, 1 ) :

			for corner in ( 0, 1, 2 ) :

				bary = imath.V3f( 0 )
				bary[corner] = 1

				self.assertTrue( evaluator.barycentricPosition( triangleIndex, bary, result ) )

				self.assertEqual(
					result.intPrimVar( m["uniform"] ),
					m["uniform"].data[m["uniform"].indices[triangleIndex]]
				)

				vertexIndex = m.vertexIds[triangleIndex*3+corner]
				self.assertEqual(
					result.floatPrimVar( m["vertex"] ),
					m["vertex"].data[m["vertex"].indices[vertexIndex]]
				)

				self.assertEqual(
					result.floatPrimVar( m["faceVarying"] ),
					m["faceVarying"].data[m["faceVarying"].indices[triangleIndex*3+corner]]
				)

	def testSignedDistanceWeirdness( self ) :
		# Previously, numerical precision problems meant that this point on the cube was reported to be
		# 0.0005 away from the cube
		m = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -0.5 ), imath.V3f( 0.5 ) ) )
		mt = IECoreScene.MeshAlgo.triangulate( m )
		meshEvaluator = IECoreScene.MeshPrimitiveEvaluator( mt )
		result = meshEvaluator.createResult()
		self.assertAlmostEqual( meshEvaluator.signedDistance( imath.V3f( 0.23601509630680084, -0.5, 0.23528452217578888), result ), 0 )

if __name__ == "__main__":
	unittest.main()

