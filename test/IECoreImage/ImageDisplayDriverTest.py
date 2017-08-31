##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
#  Copyright (c) 2012, John Haddon. All rights reserved.
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
import os
import gc
import glob
import sys
import time
import IECore
import IECoreImage

class ImageDisplayDriverTest(unittest.TestCase):

	def testConstruction( self ):
		idd = IECoreImage.ImageDisplayDriver( IECore.Box2i( IECore.V2i(0,0), IECore.V2i(100,100) ), IECore.Box2i( IECore.V2i(10,10), IECore.V2i(40,40) ), [ 'r','g','b' ], IECore.CompoundData() )
		self.assertEqual( idd.scanLineOrderOnly(), False )
		self.assertEqual( idd.displayWindow(), IECore.Box2i( IECore.V2i(0,0), IECore.V2i(100,100) ) )
		self.assertEqual( idd.dataWindow(), IECore.Box2i( IECore.V2i(10,10), IECore.V2i(40,40) ) )
		self.assertEqual( idd.channelNames(), [ 'r', 'g', 'b' ] )

	def __prepareBuf( self, buf, width, offset, red, green, blue ):
		for i in xrange( 0, width ):
			buf[3*i] = blue[i+offset]
			buf[3*i+1] = green[i+offset]
			buf[3*i+2] = red[i+offset]

	def testComplete( self ):

		img = IECore.Reader.create( "test/IECoreImage/data/tiff/bluegreen_noise.400x300.tif" )()
		img.blindData().clear()
		idd = IECoreImage.ImageDisplayDriver( img.displayWindow, img.dataWindow, list( img.channelNames() ), IECore.CompoundData() )
		self.assertEqual( img.keys(), [ 'B', 'G', 'R' ] )
		red = img['R']
		green = img['G']
		blue = img['B']
		width = img.dataWindow.max.x - img.dataWindow.min.x + 1
		buf = IECore.FloatVectorData( width * 3 )
		for i in xrange( 0, img.dataWindow.max.y - img.dataWindow.min.y + 1 ):
			self.__prepareBuf( buf, width, i*width, red, green, blue )
			idd.imageData( IECore.Box2i( IECore.V2i( img.dataWindow.min.x, i + img.dataWindow.min.y ), IECore.V2i( img.dataWindow.max.x, i + img.dataWindow.min.y) ), buf )
		idd.imageClose()
		self.assertEqual( idd.image(), img )

	def testFactory( self ):

		idd = IECoreImage.DisplayDriver.create( "ImageDisplayDriver", IECore.Box2i( IECore.V2i(0,0), IECore.V2i(100,100) ), IECore.Box2i( IECore.V2i(10,10), IECore.V2i(40,40) ), [ 'r', 'g', 'b' ], IECore.CompoundData() )
		self.failUnless( isinstance( idd, IECoreImage.ImageDisplayDriver ) )
		self.assertEqual( idd.scanLineOrderOnly(), False )
		self.assertEqual( idd.displayWindow(), IECore.Box2i( IECore.V2i(0,0), IECore.V2i(100,100) ) )
		self.assertEqual( idd.dataWindow(), IECore.Box2i( IECore.V2i(10,10), IECore.V2i(40,40) ) )
		self.assertEqual( idd.channelNames(), [ 'r', 'g', 'b' ] )

		# test if all symbols are gone after the tests.
		creator = None
		idd = None
		gc.collect()
		IECore.RefCounted.collectGarbage()
		self.assertEqual( IECore.RefCounted.numWrappedInstances(), 0 )

	def testImagePool( self ) :

		img = IECore.Reader.create( "test/IECoreImage/data/tiff/bluegreen_noise.400x300.tif" )()

		idd = IECoreImage.DisplayDriver.create(
			"ImageDisplayDriver",
			img.displayWindow,
			img.dataWindow,
			list( img.channelNames() ),
			{
				"handle" : IECore.StringData( "myHandle" )
			}
		)

		red = img['R']
		green = img['G']
		blue = img['B']
		width = img.dataWindow.max.x - img.dataWindow.min.x + 1
		buf = IECore.FloatVectorData( width * 3 )
		for i in xrange( 0, img.dataWindow.max.y - img.dataWindow.min.y + 1 ):
			self.__prepareBuf( buf, width, i*width, red, green, blue )
			idd.imageData( IECore.Box2i( IECore.V2i( img.dataWindow.min.x, i + img.dataWindow.min.y ), IECore.V2i( img.dataWindow.max.x, i + img.dataWindow.min.y) ), buf )

		idd.imageClose()

		self.assertEqual( IECoreImage.ImageDisplayDriver.storedImage( "myHandle" ), idd.image() )
		self.assertEqual( IECoreImage.ImageDisplayDriver.removeStoredImage( "myHandle" ), idd.image() )
		self.assertEqual( IECoreImage.ImageDisplayDriver.storedImage( "myHandle" ), None )

	def testAcceptsRepeatedData( self ) :

		window = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 15 ) )

		dd = IECoreImage.ImageDisplayDriver( window, window, [ "Y" ], IECore.CompoundData() )
		self.assertEqual( dd.acceptsRepeatedData(), True )

		y = IECore.FloatVectorData( [ 1 ] * 16 * 16 )
		dd.imageData( window, y )

		y = IECore.FloatVectorData( [ 0.5 ] * 16 * 16 )
		dd.imageData( window, y )

		dd.imageClose()

		i = dd.image()
		self.assertEqual( i["Y"], y )

