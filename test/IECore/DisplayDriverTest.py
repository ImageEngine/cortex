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

class TestDisplayDriver(unittest.TestCase):

	class MyDisplayDriver(DisplayDriver):

		def __init__( self, displayWindow, dataWindow, channelNames, parameters ):
			DisplayDriver.__init__( self, displayWindow, dataWindow, channelNames, parameters )
			self.count = 0

		def imageData( self, box, data):
			self.count = self.count + len(data)

		def imageClose( self ):
			self.count = -self.count

		def scanLineOrderOnly( self ):
			return True

	class MyDisplayDriverCreator( DisplayDriver.DisplayDriverCreator ):

		def create( self, displayWindow, dataWindow, channelNames, parameters ):
			return TestDisplayDriver.MyDisplayDriver( displayWindow, dataWindow, channelNames, parameters )

	def testSubclassing( self ):

		dd = self.MyDisplayDriver( Box2i(), Box2i(), [ 'r', 'g', 'b' ], CompoundData() )

		# call the base class virtual method an expect it to be redirected to the derived one ( as if it were being called by C++ ).
		v = FloatVectorData( [ 1,2,3] )
		DisplayDriver.imageData( dd, Box2i(), v )
		self.assertEqual( dd.count, 3 )
		DisplayDriver.imageClose( dd )
		self.assertEqual( dd.count, -3 )
		self.assertEqual( True, DisplayDriver.scanLineOrderOnly( dd ) )
		self.assertEqual( dd.channelNames(), ['r', 'g', 'b'] )

	def testFactory( self ):

		creator = self.MyDisplayDriverCreator()
		self.assertEqual( DisplayDriver.registerFactory( creator ), True )
		idd = DisplayDriver.create( Box2i( V2i(0,0), V2i(100,100) ), Box2i( V2i(10,10), V2i(40,40) ), [ 'r', 'g', 'b' ], CompoundData() )
		self.assertEqual( idd.scanLineOrderOnly(), True )
		self.assertEqual( idd.displayWindow(), Box2i( V2i(0,0), V2i(100,100) ) )
		self.assertEqual( idd.dataWindow(), Box2i( V2i(10,10), V2i(40,40) ) )
		self.assertEqual( idd.channelNames(), [ 'r', 'g', 'b' ] )
		self.assertEqual( DisplayDriver.unregisterFactory( creator ), True )
		self.assertEqual( DisplayDriver.unregisterFactory( creator ), False )

class TestImageDisplayDriver(unittest.TestCase):

	def testConstruction( self ):
		idd = ImageDisplayDriver( Box2i( V2i(0,0), V2i(100,100) ), Box2i( V2i(10,10), V2i(40,40) ), [ 'r','g','b' ], CompoundData() )
		self.assertEqual( idd.scanLineOrderOnly(), False )
		self.assertEqual( idd.displayWindow(), Box2i( V2i(0,0), V2i(100,100) ) )
		self.assertEqual( idd.dataWindow(), Box2i( V2i(10,10), V2i(40,40) ) )
		self.assertEqual( idd.channelNames(), [ 'r', 'g', 'b' ] )

	def __prepareBuf( self, buf, width, offset, red, green, blue ):
		for i in xrange( 0, width ):
			buf[3*i] = blue[i+offset]
			buf[3*i+1] = green[i+offset]
			buf[3*i+2] = red[i+offset]

	def testComplete( self ):

		img = Reader.create( "test/IECore/data/tiff/bluegreen_noise.400x300.tif" )()
		idd = ImageDisplayDriver( img.displayWindow, img.dataWindow, list( img.channelNames() ), CompoundData() )
		self.assertEqual( img.keys(), [ 'B', 'G', 'R' ] )
		red = img['R'].data
		green = img['G'].data
		blue = img['B'].data
		width = img.dataWindow.max.x - img.dataWindow.min.x + 1
		buf = FloatVectorData( width * 3 )
		for i in xrange( 0, img.dataWindow.max.y - img.dataWindow.min.y + 1 ):
			self.__prepareBuf( buf, width, i*width, red, green, blue )
			idd.imageData( Box2i( V2i( img.dataWindow.min.x, i + img.dataWindow.min.y ), V2i( img.dataWindow.max.x, i + img.dataWindow.min.y) ), buf )
		idd.imageClose()
		self.assertEqual( idd.image(), img )

	def testSubclassing( self ):

		class MyImageDisplayDriver(ImageDisplayDriver):

			def __init__( self, displayWindow, dataWindow, channelNames, parameters ):
				ImageDisplayDriver.__init__( self, displayWindow, dataWindow, channelNames, parameters )
				self.count = 0

			def imageData( self, box, data):
				self.count = self.count + 1

			def imageClose( self ):
				self.count = -self.count

			def scanLineOrderOnly( self ):
				return True

		img = Reader.create( "test/IECore/data/tiff/bluegreen_noise.400x300.tif" )()
		idd = MyImageDisplayDriver( img.displayWindow, img.dataWindow, list( img.channelNames() ), CompoundData() )
		self.assertEqual( img.keys(), [ 'B', 'G', 'R' ] )
		red = img['R'].data
		green = img['G'].data
		blue = img['B'].data
		width = img.dataWindow.max.x - img.dataWindow.min.x + 1
		buf = FloatVectorData( width * 3 )
		for i in xrange( 0, img.dataWindow.max.y - img.dataWindow.min.y + 1 ):
			self.__prepareBuf( buf, width, i*width, red, green, blue )
			ImageDisplayDriver.imageData( idd, Box2i( V2i( img.dataWindow.min.x, i + img.dataWindow.min.y ), V2i( img.dataWindow.max.x, i + img.dataWindow.min.y) ), buf )
		ImageDisplayDriver.imageClose( idd )
		self.assertEqual( idd.image(), img )
		self.assertEqual( idd.count, -300 )
		self.assertEqual( True, ImageDisplayDriver.scanLineOrderOnly( idd ) )
		
if __name__ == "__main__":
	unittest.main()   
	
