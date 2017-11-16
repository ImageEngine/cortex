##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
import sys
import unittest
import IECore

class TestGroup( unittest.TestCase ) :

	def test( self ) :

		g = IECore.Group()
		self.assertEqual( g.getTransform(), None )
		self.assertEqual( g.transformMatrix(), IECore.M44f() )

		g.setTransform( IECore.MatrixTransform( IECore.M44f.createScaled( IECore.V3f( 2 ) ) ) )
		self.assertEqual( g.getTransform(), IECore.MatrixTransform( IECore.M44f.createScaled( IECore.V3f( 2 ) ) ) )
		self.assertEqual( g.transformMatrix(), IECore.M44f.createScaled( IECore.V3f( 2 ) ) )

		self.assertEqual( g.children(), [] )
		self.assertEqual( g.state(), [] )

		# modifying children has no effect on the primitive - children()
		# returns the internal set copied into a list
		g.children().append( IECore.PointsPrimitive( 1 ) )
		self.assertEqual( g.children(), [] )

		g.addChild( IECore.PointsPrimitive( 1 ) )
		self.assertEqual( g.children(), [ IECore.PointsPrimitive( 1 ) ] )

		# modifying state has no effect on the primitive - state()
		# returns the internal set copied into a list
		g.state().append( IECore.AttributeState() )
		self.assertEqual( g.state(), [] )

		g.addState( IECore.AttributeState() )
		self.assertEqual( g.state(), [ IECore.AttributeState() ] )

		self.assertEqual( g, g )

		gg = g.copy()
		self.assertEqual( g, gg )
		self.assert_( not gg.children()[0].isSame( g.children()[0] ) )
		self.assert_( not gg.state()[0].isSame( g.state()[0] ) )

		IECore.ObjectWriter( g, "test/group.cob" ).write()

		ggg = IECore.ObjectReader( "test/group.cob" ).read()

		self.assertEqual( gg, ggg )
		self.assert_( not gg.children()[0].isSame(ggg.children()[0] ) )
		self.assert_( not gg.state()[0].isSame(ggg.state()[0] ) )

	def testStateAndChildOrder( self ) :

		# check the state/children don't get reordered when a group is written out to disk
		# and read back in again:
		g = IECore.Group()

		for i in range( 100 ):
			g.addState( IECore.Shader("%d" % i,"ddyup") )

			child = IECore.Group()
			child.blindData()["id"] = IECore.IntData( i )
			g.addChild( child )

		IECore.ObjectWriter( g, "test/group.cob" ).write()

		ggg = IECore.ObjectReader( "test/group.cob" ).read()

		for i in range( 100 ):

			self.assertEqual( g.state()[i].name, ggg.state()[i].name )
			self.assertEqual( g.children()[i].blindData()["id"].value, ggg.children()[i].blindData()["id"].value )



	def testParent( self ) :

		g = IECore.Group()
		g2 = IECore.Group()

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

	def testAttributes( self ) :

		# create a little hierarchy
		g = IECore.Group()
		g2 = IECore.Group()
		g3 = IECore.Group()

		g.addChild( g2 )
		g2.addChild( g3 )

		# define an attribute at the top of the hierarchy
		g.setAttribute( "toptest", IECore.BoolData( False ) )
		self.assertEqual( g.getAttribute( "toptest" ), IECore.BoolData( False ) )

		# change our mind and set it to true:
		g.setAttribute( "toptest", IECore.BoolData( True ) )
		self.assertEqual( g.getAttribute( "toptest" ), IECore.BoolData( True ) )

		# add another attribute
		g.setAttribute( "toptest2", IECore.BoolData( True ) )
		self.assertEqual( g.getAttribute( "toptest2" ), IECore.BoolData( True ) )


		# make sure there's only one AttributeState on the group:
		self.assertEqual( len( g.state() ), 1 )

		# define one in the middle
		g2.setAttribute( "middletest", IECore.BoolData( True ) )

		# override the one at the top
		g2.setAttribute( "toptest", IECore.BoolData( False ) )

		# define one at the bottom
		g3.setAttribute( "bottomtest", IECore.BoolData( False ) )

		self.assertEqual( g.getAttribute( "toptest" ), IECore.BoolData( True ) )
		self.assertEqual( g.getAttribute( "middletest" ), None )
		self.assertEqual( g.getAttribute( "bottomtest" ), None )

		self.assertEqual( g2.getAttribute( "toptest" ), IECore.BoolData( False ) )
		self.assertEqual( g2.getAttribute( "middletest" ), IECore.BoolData( True ) )
		self.assertEqual( g2.getAttribute( "bottomtest" ), None )

		self.assertEqual( g3.getAttribute( "toptest" ), IECore.BoolData( False ) )
		self.assertEqual( g3.getAttribute( "middletest" ), IECore.BoolData( True ) )
		self.assertEqual( g3.getAttribute( "bottomtest" ), IECore.BoolData( False ) )


		# check that the final attribute state is returned by getAttribute:
		g = IECore.Group()
		g.addState( IECore.AttributeState( {"toptest": IECore.BoolData( False ) } ) )
		g.addState( IECore.AttributeState( {"toptest": IECore.BoolData( True ) } ) )

		self.assertEqual( g.getAttribute( "toptest" ), IECore.BoolData( True ) )

		# make sure attributes get added to existing attributeStates:
		g = IECore.Group()
		g.addState( IECore.Shader("yup","ddyup", {}) )
		g.addState( IECore.AttributeState( {"toptest": IECore.BoolData( False ) } ) )
		g.addState( IECore.Shader("yup","yup", {}) )
		g.setAttribute( "blahblah", IECore.BoolData( True ) )

		self.assertEqual( len( g.state() ), 3 )



	def testExceptions( self ) :

		g = IECore.Group()

		self.assertRaises( Exception, g.removeChild, IECore.Group() )
		self.assertRaises( Exception, g.removeState, IECore.AttributeState() )

	def testTransformsNotState( self ) :

		g = IECore.Group()
		self.assertRaises( Exception, g.addState, IECore.MatrixTransform( IECore.M44f() ) )

	def testChildOrdering( self ) :

		g = IECore.Group()
		c1 = IECore.PointsPrimitive( 1 )
		c2 = IECore.PointsPrimitive( 2 )
		c3 = IECore.PointsPrimitive( 3 )

		g.addChild( c1 )
		g.addChild( c2 )
		g.addChild( c3 )

		c = g.children()
		self.assertEqual( len( c ), 3 )
		self.assert_( c[0].isSame( c1 ) )
		self.assert_( c[1].isSame( c2 ) )
		self.assert_( c[2].isSame( c3 ) )

	def testStateOrdering( self ) :

		g = IECore.Group()
		a1 = IECore.AttributeState()
		a2 = IECore.AttributeState()
		a3 = IECore.AttributeState()

		g.addState( a1 )
		g.addState( a2 )
		g.addState( a3 )

		s = g.state()
		self.assertEqual( len( s ), 3 )
		self.assert_( s[0].isSame( a1 ) )
		self.assert_( s[1].isSame( a2 ) )
		self.assert_( s[2].isSame( a3 ) )

	def testAddNullState( self ) :

		g = IECore.Group()
		self.assertRaises( Exception, g.addState, None )

	def testAddNullChild( self ) :

		g = IECore.Group()
		self.assertRaises( Exception, g.addChild, None )

	def testNoneRefcount( self ) :

		# exercises a bug whereby we weren't incrementing the reference
		# count for Py_None when returning it to represent a null pointer.
		# this led to "Fatal Python error: deallocating None" type crashes
		g = IECore.Group()
		for i in range( 0, sys.getrefcount( None ) + 100 ) :
			p = g.parent()

	def testMemoryUsage( self ) :

		# this used to crash if the group didn't have a transform
		g = IECore.Group()
		self.failUnless( g.memoryUsage() > 0 )

	def testHash( self ) :

		g = IECore.Group()
		h = g.hash()

		g.addChild( IECore.SpherePrimitive() )
		self.assertNotEqual( g.hash(), h )
		h = g.hash()

		g.addState( IECore.AttributeState() )
		self.assertNotEqual( g.hash(), h )
		h = g.hash()

		g.setTransform( IECore.MatrixTransform( IECore.M44f() ) )
		self.assertNotEqual( g.hash(), h )

	def testGlobalTransform( self ) :

		g = IECore.Group()
		childGroup = IECore.Group()

		g.addChild( childGroup )

		parentTransform = IECore.TransformationMatrixf()
		parentTransform.rotate = IECore.Eulerf( 0,3.1415926/2,0 )

		childTransform = IECore.TransformationMatrixf()
		childTransform.translate = IECore.V3f( 1, 0, 2 )

		childGroup.setTransform( IECore.MatrixTransform( childTransform.transform ) )
		g.setTransform( IECore.MatrixTransform( parentTransform.transform ) )

		# child group's translation should have been rotated 90 degrees about the y axis:
		childGroupGlobalTranslation = childGroup.globalTransformMatrix().extractSHRT()[3]
		self.assertAlmostEqual( childGroupGlobalTranslation.x, 2, 4 )
		self.assertAlmostEqual( childGroupGlobalTranslation.y, 0, 4 )
		self.assertAlmostEqual( childGroupGlobalTranslation.z, -1, 4 )



	def tearDown( self ) :

		if os.path.isfile("test/group.cob"):
			os.remove("test/group.cob")

if __name__ == "__main__":
	unittest.main()
