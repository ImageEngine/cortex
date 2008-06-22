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
import math

class FontTest( unittest.TestCase ) :

	def testConstructors( self ) :
	
		f = Font( "test/IECore/data/fonts/Vera.ttf" )

		self.assertRaises( Exception, Font, "notAFont" )
	
	def test( self ) :
	
		f = Font( "test/IECore/data/fonts/Vera.ttf" )
		
		g = f.meshGroup( "hello world" )
		m = f.mesh( "hello world" )
		
		self.assert_( g.isInstanceOf( Group.staticTypeId() ) )
		self.assert_( m.isInstanceOf( MeshPrimitive.staticTypeId() ) )
						
		v = 0
		for c in g.children() :
				
			self.assert_( c.isInstanceOf( Group.staticTypeId() ) )
			self.assertEqual( len( c.children() ), 1 )
			self.assert_( c.children()[0].isInstanceOf( MeshPrimitive.staticTypeId() ) )
			
			v += c.children()[0]["P"].data.size()
			
		self.assertEqual( v, m["P"].data.size() )
	
	def testCharBound( self ) :
	
		f = Font( "test/IECore/data/fonts/Vera.ttf" )
		
		b = f.bound()
		for c in range( 0, 128 ) :
		
			bb = f.bound( chr( c ) )
			self.assert_( b.contains( bb ) )
	
	def testImages( self ) :
	
		f = Font( "test/IECore/data/fonts/Vera.ttf" )
		f.setResolution( 300 )
		
		for c in range( 0, 128 ) :
				
			i = f.image( chr( c ) )
			self.assert_( i.displayWindow.contains( i.dataWindow ) )
			self.assert_( len( i ), 1 )
			self.assert_( "Y" in i )
			self.assert_( i.channelValid( "Y" ) )
		
	def testWholeImage( self ) :
	
		f = Font( "test/IECore/data/fonts/Vera.ttf" )
		f.setResolution( 50 )
		
		a = f.image( 'a' )
		i = f.image()
		
		self.assertEqual( (a.displayWindow.size().x + 1) * 16, i.displayWindow.size().x + 1 )
		self.assertEqual( (a.displayWindow.size().y + 1) * 8, i.displayWindow.size().y + 1 )
		
		self.assert_( len( i ), 1 )
		self.assert_( "Y" in i )
		self.assert_( i.channelValid( "Y" ) )
								
if __name__ == "__main__":
    unittest.main()   
