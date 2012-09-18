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

import unittest
from IECore import *

class TestEnum( unittest.TestCase ) :

	def test( self ) :

		E = Enum.create( "Red", "Green", "Blue" )
		self.assertNotEqual( E.Red, E.Green )
		self.assertEqual( E.Red, E( 0 ) )
		self.assertEqual( E.Green, E( 1 ) )
		self.assertEqual( E.Blue, E( 2 ) )
		self.assertEqual( E.Red, E( "Red" ) )
		self.assertEqual( E.Green, E( "Green" ) )
		self.assertEqual( E.Blue, E( "Blue" ) )
		self.assertEqual( str( E.Red ), "Red" )
		self.assertEqual( str( E.Green ), "Green" )
		self.assertEqual( str( E.Blue ), "Blue" )
		self.assertEqual( int( E.Red ), 0 )
		self.assertEqual( int( E.Green ), 1 )
		self.assertEqual( int( E.Blue ), 2 )
		
		self.failUnless( E.Red < E.Green )
		self.failUnless( E.Green < E.Blue )
		
		self.assertEqual( E.values(), ( E.Red, E.Green, E.Blue ) )
		
		self.assertRaises( ValueError, E, 3 )
		self.assertRaises( ValueError, E, "Tree" )

	def testHash( self ) :
	
		E = Enum.create( "Red", "Green", "Blue" )
		E2 = Enum.create( "Red", "Green", "Blue" )
		
		self.assertEqual( hash( E.Red ), hash( E.Red ) )
		self.assertEqual( hash( E2.Red ), hash( E2.Red ) )
		self.assertNotEqual( hash( E.Red ), hash( E2.Red ) )
		
		d = {}
		d[E.Red] = "a"
		d[E2.Red] = "b"
		
		self.assertEqual( len( d ), 2 )
		self.assertEqual( d[E.Red], "a" )
		self.assertEqual( d[E2.Red], "b" )

if __name__ == "__main__":
	unittest.main()

