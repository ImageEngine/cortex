##########################################################################
#
#  Copyright (c) 2007-2017, Image Engine Design Inc. All rights reserved.
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
import IECoreImage

class ImageReaderTest( unittest.TestCase ) :

	def testFactoryConstruction( self ) :

		r = IECore.Reader.create( "test/IECoreImage/data/exr/AllHalfValues.exr" )
		self.assertEqual( type( r ), IECoreImage.ImageReader )

	def testCanReadAndIsComplete( self ) :

		self.assertTrue( IECoreImage.ImageReader.canRead( "test/IECoreImage/data/exr/AllHalfValues.exr" ) )
		self.assertFalse( IECoreImage.ImageReader.canRead( "thisFileDoesntExist.exr" ) )

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/AllHalfValues.exr" )
		self.assertTrue( r.isComplete() )

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/incomplete.exr" )
		self.assertFalse( r.isComplete() )

		r = IECoreImage.ImageReader( "thisFileDoesntExist.exr" )
		self.assertFalse( r.isComplete() )

	def testDeep( self ) :

		r = IECore.Reader.create( "test/IECoreImage/data/exr/primitives.exr" )
		self.assertEqual( r.channelNames(), IECore.StringVectorData( [] ) )
		self.assertTrue( r.isComplete() )
		h = r.readHeader()
		self.assertEqual( h[ "deep" ], IECore.BoolData( True ) )

		r = IECore.Reader.create( "test/IECoreImage/data/exr/manyChannels.exr" )
		self.assertNotEqual( r.channelNames(), IECore.StringVectorData( [] ) )
		h = r.readHeader()
		self.assertTrue( h[ "deep" ], IECore.BoolData( False ) )

		r = IECore.Reader.create( "test/IECoreImage/data/exr/deepIncomplete.exr" )
		self.assertFalse( r.isComplete() )

		r = IECore.Reader.create( "test/IECoreImage/data/exr/tiledDeepComplete.exr" )
		self.assertTrue( r.isComplete() )

		r = IECore.Reader.create( "test/IECoreImage/data/exr/tiledDeepIncomplete.exr" )
		self.assertFalse( r.isComplete() )

	def testSupportedExtensions( self ) :

		e = IECore.Reader.supportedExtensions( IECoreImage.ImageReader.staticTypeId() )
		for ee in e :
			self.assertTrue( type( ee ) is str )

		# we don't need to validate the full OIIO extension list, but
		# make sure a reasonable list of image files are supported
		expectedImageReaderExtensions = [ "exr", "cin", "dpx", "sgi", "rgba", "rgb", "tga", "tif", "tiff", "tx", "jpg", "jpeg", "png" ]
		self.assertTrue( set( expectedImageReaderExtensions ).issubset( e ) )

		# non image files aren't supported
		self.assertTrue( not "pdc" in e )
		self.assertTrue( not "cob" in e )
		self.assertTrue( not "obj" in e )

	def testChannelNames( self ) :

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/AllHalfValues.exr" )
		c = r.channelNames()
		self.assertEqual( c.staticTypeId(), IECore.StringVectorData.staticTypeId() )
		self.assertEqual( len( c ), 3 )
		self.assertTrue( "R" in c )
		self.assertTrue( "G" in c )
		self.assertTrue( "B" in c )

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/manyChannels.exr" )
		c = r.channelNames()
		self.assertEqual( c.staticTypeId(), IECore.StringVectorData.staticTypeId() )
		self.assertEqual( len( c ), 7 )
		self.assertTrue( "R" in c )
		self.assertTrue( "G" in c )
		self.assertTrue( "B" in c )
		self.assertTrue( "A" in c )
		self.assertTrue( "diffuse.red" in c )
		self.assertTrue( "diffuse.green" in c )
		self.assertTrue( "diffuse.blue" in c )

		r = IECoreImage.ImageReader( "thisFileDoesntExist.exr" )
		self.assertRaises( Exception, r.channelNames )

	def testReadHeader( self ):

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/manyChannels.exr" )
		h = r.readHeader()

		c = h['channelNames']
		self.assertEqual( c.staticTypeId(), IECore.StringVectorData.staticTypeId() )
		self.assertEqual( len( c ), 7 )
		self.assertTrue( "R" in c )
		self.assertTrue( "G" in c )
		self.assertTrue( "B" in c )
		self.assertTrue( "A" in c )
		self.assertTrue( "diffuse.red" in c )
		self.assertTrue( "diffuse.green" in c )
		self.assertTrue( "diffuse.blue" in c )

		self.assertEqual( h['displayWindow'], IECore.Box2iData( imath.Box2i( imath.V2i(0,0), imath.V2i(255,255) ) ) )
		self.assertEqual( h['dataWindow'], IECore.Box2iData( imath.Box2i( imath.V2i(0,0), imath.V2i(255,255) ) ) )

	def testDataAndDisplayWindows( self ) :

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/AllHalfValues.exr" )
		self.assertEqual( r.dataWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 255 ) ) )
		self.assertEqual( r.displayWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 255 ) ) )

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/uvMapWithDataWindow.100x100.exr" )
		self.assertEqual( r.dataWindow(), imath.Box2i( imath.V2i( 25 ), imath.V2i( 49 ) ) )
		self.assertEqual( r.displayWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 99 ) ) )

		r = IECoreImage.ImageReader( "thisFileDoesntExist.exr" )
		self.assertRaises( Exception, r.dataWindow )
		self.assertRaises( Exception, r.displayWindow )

	def testReadImage( self ) :

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/uvMap.256x256.exr" )

		i = r.read()
		self.assertEqual( i.typeId(), IECoreImage.ImagePrimitive.staticTypeId() )

		self.assertEqual( i.dataWindow, imath.Box2i( imath.V2i( 0 ), imath.V2i( 255 ) ) )
		self.assertEqual( i.displayWindow, imath.Box2i( imath.V2i( 0 ), imath.V2i( 255 ) ) )

		self.assertTrue( i.channelsValid() )

		self.assertEqual( len( i ), 3 )
		for c in ["R", "G", "B"] :
			self.assertEqual( i[c].typeId(), IECore.FloatVectorData.staticTypeId() )

		r = i["R"]
		self.assertEqual( r[0], 0 )
		self.assertEqual( r[-1], 1 )
		g = i["G"]
		self.assertEqual( g[0], 0 )
		self.assertEqual( g[-1], 1 )
		for b in i["B"] :
			self.assertEqual( b, 0 )

	def testReadIndividualChannels( self ) :

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/uvMap.256x256.exr" )
		i = r.read()

		for c in ["R", "G", "B"] :

			cd = r.readChannel( c )
			self.assertEqual( i[c], cd )

	def testNonZeroDataWindowOrigin( self ) :

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/uvMapWithDataWindow.100x100.exr" )
		i = r.read()

		self.assertEqual( i.dataWindow, imath.Box2i( imath.V2i( 25 ), imath.V2i( 49 ) ) )
		self.assertEqual( i.displayWindow, imath.Box2i( imath.V2i( 0 ), imath.V2i( 99 ) ) )

		self.assertTrue( i.channelsValid() )

	def testOrientation( self ) :

		img = IECore.Reader.create( "test/IECoreImage/data/exr/uvMap.512x256.exr" ).read()

		indexedColors = {
			0 : imath.V3f( 0, 0, 0 ),
			511 : imath.V3f( 1, 0, 0 ),
			512 * 255 : imath.V3f( 0, 1, 0 ),
			512 * 255 + 511 : imath.V3f( 1, 1, 0 ),
		}

		for index, expectedColor in indexedColors.items() :

			color = imath.V3f( img["R"][index], img["G"][index], img["B"][index] )

			self.assertTrue( ( color - expectedColor).length() < 1.e-6 )

	def testIncompleteImage( self ) :

		r = IECoreImage.ImageReader( "test/IECoreImage/data/exr/incomplete.exr" )
		self.assertRaisesRegexp( Exception, "Error reading pixel data from image file", r.read )

	def testHeaderToBlindData( self ) :

		dictHeader = {
			'channelNames': IECore.StringVectorData( [ "R", "G", "B" ] ),
			'oiio:ColorSpace': IECore.StringData( "Linear" ),
			'compression': IECore.StringData( "piz" ),
			'screenWindowCenter': IECore.V2fData( imath.V2f(0,0) ),
			'displayWindow': IECore.Box2iData( imath.Box2i( imath.V2i(0,0), imath.V2i(511,255) ) ),
			'dataWindow': IECore.Box2iData( imath.Box2i( imath.V2i(0,0), imath.V2i(511,255) ) ),
			'PixelAspectRatio': IECore.FloatData( 1 ),
			'screenWindowWidth': IECore.FloatData( 1 ),
			'deep': IECore.BoolData( False ),
		}

		r = IECore.Reader.create( "test/IECoreImage/data/exr/uvMap.512x256.exr" )
		header = r.readHeader()
		self.assertEqual( header, IECore.CompoundObject(dictHeader) )

		img = r.read()
		del dictHeader['channelNames']
		del dictHeader['deep']
		self.assertEqual( img.blindData(), IECore.CompoundData(dictHeader) )

	def testTimeCodeInHeader( self ) :

		r = IECore.Reader.create( "test/IECoreImage/data/exr/uvMap.512x256.exr" )
		header = r.readHeader()
		self.failUnless( "smpte:TimeCode" not in header )

		img = r.read()
		self.failUnless( "smpte:TimeCode" not in img.blindData() )

		td = IECore.TimeCodeData( IECore.TimeCode( 12, 5, 3, 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 ) )
		img2 = img.copy()
		img2.blindData()["smpte:TimeCode"] = td
		w = IECore.Writer.create( img2, "test/IECoreImage/data/exr/output.exr" )
		w["formatSettings"]["openexr"]["compression"].setValue( img2.blindData()["compression"] )
		w.write()

		r2 = IECore.Reader.create( "test/IECoreImage/data/exr/output.exr" )
		header = r2.readHeader()
		self.failUnless( "smpte:TimeCode" in header )
		self.assertEqual( header["smpte:TimeCode"], td )

		img3 = r2.read()
		self.failUnless( "smpte:TimeCode" in img3.blindData() )
		self.assertEqual( img3.blindData()["smpte:TimeCode"], td )
		del img3.blindData()["Software"]
		del img3.blindData()["HostComputer"]
		del img3.blindData()["DateTime"]
		self.assertEqual( img2.blindData(), img3.blindData() )
		del img3.blindData()["smpte:TimeCode"]
		self.assertEqual( img.blindData(), img3.blindData() )

	def testTilesWithLeftovers( self ) :

		r = IECoreImage.ImageReader( "test/IECoreImage/data/tiff/tilesWithLeftovers.tif" )
		i = r.read()
		i2 = IECoreImage.ImageReader( "test/IECoreImage/data/exr/tiffTileTestExpectedResults.exr" ).read()

		op = IECoreImage.ImageDiffOp()
		res = op(
			imageA = i,
			imageB = i2,
			maxError = 0.004,
			skipMissingChannels = False
		)

		self.failIf( res.value )

	def testTiff( self ) :

		for ( fileName, rawType ) in (
			( "test/IECoreImage/data/tiff/uvMap.200x100.rgba.8bit.tif", IECore.UCharVectorData ),
			( "test/IECoreImage/data/tiff/uvMap.200x100.rgba.16bit.tif", IECore.UShortVectorData ),
			( "test/IECoreImage/data/tiff/uvMap.200x100.rgba.32bit.tif", IECore.FloatVectorData ),
		) :

			r = IECore.Reader.create( fileName )
			self.assertEqual( type(r), IECoreImage.ImageReader )

			img = r.read()
			r["rawChannels"] = IECore.BoolData( True )
			imgRaw = r.read()

			self.assertEqual( img.displayWindow, imath.Box2i( imath.V2i( 0, 0 ), imath.V2i( 199, 99 ) ) )
			self.assertEqual( img.dataWindow, imath.Box2i( imath.V2i( 0, 0 ), imath.V2i( 199, 99 ) ) )

			self.assertTrue( img.channelsValid() )
			self.assertTrue( imgRaw.channelsValid() )

			self.assertEqual( img.keys(), imgRaw.keys() )

			for c in img.keys() :
				self.assertEqual( type(img[c]), IECore.FloatVectorData )
				self.assertEqual( type(imgRaw[c]), rawType )

	def testRawDPX( self ) :

		r = IECore.Reader.create( "test/IECoreImage/data/dpx/uvMap.512x256.dpx" )
		self.assertEqual( type(r), IECoreImage.ImageReader )
		r['rawChannels'] = True
		img = r.read()
		self.assertEqual( type(img), IECoreImage.ImagePrimitive )
		self.assertEqual( type(img['R']), IECore.UShortVectorData )

	def testJPG( self ) :

		r = IECore.Reader.create( "test/IECoreImage/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), IECoreImage.ImageReader )
		self.assertTrue( r.isComplete() )

		r = IECore.Reader.create( "test/IECoreImage/data/jpg/uvMap.512x256.truncated.jpg" )
		self.assertEqual( type(r), IECoreImage.ImageReader )
		self.assertFalse( r.isComplete() )

	def testFramesPerSecond( self ):
		# read an image that have the FramesPerSecond set and ensure the values are correctly identified
		r = IECore.Reader.create( "test/IECoreImage/data/exr/rationalFramesPerSecond.exr" )
		h1 = r.readHeader()
		self.failUnless( "framesPerSecond" in h1 )
		self.assertEqual( h1["framesPerSecond"].getInterpretation(), IECore.GeometricData.Interpretation.Rational )

		img = r.read()

		# write the image to filesystem and read it again to check that the value was set correctly
		w = IECore.Writer.create( img, "test/IECoreImage/data/exr/output.exr" )
		w.write()

		r2 = IECore.Reader.create( "test/IECoreImage/data/exr/output.exr" )
		h2 = r2.readHeader()
		self.failUnless( "framesPerSecond" in h2 )
		self.assertEqual( h2["framesPerSecond"].getInterpretation(), IECore.GeometricData.Interpretation.Rational )

		self.assertEqual( h1["framesPerSecond"], h2["framesPerSecond"] )

	def testMiplevel( self ) :

		# Test miplevel access for mipped files
		r = IECore.Reader.create( "test/IECoreImage/data/tx/uvMap.512x256.tx" )

		r["miplevel"] = IECore.IntData( 0 )
		self.assertEqual( r.dataWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 511, 255 ) ) )
		self.assertEqual( r.displayWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 511, 255 ) ) )

		r["miplevel"] = IECore.IntData( 1 )
		self.assertEqual( r.dataWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 255, 127 ) ) )
		self.assertEqual( r.displayWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 255, 127 ) ) )

		# Test miplevel access for files without mips (OIIO creates mips on the fly)
		r = IECore.Reader.create( "test/IECoreImage/data/jpg/uvMap.512x256.jpg" )

		r["miplevel"] = IECore.IntData( 0 )
		self.assertEqual( r.dataWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 511, 255 ) ) )
		self.assertEqual( r.displayWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 511, 255 ) ) )

		r["miplevel"] = IECore.IntData( 1 )
		self.assertEqual( r.dataWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 255, 127 ) ) )
		self.assertEqual( r.displayWindow(), imath.Box2i( imath.V2i( 0 ), imath.V2i( 255, 127 ) ) )

	def setUp( self ) :

		if os.path.isfile( "test/IECoreImage/data/exr/output.exr") :
			os.remove( "test/IECoreImage/data/exr/output.exr" )

	def tearDown( self ) :

		if os.path.isfile( "test/IECoreImage/data/exr/output.exr") :
			os.remove( "test/IECoreImage/data/exr/output.exr" )

if __name__ == "__main__":
	unittest.main()
