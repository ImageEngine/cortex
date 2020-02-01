##########################################################################
#
#  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreGL

class TextureLoaderTest( unittest.TestCase ) :

	def setUp( self ) :

		IECoreGL.init( False )

	def testLoadRGB( self ) :

		l = IECoreGL.TextureLoader( IECore.SearchPath( "./" ) )
		t = l.load( "test/IECoreImage/data/exr/carPark.exr" )
		self.failUnless( isinstance( t, IECoreGL.ColorTexture ) )

	def testLoadGreyscale( self ) :

		l = IECoreGL.TextureLoader( IECore.SearchPath( "./" ) )
		t = l.load( "test/IECoreImage/data/jpg/greyscaleCheckerBoard.jpg" )
		self.failUnless( isinstance( t, IECoreGL.LuminanceTexture ) )

	def testMaximumTextureResolution( self ) :

		maxResolution = 128

		l = IECoreGL.TextureLoader( IECore.SearchPath( "./" ) )

		t = l.load( "test/IECoreImage/data/exr/carPark.exr" )
		i = t.imagePrimitive()
		size = i.dataWindow.size()

		self.assertGreater( max( size.x, size.y ), maxResolution )

		t = l.load( "test/IECoreImage/data/exr/carPark.exr", maxResolution )
		i = t.imagePrimitive()
		size = i.dataWindow.size()

		# Test if resolution is adhering to our max resolution and has a reasonable size.
		self.assertLessEqual( max( size.x, size.y ), maxResolution )
		self.assertGreater( max( size.x, size.y ), int( 0.5 * maxResolution ) )

		t = l.load( "test/IECoreImage/data/exr/carPark.exr", int( 0.5 * maxResolution ) )
		i = t.imagePrimitive()
		size = i.dataWindow.size()

		self.assertLessEqual( max( size.x, size.y ), int( 0.5 * maxResolution ) )

if __name__ == "__main__":
	unittest.main()

