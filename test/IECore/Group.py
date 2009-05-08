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

import os
import unittest
from IECore import *

class TestGroup( unittest.TestCase ) :

	def test( self ) :

		g = Group()
		self.assertEqual( g.getTransform(), None )
		self.assertEqual( g.transformMatrix(), M44f() )

		g.setTransform( MatrixTransform( M44f.createScaled( V3f( 2 ) ) ) )
		self.assertEqual( g.getTransform(), MatrixTransform( M44f.createScaled( V3f( 2 ) ) ) )
		self.assertEqual( g.transformMatrix(), M44f.createScaled( V3f( 2 ) ) )

		self.assertEqual( g.children(), [] )
		self.assertEqual( g.state(), [] )

		# modifying children has no effect on the primitive - children()
		# returns the internal set copied into a list
		g.children().append( PointsPrimitive( 1 ) )
		self.assertEqual( g.children(), [] )

		g.addChild( PointsPrimitive( 1 ) )
		self.assertEqual( g.children(), [ PointsPrimitive( 1 ) ] )

		# modifying state has no effect on the primitive - state()
		# returns the internal set copied into a list
		g.state().append( AttributeState() )
		self.assertEqual( g.state(), [] )

		g.addState( AttributeState() )
		self.assertEqual( g.state(), [ AttributeState() ] )

		self.assertEqual( g, g )

		gg = g.copy()
		self.assertEqual( g, gg )
		self.assert_( not gg.children()[0].isSame( g.children()[0] ) )
		self.assert_( not gg.state()[0].isSame( g.state()[0] ) )

		ObjectWriter( g, "test/group.cob" ).write()

		ggg = ObjectReader( "test/group.cob" ).read()

		self.assertEqual( gg, ggg )
		self.assert_( not gg.children()[0].isSame(ggg.children()[0] ) )
		self.assert_( not gg.state()[0].isSame(ggg.state()[0] ) )

	def testParent( self ) :

		g = Group()
		g2 = Group()

		self.assert_( g.parent() is None )
		self.assert_( g2.parent() is None )

		g.addChild( g2 )

		self.assert_( g.parent() is None )
		self.assert_( g2.parent().isSame( g ) )

		g.removeChild( g2 )

		self.assert_( g.parent() is None )
		self.assert_( g2.parent() is None )

		g.addChild( g2 )

		self.assert_( g.parent() is None )
		self.assert_( g2.parent().isSame( g ) )

		del g
		self.assert_( g2.parent() is None )

	def testExceptions( self ) :

		g = Group()

		self.assertRaises( Exception, g.removeChild, Group() )
		self.assertRaises( Exception, g.removeState, AttributeState() )

	def testTransformsNotState( self ) :

		g = Group()
		self.assertRaises( Exception, g.addState, MatrixTransform( M44f() ) )

	def testChildOrdering( self ) :

		g = Group()
		c1 = PointsPrimitive( 1 )
		c2 = PointsPrimitive( 2 )
		c3 = PointsPrimitive( 3 )

		g.addChild( c1 )
		g.addChild( c2 )
		g.addChild( c3 )

		c = g.children()
		self.assertEqual( len( c ), 3 )
		self.assert_( c[0].isSame( c1 ) )
		self.assert_( c[1].isSame( c2 ) )
		self.assert_( c[2].isSame( c3 ) )

	def testStateOrdering( self ) :

		g = Group()
		a1 = AttributeState()
		a2 = AttributeState()
		a3 = AttributeState()

		g.addState( a1 )
		g.addState( a2 )
		g.addState( a3 )

		s = g.state()
		self.assertEqual( len( s ), 3 )
		self.assert_( s[0].isSame( a1 ) )
		self.assert_( s[1].isSame( a2 ) )
		self.assert_( s[2].isSame( a3 ) )

	def tearDown( self ) :

		if os.path.isfile("test/group.cob"):
			os.remove("test/group.cob")

if __name__ == "__main__":
	unittest.main()
