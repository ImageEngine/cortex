##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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
import os.path

import IECore
import IECoreRI

class CoordinateSystemTest( IECoreRI.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/coordSys.rib"

	def test( self ) :

		r = IECoreRI.Renderer( self.outputFileName )

		with IECore.WorldBlock( r ) :
			c = IECore.CoordinateSystem( "helloWorld" )
			c.render( r )

		l = "".join( file( self.outputFileName ).readlines() )
		self.failUnless( "ScopedCoordinateSystem \"helloWorld\"" in l )
		self.failIf( "TransformBegin" in l )
		self.failIf( "TransformEnd" in l )
		
	def testTransform( self ) :
	
		r = IECoreRI.Renderer( self.outputFileName )

		with IECore.WorldBlock( r ) :
			c = IECore.CoordinateSystem(
				"helloWorld",
				IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 1 ) ) ),
			)
			c.render( r )

		l = "".join( file( self.outputFileName ).readlines() )
		self.failUnless( "ScopedCoordinateSystem \"helloWorld\"" in l )
		self.failUnless( "TransformBegin" in l )
		self.failUnless( "TransformEnd" in l )
		self.failUnless( "ConcatTransform" in l )
	
	def testScoping( self ) :
	
		class TestProcedural( IECore.ParameterisedProcedural ) :
		
			def __init__( self ) :
			
				IECore.ParameterisedProcedural.__init__( self, "" )
			
			def doBound( self, args ) :
			
				return IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 10 ) )
				
			def doRender( self, renderer, args ) :
			
				self.coordinateSystem = renderer.getTransform( "testCoordSys" )

	
		renderer = IECoreRI.Renderer( "" )
		
		procedurals = []
		with IECore.WorldBlock( renderer ) :
		
			for i in range( 0, 10 ) :
			
				renderer.setAttribute( "user:proceduralIndex", IECore.IntData( i ) )
				
				g = IECore.Group()
				g.addState( IECore.CoordinateSystem( "testCoordSys", IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( i ) ) ) ) )
				p = TestProcedural()
				g.addChild( p )
				procedurals.append( p )
				
				g.render( renderer )
				
		for i in range( 0, 10 ) :
			
			self.assertEqual( procedurals[i].coordinateSystem.translation(), IECore.V3f( i ) )

if __name__ == "__main__":
    unittest.main()
