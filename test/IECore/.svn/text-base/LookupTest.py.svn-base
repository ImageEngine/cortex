##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

class LookupTest( unittest.TestCase ) :

	def testLookupff( self ) :

		random.seed( 0 )
		lookup = IECore.Lookupff()

		for i in range( 0, 100 ) :

			s = IECore.Splineff()
			x = 0

			for i in range( 0, 10 ) :

				s[x] = random.uniform( 0, 4 )
				x += 1 + random.uniform( 0, 1 )

			xv = s.keys()
			yv = s.values()
			lookup.init( s, 0, 10, 100 )

			xvec = IECore.FloatVectorData()
			yvec = IECore.FloatVectorData()

			for i in range( 0, 1000 ) :

				x = i/100.0
				xvec.append(x)
				lookupY = lookup( x )
				yvec.append(lookupY)
				self.assertAlmostEqual( lookupY, s( x ), 1 )

			self.assertEqual( lookup(xvec), yvec )

if __name__ == "__main__":
    unittest.main()

