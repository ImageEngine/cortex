##########################################################################
#
#  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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
import sys
import os
import imath

import IECore
import IECoreImage

class DisplayDriverServerTest( unittest.TestCase ) :

	def __prepareBuf( self, buf, width, offset, red, green, blue ):
		for i in range( 0, width ):
			buf[3*i] = blue[i+offset]
			buf[3*i+1] = green[i+offset]
			buf[3*i+2] = red[i+offset]

	def testPortNumber( self ) :

		s1 = IECoreImage.DisplayDriverServer( 1559 )
		self.assertEqual( s1.portNumber(), 1559 )

		self.assertRaises( RuntimeError, IECoreImage.DisplayDriverServer, 1559 )

		s2 = IECoreImage.DisplayDriverServer( 0 )
		self.assertNotEqual( s2.portNumber(), 0 )
		self.assertNotEqual( s2.portNumber(), s1.portNumber() )

		s3 = IECoreImage.DisplayDriverServer( 0 )
		self.assertNotEqual( s3.portNumber(), 0 )
		self.assertNotEqual( s3.portNumber(), s2.portNumber() )

		s4 = IECoreImage.DisplayDriverServer()
		self.assertNotEqual( s4.portNumber(), 0 )
		self.assertNotEqual( s4.portNumber(), s3.portNumber() )

	def testPortRange( self ) :

		IECoreImage.DisplayDriverServer.setPortRange( ( 45000, 45010 ) )
		self.assertEqual( IECoreImage.DisplayDriverServer.getPortRange(), ( 45000, 45010 ) )

		s1 = IECoreImage.DisplayDriverServer()
		self.assertEqual( s1.portNumber(), 45000 )

		s2 = IECoreImage.DisplayDriverServer()
		self.assertEqual( s2.portNumber(), 45001 )

		# deleting servers should free the ports for reuse
		del s1, s2

		servers = []
		for p in range( 45000, 45011 ) :
			servers.append( IECoreImage.DisplayDriverServer( 0 ) )
			self.assertEqual( servers[-1].portNumber(), p )

		# make one more than the range allows
		self.assertRaisesRegex( RuntimeError, ".*Unable to find a free port in the range.*", IECoreImage.DisplayDriverServer, 0 )

		# can't resuse ports
		errorMessage = ".*Unable to connect to port 45010.*Only one usage of each socket address.*is normally permitted" if (
			sys.platform == "win32"
		) else (
			".*Unable to connect to port 45010.*Address already in use.*"
		)
		self.assertRaisesRegex( RuntimeError, errorMessage, IECoreImage.DisplayDriverServer, 45010 )

		# bad range
		self.assertRaisesRegex( RuntimeError, ".*portNumber must fall.*", IECoreImage.DisplayDriverServer, 44999 )
		self.assertRaisesRegex( RuntimeError, ".*portNumber must fall.*", IECoreImage.DisplayDriverServer, 45011 )
		self.assertRaisesRegex( RuntimeError, ".*min port must be <= max port.*", IECoreImage.DisplayDriverServer.setPortRange, ( 45010, 45000 ) )

	def testPortRangeRegistry( self ) :

		IECoreImage.DisplayDriverServer.registerPortRange( "a", ( 45000, 45010 ) )
		self.assertEqual( IECoreImage.DisplayDriverServer.registeredPortRange( "a" ), ( 45000, 45010 ) )

		IECoreImage.DisplayDriverServer.registerPortRange( "b", ( 45011, 45020 ) )
		self.assertEqual( IECoreImage.DisplayDriverServer.registeredPortRange( "b" ), ( 45011, 45020 ) )
		self.assertRaisesRegex( RuntimeError, ".*is already registered.*", IECoreImage.DisplayDriverServer.registerPortRange, "b", ( 45021, 45030 ) )

		IECoreImage.DisplayDriverServer.deregisterPortRange( "b" )
		self.assertRaisesRegex( RuntimeError, ".*is not registered.*", IECoreImage.DisplayDriverServer.deregisterPortRange, "b" )
		self.assertRaisesRegex( RuntimeError, ".*is not registered.*", IECoreImage.DisplayDriverServer.registeredPortRange, "b" )

		IECoreImage.DisplayDriverServer.registerPortRange( "b", ( 45021, 45030 ) )
		self.assertEqual( IECoreImage.DisplayDriverServer.registeredPortRange( "b" ), ( 45021, 45030 ) )

		IECoreImage.DisplayDriverServer.setPortRange( IECoreImage.DisplayDriverServer.registeredPortRange( "a" ) )
		s1 = IECoreImage.DisplayDriverServer()
		self.assertEqual( s1.portNumber(), 45000 )

		IECoreImage.DisplayDriverServer.setPortRange( IECoreImage.DisplayDriverServer.registeredPortRange( "b" ) )
		s2 = IECoreImage.DisplayDriverServer()
		self.assertEqual( s2.portNumber(), 45021 )

	def testMergeMap( self ) :
		server = IECoreImage.DisplayDriverServer( 45001 )

		img = IECore.Reader.create( os.path.join( "test", "IECoreImage", "data", "tiff", "bluegreen_noise.400x300.tif" ) )()
		self.assertEqual( img.keys(), [ 'B', 'G', 'R' ] )
		red = img['R']
		green = img['G']
		blue = img['B']
		width = img.dataWindow.max().x - img.dataWindow.min().x + 1

		params = IECore.CompoundData()
		params['displayHost'] = IECore.StringData('localhost')
		params['displayPort'] = IECore.StringData( '45001' )
		params['displayDriverServer:mergeId'] = IECore.IntData( 42 )
		params['displayDriverServer:mergeClients'] = IECore.IntData( 2 )
		params["remoteDisplayType"] = IECore.StringData( "ImageDisplayDriver" )
		params["handle"] = IECore.StringData( "myHandle1" )

		idd1 = IECoreImage.ClientDisplayDriver( img.displayWindow, img.dataWindow, list( img.channelNames() ), params )

		params["handle"] = IECore.StringData( "myHandle2" )
		idd2 = IECoreImage.ClientDisplayDriver( img.displayWindow, img.dataWindow, list( img.channelNames() ), params )

		params['displayDriverServer:mergeId'] = IECore.IntData( 666 )
		params['displayDriverServer:mergeClients'] = IECore.IntData( 1 )
		params["handle"] = IECore.StringData( "myHandle3" )
		idd3 = IECoreImage.ClientDisplayDriver( img.displayWindow, img.dataWindow, list( img.channelNames() ), params )

		buf = IECore.FloatVectorData( width * 3 )
		for i in range( 0, img.dataWindow.max().y - img.dataWindow.min().y + 1 ):
			self.__prepareBuf( buf, width, i*width, red, green, blue )
			idd1.imageData( imath.Box2i( imath.V2i( img.dataWindow.min().x, i + img.dataWindow.min().y ), imath.V2i( img.dataWindow.max().x, i + img.dataWindow.min().y) ), buf )
			idd2.imageData( imath.Box2i( imath.V2i( img.dataWindow.min().x, i + img.dataWindow.min().y ), imath.V2i( img.dataWindow.max().x, i + img.dataWindow.min().y) ), buf )
			idd3.imageData( imath.Box2i( imath.V2i( img.dataWindow.min().x, i + img.dataWindow.min().y ), imath.V2i( img.dataWindow.max().x, i + img.dataWindow.min().y) ), buf )

		idd1.imageClose()
		idd2.imageClose()
		idd3.imageClose()

		newImg = IECoreImage.ImageDisplayDriver.removeStoredImage( "myHandle1" )
		newImg2 = IECoreImage.ImageDisplayDriver.removeStoredImage( "myHandle2" )
		newImg3 = IECoreImage.ImageDisplayDriver.removeStoredImage( "myHandle3" )

		# merge drivers share the same display driver - so second image should be none,
		# as there is no image drivere associated with it.
		self.assertIsNone( newImg2 )

		# remove blindData for comparison
		newImg.blindData().clear()
		img.blindData().clear()
		self.assertEqual( newImg, img )

		newImg3.blindData().clear()
		self.assertEqual( newImg3, img )

		server = None

if __name__ == "__main__":
	unittest.main()

