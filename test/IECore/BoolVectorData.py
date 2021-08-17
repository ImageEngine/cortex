##########################################################################
#
#  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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
import IECore
import random
import os

class BoolVectorDataTest( unittest.TestCase ) :

	def test( self ) :

		trueFalse = [ True, False ]

		random.seed( 0 )

		for i in range( 0, 100 ) :

			s = random.randint( 0, 100 )
			b = IECore.BoolVectorData( s )
			self.assertEqual( s, len( b ) )
			for j in range( 0, len( b ) ) :
				self.assertEqual( b[j], False )
				v = random.choice( trueFalse )
				b[j] = v
				self.assertEqual( b[j], v )

			bb = b.copy()
			self.assertEqual( b, bb )

			IECore.ObjectWriter( b, os.path.join( "test", "boolVector.cob" ) ).write()

			bbb = IECore.ObjectReader( os.path.join( "test", "boolVector.cob" ) ).read()

			self.assertEqual( b, bbb )

	def testStrAndRepr( self ) :

		self.assertEqual( str( IECore.BoolVectorData( [True, False] ) ), "1 0" )
		self.assertEqual( repr( IECore.BoolVectorData( [False, True] ) ), "IECore.BoolVectorData( [ 0, 1 ] )" )

	def testHasBase( self ) :

		self.assertFalse( IECore.BoolVectorData.hasBase() )

	def tearDown( self ) :

		if os.path.isfile( os.path.join( "test", "boolVector.cob" ) ):
			os.remove( os.path.join( "test", "boolVector.cob" ) )

if __name__ == "__main__":
    unittest.main()

