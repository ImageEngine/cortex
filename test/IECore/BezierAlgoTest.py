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
from IECore import *
import math

class BezierAlgoTest( unittest.TestCase ) :

	def testCubic( self ) :

		def c( p, v ) :

			p.append( V3f( v[0], v[1], 0 ) )

		p = V3fVectorData()
		bezierSubdivide( V2f( 0 ), V2f( 0, 1 ), V2f( 1, 1 ), V2f( 1, 0 ), 0.01, lambda v : c( p, v ) )

		self.assertEqual( p[0], V3f( 0 ) )
		self.assertEqual( p[-1], V3f( 1, 0, 0 ) )


		b = Box3f( V3f( 0, 0, 0 ), V3f( 1, 1, 0 ) )
		for i in range( 0, p.size()-1 ) :

			self.assert_( b.intersects( p[i] ) )
			self.assert_( (p[i] - p[i+1]).length() < 0.18 )

	def testQuadratic( self ) :

		def c( p, v ) :

			p.append( V3f( v[0], v[1], 0 ) )

		p = V3fVectorData()
		bezierSubdivide( V2f( 0 ), V2f( 1, 1 ), V2f( 2, 0 ), 0.01, lambda v : c( p, v ) )

		self.assertEqual( p[0], V3f( 0 ) )
		self.assertEqual( p[-1], V3f( 2, 0, 0 ) )


		b = Box3f( V3f( 0, 0, 0 ), V3f( 2, 1, 0 ) )
		for i in range( 0, p.size()-1 ) :

			self.assert_( b.intersects( p[i] ) )
			self.assert_( (p[i] - p[i+1]).length() < 0.18 )

		# useful for making a visual check
		#m = MeshPrimitive( IntVectorData( [ p.size() ] ), IntVectorData( range(p.size() ) ), "linear", p )
		#Writer.create( m, "/tmp/bezier.cob" ).write()


if __name__ == "__main__":
    unittest.main()
