##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

class TestPointBoundsOp( unittest.TestCase ) :

	def test( self ) :

		o = PointBoundsOp()
		b = o( points = V3fVectorData( [ V3f( 0 ) ] ) )
		self.assert_( b.isInstanceOf( Box3fData.staticTypeId() ) )
		self.assertEqual( b.value, Box3f( V3f( 0 ), V3f( 0 ) ) )

		o = PointBoundsOp()
		b = o(
			points = V3fVectorData( [ V3f( 0 ) ] ),
			velocities = V3fVectorData( [ V3f( 1 ) ] )
		)
		self.assertEqual( b.value, Box3f( V3f( 0 ), V3f( 1 ) ) )

		o = PointBoundsOp()
		b = o(
			points = V3fVectorData( [ V3f( 0 ) ] ),
			velocities = V3fVectorData( [ V3f( 1 ) ] ),
			velocityMultiplier = 0.5
		)
		self.assertEqual( b.value, Box3f( V3f( 0 ), V3f( 0.5 ) ) )

		o = PointBoundsOp()
		b = o(
			points = V3fVectorData( [ V3f( 0 ) ] ),
			velocities = V3fVectorData( [ V3f( 1 ) ] ),
			velocityMultiplier = 0.5,
			radii = FloatVectorData( [ 1 ] ),
		)
		self.assertEqual( b.value, Box3f( V3f( -1 ), V3f( 1.5 ) ) )

		o = PointBoundsOp()
		b = o(
			points = V3fVectorData( [ V3f( 0 ) ] ),
			velocities = V3fVectorData( [ V3f( 1 ) ] ),
			velocityMultiplier = 0.5,
			radii = FloatVectorData( [ 1 ] ),
			radiusMultiplier = 0.5,
		)
		self.assertEqual( b.value, Box3f( V3f( -0.5 ), V3f( 1 ) ) )

		o = PointBoundsOp()
		b = o(
			points = V3fVectorData( [ V3f( 0, 1, 2 ), V3f( 4, 5, 6 ) ] ),
		)
		self.assertEqual( b.value, Box3f( V3f( 0, 1, 2 ), V3f( 4, 5, 6 ) ) )

if __name__ == "__main__":
	unittest.main()

