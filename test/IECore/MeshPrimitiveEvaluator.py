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
from IECore import *

class TestMeshPrimitiveEvaluator( unittest.TestCase ) :

	def testEmptyMesh( self ) :
		""" Testing MeshPrimitiveEvaluator with empty mesh"""
		
		m = MeshPrimitive()
		m["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData() )
		
		mpe = PrimitiveEvaluator.create( m )
		
		self.assert_( mpe.isInstanceOf( "MeshPrimitiveEvaluator" ) )
		
		r = mpe.createResult()
		
		foundClosest = mpe.closestPoint( V3f( 0, 10, 0 ), r )
		
		self.failIf( foundClosest )
		
	def testSimpleMesh( self ) :
		""" Testing MeshPrimitiveEvaluator with mesh containing single triangle"""	
		verticesPerFace = IntVectorData()
		verticesPerFace.append( 3 )
		
		vertexIds = IntVectorData()
		vertexIds.append( 0 )
		vertexIds.append( 1 )		
		vertexIds.append( 2 )		
		
		translation = V3f( 3, 3, 3 )
		
		P = V3fVectorData()
		P.append( V3f( -1, 0,  0 ) + translation )
		P.append( V3f(  0, 0, -1 ) + translation )
		P.append( V3f( -1, 0, -1 ) + translation )				
		
		m = MeshPrimitive( verticesPerFace, vertexIds )
		m["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, P )
		mpe = PrimitiveEvaluator.create( m )
		r = mpe.createResult()
		foundClosest = mpe.closestPoint( V3f( 0, 10, 0 ) + translation , r )
		
		self.assert_( foundClosest )
		
		self.assertAlmostEqual( ( V3f( -0.5, 0, -0.5 ) + translation - r.point()).length(), 0 )
		self.assertAlmostEqual( math.fabs( r.normal().dot( V3f(0, 1, 0 ) ) ) , 1, places = 3  )
		
	def testSphereMesh( self ) :
		""" Testing MeshPrimitiveEvaluator with sphere mesh"""	
		
		# File represents a sphere of radius 1.0 at the origin
		reader = Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" )
		m = reader.read()
		
		self.assert_( m.isInstanceOf( "MeshPrimitive" ) )
		
		numTriangles = len( m.verticesPerFace )
		
		mpe = PrimitiveEvaluator.create( m )
		r = mpe.createResult()
		
		random.seed( 1 )
		
		maxAbsError = 0.2
		
		# Perform 100 closest point queries
		for i in range(0, 100):
		
			# Pick a random point outside the sphere
			testPt = None
			while not testPt or testPt.length() < 1.5:
				testPt = 3 * V3f( random.uniform(-1, 1), random.uniform(-1, 1), random.uniform(-1, 1) )

			foundClosest = mpe.closestPoint( testPt, r )
			
			self.assert_( foundClosest )
			
			# Closest point should lie on unit sphere
			self.assert_( math.fabs( r.point().length() - 1.0 ) < maxAbsError  )
			
			# Distance to closest point should be approximately distance to origin minus sphere radius - allow some error
			# because our source mesh does not represent a perfect sphere.			
			absError = math.fabs( ( testPt  - r.point() ).length() - ( testPt.length() - 1.0 ) )

			self.assert_( absError < maxAbsError )
			
			self.assert_( r.triangleIndex() >= 0 )
			self.assert_( r.triangleIndex() < numTriangles )

			# Origin->Closest point should be roughly same direction as Origin->Test point, for a sphere			
			self.assert_( r.point().normalized().dot( testPt.normalized() ) > 0.5 )
			
			geometricNormal = r.normal().normalized()
			shadingNormal = r.vectorPrimVar( m["N"] ).normalized()			
			
			# Geometric and shading normals should be facing the same way, roughly
			self.assert_( geometricNormal.dot( shadingNormal ) > 0.5 )
			
			# Shading normal should be pointing away from the origin at the closest point
			self.assert_( shadingNormal.dot( r.point().normalized() ) > 0.5 )
			
			# Vector from closest point to test point should be roughly the same direction as the normal
			self.assert_( shadingNormal.dot( ( testPt - r.point() ).normalized() ) > 0.5 )
			
		rand = Rand48()	
		
		# Perform 100 ray intersection queries from inside the sphere, in random directions	
		for i in range(0, 100):
		
			origin = Rand48.solidSpheref(rand) * 0.5
			direction = Rand48.hollowSpheref(rand)	
			hit = mpe.intersectionPoint( origin, direction, r )			
			self.assert_( hit )
			self.assert_( math.fabs( r.point().length() -1 ) < 0.1 )
			
			hits = mpe.intersectionPoints( origin, direction )
			self.assertEqual( len(hits), 1 )
			
			for hit in hits:
				self.assert_( math.fabs( hit.point().length() -1 ) < 0.1 )
			
		# Perform 100 nearest ray intersection queries from outside the sphere, going outwards
		for i in range(0, 100):
		
			direction = Rand48.hollowSpheref(rand)
			origin = direction * 2
				
			hit = mpe.intersectionPoint( origin, direction, r )			
			self.failIf( hit )
			
			hits = mpe.intersectionPoints( origin, direction )
			self.failIf( hits )
			
		# Perform 100 nearest ray intersection queries from outside the sphere, going inwards	
		for i in range(0, 100):
		
			direction = -Rand48.hollowSpheref(rand)
			origin = -direction * 2
				
			hit = mpe.intersectionPoint( origin, direction, r )			
			self.assert_( hit )
			self.assert_( math.fabs( r.point().length() -1 ) < 0.1 )
			
			hits = mpe.intersectionPoints( origin, direction )
			
			# There should be 0, 1, or 2 intersections
			self.assert_( len(hits) >= 0 )
			self.assert_( len(hits) <= 2 )
			
			for hit in hits:
				self.assert_( math.fabs( hit.point().length() -1 ) < 0.1 )			

		
			
if __name__ == "__main__":
	unittest.main()
	
