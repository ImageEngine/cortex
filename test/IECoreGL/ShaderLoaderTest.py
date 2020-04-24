##########################################################################
#
#  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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
import IECoreGL

IECoreGL.init( False )

class ShaderLoaderTest( unittest.TestCase ) :

	def test( self ) :

		sp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders" )
		l = IECoreGL.ShaderLoader( sp )

		s = l.load( "3dLabs/Toon" )
		self.assertTrue( s.typeName()=="IECoreGL::Shader" )

		ss = l.load( "3dLabs/Toon" )
		self.assertTrue( s.isSame( ss ) )

		# shader is too complicated for my graphics card
		s = l.load( "3dLabs/Mandel" )
		self.assertTrue( s.typeName()=="IECoreGL::Shader" )

		self.assertTrue( IECoreGL.ShaderLoader.defaultShaderLoader().isSame( IECoreGL.ShaderLoader.defaultShaderLoader() ) )

	def testPreprocessing( self ) :

		sp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders" )
		psp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders/include" )

		# this should work
		l = IECoreGL.ShaderLoader( sp, psp )
		s = l.load( "failWithoutPreprocessing" )

		# but turning off preprocessing should cause a throw
		l = IECoreGL.ShaderLoader( sp )
		self.assertRaises( RuntimeError, l.load, "failWithoutPreprocessing" )

	def testPreprocessingAllowsVersionAndExtension( self ) :

		sp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders" )
		psp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders/include" )
		l = IECoreGL.ShaderLoader( sp, psp )

		l.load( "versionAndExtension" )

	def testPreprocessingThrowsOnBadDirective( self ) :

		sp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders" )
		psp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders/include" )
		l = IECoreGL.ShaderLoader( sp, psp )

		self.assertRaises( RuntimeError, l.load, "badPreprocessingDirective" )

	def testLoadSourceMessagesAndCaching( self ) :

		sp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders" )
		psp = IECore.SearchPath( os.path.dirname( __file__ ) + "/shaders/include" )
		l = IECoreGL.ShaderLoader( sp, psp )

		with IECore.CapturingMessageHandler() as mh :

			source = l.loadSource( "thisShaderDoesntExist" )
			self.assertEqual( source, ( "", "", "" ) )

			source = l.loadSource( "thisShaderDoesntExist" )
			self.assertEqual( source, ( "", "", "" ) )

		# we don't want messages over and over for repeated failures to
		# load.
		self.assertEqual( len( mh.messages ), 1 )
		# but we do want a nice sensible message the first time.
		self.assertTrue( "thisShaderDoesntExist" in mh.messages[0].message )

	def testClear( self ) :

		sp = IECore.SearchPath( "/tmp" )
		l = IECoreGL.ShaderLoader( sp )

		f = open('/tmp/testShader.frag','w')
		f.write(
			"""void main()
			{
				gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
			}"""
		)
		f.close()

		s = l.load( "testShader" )

		f = open('/tmp/testShader.frag','w')
		f.write(
			"""void main()
			{
				gl_FragColor = vec4( 0.0, 1.0, 0.0, 1.0 );
			}"""
		)
		f.close()

		# Source is updated, but we will still reuse the cache
		s2 = l.load( "testShader" )
		self.assertTrue( s.isSame( s2 ) )

		l.clear()

		# After clearing, the shader is now updated.  ( Ideally we would test the modified functionality of the shader here, but that seems hard. )
		s3 = l.load( "testShader" )
		self.assertTrue( not s.isSame( s3 ) )


if __name__ == "__main__":
    unittest.main()
