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

class TestSpherePrimitiveEvaluator( unittest.TestCase ) :

	def testSimple( self ) :
		""" Test SpherePrimitiveEvaluator """

		random.seed( 1 )
		
		rand = Rand48( 1 )	
		
		numTests = 50
		for i in range(0, numTests) :

			center = V3f( 0, 0, 0 )
			radius = random.uniform( 0.1, 5 )		
			sphere = SpherePrimitive( radius )
			
			# Add some UV data in "bowtie" order - when we read it back it should then match the geometric UVs
			# if we're doing everything correctly.
			testData = V3fVectorData()
			testData.append( V3f( 0, 0, 0 ) )
			testData.append( V3f( 1, 0, 0 ) )
			testData.append( V3f( 0, 1, 0 ) )
			testData.append( V3f( 1, 1, 0 ) )	
			testPrimVar = PrimitiveVariable( PrimitiveVariable.Interpolation.Varying, testData )							
			sphere["testPrimVar"] = testPrimVar
		
			se = PrimitiveEvaluator.create( sphere )
			
			result = se.createResult()
			
			testPoint = V3f( random.uniform( -10, 10 ), random.uniform( -10, 10 ), random.uniform( -10, 10 ) )
			
			found = se.closestPoint( testPoint, result )
			
			self.assert_( found )
			
			# The closest point should lie on the sphere
			self.assert_( math.fabs( ( result.point() - center ).length() - radius ) < 0.001 )
			
			uv = result.uv()
			self.assert_( uv[0] >= 0 )
			self.assert_( uv[0] <= 1 )
			self.assert_( uv[1] >= 0 )
			self.assert_( uv[1] <= 1 )
			
			# Make sure our "fake" UVs match the geometric UVs
			testPrimVarValue = result.vectorPrimVar( testPrimVar )			
			self.assertAlmostEqual( testPrimVarValue[0], uv[0], 3 )
			self.assertAlmostEqual( testPrimVarValue[1], uv[1], 3 )	
			
			closestPoint = result.point()
			
			found = se.pointAtUV( uv, result )
			self.assert_( found )
			
			self.assertAlmostEqual( result.uv()[0], uv[0], 3 )
			self.assertAlmostEqual( result.uv()[1], uv[1], 3 )
									
			# Pick a random point inside the sphere...
			origin = center + Rand48.solidSpheref(rand) * radius * 0.9			
			self.assert_( ( origin - center ).length() < radius )
			
			# And a random (unnormalized!) direction
			direction = Rand48.hollowSpheref(rand)	* random.uniform( 0.5, 10 )
			
			found = se.intersectionPoint( origin, direction, result )
			if found:
				# The intersection point should lie on the sphere
				self.assert_( math.fabs( ( result.point() - center ).length() - radius ) < 0.001 )
							
			results = se.intersectionPoints( origin, direction )	
			
			self.assert_( len(results) >= 0 )
			self.assert_( len(results) <= 1 )			
			
			for result in results:
				# The intersection point should lie on the sphere
				self.assert_( math.fabs( ( result.point() - center ).length() - radius ) < 0.001 )
				
			# Pick a random point outside the sphere...
			origin = center + Rand48.hollowSpheref(rand) * radius * 2
			self.assert_( ( origin - center ).length() > radius )
			
			found = se.intersectionPoint( origin, direction, result )
			if found:
				# The intersection point should lie on the sphere
				self.assert_( math.fabs( ( result.point() - center ).length() - radius ) < 0.001 )
				
			results = se.intersectionPoints( origin, direction )	
			
			# We can get a maximum of 2 intersection points from outside the sphere
			self.assert_( len(results) >= 0 )
			self.assert_( len(results) <= 2 )			
			
			# If we get 1 result, the ray glances the sphere. Assert this.
			if len( results ) == 1:				
				self.assert_( math.fabs( direction.dot( result.normal() )  < 0.1 ) )
			
			for result in results:
				# The intersection point should lie on the sphere
				self.assert_( math.fabs( ( result.point() - center ).length() - radius ) < 0.001 )	
	
			
		
			
if __name__ == "__main__":
	unittest.main()
	
