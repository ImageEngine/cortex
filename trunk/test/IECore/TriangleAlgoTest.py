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

import unittest
import math
from IECore import *

class TriangleAlgoTest( unittest.TestCase ) :

	def testContainsPoint( self ) :

		r = Rand32()
		v0 = V3f( 0, 0, 0 )
		v1 = V3f( 1, 0, 0 )
		v2 = V3f( 0, 1, 0 )
		for i in range( 0, 10000 ) :

			p = V3f( r.nextf( -1, 1 ), r.nextf( -1, 1 ), 0 )
			if p.x < 0 or p.y < 0 or p.x + p.y > 1 :
				self.failIf( triangleContainsPoint( v0, v1, v2, p ) )
			else :
				self.failUnless( triangleContainsPoint( v0, v1, v2, p ) )

		r = Rand32()
		for i in range( 0, 10000 ) :

			v0 = r.nextV3f()
			v1 = r.nextV3f()
			v2 = r.nextV3f()
			if triangleArea( v0, v1, v2 ) > 0.01 :

				u = r.nextf( 0, 1 )
				v = r.nextf( 0, 1 )
				if u + v < 1 :

					w = 1 - ( u + v )
					p = u * v0 + v * v1 + w * v2
					self.failUnless( triangleContainsPoint( v0, v1, v2, p ) )

	def testNormal( self ) :

		v0 = V3f( 0, 0, 0 )
		v1 = V3f( 1, 0, 0 )
		v2 = V3f( 0, 1, 0 )

		n = triangleNormal( v0, v1, v2 )
		self.assertEqual( n, V3f( 0, 0, 1 ) )
		
	def testContainsPointWithBarycentric( self ) :
	
		r = Rand32()
		v0 = V3f( 0, 0, 0 )
		v1 = V3f( 1, 0, 0 )
		v2 = V3f( 0, 1, 0 )

		for i in range( 0, 10000 ) :
		
			b = V3f( r.nextf( -1, 1 ), r.nextf( -1, 1 ), 0 )
			b.z = 1 - ( b.x + b.y )
			p = trianglePoint( v0, v1, v2, b )
			if p.x < 0 or p.y < 0 or p.x + p.y > 1 :
				self.failIf( triangleContainsPoint( v0, v1, v2, p ) )
			else :
				bb = triangleContainsPoint( v0, v1, v2, p )
				self.failUnless( bb.equalWithAbsError( b, 0.0001 ) )
				

if __name__ == "__main__":
    unittest.main()
