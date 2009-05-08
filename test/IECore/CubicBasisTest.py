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
from IECore import *

class CubicTest( unittest.TestCase ) :

	def testConstructor( self ) :

		m = M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 )
		b = CubicBasisf( m, 10 )
		self.assertEqual( b.matrix, m )
		self.assertEqual( b.step, 10 )

	def testAccessors( self ) :

		m = M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 )
		b = CubicBasisf( m, 10 )
		self.assertEqual( b.matrix, m )
		self.assertEqual( b.step, 10 )

		m2 = M44f( 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 )
		b.matrix = m2
		self.assertEqual( b.matrix, m2 )

		b.step = 1
		self.assertEqual( b.matrix, m2 )

	def testStaticMethods( self ) :

		b = CubicBasisf.bezier()
		b = CubicBasisf.catmullRom()
		b = CubicBasisf.bSpline()

	def testCoefficients( self ) :

		b = CubicBasisf.bezier()

		c = b.coefficients( 0 )
		self.assertEqual( len( c ), 4 )
		self.assertEqual( c, ( 1, 0, 0, 0 ) )

		c = b.coefficients( 1 )
		self.assertEqual( len( c ), 4 )
		self.assertEqual( c, ( 0, 0, 0, 1 ) )

	def testCall( self ) :

		b = CubicBasisf.bezier()

		p0 = V3f( 0 )
		p1 = V3f( 0, 1, 0 )
		p2 = V3f( 1, 1, 0 )
		p3 = V3f( 1, 0, 0 )

		p = b( 0, p0, p1, p2, p3 )
		self.assertEqual( p, p0 )

		p = b( 1, p0, p1, p2, p3 )
		self.assertEqual( p, p3 )

	def testCallWithScalars( self ) :

		b = CubicBasisf.bezier()

		p0 = 0
		p1 = 1
		p2 = 1
		p3 = 0

		p = b( 0, p0, p1, p2, p3 )
		self.assertEqual( p, p0 )

		p = b( 1, p0, p1, p2, p3 )
		self.assertEqual( p, p3 )

	## This test isn't very useful as it doesn't assert anything,
	# but it can be handy for making a visual verification.
	def testPoints( self ) :

		b = CubicBasisf.bezier()

		p0 = V3f( 0 )
		p1 = V3f( 0, 1, 0 )
		p2 = V3f( 1, 1, 0 )
		p3 = V3f( 1, 0, 0 )

		p = V3fVectorData()
		for i in range( 0, 1000 ) :

			t = i / 999.0
			p.append( b( t, p0, p1, p2, p3 ) * 100 )

		pp = PointsPrimitive( p )

	def testEquality( self ) :

		self.assertEqual( CubicBasisf.bezier(), CubicBasisf.bezier() )
		self.assertNotEqual( CubicBasisf.linear(), CubicBasisf.bezier() )

	def testRepr( self ) :

		import IECore

		b = IECore.CubicBasisf.bezier()
		bb = eval( repr( b ) )
		self.assertEqual( b, bb )

if __name__ == "__main__":
    unittest.main()

