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
import imath
import IECore
import IECoreScene

class TestGroup( unittest.TestCase ) :

	def test( self ) :

		g = IECoreScene.Group()
		self.assertEqual( g.getTransform(), None )
		self.assertEqual( g.transformMatrix(), imath.M44f() )

		g.setTransform( IECoreScene.MatrixTransform( imath.M44f().scale( imath.V3f( 2 ) ) ) )
		self.assertEqual( g.getTransform(), IECoreScene.MatrixTransform( imath.M44f().scale( imath.V3f( 2 ) ) ) )
		self.assertEqual( g.transformMatrix(), imath.M44f().scale( imath.V3f( 2 ) ) )

		self.assertEqual( g.children(), [] )
		self.assertEqual( g.state(), [] )

		# modifying children has no effect on the primitive - children()
		# returns the internal set copied into a list
		g.children().append( IECoreScene.PointsPrimitive( 1 ) )
		self.assertEqual( g.children(), [] )

		g.addChild( IECoreScene.PointsPrimitive( 1 ) )
		self.assertEqual( g.children(), [ IECoreScene.PointsPrimitive( 1 ) ] )

		# modifying state has no effect on the primitive - state()
		# returns the internal set copied into a list
		g.state().append( IECoreScene.AttributeState() )
		self.assertEqual( g.state(), [] )

		g.addState( IECoreScene.AttributeState() )
		self.assertEqual( g.state(), [ IECoreScene.AttributeState() ] )

		self.assertEqual( g, g )

		gg = g.copy()
		self.assertEqual( g, gg )
		self.assertTrue( not gg.children()[0].isSame( g.children()[0] ) )
		self.assertTrue( not gg.state()[0].isSame( g.state()[0] ) )

		IECore.ObjectWriter( g, os.path.join( "test", "group.cob" ) ).write()

		ggg = IECore.ObjectReader( os.path.join( "test", "group.cob" ) ).read()

		self.assertEqual( gg, ggg )
		self.assertTrue( not gg.children()[0].isSame(ggg.children()[0] ) )
		self.assertTrue( not gg.state()[0].isSame(ggg.state()[0] ) )

	def testStateAndChildOrder( self ) :

		# check the state/children don't get reordered when a group is written out to disk
		# and read back in again:
		g = IECoreScene.Group()

		for i in range( 100 ):
			g.addState( IECoreScene.Shader("%d" % i,"ddyup") )

			child = IECoreScene.Group()
			child.blindData()["id"] = IECore.IntData( i )
			g.addChild( child )

		IECore.ObjectWriter( g, os.path.join( "test", "group.cob" ) ).write()

		ggg = IECore.ObjectReader( os.path.join( "test", "group.cob" ) ).read()

		for i in range( 100 ):

			self.assertEqual( g.state()[i].name, ggg.state()[i].name )
			self.assertEqual( g.children()[i].blindData()["id"].value, ggg.children()[i].blindData()["id"].value )



	def testParent( self ) :

		g = IECoreScene.Group()
		g2 = IECoreScene.Group()

		self.assertTrue( g.parent() is None )
		self.assertTrue( g2.parent() is None )

		g.addChild( g2 )

		self.assertTrue( g.parent() is None )
		self.assertTrue( g2.parent().isSame( g ) )

		g.removeChild( g2 )

		self.assertTrue( g.parent() is None )
		self.assertTrue( g2.parent() is None )

		g.addChild( g2 )

		self.assertTrue( g.parent() is None )
		self.assertTrue( g2.parent().isSame( g ) )

		del g
		self.assertTrue( g2.parent() is None )

	def testAttributes( self ) :

		# create a little hierarchy
		g = IECoreScene.Group()
		g2 = IECoreScene.Group()
		g3 = IECoreScene.Group()

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
		g = IECoreScene.Group()
		g.addState( IECoreScene.AttributeState( {"toptest": IECore.BoolData( False ) } ) )
		g.addState( IECoreScene.AttributeState( {"toptest": IECore.BoolData( True ) } ) )

		self.assertEqual( g.getAttribute( "toptest" ), IECore.BoolData( True ) )

		# make sure attributes get added to existing attributeStates:
		g = IECoreScene.Group()
		g.addState( IECoreScene.Shader("yup","ddyup", {}) )
		g.addState( IECoreScene.AttributeState( {"toptest": IECore.BoolData( False ) } ) )
		g.addState( IECoreScene.Shader("yup","yup", {}) )
		g.setAttribute( "blahblah", IECore.BoolData( True ) )

		self.assertEqual( len( g.state() ), 3 )



	def testExceptions( self ) :

		g = IECoreScene.Group()

		self.assertRaises( Exception, g.removeChild, IECoreScene.Group() )
		self.assertRaises( Exception, g.removeState, IECoreScene.AttributeState() )

	def testTransformsNotState( self ) :

		g = IECoreScene.Group()
		self.assertRaises( Exception, g.addState, IECoreScene.MatrixTransform( imath.M44f() ) )

	def testChildOrdering( self ) :

		g = IECoreScene.Group()
		c1 = IECoreScene.PointsPrimitive( 1 )
		c2 = IECoreScene.PointsPrimitive( 2 )
		c3 = IECoreScene.PointsPrimitive( 3 )

		g.addChild( c1 )
		g.addChild( c2 )
		g.addChild( c3 )

		c = g.children()
		self.assertEqual( len( c ), 3 )
		self.assertTrue( c[0].isSame( c1 ) )
		self.assertTrue( c[1].isSame( c2 ) )
		self.assertTrue( c[2].isSame( c3 ) )

	def testStateOrdering( self ) :

		g = IECoreScene.Group()
		a1 = IECoreScene.AttributeState()
		a2 = IECoreScene.AttributeState()
		a3 = IECoreScene.AttributeState()

		g.addState( a1 )
		g.addState( a2 )
		g.addState( a3 )

		s = g.state()
		self.assertEqual( len( s ), 3 )
		self.assertTrue( s[0].isSame( a1 ) )
		self.assertTrue( s[1].isSame( a2 ) )
		self.assertTrue( s[2].isSame( a3 ) )

	def testAddNullState( self ) :

		g = IECoreScene.Group()
		self.assertRaises( Exception, g.addState, None )

	def testAddNullChild( self ) :

		g = IECoreScene.Group()
		self.assertRaises( Exception, g.addChild, None )

	def testNoneRefcount( self ) :

		# exercises a bug whereby we weren't incrementing the reference
		# count for Py_None when returning it to represent a null pointer.
		# this led to "Fatal Python error: deallocating None" type crashes
		g = IECoreScene.Group()
		for i in range( 0, sys.getrefcount( None ) + 100 ) :
			p = g.parent()

	def testMemoryUsage( self ) :

		# this used to crash if the group didn't have a transform
		g = IECoreScene.Group()
		self.assertTrue( g.memoryUsage() > 0 )

	def testHash( self ) :

		g = IECoreScene.Group()
		h = g.hash()

		g.addChild( IECoreScene.SpherePrimitive() )
		self.assertNotEqual( g.hash(), h )
		h = g.hash()

		g.addState( IECoreScene.AttributeState() )
		self.assertNotEqual( g.hash(), h )
		h = g.hash()

		g.setTransform( IECoreScene.MatrixTransform( imath.M44f() ) )
		self.assertNotEqual( g.hash(), h )

	def testGlobalTransform( self ) :

		g = IECoreScene.Group()
		childGroup = IECoreScene.Group()

		g.addChild( childGroup )

		parentTransform = IECore.TransformationMatrixf()
		parentTransform.rotate = imath.Eulerf( 0,3.1415926/2,0 )

		childTransform = IECore.TransformationMatrixf()
		childTransform.translate = imath.V3f( 1, 0, 2 )

		childGroup.setTransform( IECoreScene.MatrixTransform( childTransform.transform ) )
		g.setTransform( IECoreScene.MatrixTransform( parentTransform.transform ) )

		# child group's translation should have been rotated 90 degrees about the y axis:
		s = imath.V3f()
		h = imath.V3f()
		r = imath.V3f()
		childGroupGlobalTranslation = imath.V3f()
		childGroup.globalTransformMatrix().extractSHRT( s, h, r, childGroupGlobalTranslation )
		self.assertAlmostEqual( childGroupGlobalTranslation.x, 2, 4 )
		self.assertAlmostEqual( childGroupGlobalTranslation.y, 0, 4 )
		self.assertAlmostEqual( childGroupGlobalTranslation.z, -1, 4 )



	def tearDown( self ) :

		if os.path.isfile(os.path.join( "test", "group.cob" )):
			os.remove(os.path.join( "test", "group.cob" ))

if __name__ == "__main__":
	unittest.main()
