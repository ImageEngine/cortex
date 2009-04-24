##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
import os

class TestFrameListParameter( unittest.TestCase ) :

	def test( self ) :	
	
		p = IECore.FrameListParameter( "n", "d", "" )
		p.validate()
		self.assert_( isinstance( p.getFrameListValue(), IECore.EmptyFrameList ) )
		
		p.setTypedValue( "1-100" )
		p.validate()
		self.assert_( isinstance( p.getFrameListValue(), IECore.FrameRange ) )
		
		p.setValue( IECore.StringData( "i'mNotAFrameList" ) )
		self.assertRaises( RuntimeError, p.validate )
		
	def testNoEmptyList( self ) :
	
		p = IECore.FrameListParameter( "n", "d", "1-10", False )
		p.validate()
		
		p.setTypedValue( "" )
		self.assertRaises( RuntimeError, p.validate )

		p.setTypedValue( "1" )
		p.validate()
		
	def testSetAndGet( self ) :
	
		p = IECore.FrameListParameter( "n", "d", "" )
		p.setFrameListValue( IECore.FrameRange(  0, 100, 2 ) )
		self.assertEqual( p.getTypedValue(), "0-100x2" )
		
		self.assertEqual( p.getFrameListValue().asList(), IECore.FrameRange(  0, 100, 2 ).asList() )
	
	def testDefaultValueAsFrameList( self ) :
	
		p = IECore.FrameListParameter( "n", "d", IECore.FrameRange( 0, 10 ) )
		self.assertEqual( p.getFrameListValue().asList(), range( 0, 11 ) )
	
	def testOps( self ) :
	
		o1 = IECore.CheckImagesOp()
		o2 = IECore.FileSequenceGraphOp()
			
if __name__ == "__main__":
        unittest.main()
