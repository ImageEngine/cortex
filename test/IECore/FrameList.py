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

import IECore

class TestFrameList( unittest.TestCase ) :

	def test( self ) :

		f = IECore.FrameList.parse( "" )
		self.assertTrue( isinstance( f, IECore.EmptyFrameList ) )
		self.assertEqual( len( f.asList() ), 0 )
		self.assertEqual( repr( f ), "IECore.EmptyFrameList()" )

		f = IECore.FrameList.parse( " " )
		self.assertTrue( isinstance( f, IECore.EmptyFrameList ) )
		self.assertEqual( len( f.asList() ), 0 )

	def testReverseConstruction( self ) :

		f = IECore.FrameList.parse( "1-5r" )
		self.assertEqual( f.asList(), [ 5, 4, 3, 2, 1 ] )
		self.assertEqual( IECore.frameListFromList( [ 5, 4, 3, 2, 1 ] ), f )

	def testFrameRange( self ) :

		f = IECore.FrameList.parse( "1-5" )
		self.assertTrue( isinstance( f, IECore.FrameRange ) )
		self.assertEqual( f.asList(), [ 1, 2, 3, 4, 5 ] )
		# test step
		f = IECore.FrameList.parse( "10-20x5" )
		self.assertTrue( isinstance( f, IECore.FrameRange ) )
		self.assertEqual( f.asList(), [ 10, 15, 20 ] )
		# start must be smaller or equal to end
		self.assertRaises( Exception, IECore.FrameList.parse, "5-1"  )
		# step must be positive
		self.assertRaises( Exception, IECore.FrameList.parse, "1-5x0" )
		self.assertRaises( Exception, IECore.FrameList.parse, "1-5x-1" )
		self.assertRaises( Exception, IECore.FrameList.parse, "5-1x-1" )


	## \todo: there should probably be a lot more tests in here...

if __name__ == "__main__":
	unittest.main()
