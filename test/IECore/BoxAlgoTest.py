##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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
import imath

import IECore

class BoxAlgoTest( unittest.TestCase ) :

	def testContains( self ) :

		b1 = imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) )
		b2 = imath.Box3f( imath.V3f( 0, -0.5, 0.5 ), imath.V3f( 0.1, 0, 0.9 ) )
		b3 = imath.Box3f( imath.V3f( -1.2, -0.6, 0.4 ), imath.V3f( 0.2, 0.1, 1 ) )

		self.assertTrue( IECore.BoxAlgo.contains( b1, b2 ) )
		self.assertFalse( IECore.BoxAlgo.contains( b2, b1 ) )

		self.assertFalse( IECore.BoxAlgo.contains( b2, b3 ) )
		self.assertTrue( IECore.BoxAlgo.contains( b3, b2 ) )

		self.assertFalse( IECore.BoxAlgo.contains( b3, b1 ) )
		self.assertFalse( IECore.BoxAlgo.contains( b1, b3 ) )

	def testSplit( self ) :

		r = imath.Rand32()
		for i in range( 0, 100 ) :

			b = imath.Box3f()
			b.extendBy( imath.V3f( r.nextf(), r.nextf(), r.nextf() ) )
			b.extendBy( imath.V3f( r.nextf(), r.nextf(), r.nextf() ) )

			major = b.majorAxis()

			low, high = IECore.BoxAlgo.split( b )
			low2, high2 = IECore.BoxAlgo.split( b, major )

			self.assertEqual( low, low2 )
			self.assertEqual( high, high2 )

			b2 = imath.Box3f()
			b2.extendBy( low )
			b2.extendBy( high )

			self.assertEqual( b, b2 )

if __name__ == "__main__":
	unittest.main()
