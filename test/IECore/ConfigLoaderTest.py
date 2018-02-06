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

import os
import unittest

import IECore

class ConfigLoaderTest( unittest.TestCase ) :

	def testLoadConfig( self ) :

		contextDict = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config/orderOne" ),
			contextDict,

		)

		self.assertEqual( contextDict["a"], 1 )

	def testOrder( self ) :

		contextDict = {}
		IECore.loadConfig(

			IECore.SearchPath( [
				os.path.dirname( __file__ ) + "/config/orderTwo",
				os.path.dirname( __file__ ) + "/config/orderOne",
			] ),

			contextDict,

		)

		self.assertEqual( contextDict["a"], 2 )

	def testIgnoreExceptions( self ) :

		contextDict = {}

		m = IECore.CapturingMessageHandler()
		with m :

			IECore.loadConfig(

				IECore.SearchPath( [
					os.path.dirname( __file__ ) + "/config/orderOne",
					os.path.dirname( __file__ ) + "/config/exceptions",
				] ),

				contextDict,
				raiseExceptions = False

			)

		errors = [ msg for msg in m.messages if msg.level == IECore.Msg.Level.Error ]
		self.assertEqual( len( errors ), 1 )
		self.assertEqual( errors[0].level, IECore.Msg.Level.Error )
		self.failUnless( "I am a very naughty boy" in errors[0].message )

		self.assertEqual( contextDict["a"], 1 )

	def testThrowExceptions( self ) :

		contextDict = {}
		self.assertRaises(

			RuntimeError,
			IECore.loadConfig,
			IECore.SearchPath( [
				os.path.dirname( __file__ ) + "/config/orderOne",
				os.path.dirname( __file__ ) + "/config/exceptions",
			] ),
			contextDict,
			raiseExceptions = True

		)

		self.failIf( "a" in contextDict )

	def testScope( self ) :

		contextDict = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config/scope" ),
			contextDict,
			raiseExceptions = True

		)

		contextDict["functionToCallLater"]()

	def testIgnoreFiles( self ) :

		contextDict = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config/ignoreFiles" ),
			contextDict,

		)

		self.failIf( "tildeConfigRan" in contextDict )
		self.failIf( "notDotPyRan" in contextDict )

		self.assertEqual( contextDict["a"], 1000 )

	def testOrderWithinDirectory( self ) :

		os.utime( os.path.dirname( __file__ ) + "/config/orderDir/a.py", None )

		contextDict = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config/orderDir" ),
			contextDict,

		)

		self.assertEqual( contextDict["lastRun"], "b" )

	def testSubdirectory( self ) :

		contextDict = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config" ),
			contextDict,
			subdirectory = "orderDir",

		)

		self.assertTrue( "lastRun" in contextDict )
		self.assertFalse( "a" in contextDict )

	def testSearchPathAsEnvVar( self ) :

		os.environ["IECORE_CONFIGLOADERTEST_PATHS"] = "%s:%s" % (
			os.path.dirname( __file__ ) + "/config/orderOne",
			os.path.dirname( __file__ ) + "/config/orderTwo"
		)

		contextDict = {}
		IECore.loadConfig(

			"IECORE_CONFIGLOADERTEST_PATHS",
			contextDict,

		)

		self.assertEqual( contextDict["a"], 1 )

		os.environ["IECORE_CONFIGLOADERTEST_PATHS"] = "%s:%s" % (
			os.path.dirname( __file__ ) + "/config/orderTwo",
			os.path.dirname( __file__ ) + "/config/orderOne"
		)

		contextDict = {}
		IECore.loadConfig(

			"IECORE_CONFIGLOADERTEST_PATHS",
			contextDict,

		)

		self.assertEqual( contextDict["a"], 2 )

	def testFile( self ) :

		contextDict = {}
		path = os.path.dirname( __file__ ) + "/config/getFile"
		IECore.loadConfig(

			IECore.SearchPath( path ),
			contextDict,

		)

		expectedFile = os.path.abspath( os.path.join( path, "config.py" ) )
		self.assertEqual( contextDict["myFile"], expectedFile )

	def testDuplicatePathsIgnored( self ) :

		contextDict = {}
		IECore.loadConfig(

			IECore.SearchPath( [
				os.path.dirname( __file__ ) + "/config/orderOne",
				os.path.dirname( __file__ ) + "/config/orderTwo",
				os.path.dirname( __file__ ) + "/config/orderOne",
			] ),

			contextDict,

		)

		self.assertEqual( contextDict["a"], 2 )

if __name__ == "__main__":
	unittest.main()

