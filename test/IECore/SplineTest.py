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
import IECore

class SplineTest( unittest.TestCase ) :

	def testConstructor( self ) :
		
		s = IECore.Splineff()
		self.assertEqual( s.basis, IECore.CubicBasisf.catmullRom() )
		self.assertEqual( s.keys(), () )
		self.assertEqual( s.values(), () )
		self.assertEqual( s.items(), () )
		self.assertEqual( len( s ), 0 )
		
		s = IECore.Splineff( IECore.CubicBasisf.bezier() )
		self.assertEqual( s.basis, IECore.CubicBasisf.bezier() )
		self.assertEqual( s.keys(), () )
		self.assertEqual( s.values(), () )
		self.assertEqual( s.items(), () )
		self.assertEqual( len( s ), 0 )
		
		s = IECore.Splineff( IECore.CubicBasisf.bSpline(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 20, 0 ), ( 21, 2 ) ) )
		self.assertEqual( s.basis, IECore.CubicBasisf.bSpline() )
		self.assertEqual( s.keys(), ( 0, 10, 20, 20, 21 ) )
		self.assertEqual( s.values(), ( 1, 2, 0, 0, 2 ) )
		self.assertEqual( s.items(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 20, 0 ), ( 21, 2 ) ) )
		self.assertEqual( len( s ), 5 )
		
	def testPointEditing( self ) :
	
		s = IECore.Splineff()
		self.assertEqual( len( s ), 0 )
		
		s[0] = 10
		s[1] = 20
		s[2] = 40
		s[10] = 50
		
		self.assertEqual( len( s ), 4 )
		
		self.assertEqual( s.keys(), ( 0, 1, 2, 10 ) )
		self.assertEqual( s.values(), ( 10, 20, 40, 50 ) )
		self.assertEqual( s.points(), s.items() )
		self.assertEqual( s.points(), ( ( 0, 10 ), ( 1, 20 ), ( 2, 40 ), ( 10, 50 ) ) )
		
		self.assert_( 0 in s )
		self.assert_( 1 in s )
		self.assert_( 2 in s )
		self.assert_( 10 in s )
		self.assert_( not 20 in s )
		
		self.assertEqual( s[0], 10 )
		self.assertEqual( s[1], 20 )
		self.assertEqual( s[2], 40 )
		self.assertEqual( s[10], 50 )
		
		self.assertRaises( IndexError, s.__getitem__, 100 )
		
		del s[0]
		
		self.assertEqual( len( s ), 3 )

		self.assert_( not 0 in s )
		self.assert_( 1 in s )
		self.assert_( 2 in s )
		self.assert_( 10 in s )
		self.assertEqual( s.keys(), ( 1, 2, 10 ) )
		self.assertEqual( s.values(), ( 20, 40, 50 ) )
		self.assertEqual( s.points(), s.items() )
		self.assertEqual( s.points(), ( ( 1, 20 ), ( 2, 40 ), ( 10, 50 ) ) )
		
	def testPointMultiplicity( self ) :
	
		## We use a multimap to store the points so that we can have knot multiplicities.
		# This makes the dictionary style syntax slightly less intuitive.
		
		s = IECore.Splineff()
		self.assertEqual( len( s ), 0 )
		
		s[0] = 1
		self.assertEqual( len( s ), 1 )
		s[0] = 1
		self.assertEqual( len( s ), 2 )
		
		self.assertEqual( s[0], 1 )
		self.assertEqual( s.points(), ( ( 0, 1 ), ( 0, 1 ) ) )
		self.assertEqual( s.keys(), ( 0, 0 ) )
		self.assertEqual( s.values(), ( 1, 1 ) )
		
		del s[0]
		self.assertEqual( len( s ), 0 )
		
	def testSolveAndCall( self ) :
	
		random.seed( 0 )
		for i in range( 0, 100 ) :
				
			s = IECore.Splineff()
			x = 0
		
			for i in range( 0, 40 ) :
			
				s[x] = random.uniform( 0, 10 )
				x += 1 + random.uniform( 0, 1 )
			
			xv = s.keys()
			yv = s.values()
		
			for i in range( 0, 1000 ) :
			
				# evaluate an x,y point on the curve directly
				# ourselves
				t = i / 1000.0
				c = s.basis.coefficients( t )
				x = xv[0] * c[0] + xv[1] * c[1] + xv[2] * c[2] + xv[3] * c[3]
				y = yv[0] * c[0] + yv[1] * c[1] + yv[2] * c[2] + yv[3] * c[3]
				
				# then check that solving for x gives y
				yy = s( x )
				
				self.assertAlmostEqual( yy, y, 3 )

	def testRepr( self ) :
	
		s = IECore.Splineff()
		s[0] = 10
		s[1] = 20
		s[2] = 40
		s[10] = 50

		ss = eval( repr( s ) )
		self.assertEqual( s, ss )
		
		s = IECore.SplinefColor3f()
		s[0] = IECore.Color3f( 1, 0, 0 )
		s[1] = IECore.Color3f( 0, 1, 0 )
		s[2] = IECore.Color3f( 1, 1, 0 )
		s[10] = IECore.Color3f( 0, 0, 1 )

		ss = eval( repr( s ) )
		self.assertEqual( s, ss )
	
	def testInterval( self ) :
	
		s = IECore.Splineff()
		s[10] = 100
		s[20] = 200
		
		self.assertEqual( s.interval(), ( 10, 20 ) )
		
if __name__ == "__main__":
    unittest.main()   

