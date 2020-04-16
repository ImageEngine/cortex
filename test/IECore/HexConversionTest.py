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

import unittest
import random

import IECore

class HexConversionTest( unittest.TestCase ) :

	def testChar( self ) :

		hs = set()
		for i in range( 0, 128 ) :

			h = IECore.decToHexChar( chr( i ) )
			self.failUnless( isinstance( h, str ) )
			self.assertEqual( len( h ), 2 )
			self.failUnless( h[0] in "0123456789ABCDEF" )
			self.failUnless( h[1] in "0123456789ABCDEF" )

			d = IECore.hexToDecChar( h )
			self.assertEqual( ord( d ), i )

			hs.add( h )

		self.assertEqual( len( hs ), 128 )

	def testUInt( self ) :

		hs = set()
		m = 2 ** 32 - 1
		i = 0
		numTested = 0
		while i < m :

			h = IECore.decToHexUInt( i )
			self.failUnless( isinstance( h, str ) )
			self.assertEqual( len( h ), 8 )
			for c in h :
				self.failUnless( c in "0123456789ABCDEF" )

			d = IECore.hexToDecUInt( h )
			self.assertEqual(d, i )

			hs.add( h )

			i += random.randint( 10000, 100000 )
			numTested += 1

		self.assertEqual( len( hs ), numTested )

	def testCharVector( self ) :

		cv = IECore.CharVectorData( [ chr( x ) for x in range( 0, 256 ) ] )

		h = IECore.decToHexCharVector( cv )

		self.failUnless( isinstance( h, str ) )
		self.assertEqual( len( h ), 512 )

		cv2 = IECore.hexToDecCharVector( h )
		self.assertEqual( cv, cv2 )

if __name__ == "__main__":
	unittest.main()

