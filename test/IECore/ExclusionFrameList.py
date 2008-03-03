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

class ExclusionFrameListTest( unittest.TestCase ) :

	def test( self ) :
	
		f = ExclusionFrameList( FrameRange( 1, 10 ), FrameRange( 1, 11, 2 ) )
		self.assert_( isinstance( f, ExclusionFrameList ) )
		self.assertEqual( f.asList(), [ 2, 4, 6, 8, 10 ] )

	def testStr( self ) :
	
		f = ExclusionFrameList( FrameRange( 1, 10 ), FrameRange( 1, 11, 2 ) )
		self.assert_( isinstance( f, ExclusionFrameList ) )
		self.assertEqual( str( f ), "1-10!1-11x2" )

	def testParsing( self ) :

		f = FrameList.parse( "20-30!25" )
		self.assert_( isinstance( f, ExclusionFrameList ) )
		self.assertEqual( f.asList(), [ 20, 21, 22, 23, 24, 26, 27, 28, 29, 30 ] )
		
	def testParsingPrecedence( self ) :
	
		"""CompoundFrameList takes precedence over ExclusionFrameList when parsing."""
	
		f = FrameList.parse( "1,2,10-15!12,17" )
		self.assert_( isinstance( f, CompoundFrameList ) )
		self.assertEqual( f.asList(), [ 1, 2, 10, 11, 13, 14, 15, 17 ] )

if __name__ == "__main__":
	unittest.main()
