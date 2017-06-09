##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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
import glob
import sys
import random
import shutil
import os

from IECore import *

class TestPNGReader(unittest.TestCase):

	def testConstruction( self ):

		r = Reader.create( "test/IECore/data/png/PngTestSuite/basn2c08.png" )
		self.assertEqual( type(r), PNGImageReader )

	def testCanRead( self ):

		self.assert_( PNGImageReader.canRead( "test/IECore/data/png/PngTestSuite/basn2c08.png" ) )

	def testIsComplete( self ):

		r = Reader.create( "test/IECore/data/png/kodak_dx7590_test.png" )
		self.assertEqual( type(r), PNGImageReader )

		self.assert_( r.isComplete() )

		r = Reader.create( "test/IECore/data/png/kodak_dx7590_test_truncated.png" )
		self.assertEqual( type(r), PNGImageReader )

		self.failIf( r.isComplete() )

	def testChannelNames( self ):

		# Test RGBA
		r = Reader.create( "test/IECore/data/png/PngTestSuite/basn6a08.png" )
		self.assertEqual( type(r), PNGImageReader )

		channelNames = r.channelNames()

		self.assertEqual( len( channelNames ), 4 )

		self.assert_( "R" in channelNames )
		self.assert_( "G" in channelNames )
		self.assert_( "B" in channelNames )
		self.assert_( "A" in channelNames )

		# Test YA
		r = Reader.create( "test/IECore/data/png/PngTestSuite/basn4a08.png" )
		self.assertEqual( type(r), PNGImageReader )

		channelNames = r.channelNames()

		self.assertEqual( len( channelNames ), 2 )

		self.assert_( "Y" in channelNames )
		self.assert_( "A" in channelNames )


	def testReadHeader( self ):

		r = Reader.create( "test/IECore/data/png/PngTestSuite/basn2c08.png" )
		self.assertEqual( type(r), PNGImageReader )
		h = r.readHeader()

		channelNames = h['channelNames']
		self.assertEqual( len( channelNames ), 3 )
		self.assert_( "R" in channelNames )
		self.assert_( "G" in channelNames )
		self.assert_( "B" in channelNames )

		self.assertEqual( h['displayWindow'], Box2iData( Box2i( V2i(0,0), V2i(31,31) ) ) )
		self.assertEqual( h['dataWindow'], Box2iData( Box2i( V2i(0,0), V2i(31,31) ) ) )

	def testRead( self ):

		r = Reader.create( "test/IECore/data/png/PngTestSuite/basn2c08.png" )
		self.assertEqual( type(r), PNGImageReader )

		img = r.read()

		self.assertEqual( type(img), ImagePrimitive )

		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 31, 31 ) ) )
		self.assertEqual( img.dataWindow, Box2i( V2i( 0, 0 ), V2i( 31, 31 ) ) )

	def testReadChannel( self ):

		r = Reader.create( "test/IECore/data/png/PngTestSuite/basn2c08.png" )
		self.assertEqual( type(r), PNGImageReader )

		red = r.readChannel( "R" )
		self.assert_( red )

		green = r.readChannel( "G" )
		self.assert_( green )

		blue = r.readChannel( "B" )
		self.assert_( blue )

		self.assertRaises( RuntimeError, r.readChannel, "NonExistantChannel" )

		self.assertEqual( len(red), len(green) )
		self.assertEqual( len(red), len(blue) )
		self.assertEqual( len(red), 32 * 32 )

	def testDataWindowRead( self ):

		r = Reader.create( "test/IECore/data/png/kodak_dx7590_test.png" )
		self.assertEqual( type(r), PNGImageReader )

		dataWindow = Box2i(
			V2i( 50, 50 ),
			V2i( 450, 200 )
		)
		
		imgOriginal = r.read()
		self.assertEqual( type(imgOriginal), ImagePrimitive )
		
		r.parameters()["dataWindow"].setValue( Box2iData( dataWindow ) )
		
		img = r.read()
		self.assertEqual( type(img), ImagePrimitive )
		
		self.assertEqual( img.dataWindow, dataWindow )
		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 2575, 1931 ) ) )

		self.assertEqual( len(img["R"].data), 401 * 151 )
		self.assertEqual( len(img["G"].data), 401 * 151 )
		self.assertEqual( len(img["B"].data), 401 * 151 )

		ipe = PrimitiveEvaluator.create( img )
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf ( ipe.A() )

		result = ipe.createResult()

		ipeOriginal = PrimitiveEvaluator.create( imgOriginal )

		resultOriginal = ipeOriginal.createResult()

		random.seed( 1 )

		# Test for equivalence using 50 random pixels. Inside the data window, we expect the
		# pixel values to be the same. Outside the data window we expect black.
		for i in range( 0, 50 ):

			pixel = V2i( int( random.uniform( 0, 511 ) ), int( random.uniform( 0, 255 ) ) )

			found = ipe.pointAtPixel( pixel, result )
			self.assert_( found )

			found = ipeOriginal.pointAtPixel( pixel, resultOriginal )
			self.assert_( found )

			color = V3f(
				result.floatPrimVar( ipe.R() ),
				result.floatPrimVar( ipe.G() ),
				result.floatPrimVar( ipe.B() )
			)

			if ( pixel.x >= dataWindow.min.x ) and ( pixel.x < dataWindow.max.x ) and (pixel.y >= dataWindow.min.y ) and ( pixel.y < dataWindow.max.y ) :

				expectedColor = V3f(
						resultOriginal.floatPrimVar( ipeOriginal.R() ),
						resultOriginal.floatPrimVar( ipeOriginal.G() ),
						resultOriginal.floatPrimVar( ipeOriginal.B() )
					)

			else :

				expectedColor = V3f( 0, 0, 0 )

			self.assert_( ( color - expectedColor).length() < 1.e-3 )

	def testOrientation( self ) :

		""" Test orientation of PNG files """

		img = Reader.create( "test/IECore/data/png/uvMap.512x256.8bit.png" ).read()

		ipe = PrimitiveEvaluator.create( img )
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf ( ipe.A() )

		result = ipe.createResult()

		colorMap = {
			V2i( 0 ,    0 ) :  V3f( 0, 0, 0 ),
			V2i( 511,   0 ) :  V3f( 1, 0, 0 ),
			V2i( 0,   255 ) :  V3f( 0, 1, 0 ),
			V2i( 511, 255 ) :  V3f( 1, 1, 0 ),
		}

		for point, expectedColor in colorMap.items() :

			found = ipe.pointAtPixel( point, result )
			self.assert_( found )

			color = V3f(
				result.floatPrimVar( ipe.R() ),
				result.floatPrimVar( ipe.G() ),
				result.floatPrimVar( ipe.B() )
			)

			self.assert_( ( color - expectedColor).length() < 1.e-6 )


	def testErrors( self ):

		r = PNGImageReader()
		self.assertRaises( RuntimeError, r.read )
		self.assertRaises( RuntimeError, r.readChannel, "R" )

		r = PNGImageReader( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertRaises( RuntimeError, r.read )

	def testPngTestSuite( self ):

		fileNames = glob.glob( "test/IECore/data/png/PngTestSuite/*.png" )

		expectedFailures = [
			
			"test/IECore/data/png/PngTestSuite/xc1n0g08.png",
			"test/IECore/data/png/PngTestSuite/xc9n2c08.png",
			"test/IECore/data/png/PngTestSuite/xcrn0g04.png",
			"test/IECore/data/png/PngTestSuite/xcsn0g01.png",
			"test/IECore/data/png/PngTestSuite/xd0n2c08.png",
			"test/IECore/data/png/PngTestSuite/xd3n2c08.png",
			"test/IECore/data/png/PngTestSuite/xd9n2c08.png",
			"test/IECore/data/png/PngTestSuite/xdtn0g01.png",
			"test/IECore/data/png/PngTestSuite/xhdn0g08.png",
			"test/IECore/data/png/PngTestSuite/xlfn0g04.png",
			"test/IECore/data/png/PngTestSuite/xs1n0g01.png",
			"test/IECore/data/png/PngTestSuite/xs2n0g01.png",
			"test/IECore/data/png/PngTestSuite/xs4n0g01.png",
			"test/IECore/data/png/PngTestSuite/xs7n0g01.png",
		]

		for f in fileNames:

			r = PNGImageReader( f )

			if f in expectedFailures :

				self.assertRaises( RuntimeError, r.read )

			else :
				self.assert_( PNGImageReader.canRead( f ) )
				self.failIf( TIFFImageReader.canRead( f ) )
				self.failIf( JPEGImageReader.canRead( f ) )
				self.failIf( EXRImageReader.canRead( f ) )
				self.failIf( CINImageReader.canRead( f ) )

				img = r.read()
				self.assertEqual( type(img), ImagePrimitive )
				self.assert_( img.arePrimitiveVariablesValid() )

	def testReadWithIncorrectExtension( self ) :
	
		shutil.copyfile( "test/IECore/data/png/kodak_dx7590_test.png", "test/IECore/data/png/kodak_dx7590_test.dpx" )
		
		# should be able to infer a correct reader even though the extension is incorrect
		r = Reader.create( "test/IECore/data/png/kodak_dx7590_test.dpx" )
		self.failUnless( isinstance( r, PNGImageReader ) )
		
		i = r.read()
		self.failUnless( isinstance( i, ImagePrimitive ) )
	
	def tearDown( self ) :
	
		for f in [
			 "test/IECore/data/png/kodak_dx7590_test.dpx",
		] :
			if os.path.exists( f ) :
				os.remove( f )
		
if __name__ == "__main__":
	unittest.main()

