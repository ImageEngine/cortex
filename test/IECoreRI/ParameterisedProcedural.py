##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
import os
import IECore
import IECoreRI

class TeapotProcedural( IECore.ParameterisedProcedural ) :

	def __init__( self ) :

		IECore.ParameterisedProcedural.__init__( self )

		self.boundCalled = False
		self.renderCalled = False
		self.renderStateCalled = False

	def doBound( self, args ) :

		self.boundCalled = True
		return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )

	def doRenderState( self, renderer, args ) :

		self.renderStateCalled = True
		renderer.setAttribute( "ri:visibility:diffuse", IECore.BoolData( 1 ) )

	def doRender( self, renderer, args ) :

		self.renderCalled = True
		renderer.geometry( "teapot", {}, {} )


class ParameterisedProceduralTest( unittest.TestCase ) :

	def checkContents( self, fileName, expectedElements, unexpectedElements ) :

		l = file( fileName ).readlines()
		lineIndex = 0
		for expected in expectedElements :
			found = False
			for i in range( lineIndex, len( l ) ) :
				if expected in l[i] :
					lineIndex = i
					found = True
					break
			self.assert_( found )

		for e in unexpectedElements :
			for ll in l :
				self.assert_( e not in ll )

	def testNormalCall( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/testParameterisedProcedural.rib" )
		r.worldBegin()

		t = TeapotProcedural()

		t.render( r )

		r.worldEnd()

		self.checkContents(
			"test/IECoreRI/output/testParameterisedProcedural.rib",
			[
				"AttributeBegin",
				"Attribute \"visibility\" \"int diffuse\" [ 1 ]",
				"Geometry \"teapot\"",
				"AttributeEnd",
			],
			[]
		)

		self.assertEqual( t.renderStateCalled, True )
		self.assertEqual( t.boundCalled, True )
		self.assertEqual( t.renderCalled, True )

	def testStateOnly( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/testParameterisedProcedural.rib" )
		r.worldBegin()

		t = TeapotProcedural()

		t.render( r, inAttributeBlock=False, withState=True, withGeometry=False )

		r.worldEnd()

		self.checkContents(
			"test/IECoreRI/output/testParameterisedProcedural.rib",
			[
				"Attribute \"visibility\" \"int diffuse\" [ 1 ]",
			],
			[
				"AttributeBegin",
				"Geometry \"teapot\"",
				"AttributeEnd",
			],
		)

		self.assertEqual( t.renderStateCalled, True )
		self.assertEqual( t.boundCalled, False )
		self.assertEqual( t.renderCalled, False )

	def testImmediateGeometryOnly( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/testParameterisedProcedural.rib" )
		r.worldBegin()

		t = TeapotProcedural()

		t.render( r, inAttributeBlock=False, withState=False, withGeometry=True, immediateGeometry=True )

		r.worldEnd()

		self.checkContents(
			"test/IECoreRI/output/testParameterisedProcedural.rib",
			[
				"Geometry \"teapot\"",
			],
			[
				"AttributeBegin",
				"Attribute \"visibility\" \"int diffuse\" [ 1 ]",
				"AttributeEnd",
			],
		)

		self.assertEqual( t.renderStateCalled, False )
		self.assertEqual( t.boundCalled, False )
		self.assertEqual( t.renderCalled, True )

	def tearDown( self ) :

		files = [
			"test/IECoreRI/output/testParameterisedProcedural.rib"
		]
		for f in files :
			if os.path.exists( f ):
				os.remove( f )


if __name__ == "__main__":
    unittest.main()
