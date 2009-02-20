##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
from IECore import *

class testBinaryFrameList( unittest.TestCase ) :

	def test( self ) :
	
		r = BinaryFrameList( FrameRange( 1, 5) )
		self.assertEqual( r.asList(), [ 1, 5, 3, 2, 4 ] )
		
		r = BinaryFrameList( FrameRange( 1, 3) )
		self.assertEqual( r.asList(), [ 1, 3, 2 ] )
		
		r = BinaryFrameList( FrameRange( 1, 6) )
		self.assertEqual( r.asList(), [ 1, 6, 3, 2, 4, 5 ] )
		
		r = BinaryFrameList( FrameRange( 1, 7) )
		self.assertEqual( r.asList(), [ 1, 7, 4, 2, 5, 3, 6 ] )
		
	def testParsing( self ) :
	
		r = FrameList.parse( "1-5b" )
		self.assert_( isinstance( r, BinaryFrameList ) )
		self.assertEqual( r.asList(), [ 1, 5, 3, 2, 4 ] )
		
	def testStr( self ) :
	
		r = BinaryFrameList( FrameRange( 1, 11, 2 ) )
		self.assertEqual( str( r ), "1-11x2b" )
		
		r = BinaryFrameList( CompoundFrameList( [ FrameRange( 1, 10 ), FrameRange( 20, 30 ) ] )  )
		self.assertEqual( str( r ), "(1-10,20-30)b" )
		
	def testRepr( self ) :
		import IECore
		
		r = BinaryFrameList( FrameRange( 1, 11, 2 ) )
		self.assertEqual( r, eval( repr( r ) ) )
		
		r = BinaryFrameList( CompoundFrameList( [ FrameRange( 1, 10 ), FrameRange( 20, 30 ) ] )  )
		self.assertEqual( r, eval( repr( r ) ) )
		
	def testPreservation( self ) :
	
		for i in range( 1, 1000 ) :
		
			s = FrameRange( random.randint( 1, 100 ), random.randint( 100, 200 ) )
			ss = BinaryFrameList( s )
			
			sl = s.asList()
			ssl = ss.asList()
			self.assertEqual( len( sl ), len( ssl ) )
			
			for f in sl :
				self.assert_( f in ssl )

if __name__ == "__main__":
	unittest.main()
