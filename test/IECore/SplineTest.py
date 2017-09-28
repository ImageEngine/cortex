##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

				# select a segment
				seg = int(random.uniform( 0, int(len(xv) / 4) ))
				seg -= seg % s.basis.step
				# evaluate an x,y point on the curve directly
				# ourselves
				t = i / 1000.0
				c = s.basis.coefficients( t )
				x = xv[seg+0] * c[0] + xv[seg+1] * c[1] + xv[seg+2] * c[2] + xv[seg+3] * c[3]
				y = yv[seg+0] * c[0] + yv[seg+1] * c[1] + yv[seg+2] * c[2] + yv[seg+3] * c[3]

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

		def computeInterval( s ):
			# compute interval from spline until interval() is not fixed....
			cc = s.basis.coefficients(0)
			xs = s.keys()
			xx = list( xs )[ 0:4 ]
			start = cc[0]*xx[0]+cc[1]*xx[1]+cc[2]*xx[2]+cc[3]*xx[3]
			cc = s.basis.coefficients(1)
			xx = list( xs )[ len(xs)-4:len(xs) ]
			end = cc[0]*xx[0]+cc[1]*xx[1]+cc[2]*xx[2]+cc[3]*xx[3]
			return ( start, end )

		def compareIntervals( i1, i2 ):
			return ( abs(i1[0]-i2[0]) + abs(i1[1]-i2[1]) < 0.0001 )

		s = IECore.Splineff( IECore.CubicBasisf.bSpline(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ) ) )
		self.assert_( compareIntervals( computeInterval( s ), s.interval() ) )
		s = IECore.Splineff( IECore.CubicBasisf.bSpline(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ), ( 21, 2 ), ( 22, 4 ) ) )
		self.assert_( compareIntervals( computeInterval( s ), s.interval() ) )
		(t,seg) = s.solve( s.interval()[0] )
		(t2,seg2) = s.solve( s.interval()[0] + 0.1 )
		self.assert_( t2 > t )
		(t,seg) = s.solve( s.interval()[1] )
		(t2,seg2) = s.solve( s.interval()[1] - 0.1 )
		self.assert_( t2 < t )

	def testIntegral( self ) :

		def computeIntegrals( s, interval = None ):

			if interval:
				integral = s.integral( interval[0], interval[1] )
			else:
				interval = s.interval()
				integral = s.integral()

			base = interval[0]
			scale = interval[1]-interval[0]

			N = 200
			dx = (float(scale) / N )
			summedArea = 0
			for i in xrange(0,N):
				x = (i+0.5)*dx + base
				v = s( x )
				summedArea += v*dx

			return ( integral, summedArea )

		# testing bSpline basis integral with 4 control points
		s = IECore.Splineff( IECore.CubicBasisf.bSpline(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ) ) )
		( integral, summedArea ) = computeIntegrals(s)
		self.assertAlmostEqual( integral, summedArea, 2 )

		# testing bSpline basis integral with 5 control points
		s = IECore.Splineff( IECore.CubicBasisf.bSpline(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ), ( 22, 4 ) ) )
		( integral, summedArea ) = computeIntegrals(s)
		self.assertAlmostEqual( integral, summedArea, 2 )
		( integral, summedArea ) = computeIntegrals(s, [12,20])
		self.assertAlmostEqual( integral, summedArea, 2 )

		# testing catmullRom basis integral
		s = IECore.Splineff( IECore.CubicBasisf.catmullRom(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ), ( 22, 4 ) ) )
		( integral, summedArea ) = computeIntegrals(s)
		self.assertAlmostEqual( integral, summedArea, 2 )
		( integral, summedArea ) = computeIntegrals(s, [12,20])
		self.assertAlmostEqual( integral, summedArea, 2 )

		# testing bezier basis integral
		s = IECore.Splineff( IECore.CubicBasisf.bezier(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ), ( 22, 4 ), ( 23, 0 ), ( 24, 1 ) ) )
		( integral, summedArea ) = computeIntegrals(s)
		self.assertAlmostEqual( integral, summedArea, 3 )
		( integral, summedArea ) = computeIntegrals(s, [12,20])
		self.assertAlmostEqual( integral, summedArea, 3 )

		# testing linear basis integral with 4 control points
		s = IECore.Splineff( IECore.CubicBasisf.linear(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ) ) )
		( integral, summedArea ) = computeIntegrals(s)
		self.assertAlmostEqual( integral, summedArea, 2 )
		# Test against a quick analytic integration by hand
		self.assertAlmostEqual( integral, 0.5 * ( ( 10 - 0 ) * ( 1 + 2 ) + ( 20 - 10 ) * ( 2 + 0 ) + ( 21 - 20 ) * ( 0 + 2 ) ), 7 )

if __name__ == "__main__":
    unittest.main()

