##########################################################################
#
#  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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
import shutil
import unittest

import IECore
import IECoreAppleseed

class RendererTest( unittest.TestCase ):

	__geometryDir = "contrib/IECoreAppleseed/test/IECoreAppleseed/_geometry"
	__appleseedFileName = "contrib/IECoreAppleseed/test/IECoreAppleseed/output.appleseed"

	def testTypeId( self ) :

		self.assertEqual( IECoreAppleseed.Renderer().typeId(), IECoreAppleseed.Renderer.staticTypeId() )
		self.assertNotEqual( IECoreAppleseed.Renderer.staticTypeId(), IECore.Renderer.staticTypeId() )

	def testTypeName( self ) :

		r = IECoreAppleseed.Renderer()
		self.assertEqual( r.typeName(), "IECoreAppleseed::Renderer" )

	def testAppleseedOutput( self ) :
		r = IECoreAppleseed.Renderer( self.__appleseedFileName )
		self.failIf( os.path.exists( self.__appleseedFileName ) )

		with IECore.WorldBlock( r ) :

			self.__createDefaultShader( r )
			m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			m.render( r )

		self.failUnless( os.path.exists( self.__appleseedFileName ) )

	def __createDefaultShader( self, r ) :
		s = IECore.Shader( "data/shaders/matte.oso", "surface" )
		s.render( r )

	def tearDown( self ) :

		for f in [
			self.__appleseedFileName,
		] :
			if os.path.exists( f ) :
				os.remove( f )

		shutil.rmtree( self.__geometryDir, ignore_errors = True )

if __name__ == "__main__":
	unittest.main()

