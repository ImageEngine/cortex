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

import unittest
import glob
import sys
import random
import shutil
import os

from IECore import *

def extractColorPixel( file, index ):
	return Color3f( file.read()["R"].data[index], file.read()["G"].data[index], file.read()["B"].data[index] )

class TestTIFFReader(unittest.TestCase):

	def testConstruction( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )

	def testCanRead( self ):

		self.assert_( TIFFImageReader.canRead( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" ) )

	def testIsComplete( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )

		self.assert_( r.isComplete() )

		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.16bit.truncated.tif" )
		self.assertEqual( type(r), TIFFImageReader )

		self.failIf( r.isComplete() )

	def testChannelNames( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )

		channelNames = r.channelNames()

		self.assertEqual( len( channelNames ), 3 )

		self.assert_( "R" in channelNames )
		self.assert_( "G" in channelNames )
		self.assert_( "B" in channelNames )

	def testReadHeader( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )
		h = r.readHeader()

		channelNames = h['channelNames']
		self.assertEqual( len( channelNames ), 3 )
		self.assert_( "R" in channelNames )
		self.assert_( "G" in channelNames )
		self.assert_( "B" in channelNames )

		self.assertEqual( h['displayWindow'], Box2iData( Box2i( V2i(0,0), V2i(511,255) ) ) )
		self.assertEqual( h['dataWindow'], Box2iData( Box2i( V2i(0,0), V2i(511,255) ) ) )

	def testManyChannels( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.100x100.manyChannels.16bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )

		img = r.read()

		self.assertEqual( type(img), ImagePrimitive )

		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 99, 99 ) ) )
		self.assertEqual( img.dataWindow, Box2i( V2i( 0, 0 ), V2i( 99, 99 ) ) )

		ipe = PrimitiveEvaluator.create( img )
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.assert_( ipe.A() )

		self.assert_( "Data1" in img )
		self.assert_( "Data2" in img )

	def testRead( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )

		img = r.read()

		self.assertEqual( type(img), ImagePrimitive )

		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )
		self.assertEqual( img.dataWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )

	def testReadChannel( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )

		red = r.readChannel( "R" )
		self.assert_( red )

		green = r.readChannel( "G" )
		self.assert_( green )

		blue = r.readChannel( "B" )
		self.assert_( blue )

		self.assertRaises( RuntimeError, r.readChannel, "NonExistantChannel" )

		self.assertEqual( len(red), len(green) )
		self.assertEqual( len(red), len(blue) )
		self.assertEqual( len(red), 512 * 256 )

	def testSourceColorSpace( self ):

		# for 32-bit floating point channels, it should be linear
		self.assertEqual( Reader.create( "test/IECore/data/tiff/uvMap.200x100.rgba.32bit.tif" ).sourceColorSpace(), "linear" )
		# for other bit-depths it assumes srgb
		self.assertEqual( Reader.create( "test/IECore/data/tiff/uvMap.200x100.rgba.8bit.tif" ).sourceColorSpace(), "srgb" )

	def testTDLColorSpace( self ):

		linearFile = Reader.create( "test/IECore/data/tdl/grey50_linear.tdl" )
		self.assertEqual( linearFile.sourceColorSpace(), "linear" )
		self.assertEqual( extractColorPixel( linearFile, 0 ), Color3f( 0.501960814 ) )
		srgbFile = Reader.create( "test/IECore/data/tdl/grey50_srgb.tdl" )
		self.assertEqual( srgbFile.sourceColorSpace(), "srgb" )
		self.assertEqual( extractColorPixel( srgbFile, 0 ), Color3f( 0.215860516 ) )
		rec709File = Reader.create( "test/IECore/data/tdl/grey50_BT709.tdl" )
		self.assertEqual( rec709File.sourceColorSpace(), "rec709" )
		self.assertEqual( extractColorPixel( rec709File, 0 ), Color3f( 0.261481494 ) )
		

	def testDataWindow( self ):
		r = Reader.create( "test/IECore/data/tiff/cropWindow.640x480.16bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )

		img = r.read()

		expectedDataWindow = Box2i(
			V2i( 320, 240 ),
			V2i( 479, 359 ),
		)

		expectedDisplayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 639,479 )
		)

		self.assertEqual( img.dataWindow, expectedDataWindow )
		self.assertEqual( img.displayWindow, expectedDisplayWindow )


	def testDataWindowRead( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		self.assertEqual( type(r), TIFFImageReader )

		dataWindow = Box2i(
			V2i( 360, 160 ),
			V2i( 399, 199 )
		)

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
		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )

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

		""" Test orientation of TIFF files """

		img = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" ).read()

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

	def testMultiDirectory( self ):

		r = Reader.create( "test/IECore/data/tiff/uvMap.multiRes.32bit.tif" )
		r['colorSpace'] = 'linear'
		self.assertEqual( type(r), TIFFImageReader )

		self.assertEqual( r.numDirectories(), 10 )

		self.assertRaises( RuntimeError, r.setDirectory, 10 )
		self.assertRaises( RuntimeError, r.setDirectory, 11 )
		self.assertRaises( RuntimeError, r.setDirectory, 200 )

		directoryResolutions = {
			0  : V2i( 256, 128 ),
			1  : V2i( 512, 256 ),
			2  : V2i( 256, 128 ),
			3  : V2i( 128,  64 ),
			4  : V2i(  64,  32 ),
			5  : V2i(  32,  16 ),
			6  : V2i(  16,   8 ),
			7  : V2i(   8,   4 ),
			8  : V2i(   4,   2 ),
			9  : V2i(   2,   1 ),
		}

		for dirIndex, resolution in directoryResolutions.items() :

			r.setDirectory( dirIndex )

			img = r.read()

			self.assertEqual( type(img), ImagePrimitive )

			bottomRight = V2i( resolution.x - 1, resolution.y - 1)

			self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), bottomRight ) )
			self.assertEqual( img.dataWindow, Box2i( V2i( 0, 0 ), bottomRight ) )

			expectedResult = Reader.create( "test/IECore/data/expectedResults/multiDirTiff" + str( dirIndex) + ".exr" ).read()

			op = ImageDiffOp()

			res = op(
				imageA = img,
				imageB = expectedResult,
				maxError = 0.004,
				skipMissingChannels = True
			)

			self.failIf( res.value )

	def testErrors( self ):

		r = TIFFImageReader()
		self.assertRaises( RuntimeError, r.read )
		self.assertRaises( RuntimeError, r.readChannel, "R" )

		r = TIFFImageReader( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertRaises( RuntimeError, r.read )

	def testAll( self ):

		fileNames = glob.glob( "test/IECore/data/tiff/*.tif" ) + glob.glob( "test/IECore/data/tiff/*.tiff" )

		expectedFailures = [
			"test/IECore/data/tiff/rgb_black_circle.256x256.4bit.tiff",
			"test/IECore/data/tiff/rgb_black_circle.256x256.2bit.tiff",
			"test/IECore/data/tiff/rgb_black_circle.256x256.1bit.tiff",
			"test/IECore/data/tiff/uvMap.512x256.16bit.truncated.tif",
		]

		for f in fileNames:

			r = TIFFImageReader( f )

			if f in expectedFailures :

				self.assertRaises( RuntimeError, r.read )

			else :
				self.assert_( TIFFImageReader.canRead( f ) )
				self.failIf( JPEGImageReader.canRead( f ) )
				self.failIf( EXRImageReader.canRead( f ) )
				self.failIf( CINImageReader.canRead( f ) )

				img = r.read()
				self.assertEqual( type(img), ImagePrimitive )
				self.assert_( img.arePrimitiveVariablesValid() )

	def testTilesWithLeftovers( self ) :

		"""Check we cope with tiled images where the width and height aren't multiples of the tile size."""

		r = TIFFImageReader( "test/IECore/data/tiff/tilesWithLeftovers.tif" )
		r['colorSpace'] = 'linear'
		i = r.read()
		i2 = EXRImageReader( "test/IECore/data/exrFiles/tiffTileTestExpectedResults.exr" ).read()

		op = ImageDiffOp()
		res = op(
			imageA = i,
			imageB = i2,
			maxError = 0.004,
			skipMissingChannels = False
		)

		self.failIf( res.value )
		
	def testReadWithIncorrectExtension( self ) :
	
		shutil.copyfile( "test/IECore/data/tiff/uvMap.512x256.8bit.tif", "test/IECore/data/tiff/uvMap.512x256.8bit.dpx" )
		
		# should be able to infer a correct reader even though the extension is incorrect
		r = Reader.create( "test/IECore/data/tiff/uvMap.512x256.8bit.dpx" )
		self.failUnless( isinstance( r, TIFFImageReader ) )
		
		i = r.read()
		self.failUnless( isinstance( i, ImagePrimitive ) )
	
	def testProblemTDL( self ) :
	
		# 3delight has started using the SMinSampleValue and SMaxSampleValue tags to store
		# the range of values in a tdl file. this is jolly useful for shader writers but a pain
		# for anyone using libtiff to read the images. libtiff currently doesn't support the
		# storage of different values per sample, and therefore complains when given
		# one of these files. we deal with this by pretending nothing has happened and allowing
		# all directories except the last one to be read (it's only the last one that has the
		# problem).
	
		r = TIFFImageReader( "test/IECore/data/tiff/problem.tdl" )
		
		expectedResolutions = [
			( 64, 32 ),
			( 32, 16 ),
			( 16, 8 ),
			( 8, 4 ),
			( 4, 2 ),
			# there should be a ( 2, 1 ) as well, but the best we can do is
			# ignore it.
		]
		
		self.assertEqual( r.numDirectories(), len( expectedResolutions ) )
		
		for i in range( 0, len( expectedResolutions ) ) :
			r.setDirectory( i )
			image = r.read()
			size = image.dataWindow.size()
			self.assertEqual( size.x + 1, expectedResolutions[i][0] )
			self.assertEqual( size.y + 1, expectedResolutions[i][1] )
			
	def tearDown( self ) :
	
		for f in [
			 "test/IECore/data/tiff/uvMap.512x256.8bit.dpx",
		] :
			if os.path.exists( f ) :
				os.remove( f )
		
if __name__ == "__main__":
	unittest.main()

