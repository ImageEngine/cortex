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

class CoshaderTest( IECoreRI.TestCase ) :

	def testRendererSupport( self ) :

		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/coshaderTest.sdl test/IECoreRI/shaders/coshaderTest.sl" ), 0 )

		r = IECoreRI.Renderer( "test/IECoreRI/output/testCoshader.rib" )

		with IECore.WorldBlock( r ) :

			r.shader( "shader", "test/IECoreRI/shaders/coshaderTest", { "f" : 1.0, "s" : "hello", "__handle" : "h" } )

		r = "".join( file( "test/IECoreRI/output/testCoshader.rib" ).readlines() )

		self.failUnless( 'Shader "test/IECoreRI/shaders/coshaderTest" "h"' in r )
		self.failUnless( '"string s" [ "hello" ]' in r )
		self.failUnless( '"float f" [ 1 ]' in r )
		self.failIf( "__handle" in r )

	def testSLOReaderSupport( self ) :

		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/coshaderTest.sdl test/IECoreRI/shaders/coshaderTest.sl" ), 0 )

		s = IECoreRI.SLOReader( "test/IECoreRI/shaders/coshaderTest.sdl" ).read()

		# old versions of 3delight reported the type as <unknown> (which we don't really want) but
		# new versions report it correctly as "shader".
		self.failUnless( s.type == "<unknown>" or s.type == "shader" )

		k = s.parameters.keys()
		self.assertEqual( len( k ), 2 )
		self.failUnless( "f" in k )
		self.failUnless( "s" in k )

	def tearDown( self ) :

		IECoreRI.TestCase.tearDown( self )

		for f in [
			"test/IECoreRI/shaders/coshaderTest.sdl",
		] :

			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
    unittest.main()
