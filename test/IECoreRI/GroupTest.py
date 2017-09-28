##########################################################################
#
#  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import unittest
import os

import IECore
import IECoreRI

class GroupTest( IECoreRI.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/groupTest.rib"

	def testRender( self ) :

		g = IECore.Group()
		g.setTransform( IECore.MatrixTransform( IECore.M44f() ) )
		g.addState( IECore.AttributeState( { "name" : IECore.StringData( "bob" ) } ) )
		g.addChild( IECore.SpherePrimitive() )

		r = IECoreRI.Renderer( self.outputFileName )

		with IECore.WorldBlock( r ) :

			g.render( r )

		l = "\n".join( open( self.outputFileName ).readlines() )

		self.failUnless( "AttributeBegin" in l )
		self.failUnless( "ConcatTransform" in l )
		self.failUnless( "string name" in l )
		self.failUnless( "Sphere" in l )
		self.failUnless( "AttributeEnd" in l )

	def testRenderNoAttributeBlock( self ) :

		g = IECore.Group()
		g.setTransform( IECore.MatrixTransform( IECore.M44f() ) )
		g.addState( IECore.AttributeState( { "name" : IECore.StringData( "bob" ) } ) )
		g.addChild( IECore.SpherePrimitive() )

		r = IECoreRI.Renderer( self.outputFileName )

		with IECore.WorldBlock( r ) :

			g.render( r, False )

		l = "\n".join( open( self.outputFileName ).readlines() )

		self.failIf( "AttributeBegin" in l )
		self.failUnless( "ConcatTransform" in l )
		self.failUnless( "string name" in l )
		self.failUnless( "Sphere" in l )
		self.failIf( "AttributeEnd" in l )

	def testRenderChildren( self ) :

		g = IECore.Group()
		g.setTransform( IECore.MatrixTransform( IECore.M44f() ) )
		g.addState( IECore.AttributeState( { "name" : IECore.StringData( "bob" ) } ) )
		g.addChild( IECore.SpherePrimitive() )

		r = IECoreRI.Renderer( self.outputFileName )

		with IECore.WorldBlock( r ) :

			g.renderChildren( r )

		l = "\n".join( open( self.outputFileName ).readlines() )

		self.failIf( "AttributeBegin" in l )
		self.failIf( "ConcatTransform" in l )
		self.failIf( "string name" in l )
		self.failUnless( "Sphere" in l )
		self.failIf( "AttributeEnd" in l )

	def testRenderState( self ) :

		g = IECore.Group()
		g.setTransform( IECore.MatrixTransform( IECore.M44f() ) )
		g.addState( IECore.AttributeState( { "name" : IECore.StringData( "bob" ) } ) )
		g.addChild( IECore.SpherePrimitive() )

		r = IECoreRI.Renderer( self.outputFileName )

		with IECore.WorldBlock( r ) :

			g.renderState( r )

		l = "\n".join( open( self.outputFileName ).readlines() )

		self.failIf( "AttributeBegin" in l )
		self.failIf( "ConcatTransform" in l )
		self.failUnless( "string name" in l )
		self.failIf( "Sphere" in l )
		self.failIf( "AttributeEnd" in l )

if __name__ == "__main__":
    unittest.main()
