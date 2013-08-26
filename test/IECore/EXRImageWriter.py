##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
import sys, os
from IECore import *

from math import pow

class TestEXRWriter(unittest.TestCase):

	def __verifyImageRGB( self, imgNew, imgOrig, maxError = 0.002 ):

		self.assertEqual( type(imgNew), ImagePrimitive )

		if "R" in imgOrig :
			self.assert_( "R" in imgNew )

		if "G" in imgOrig :
			self.assert_( "G" in imgNew )

		if "B" in imgOrig :
			self.assert_( "B" in imgNew )

		if "A" in imgOrig :
			self.assert_( "A" in imgNew )

		if "Y" in imgOrig :
			self.assert_( "Y" in imgNew )

		op = ImageDiffOp()

		res = op(
			imageA = imgNew,
			imageB = imgOrig,
			maxError = maxError,
			skipMissingChannels = True
		)

		self.failIf( res.value )

	def __makeFloatImage( self, dataWindow, displayWindow, withAlpha = False, dataType = FloatVectorData ) :

		img = ImagePrimitive( dataWindow, displayWindow )

		w = dataWindow.max.x - dataWindow.min.x + 1
		h = dataWindow.max.y - dataWindow.min.y + 1

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

		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )

		if withAlpha:
			img["A"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, A )

		return img

	def testWrite( self ) :

		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)

		dataWindow = displayWindow

		for dataType in [ FloatVectorData ] :

			self.setUp()

			imgOrig = self.__makeFloatImage( dataWindow, displayWindow, dataType = dataType )
			w = Writer.create( imgOrig, "test/IECore/data/exrFiles/output.exr" )
			self.assertEqual( type(w), EXRImageWriter )
			w.write()

			self.assert_( os.path.exists( "test/IECore/data/exrFiles/output.exr" ) )

			# Now we've written the image, verify the rgb

			r = Reader.create( "test/IECore/data/exrFiles/output.exr" )
			imgNew = r.read()

			self.assertEqual( type(imgNew['R'].data), FloatVectorData )
			self.__verifyImageRGB( imgOrig, imgNew )

			self.tearDown()

	def testColorConversion(self):

		r = Reader.create( "test/IECore/data/exrFiles/ramp.exr" )
		imgOrig = r.read()
		self.assertEqual( type(imgOrig), ImagePrimitive )
		w = Writer.create( imgOrig, "test/IECore/data/exrFiles/output.exr" )
		self.assertEqual( type(w), EXRImageWriter )
		w.write()
		w = None
		r = Reader.create( "test/IECore/data/exrFiles/output.exr" )
		imgNew = r.read()
		self.assertEqual( type(imgNew), ImagePrimitive )
		self.assertEqual( imgOrig, imgNew )

	def testWriteIncomplete( self ) :

		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)

		dataWindow = displayWindow

		imgOrig = self.__makeFloatImage( dataWindow, displayWindow )

		# We don't have enough data to fill this dataWindow
		imgOrig.dataWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 199, 199 )
		)

		self.failIf( imgOrig.arePrimitiveVariablesValid() )

		w = Writer.create( imgOrig, "test/IECore/data/exrFiles/output.exr" )
		self.assertEqual( type(w), EXRImageWriter )

		self.assertRaises( RuntimeError, w.write )
		self.failIf( os.path.exists( "test/IECore/data/exrFiles/output.exr" ) )

	def testWindowWrite( self ) :

		dataWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)

		imgOrig = self.__makeFloatImage( dataWindow, dataWindow )

		imgOrig.displayWindow = Box2i(
			V2i( -20, -20 ),
			V2i( 199, 199 )
		)

		w = Writer.create( imgOrig, "test/IECore/data/exrFiles/output.exr" )
		self.assertEqual( type(w), EXRImageWriter )
		w.write()

		self.assert_( os.path.exists( "test/IECore/data/exrFiles/output.exr" ) )

		r = Reader.create( "test/IECore/data/exrFiles/output.exr" )
		imgNew = r.read()

		self.__verifyImageRGB( imgNew, imgOrig )

	def testOversizeDataWindow( self ) :

		r = Reader.create( "test/IECore/data/exrFiles/oversizeDataWindow.exr" )
		img = r.read()

		w = Writer.create( img, "test/IECore/data/exrFiles/output.exr" )
		self.assertEqual( type(w), EXRImageWriter )
		w.write()

		r = Reader.create( "test/IECore/data/exrFiles/output.exr" )
		imgNew = r.read()

		r = Reader.create( "test/IECore/data/expectedResults/oversizeDataWindow.dpx" )
		imgExpected = r.read()

		self.__verifyImageRGB( imgNew, imgExpected )

	def testCompressionParameter( self ):
		
		r = Reader.create( "test/IECore/data/exrFiles/oversizeDataWindow.exr" )
		img = r.read()

		w = Writer.create( img, "test/IECore/data/exrFiles/output.exr" )
		w['compression'].setValue( w['compression'].getPresets()['zip'] )
		w.write()

		w = EXRImageWriter()
		w['compression'].setValue( w['compression'].getPresets()['zip'] )

	def testBlindDataToHeader( self ) :

		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 9, 9 )
		)
		dataWindow = displayWindow

		headerValues = {
				"one": IntData( 1 ),
				"two": FloatData( 2 ),
				"three": DoubleData( 3 ),
				"four" : {
					"five": V2fData( V2f(5) ),
					"six": V2iData( V2i(6) ),
					"seven": V3fData( V3f(7) ),
					"eight": V3iData( V3i(8) ),
					"nine": {
						"ten": Box2iData( Box2i( V2i(0), V2i(10) ) ),
						"eleven": Box2fData( Box2f( V2f(0), V2f(11) ) ),
						"twelve": M33fData( M33f(12) ),
						"thirteen": M44fData( M44f(13) ),
					},
					"fourteen": StringData( "fourteen" ),
				}
			}

		imgOrig = self.__makeFloatImage( dataWindow, dataWindow )
		imgOrig.blindData().update( headerValues.copy() )
		# now add some unsupported types
		imgOrig.blindData()['notSupported1'] = FloatVectorData( [ 1,2,3] )
		imgOrig.blindData()['four']['notSupported2'] = DoubleVectorData( [1,2,3] )

		w = Writer.create( imgOrig, "test/IECore/data/exrFiles/output.exr" )
		self.assertEqual( type(w), EXRImageWriter )
		w.write()

		self.assert_( os.path.exists( "test/IECore/data/exrFiles/output.exr" ) )

		r = Reader.create( "test/IECore/data/exrFiles/output.exr" )
		imgNew = r.read()
		imgBlindData = imgNew.blindData()
		# eliminate default header info that comes on EXR..
		del imgBlindData['screenWindowCenter']
		del imgBlindData['displayWindow']
		del imgBlindData['dataWindow']
		del imgBlindData['pixelAspectRatio']
		del imgBlindData['screenWindowWidth']

		self.assertEqual( imgBlindData, CompoundData( headerValues ) )

	def setUp( self ) :

		if os.path.isfile( "test/IECore/data/exrFiles/output.exr") :
			os.remove( "test/IECore/data/exrFiles/output.exr" )

	def tearDown( self ) :

		if os.path.isfile( "test/IECore/data/exrFiles/output.exr") :
			os.remove( "test/IECore/data/exrFiles/output.exr" )


if __name__ == "__main__":
	unittest.main()

