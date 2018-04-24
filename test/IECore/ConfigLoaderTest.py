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

		config = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config/orderOne" ),
			contextDict = { "config" : config },

		)

		self.assertEqual( config["a"], 1 )

	def testOrder( self ) :

		config = {}
		IECore.loadConfig(

			IECore.SearchPath( [
				os.path.dirname( __file__ ) + "/config/orderTwo",
				os.path.dirname( __file__ ) + "/config/orderOne",
			] ),

			contextDict = { "config" : config },

		)

		self.assertEqual( config["a"], 2 )

	def testIgnoreExceptions( self ) :

		config = {}

		m = IECore.CapturingMessageHandler()
		with m :

			IECore.loadConfig(

				IECore.SearchPath( [
					os.path.dirname( __file__ ) + "/config/orderOne",
					os.path.dirname( __file__ ) + "/config/exceptions",
				] ),

				contextDict = { "config" : config },
				raiseExceptions = False

			)

		errors = [ msg for msg in m.messages if msg.level == IECore.Msg.Level.Error ]
		self.assertEqual( len( errors ), 1 )
		self.assertEqual( errors[0].level, IECore.Msg.Level.Error )
		self.failUnless( "I am a very naughty boy" in errors[0].message )

		self.assertEqual( config["a"], 1 )

	def testThrowExceptions( self ) :

		config = {}
		self.assertRaises(

			RuntimeError,
			IECore.loadConfig,
			IECore.SearchPath( [
				os.path.dirname( __file__ ) + "/config/orderOne",
				os.path.dirname( __file__ ) + "/config/exceptions",
			] ),
			contextDict = { "config" : config },
			raiseExceptions = True

		)

		self.failIf( "a" in config )

	def testScope( self ) :

		config = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config/scope" ),
			contextDict = { "config" : config },
			raiseExceptions = True

		)

		config["functionToCallLater"]()

	def testIgnoreFiles( self ) :

		config = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config/ignoreFiles" ),
			contextDict = { "config" : config },

		)

		self.failIf( "tildeConfigRan" in config )
		self.failIf( "notDotPyRan" in config )

		self.assertEqual( config["a"], 1000 )

	def testOrderWithinDirectory( self ) :

		os.utime( os.path.dirname( __file__ ) + "/config/orderDir/a.py", None )

		config = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config/orderDir" ),
			contextDict = { "config" : config },

		)

		self.assertEqual( config["lastRun"], "b" )

	def testSubdirectory( self ) :

		config = {}
		IECore.loadConfig(

			IECore.SearchPath( os.path.dirname( __file__ ) + "/config" ),
			contextDict = { "config" : config },
			subdirectory = "orderDir",

		)

		self.assertTrue( "lastRun" in config )
		self.assertFalse( "a" in config )

	def testSearchPathAsEnvVar( self ) :

		os.environ["IECORE_CONFIGLOADERTEST_PATHS"] = "%s:%s" % (
			os.path.dirname( __file__ ) + "/config/orderOne",
			os.path.dirname( __file__ ) + "/config/orderTwo"
		)

		config = {}
		IECore.loadConfig(

			"IECORE_CONFIGLOADERTEST_PATHS",
			contextDict = { "config" : config },

		)

		self.assertEqual( config["a"], 1 )

		os.environ["IECORE_CONFIGLOADERTEST_PATHS"] = "%s:%s" % (
			os.path.dirname( __file__ ) + "/config/orderTwo",
			os.path.dirname( __file__ ) + "/config/orderOne"
		)

		config = {}
		IECore.loadConfig(

			"IECORE_CONFIGLOADERTEST_PATHS",
			contextDict = { "config" : config },

		)

		self.assertEqual( config["a"], 2 )

	def testFile( self ) :

		config = {}
		path = os.path.dirname( __file__ ) + "/config/getFile"
		IECore.loadConfig(

			IECore.SearchPath( path ),
			contextDict = { "config" : config },

		)

		expectedFile = os.path.abspath( os.path.join( path, "config.py" ) )
		self.assertEqual( config["myFile"], expectedFile )

	def testDuplicatePathsIgnored( self ) :

		config = {}
		IECore.loadConfig(

			IECore.SearchPath( [
				os.path.dirname( __file__ ) + "/config/orderOne",
				os.path.dirname( __file__ ) + "/config/orderTwo",
				os.path.dirname( __file__ ) + "/config/orderOne",
			] ),

			contextDict = { "config" : config },

		)

		self.assertEqual( config["a"], 2 )

	def testConfigIsolation( self ) :

		IECore.loadConfig(

			IECore.SearchPath( [
				os.path.dirname( __file__ ) + "/config/isolation",
			] ),

			raiseExceptions = True

		)

		IECore.loadConfig(

			IECore.SearchPath( [
				os.path.dirname( __file__ ) + "/config/isolation",
			] ),
			{},

			raiseExceptions = True

		)

if __name__ == "__main__":
	unittest.main()

