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
from IECore import *

class testReversedFrameList( unittest.TestCase ) :

	def test( self ) :
	
		r = ReversedFrameList( FrameRange( 1, 11, 2 ) )
		self.assertEqual( r.asList(), [ 11, 9, 7, 5, 3, 1 ] )
		
	def testParsing( self ) :
	
		r = FrameList.parse( "1-10r" )
		self.assert_( isinstance( r, ReversedFrameList ) )
		rr = range( 1, 11 )
		rr.reverse()
		self.assertEqual( r.asList(), rr )
		
		r = FrameList.parse( "1-4,5-10r" )
		self.assert_( isinstance( r, CompoundFrameList ) )
		self.assertEqual( r.asList(), [ 1, 2, 3, 4, 10, 9, 8, 7, 6, 5 ] )
		
		r = FrameList.parse( "(1-4,5-10)r" )
		self.assert_( isinstance( r, ReversedFrameList ) )
		self.assertEqual( r.asList(), [ 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 ] )
		
		r = FrameList.parse( "( 5-10 )r" )
		self.assert_( isinstance( r, ReversedFrameList ) )
		self.assertEqual( r.asList(), [ 10, 9, 8, 7, 6, 5 ] )
		
	def testStr( self ) :
	
		r = ReversedFrameList( FrameRange( 1, 11, 2 ) )
		self.assertEqual( str( r ), "1-11x2r" )
		
		r = ReversedFrameList( CompoundFrameList( [ FrameRange( 1, 10 ), FrameRange( 20, 30 ) ] )  )
		self.assertEqual( str( r ), "(1-10,20-30)r" )
		
		r = CompoundFrameList( [ FrameRange( 1, 10 ), ReversedFrameList( FrameRange( 20, 30 ) ) ] )
		self.assertEqual( str( r ), "1-10,20-30r" )
		
	def testRepr( self ) :
	
		import IECore
		
		r = ReversedFrameList( FrameRange( 1, 11, 2 ) )
		self.assertEqual( r, eval( repr( r ) ) )
		
		r = ReversedFrameList( CompoundFrameList( [ FrameRange( 1, 10 ), FrameRange( 20, 30 ) ] )  )
		self.assertEqual( r, eval( repr( r ) ) )
		
		r = CompoundFrameList( [ FrameRange( 1, 10 ), ReversedFrameList( FrameRange( 20, 30 ) ) ] )
		self.assertEqual( r, eval( repr( r ) ) )
		
	def testImpactOnOtherParsing( self ) :
	
		r = FrameList.parse( "(1-10)" )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 10 )
		self.assertEqual( r.step, 1 )
		
		r = FrameList.parse( "( 1-10 )" )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 10 )
		self.assertEqual( r.step, 1 )


if __name__ == "__main__":
	unittest.main()