class ClientServerDisplayDriverTest(unittest.TestCase):

	def setUp( self ):

		gc.collect()
		IECore.RefCounted.collectGarbage()
		# make sure we don't have symbols from previous tests
		self.assertEqual( IECore.RefCounted.numWrappedInstances(), 0 )

		# this is necessary so python will allow threads created by the display driver server
		# to enter into python when those threads execute procedurals.
		IECore.initThreads()

		self.server = IECoreImage.DisplayDriverServer( 1559 )
		time.sleep(2)

	def __prepareBuf( self, buf, width, offset, red, green, blue ):
		for i in xrange( 0, width ):
			buf[3*i] = blue[i+offset]
			buf[3*i+1] = green[i+offset]
			buf[3*i+2] = red[i+offset]

	def testUsedPortException( self ):

		self.assertRaises( RuntimeError, lambda : IECoreImage.DisplayDriverServer( 1559 ) )

	def testTransfer( self ):

		img = IECore.Reader.create( "test/IECoreImage/data/tiff/bluegreen_noise.400x300.tif" )()
		self.assertEqual( img.keys(), [ 'B', 'G', 'R' ] )
		red = img['R']
		green = img['G']
		blue = img['B']
		width = img.dataWindow.max.x - img.dataWindow.min.x + 1

		params = IECore.CompoundData()
		params['displayHost'] = IECore.StringData('localhost')
		params['displayPort'] = IECore.StringData( '1559' )
		params["remoteDisplayType"] = IECore.StringData( "ImageDisplayDriver" )
		params["handle"] = IECore.StringData( "myHandle" )
		params["header:myMetadata"] = IECore.StringData( "Metadata!" )
		idd = IECoreImage.ClientDisplayDriver( img.displayWindow, img.dataWindow, list( img.channelNames() ), params )

		buf = IECore.FloatVectorData( width * 3 )
		for i in xrange( 0, img.dataWindow.max.y - img.dataWindow.min.y + 1 ):
			self.__prepareBuf( buf, width, i*width, red, green, blue )
			idd.imageData( IECore.Box2i( IECore.V2i( img.dataWindow.min.x, i + img.dataWindow.min.y ), IECore.V2i( img.dataWindow.max.x, i + img.dataWindow.min.y) ), buf )
		idd.imageClose()

		newImg = IECoreImage.ImageDisplayDriver.removeStoredImage( "myHandle" )
		params["clientPID"] = IECore.IntData( os.getpid() )

		# only data prefixed by 'header:' will come through as blindData/metadata
		self.assertEqual( newImg.blindData(), IECore.CompoundData({"myMetadata": IECore.StringData( "Metadata!" )}) )

		# remove blindData for comparison
		newImg.blindData().clear()
		img.blindData().clear()
		self.assertEqual( newImg, img )

	def testWrongSocketException( self ) :

		parameters = IECore.CompoundData( {
			"displayHost" : "localhost",
			"displayPort" : "1560", # wrong port
			"remoteDisplayType" : "ImageDisplayDriver",
		} )

		dw = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 255 ) )
		self.assertRaises( RuntimeError, IECoreImage.ClientDisplayDriver, dw, dw, [ "R", "G", "B" ], parameters )

		try :
			IECoreImage.ClientDisplayDriver( dw, dw, [ "R", "G", "B" ], parameters )
		except Exception, e :
			pass

		self.failUnless( "Could not connect to remote display driver server : Connection refused" in str( e ) )

	def testWrongHostException( self ) :

		parameters = IECore.CompoundData( {
			"displayHost" : "thisHostDoesNotExist",
			"displayPort" : "1559", # wrong port
			"remoteDisplayType" : "ImageDisplayDriver",
		} )

		dw = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 255 ) )
		self.assertRaises( RuntimeError, IECoreImage.ClientDisplayDriver, dw, dw, [ "R", "G", "B" ], parameters )

		try :
			IECoreImage.ClientDisplayDriver( dw, dw, [ "R", "G", "B" ], parameters )
		except Exception, e :
			pass

		self.failUnless( "Could not connect to remote display driver server : Host not found" in str( e ) )

	def testAcceptsRepeatedData( self ) :

		window = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 15 ) )

		dd = IECoreImage.ClientDisplayDriver(
			window, window,
			[ "Y" ],
			IECore.CompoundData( {
				"displayHost" : "localhost",
				"displayPort" : "1559",
				"remoteDisplayType" : "ImageDisplayDriver",
				"handle" : "myHandle"
			} )
		)

		self.assertEqual( dd.acceptsRepeatedData(), True )

		y = IECore.FloatVectorData( [ 1 ] * 16 * 16 )
		dd.imageData( window, y )

		y = IECore.FloatVectorData( [ 0.5 ] * 16 * 16 )
		dd.imageData( window, y )

		dd.imageClose()

		i = IECoreImage.ImageDisplayDriver.removeStoredImage( "myHandle" )
		self.assertEqual( i["Y"], y )

	def tearDown( self ):

		self.server = None

		# test if all symbols are gone after the tests.
		gc.collect()
		IECore.RefCounted.collectGarbage()
		self.assertEqual( IECore.RefCounted.numWrappedInstances(), 0 )

if __name__ == "__main__":
	unittest.main()
