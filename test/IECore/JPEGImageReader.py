##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
from IECore import *

class TestJPEGReader(unittest.TestCase):

	def testConstruction( self ):
	
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
	def testCanRead( self ):
	
		self.assert_( JPEGImageReader.canRead( "test/IECore/data/jpg/uvMap.512x256.jpg" ) )
		self.assert_( JPEGImageReader.canRead( "test/IECore/data/jpg/uvMap.512x256.truncated.jpg" ) )
		
	def testIsComplete( self ):	
	
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
		self.assert_( r.isComplete() )
		
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.truncated.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		self.assert_( not r.isComplete() )
		
	def testChannelNames( self ):	
	
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
		channelNames = r.channelNames()
		self.assertEqual( len( channelNames ), 3 )
		self.assert_( "R" in channelNames )
		self.assert_( "G" in channelNames )
		self.assert_( "B" in channelNames )
		
		r = Reader.create( "test/IECore/data/jpg/greyscaleCheckerBoard.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		channelNames = r.channelNames()
		self.assertEqual( len( channelNames ), 1 )
		self.assert_( channelNames[0]=="Y" )
		
	def testRead( self ):

		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )

		img = r.read()

		self.assertEqual( type(img), ImagePrimitive )
		
		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )
		self.assertEqual( img.dataWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )
		
	def testReadChannel( self ):
	
		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
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

	def testDataWindowRead( self ):

		r = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" )
		self.assertEqual( type(r), JPEGImageReader )
		
		dataWindow = Box2i(
			V2i(360, 160), 
			V2i(399, 199)
		)
		
		r.parameters().dataWindow.setValue( Box2iData( dataWindow ) )

		img = r.read()

		self.assertEqual( type(img), ImagePrimitive )
		
		self.assertEqual( img.dataWindow, dataWindow )
		self.assertEqual( img.displayWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )
		
		self.assertEqual( len(img["R"].data), 40 * 40 )
		
		ipe = PrimitiveEvaluator.create( img )
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf ( ipe.A() )
		
		result = ipe.createResult()
				
		# Check that the color at the bottom-right of the image is black - ordinarialy it would
		# be yellow, but we're deliberately not reading the entire image
		found = ipe.pointAtPixel( V2i( 511, 255 ), result )
		self.assert_( found )		
		color = V3f(
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)		
		expectedColor = V3f( 0, 0, 0 )
		self.assert_( ( color - expectedColor).length() < 1.e-3 )
		
		found = ipe.pointAtPixel( V2i( 380, 180 ), result )
		self.assert_( found )
		
		color = V3f(
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)
			
		expectedColor = V3f( 0.741211, 0.706055, 0 )	
		self.assert_( ( color - expectedColor).length() < 1.e-3 )
					
	def testOrientation( self ) :
		""" Test orientation of JPEG files """
	
		img = Reader.create( "test/IECore/data/jpg/uvMap.512x256.jpg" ).read()
		
		ipe = PrimitiveEvaluator.create( img )
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf ( ipe.A() )
		
		result = ipe.createResult()
		
		colorMap = {
			V2i( 0 ,    0 ) :  V3f( 0, 0, 0 ),
			V2i( 511,   0 ) :  V3f( 0.996, 0, 0 ),
			V2i( 0,   255 ) :  V3f( 0, 1, 0.004 ),
			V2i( 511, 255 ) :  V3f( 1, 1, 0.004 ),
		}
		
		for point, expectedColor in colorMap.items() :
		
			found = ipe.pointAtPixel( point, result )
			self.assert_( found )
			
			color = V3f(
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)
						
			self.assert_( ( color - expectedColor).length() < 1.e-3 )
			
	def testErrors( self ):
	
		r = JPEGImageReader()
		self.assertRaises( RuntimeError, r.read )
			
		r = JPEGImageReader( "test/IECore/data/tiff/uvMap.512x256.8bit.tif" )
		self.assertRaises( RuntimeError, r.read )
		
	def testAll( self ):
		
		fileNames = glob.glob( "test/IECore/data/jpg/*.jpg" ) + glob.glob( "test/IECore/data/jpg/*.jpeg" )
		
		expectedFailures = "test/IECore/data/jpg/uvMap.512x256.truncated.jpg"
		
		# Silence any warnings while the tests run
		MessageHandler.pushHandler( NullMessageHandler() )
		
		try:
		
			for f in fileNames:

				r = JPEGImageReader( f ) 
				
				if f in expectedFailures :
					
					self.assertRaises( RuntimeError, r.read )
					
				else:
				
					img = r.read()
					self.assertEqual( type(img), ImagePrimitive )
					self.assert_( img.arePrimitiveVariablesValid() )	
				
		except:
		
			raise	
			
		finally:
			
			MessageHandler.popHandler()	
	
		
if __name__ == "__main__":
	unittest.main()   
	
