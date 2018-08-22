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
import datetime
import unittest

import IECore
import IECoreImage

import imath

class ImageWriterTest( unittest.TestCase ) :

	def __verifyImageRGB( self, imgNew, imgOrig, maxError = 0.002, same=True ):

		self.assertEqual( type(imgNew), IECoreImage.ImagePrimitive )

		if "R" in imgOrig :
			self.assertTrue( "R" in imgNew )

		if "G" in imgOrig :
			self.assertTrue( "G" in imgNew )

		if "B" in imgOrig :
			self.assertTrue( "B" in imgNew )

		if "A" in imgOrig :
			self.assertTrue( "A" in imgNew )

		if "Y" in imgOrig :
			self.assertTrue( "Y" in imgNew )

		op = IECoreImage.ImageDiffOp()

		res = op(
			imageA = imgNew,
			imageB = imgOrig,
			maxError = maxError,
			skipMissingChannels = True
		)

		if same :
			self.assertFalse( res.value )
		else :
			self.assertTrue( res.value )

	def __makeFloatImage( self, dataWindow, displayWindow, withAlpha = False, dataType = IECore.FloatVectorData ) :

		img = IECoreImage.ImagePrimitive( dataWindow, displayWindow )

		w = dataWindow.max().x - dataWindow.min().x + 1
		h = dataWindow.max().y - dataWindow.min().y + 1

		area = w * h
		R = dataType( area )
		G = dataType( area )
		B = dataType( area )

		if withAlpha:
			A = dataType( area )

		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :

				R[offset] = float(x) / (w - 1)
				G[offset] = float(y) / (h - 1)
				B[offset] = 0.0
				if withAlpha:
					A[offset] = 0.5

				offset = offset + 1

		img["R"] = R
		img["G"] = G
		img["B"] = B

		if withAlpha:
			img["A"] = A

		return img

	def __makeIntImage( self, dataWindow, displayWindow, dataType = IECore.UIntVectorData, maxInt = 2**32-1 ) :

		img = IECoreImage.ImagePrimitive( dataWindow, displayWindow )

		w = dataWindow.max().x - dataWindow.min().x + 1
		h = dataWindow.max().y - dataWindow.min().y + 1

		area = w * h
		R = dataType( area )
		G = dataType( area )
		B = dataType( area )

		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :

				R[offset] = int( maxInt * float(x) / (w - 1) )
				G[offset] = int( maxInt * float(y) / (h - 1) )
				B[offset] = 0

				offset = offset + 1

		img["R"] = R
		img["G"] = G
		img["B"] = B

		return img

	def __makeGreyscaleImage( self, dataWindow, displayWindow ) :

		img = IECoreImage.ImagePrimitive( dataWindow, displayWindow )

		w = dataWindow.max().x - dataWindow.min().x + 1
		h = dataWindow.max().y - dataWindow.min().y + 1

		area = w * h
		Y = IECore.FloatVectorData( area )

		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :

				Y[offset] = float(x) / (w - 1)	* float(y) / (h - 1)

				offset = offset + 1

		img["Y"] = Y

		return img

	def testConstruction( self ) :

		w = IECoreImage.ImageWriter( IECoreImage.ImagePrimitive(), "test/IECoreImage/data/exr/AllHalfValues.exr" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )

		w = IECore.Writer.create( "test/IECoreImage/data/exr/AllHalfValues.exr" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )

	def testSupportedExtensions( self ) :

		e = IECore.Writer.supportedExtensions( IECoreImage.ImageWriter.staticTypeId() )
		for ee in e :
			self.assertTrue( type( ee ) is str )

		# we don't need to validate the full OIIO extension list, but
		# make sure a reasonable list of image files are supported
		expectedImageWriterExtensions = [ "exr", "cin", "dpx", "sgi", "rgba", "rgb", "tga", "tif", "tiff", "tx", "jpg", "jpeg", "png" ]
		self.assertTrue( set( expectedImageWriterExtensions ).issubset( e ) )

		# non image files aren't supported
		self.assertTrue( not "pdc" in e )
		self.assertTrue( not "cob" in e )
		self.assertTrue( not "obj" in e )

	def testCanWrite( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = displayWindow

		image = self.__makeFloatImage( dataWindow, displayWindow, dataType = IECore.FloatVectorData )
		self.assertTrue( IECoreImage.ImageWriter.canWrite( image, "test/IECoreImage/data/exr/output.exr" ) )

		# we dont support writing images of different channel types
		image["R"] = IECore.DoubleVectorData( [ x for x in image["R"] ] )
		self.assertFalse( IECoreImage.ImageWriter.canWrite( image, "test/IECoreImage/data/exr/output.exr" ) )

		# we dont support writing images if OIIO doesn't know how to use the channels
		image["R"] = IECore.StringVectorData( [ str(x) for x in image["R"] ] )
		image["G"] = IECore.StringVectorData( [ str(x) for x in image["G"] ] )
		image["B"] = IECore.StringVectorData( [ str(x) for x in image["B"] ] )
		self.assertFalse( IECoreImage.ImageWriter.canWrite( image, "test/IECoreImage/data/exr/output.exr" ) )

		# we dont support writing non-images
		self.assertFalse( IECoreImage.ImageWriter.canWrite( IECore.CompoundObject(), "test/IECoreImage/data/exr/output.exr" ) )

	def testWrite( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = displayWindow

		for dataType in [ IECore.FloatVectorData ] :

			self.setUp()

			imgOrig = self.__makeFloatImage( dataWindow, displayWindow, dataType = dataType )
			w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/exr/output.exr" )
			self.assertEqual( type(w), IECoreImage.ImageWriter )
			w.write()

			self.assertTrue( os.path.exists( "test/IECoreImage/data/exr/output.exr" ) )

			# Now we've written the image, verify the rgb

			r = IECore.Reader.create( "test/IECoreImage/data/exr/output.exr" )
			imgNew = r.read()

			self.assertEqual( type(imgNew['R']), IECore.FloatVectorData )
			self.__verifyImageRGB( imgOrig, imgNew )

			self.tearDown()

	def testWriteIncomplete( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = displayWindow

		imgOrig = self.__makeFloatImage( dataWindow, displayWindow )

		# We don't have enough data to fill this dataWindow
		imgOrig.dataWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 199, 199 )
		)

		self.failIf( imgOrig.channelsValid() )

		def fn():
			w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/exr/output.exr" )

		self.assertRaises( RuntimeError, fn )


	def testWindowWrite( self ) :

		dataWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		imgOrig = self.__makeFloatImage( dataWindow, dataWindow )

		imgOrig.displayWindow = imath.Box2i(
			imath.V2i( -20, -20 ),
			imath.V2i( 199, 199 )
		)

		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/exr/output.exr" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )
		w.write()

		self.assertTrue( os.path.exists( "test/IECoreImage/data/exr/output.exr" ) )

		r = IECore.Reader.create( "test/IECoreImage/data/exr/output.exr" )
		imgNew = r.read()

		self.__verifyImageRGB( imgNew, imgOrig )

	def testOversizeDataWindow( self ) :

		r = IECore.Reader.create( "test/IECoreImage/data/exr/oversizeDataWindow.exr" )
		img = r.read()

		w = IECore.Writer.create( img, "test/IECoreImage/data/exr/output.exr" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )
		w.write()

		r = IECore.Reader.create( "test/IECoreImage/data/exr/output.exr" )
		imgNew = r.read()

		r = IECore.Reader.create( "test/IECoreImage/data/exr/oversizeDataWindow.exr" )
		imgExpected = r.read()

		self.__verifyImageRGB( imgNew, imgExpected )


	def testWriteDataWindowFormatToNonDataWindowFormat( self ):
		# source data window inside display window
		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = imath.Box2i(
			imath.V2i( 10, 10 ),
			imath.V2i( 50, 50 )
		)

		imgOrig = self.__makeFloatImage( dataWindow, displayWindow )

		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/jpg/output.jpg" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )
		w.write()

		self.assertTrue( os.path.exists( "test/IECoreImage/data/jpg/output.jpg" ) )

		r = IECore.Reader.create( "test/IECoreImage/data/jpg/output.jpg" )
		imgNew = r.read()

		self.__verifyImageRGB( imgNew, imgOrig, maxError = 0.05 )

		# source data window outside display window
		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = imath.Box2i(
			imath.V2i( -10, -10 ),
			imath.V2i( 99, 99)
		)

		imgOrig = self.__makeFloatImage( dataWindow, displayWindow )

		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/jpg/output.jpg" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )
		w.write()

		self.assertTrue( os.path.exists( "test/IECoreImage/data/jpg/output.jpg" ) )

		# test if the scan line doesn't overlap the
		# display window at all.
		dataWindow = imath.Box2i(
			imath.V2i( 200, 50),
			imath.V2i( 505, 55)
		)

		imgOrig2 = self.__makeFloatImage( dataWindow, displayWindow )

		w = IECore.Writer.create( imgOrig2, "test/IECoreImage/data/jpg/output.png" )
		self.assertEqual( type( w ), IECoreImage.ImageWriter )
		w.write()

		self.assertTrue( os.path.exists( "test/IECoreImage/data/jpg/output.png" ) )


	def testBlindDataToHeader( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 9, 9 )
		)
		dataWindow = displayWindow

		headerValues = {
			"one": IECore.IntData( 1 ),
			"two": IECore.FloatData( 2 ),
			"three": IECore.DoubleData( 3 ),
			"four" : {
				"five": IECore.V2fData( imath.V2f(5) ),
				"six": IECore.V2iData( imath.V2i(6) ),
				"seven": IECore.V3fData( imath.V3f(7) ),
				"eight": IECore.V3iData( imath.V3i(8) ),
				"nine": {
					"ten": IECore.Box2iData( imath.Box2i( imath.V2i(0), imath.V2i(10) ) ),
					"eleven": IECore.Box2fData( imath.Box2f( imath.V2f(0), imath.V2f(11) ) ),
					"twelve": IECore.M33fData( imath.M33f(12) ),
					"thirteen": IECore.M44fData( imath.M44f(13) ),
				},
				"fourteen": IECore.StringData( "fourteen" ),
				"fifteen": IECore.TimeCodeData( IECore.TimeCode( 1, 2, 3, 4, dropFrame = True, bgf2 = True, binaryGroup4 = 4 ) ),
			}
		}

		imgOrig = self.__makeFloatImage( dataWindow, dataWindow )
		imgOrig.blindData().update( headerValues.copy() )
		# now add some unsupported types
		imgOrig.blindData()['notSupported1'] = IECore.FloatVectorData( [ 0,1,2,3,4 ] )
		imgOrig.blindData()['four']['notSupported2'] = IECore.DoubleVectorData( [ 0,1,2,3,4 ] )

		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/exr/output.exr" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )
		w.write()

		self.assertTrue( os.path.exists( "test/IECoreImage/data/exr/output.exr" ) )

		r = IECore.Reader.create( "test/IECoreImage/data/exr/output.exr" )
		imgNew = r.read()
		imgBlindData = imgNew.blindData()
		# eliminate default header info that comes from OIIO
		del imgBlindData['oiio:ColorSpace']
		del imgBlindData['compression']
		del imgBlindData['PixelAspectRatio']
		del imgBlindData['displayWindow']
		del imgBlindData['dataWindow']
		del imgBlindData['screenWindowCenter']
		del imgBlindData['screenWindowWidth']
		del imgBlindData["Software"]
		del imgBlindData["HostComputer"]
		del imgBlindData["DateTime"]

		self.assertEqual( imgBlindData, IECore.CompoundData( headerValues ) )

	def testJPG( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = displayWindow

		# JPEG default channels are 8-bit
		rawImage = self.__makeIntImage( dataWindow, displayWindow, dataType = IECore.UCharVectorData, maxInt = 2**8-1 )

		for dataType in [ IECore.FloatVectorData, IECore.HalfVectorData, IECore.DoubleVectorData ] :

			self.setUp()

			rawMode = ( dataType != IECore.FloatVectorData )
			imgOrig = self.__makeFloatImage( dataWindow, displayWindow, dataType = dataType )
			w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/jpg/output.jpg" )
			self.assertEqual( type(w), IECoreImage.ImageWriter )
			w['rawChannels'].setTypedValue( rawMode )
			w.write()

			self.assertTrue( os.path.exists( "test/IECoreImage/data/jpg/output.jpg" ) )

			# Now we've written the image, verify the rgb

			r = IECore.Reader.create( "test/IECoreImage/data/jpg/output.jpg" )
			r['rawChannels'].setTypedValue( rawMode )
			imgNew = r.read()

			if rawMode :
				self.assertEqual( type(imgNew['R']), IECore.UCharVectorData )
				self.__verifyImageRGB( rawImage, imgNew, 0.008 )
			else :
				self.assertEqual( type(imgNew['R']), IECore.FloatVectorData )
				self.__verifyImageRGB( imgOrig, imgNew, 0.008 )

			self.tearDown()

		for dataType in [ ( IECore.UIntVectorData, 2**32-1), (IECore.UCharVectorData, 2**8-1 ) ] :

			self.setUp()

			imgOrig = self.__makeIntImage( dataWindow, displayWindow, dataType = dataType[0], maxInt = dataType[1] )
			w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/jpg/output.jpg" )
			self.assertEqual( type(w), IECoreImage.ImageWriter )
			w['rawChannels'].setTypedValue( True )
			w.write()

			self.assertTrue( os.path.exists( "test/IECoreImage/data/jpg/output.jpg" ) )

			# Now we've written the image, verify the rgb
			r = IECore.Reader.create( "test/IECoreImage/data/jpg/output.jpg" )
			r['rawChannels'].setTypedValue( True )
			imgNew = r.read()
			self.__verifyImageRGB( rawImage, imgNew, 0.008 )

			self.tearDown()

	def testGreyscaleJPG( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 199, 99 )
		)

		dataWindow = displayWindow

		imgOrig = self.__makeGreyscaleImage( dataWindow, displayWindow )

		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/jpg/output.jpg" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )

		w.write()

		self.assertTrue( os.path.exists( "test/IECoreImage/data/jpg/output.jpg" ) )

		r = IECore.Reader.create( "test/IECoreImage/data/jpg/output.jpg" )
		imgNew = r.read()

		channelNames = r.channelNames()
		self.assertEqual( len(channelNames), 1 )

		self.__verifyImageRGB( imgNew, imgOrig )

	@unittest.skipIf( not os.path.exists( os.environ.get( "OCIO", "" ) ), "Insufficient color specification. Linear -> Cineon conversion is not possible with an OCIO config" )
	def testDPX( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = displayWindow

		# DPX default channels are ushort 16-bit
		rawImage = self.__makeIntImage( dataWindow, displayWindow, dataType = IECore.UShortVectorData, maxInt = 2**16-1 )

		for dataType in [ IECore.FloatVectorData, IECore.HalfVectorData, IECore.DoubleVectorData ] :

			self.setUp()

			rawMode = ( dataType != IECore.FloatVectorData )
			imgOrig = self.__makeFloatImage( dataWindow, displayWindow, dataType = dataType )
			w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/dpx/output.dpx" )
			self.assertEqual( type(w), IECoreImage.ImageWriter )
			w['rawChannels'].setTypedValue( rawMode )
			w.write()

			self.assertTrue( os.path.exists( "test/IECoreImage/data/dpx/output.dpx" ) )

			# Now we've written the image, verify the rgb

			r = IECore.Reader.create( "test/IECoreImage/data/dpx/output.dpx" )
			r['rawChannels'].setTypedValue( rawMode )
			imgNew = r.read()

			if rawMode :
				self.assertEqual( type(imgNew['R']), IECore.UShortVectorData )
				self.__verifyImageRGB( rawImage, imgNew )
			else :
				self.assertEqual( type(imgNew['R']), IECore.FloatVectorData )
				self.__verifyImageRGB( imgOrig, imgNew )

			self.tearDown()

		for dataType in [ ( IECore.UIntVectorData, 2**32-1), (IECore.UCharVectorData, 2**8-1 ) ] :

			self.setUp()

			imgOrig = self.__makeIntImage( dataWindow, displayWindow, dataType = dataType[0], maxInt = dataType[1] )
			w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/dpx/output.dpx" )
			self.assertEqual( type(w), IECoreImage.ImageWriter )
			w['rawChannels'].setTypedValue( True )
			w.write()

			self.assertTrue( os.path.exists( "test/IECoreImage/data/dpx/output.dpx" ) )

			# Now we've written the image, verify the rgb
			r = IECore.Reader.create( "test/IECoreImage/data/dpx/output.dpx" )
			r['rawChannels'].setTypedValue( True )
			imgNew = r.read()

			self.__verifyImageRGB( rawImage, imgNew, 0.003 )

			self.tearDown()

	def testTIF( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = displayWindow

		# TIFF default channels are uchar 8-bit
		rawImage = self.__makeIntImage( dataWindow, displayWindow, dataType = IECore.UCharVectorData, maxInt = 2**8-1 )

		for dataType in [ IECore.FloatVectorData, IECore.HalfVectorData, IECore.DoubleVectorData ] :

			self.setUp()

			rawMode = ( dataType != IECore.FloatVectorData )
			imgOrig = self.__makeFloatImage( dataWindow, displayWindow, dataType = dataType )
			w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/tiff/output.tif" )
			self.assertEqual( type(w), IECoreImage.ImageWriter )
			w['rawChannels'].setTypedValue( rawMode )
			w.write()

			self.assertTrue( os.path.exists( "test/IECoreImage/data/tiff/output.tif" ) )

			# Now we've written the image, verify the rgb
			r = IECore.Reader.create( "test/IECoreImage/data/tiff/output.tif" )
			r['rawChannels'].setTypedValue( rawMode )
			imgNew = r.read()

			if rawMode :
				self.assertEqual( type(imgNew['R']), IECore.UCharVectorData )
				self.__verifyImageRGB( rawImage, imgNew, 0.003 )
			else :
				self.assertEqual( type(imgNew['R']), IECore.FloatVectorData )
				self.__verifyImageRGB( imgOrig, imgNew )

			self.tearDown()

		for dataType in [ ( IECore.UIntVectorData, 2**32-1), (IECore.UCharVectorData, 2**8-1 ) ] :

			self.setUp()

			imgOrig = self.__makeIntImage( dataWindow, displayWindow, dataType = dataType[0], maxInt = dataType[1] )
			w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/tiff/output.tif" )
			self.assertEqual( type(w), IECoreImage.ImageWriter )
			w['rawChannels'].setTypedValue( True )
			w.write()

			self.assertTrue( os.path.exists( "test/IECoreImage/data/tiff/output.tif" ) )

			# Now we've written the image, verify the rgb
			r = IECore.Reader.create( "test/IECoreImage/data/tiff/output.tif" )
			r['rawChannels'].setTypedValue( True )
			imgNew = r.read()
			self.__verifyImageRGB( rawImage, imgNew, 0.003 )

			self.tearDown()

	def testAlphaTIF( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99 )
		)

		dataWindow = displayWindow

		imgOrig = self.__makeFloatImage( dataWindow, displayWindow, withAlpha = True )

		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/tiff/output.tif" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )
		w.write()
		self.assertTrue( os.path.exists( "test/IECoreImage/data/tiff/output.tif" ) )

		# Now we've written the image, verify the rgb
		imgNew = IECore.Reader.create( "test/IECoreImage/data/tiff/output.tif" ).read()
		self.__verifyImageRGB( imgOrig, imgNew )

		self.assertTrue( "A" in imgNew )

	def testGreyscaleTIF( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 199, 99 )
		)

		dataWindow = displayWindow

		imgOrig = self.__makeGreyscaleImage( dataWindow, displayWindow )

		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/tiff/output.tif" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )

		w.write()

		self.assertTrue( os.path.exists( "test/IECoreImage/data/tiff/output.tif" ) )

		r = IECore.Reader.create( "test/IECoreImage/data/tiff/output.tif" )
		imgNew = r.read()

		channelNames = r.channelNames()
		self.assertEqual( len(channelNames), 1 )

	def testRoundTripTIF( self ) :

		imgOrig = IECoreImage.ImageReader( "test/IECoreImage/data/tiff/bluegreen_noise.400x300.tif" ).read()
		self.assertTrue( imgOrig.channelsValid() )
		self.assertTrue( isinstance( imgOrig["R"], IECore.FloatVectorData ) )

		IECoreImage.ImageWriter( imgOrig, "test/IECoreImage/data/tiff/output.tif" ).write()
		self.assertTrue( os.path.exists( "test/IECoreImage/data/tiff/output.tif" ) )

		reader = IECoreImage.ImageReader( "test/IECoreImage/data/tiff/output.tif" )
		imgNew = reader.read()
		self.assertTrue( imgNew.channelsValid() )
		self.assertTrue( isinstance( imgNew["R"], IECore.FloatVectorData ) )
		self.__verifyImageRGB( imgOrig, imgNew, same=True )

		reader["rawChannels"].setTypedValue( True )
		imgRaw = reader.read()
		self.assertTrue( imgRaw.channelsValid() )
		self.assertTrue( isinstance( imgRaw["R"], IECore.UCharVectorData ) )
		self.__verifyImageRGB( imgRaw, imgNew, same=False )
		self.__verifyImageRGB( imgRaw, imgOrig, same=False )

		self.tearDown()
		self.assertFalse( os.path.exists( "test/IECoreImage/data/tiff/output.tif" ) )
		w = IECoreImage.ImageWriter( imgRaw, "test/IECoreImage/data/tiff/output.tif" )
		w['rawChannels'].setTypedValue( True )
		w.write()
		self.assertTrue( os.path.exists( "test/IECoreImage/data/tiff/output.tif" ) )

		reader = IECoreImage.ImageReader( "test/IECoreImage/data/tiff/output.tif" )
		reader["rawChannels"].setTypedValue( True )
		imgRawNew = reader.read()
		self.assertTrue( isinstance( imgRawNew["R"], IECore.UCharVectorData ) )
		self.__verifyImageRGB( imgRawNew, imgNew, same=False )
		self.__verifyImageRGB( imgRawNew, imgOrig, same=False )
		self.__verifyImageRGB( imgRawNew, imgRaw, same=True )

	def testEXRCompressionParameter( self ):

		r = IECore.Reader.create( "test/IECoreImage/data/exr/oversizeDataWindow.exr" )
		img = r.read()

		w = IECore.Writer.create( img, "test/IECoreImage/data/exr/output.exr" )
		w['formatSettings']['openexr']['compression'].setValue( 'zip' )
		w.write()

		self.assertEqual( IECore.Reader.create( "test/IECoreImage/data/exr/output.exr" ).readHeader()["compression"].value, "zip" )

		w = IECoreImage.ImageWriter()
		w['object'].setValue( img )
		w['fileName'].setTypedValue( "test/IECoreImage/data/exr/output.exr" )
		w['formatSettings']['openexr']['compression'].setValue( 'zips' )
		w.write()
		self.assertEqual( IECore.Reader.create( "test/IECoreImage/data/exr/output.exr" ).readHeader()["compression"].value, "zips" )

	def testJPGQualityParameter( self ) :

		w = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 99, 99)
		)

		img = self.__makeFloatImage( w, w )
		w = IECore.Writer.create( img, "test/IECoreImage/data/jpg/output.jpg" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )

		qualitySizeMap = {}
		lastSize = None
		for q in [ 0, 10, 50, 80, 100 ]:

			w['formatSettings']['jpeg']["quality"].setTypedValue( q )

			if os.path.exists( "test/IECoreImage/data/jpg/output.jpg" ) :
				os.remove( "test/IECoreImage/data/jpg/output.jpg" )

			w.write()
			self.assertTrue( os.path.exists( "test/IECoreImage/data/jpg/output.jpg" ) )

			size = os.path.getsize( "test/IECoreImage/data/jpg/output.jpg" )
			qualitySizeMap[q] = size

			if lastSize :
				self.assertTrue( size >= lastSize )

			lastSize = size

		self.assertTrue( qualitySizeMap[100] > qualitySizeMap[0] )

	def testTIFBitdepthParameter( self ) :

		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 199, 99 )
		)

		dataWindow = displayWindow

		imgOrig = self.__makeFloatImage( dataWindow, displayWindow )

		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/tiff/output.tif" )
		self.assertEqual( type(w), IECoreImage.ImageWriter )

		for b in w['formatSettings']['tiff']["dataType"].getPresets().keys() :

			self.setUp()

			w['formatSettings']['tiff']["dataType"].setValue( b )
			w.write()

			self.assertTrue( os.path.exists( "test/IECoreImage/data/tiff/output.tif" ) )

			imgNew = IECore.Reader.create( "test/IECoreImage/data/tiff/output.tif" ).read()
			self.__verifyImageRGB( imgOrig, imgNew )

			self.tearDown()

	def testEXRStringArrayMetadata( self ):
		displayWindow = imath.Box2i(
			imath.V2i( 0, 0 ),
			imath.V2i( 199, 99 )
		)

		dataWindow = displayWindow

		imgOrig = self.__makeFloatImage( dataWindow, displayWindow )

		imgOrig.blindData()["foobar"] = IECore.StringVectorData( ["abc", "def", "ghi"] )
		w = IECore.Writer.create( imgOrig, "test/IECoreImage/data/exr/metadata.exr" )

		w.write()

		imgNew = IECore.Reader.create( "test/IECoreImage/data/exr/metadata.exr" ).read()

		self.assertEqual( imgNew.blindData()["foobar"], IECore.StringVectorData( ["abc", "def", "ghi"] ) )

	def setUp( self ) :

		for f in (
			"test/IECoreImage/data/exr/output.exr",
			"test/IECoreImage/data/jpg/output.jpg",
			"test/IECoreImage/data/dpx/output.dpx",
			"test/IECoreImage/data/tiff/output.tif",
		) :
			if os.path.isfile( f ) :
				os.remove( f )

	def tearDown( self ) :
		for f in (
			"test/IECoreImage/data/exr/output.exr",
			"test/IECoreImage/data/jpg/output.jpg",
			"test/IECoreImage/data/dpx/output.dpx",
			"test/IECoreImage/data/tiff/output.tif",
		) :
			if os.path.isfile( f ) :
				os.remove( f )

if __name__ == "__main__":
	unittest.main()
