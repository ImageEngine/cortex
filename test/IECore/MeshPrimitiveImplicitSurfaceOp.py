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



class TestMeshPrimitiveImplicitSurfaceOp( unittest.TestCase ) :

	def test( self ) :
		""" Test MeshPrimitiveImplicitSurfaceOp """

		# Poly sphere of radius 1
		m = Reader.create( "test/IECore/data/cobFiles/polySphereQuads.cob" ).read()
		radius = 1.0

		a = MeshPrimitiveImplicitSurfaceOp()

		thresholds = [ -0.8, -0.5, -0.2, 0.0, 0.2, 0.5, 0.8 ]

		maxAbsError = 0.2

		for threshold in thresholds:

			res = a(
				resolution = V3i( 20, 20, 20 ),
				bound = Box3f(
					V3f( -2, -2, -2 ),
					V3f(  2,  2,  2 )
				),
				threshold = threshold,

				input = m
			)

			pData = res["P"].data

			# It's now easy to establish how big the new sphere should be, from its original radius and
			# the threshold
			for p in pData:
				self.assert_( math.fabs( p.length() - ( radius + threshold ) ) < maxAbsError )




if __name__ == "__main__":
    unittest.main()
