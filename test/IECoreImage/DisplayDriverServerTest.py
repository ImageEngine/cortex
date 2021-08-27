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

import six
import unittest
import sys

import IECore
import IECoreImage

class DisplayDriverServerTest( unittest.TestCase ) :

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
		six.assertRaisesRegex( self, RuntimeError, ".*Unable to find a free port in the range.*", IECoreImage.DisplayDriverServer, 0 )

		# can't resuse ports
		errorMessage = ".*Unable to connect to port 45010.*Only one usage of each socket address.*is normally permitted" if (
			sys.platform == "win32"
		) else (
			".*Unable to connect to port 45010.*Address already in use.*"
		)
		six.assertRaisesRegex( self, RuntimeError, errorMessage, IECoreImage.DisplayDriverServer, 45010 )

		# bad range
		six.assertRaisesRegex( self, RuntimeError, ".*portNumber must fall.*", IECoreImage.DisplayDriverServer, 44999 )
		six.assertRaisesRegex( self, RuntimeError, ".*portNumber must fall.*", IECoreImage.DisplayDriverServer, 45011 )
		six.assertRaisesRegex( self, RuntimeError, ".*min port must be <= max port.*", IECoreImage.DisplayDriverServer.setPortRange, ( 45010, 45000 ) )

	def testPortRangeRegistry( self ) :

		IECoreImage.DisplayDriverServer.registerPortRange( "a", ( 45000, 45010 ) )
		self.assertEqual( IECoreImage.DisplayDriverServer.registeredPortRange( "a" ), ( 45000, 45010 ) )

		IECoreImage.DisplayDriverServer.registerPortRange( "b", ( 45011, 45020 ) )
		self.assertEqual( IECoreImage.DisplayDriverServer.registeredPortRange( "b" ), ( 45011, 45020 ) )
		six.assertRaisesRegex( self, RuntimeError, ".*is already registered.*", IECoreImage.DisplayDriverServer.registerPortRange, "b", ( 45021, 45030 ) )

		IECoreImage.DisplayDriverServer.deregisterPortRange( "b" )
		six.assertRaisesRegex( self, RuntimeError, ".*is not registered.*", IECoreImage.DisplayDriverServer.deregisterPortRange, "b" )
		six.assertRaisesRegex( self, RuntimeError, ".*is not registered.*", IECoreImage.DisplayDriverServer.registeredPortRange, "b" )

		IECoreImage.DisplayDriverServer.registerPortRange( "b", ( 45021, 45030 ) )
		self.assertEqual( IECoreImage.DisplayDriverServer.registeredPortRange( "b" ), ( 45021, 45030 ) )

		IECoreImage.DisplayDriverServer.setPortRange( IECoreImage.DisplayDriverServer.registeredPortRange( "a" ) )
		s1 = IECoreImage.DisplayDriverServer()
		self.assertEqual( s1.portNumber(), 45000 )

		IECoreImage.DisplayDriverServer.setPortRange( IECoreImage.DisplayDriverServer.registeredPortRange( "b" ) )
		s2 = IECoreImage.DisplayDriverServer()
		self.assertEqual( s2.portNumber(), 45021 )

if __name__ == "__main__":
	unittest.main()

