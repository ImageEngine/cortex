##########################################################################
#
#  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import os
import sys
import shutil
import unittest
import IECore

class TestBasicPreset( unittest.TestCase ) :

	def testCopy( self ) :

		testObj = IECore.Parameterised( "testParameterised1" )
		testObj.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", True ),
				IECore.FloatParameter( "b", "", 1.0 ),
			]
		)

		testObj2 = IECore.Parameterised( "testParameterised2" )
		testObj2.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", False ),
				IECore.FloatParameter( "c", "", 0.0	),
			]
		)

		p = IECore.BasicPreset( testObj, testObj.parameters() )

		self.failUnless( p.applicableTo( testObj, testObj.parameters() ) )
		self.failIf( p.applicableTo( testObj2, testObj2.parameters() ) )

		testObj.parameters()["a"].setTypedValue( False )
		testObj.parameters()["b"].setTypedValue( 0.0 )

		p( testObj, testObj.parameters() )

		self.assertEqual( testObj.parameters()["a"].getTypedValue(), True )
		self.assertEqual( testObj.parameters()["b"].getTypedValue(), 1.0 )

		p2 = IECore.BasicPreset( testObj, testObj.parameters(), parameters=( testObj.parameters()["a"], ) )

		self.failUnless( p2.applicableTo( testObj, testObj.parameters() ) )
		self.failUnless( p2.applicableTo( testObj2, testObj.parameters() ) )

		p2( testObj2, testObj2.parameters() )

		self.assertEqual( testObj2.parameters()["a"].getTypedValue(), True )
		self.assertEqual( testObj2.parameters()["c"].getTypedValue(), 0.0 )

	def testLoad( self ) :

		testObj = IECore.Parameterised( "testParameterised1" )
		testObj.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", True ),
				IECore.FloatParameter( "b", "", 1.0 ),
			]
		)

		testObj2 = IECore.Parameterised( "testParameterised1" )
		testObj2.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", False ),
				IECore.FloatParameter( "c", "", 0.0	),
			]
		)

		savePath = os.path.abspath( "%s/%s" % ( os.path.dirname( __file__ ), "data/basicPreset" ) )

		messageHandler = IECore.CapturingMessageHandler()
		with messageHandler :

			p = IECore.BasicPreset( "%s/%s" % ( savePath, "basicPresetLoadTest/basicPresetLoadTest-1.cob" ) )

		self.assertEqual( len( messageHandler.messages ), 0 )

		self.failUnless( p.applicableTo( testObj, testObj.parameters() ) )
		self.failIf( p.applicableTo( testObj2, testObj2.parameters() ) )

		testObj.parameters()["a"].setTypedValue( False )
		testObj.parameters()["b"].setTypedValue( 0.0 )

		p( testObj, testObj.parameters() )

		self.assertEqual( testObj.parameters()["a"].getTypedValue(), True )
		self.assertEqual( testObj.parameters()["b"].getTypedValue(), 1.0 )

	def testSave( self ) :

		testObj = IECore.Parameterised( "testParameterised1" )
		testObj.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", True ),
				IECore.FloatParameter( "b", "", 1.0 ),
			]
		)

		testObj2 = IECore.Parameterised( "testParameterised1" )
		testObj2.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", False ),
				IECore.FloatParameter( "c", "", 0.0	),
			]
		)

		savePath = os.path.abspath( "%s/%s" % ( os.path.dirname( __file__ ), "data/basicPreset" ) )

		preset = IECore.BasicPreset( testObj, testObj.parameters() )

		# Save for the classLoader and check its there, we test the 'loadability' later...
		preset.save( savePath, "basicPresetTest" )
		self.failUnless( os.path.isfile( "%s/basicPresetTest/basicPresetTest-1.cob" % savePath ) )
		self.failUnless( os.path.isfile( "%s/basicPresetTest/basicPresetTest-1.py" % savePath ) )

		# save without the classLoader and check its there
		preset.save( savePath, "basicPresetTest", classLoadable=False )
		self.failUnless( os.path.isfile( "%s/basicPresetTest.cob" % savePath ) )

		# reload
		p = IECore.BasicPreset( "%s/basicPresetTest.cob" % savePath )

		self.failUnless( p.applicableTo( testObj, testObj.parameters() ) )
		self.failIf( p.applicableTo( testObj2, testObj2.parameters() ) )

		testObj.parameters()["a"].setTypedValue( False )
		testObj.parameters()["b"].setTypedValue( 0.0 )

		p( testObj, testObj.parameters() )

		self.assertEqual( testObj.parameters()["a"].getTypedValue(), True )
		self.assertEqual( testObj.parameters()["b"].getTypedValue(), 1.0 )

		preset2 = IECore.BasicPreset( testObj, testObj.parameters(), parameters=( testObj.parameters()["a"], ) )
		preset2.save( savePath, "basicPresetTest2", classLoadable=False )

		#reload
		p2 = IECore.BasicPreset( "%s/basicPresetTest2.cob" % savePath )

		self.failUnless( p2.applicableTo( testObj, testObj.parameters() ) )
		self.failUnless( p2.applicableTo( testObj2, testObj.parameters() ) )

		p2( testObj2, testObj2.parameters() )

		self.assertEqual( testObj2.parameters()["a"].getTypedValue(), True )
		self.assertEqual( testObj2.parameters()["c"].getTypedValue(), 0.0 )

	def testClassLoader( self ) :

		testObj = IECore.Parameterised( "testParameterised1" )
		testObj.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", True ),
				IECore.FloatParameter( "b", "", 1.0 ),
			]
		)

		savePath = os.path.abspath( "%s/%s" % ( os.path.dirname( __file__ ), "data/basicPreset" ) )
		preset = IECore.BasicPreset( testObj, testObj.parameters() )
		preset.save( savePath, "basicPresetTestClassLoader" )

		# make sure that no messages are emitted during loading
		messageHandler = IECore.CapturingMessageHandler()
		with messageHandler :

			loader = IECore.ClassLoader( IECore.SearchPath( savePath, ":" ) )
			p = loader.load( "basicPresetTestClassLoader" )()

		self.assertEqual( len( messageHandler.messages ), 0 )

		self.failUnless( isinstance( p, IECore.BasicPreset ) )

		p.metadata()

	def testClasses( self ) :

		testObj = IECore.Parameterised( "testParameterised1" )
		testObj.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", True ),
				IECore.ClassParameter( "b", "", "IECORE_OP_PATHS", "maths/multiply", 2 ),
			]
		)

		testObj2 = IECore.Parameterised( "testParameterised2" )
		testObj2.parameters().addParameters(
			[
				IECore.ClassParameter( "c", "",	"IECORE_OP_PATHS" ),
			]
		)

		classes1 = testObj.parameters()["b"].getClass( True )
		classes2 = testObj2.parameters()["c"].getClass( True )
		self.assertNotEqual( classes1[1:], classes2[1:] )

		p = IECore.BasicPreset( testObj, testObj.parameters()["b"] )

		self.failUnless( p.applicableTo( testObj, testObj.parameters()["b"] ) )
		self.failIf( p.applicableTo( testObj, testObj.parameters() ) )
		self.failUnless( p.applicableTo( testObj2, testObj2.parameters()["c"] ) )

		p( testObj2, testObj2.parameters()["c"] )

		classes1 = testObj.parameters()["b"].getClass( True )
		classes2 = testObj2.parameters()["c"].getClass( True )

		self.assertEqual( classes1[1:], classes2[1:] )

	def testClassVectors( self ) :

		testObj = IECore.Parameterised( "testParameterised1" )
		testObj.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", True ),
				IECore.ClassVectorParameter( "b", "", "IECORE_OP_PATHS" ),
			]
		)

		testObj.parameters()["b"].setClasses(
			[
				( "mult", "maths/multiply", 2 ),
				( "coIO", "compoundObjectInOut", 1 ),
			]
		)

		testObj2 = IECore.Parameterised( "testParameterised2" )
		testObj2.parameters().addParameters(
			[
				IECore.ClassVectorParameter( "c", "", "IECORE_OP_PATHS" ),
			]
		)

		classes1 = [ c[1:] for c in testObj.parameters()["b"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["c"].getClasses( True ) ]

		self.assertNotEqual( classes1, classes2 )

		p = IECore.BasicPreset( testObj, testObj.parameters()["b"] )

		self.failUnless( p.applicableTo( testObj, testObj.parameters()["b"] ) )
		self.failIf( p.applicableTo( testObj, testObj.parameters() ) )
		self.failUnless( p.applicableTo( testObj2, testObj2.parameters()["c"] ) )

		p( testObj2, testObj2.parameters()["c"] )

		classes1 = [ c[1:] for c in testObj.parameters()["b"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["c"].getClasses( True ) ]

		self.assertEqual( classes1, classes2 )

	def testCompoundVectorParameter( self ) :

		p = IECore.Parameterised( "test" )
		p.parameters().addParameters(
			[
				IECore.BoolParameter( "a", "", False ),
				IECore.CompoundVectorParameter(
					"c",
					"",
					members = [
						IECore.StringVectorParameter( "s", "", IECore.StringVectorData() ),
						IECore.BoolVectorParameter( "b", "", IECore.BoolVectorData() ),
					]
				)
			]
		)

		p["c"]["s"].setValue( IECore.StringVectorData( [ "1", "2", "3" ] ) )
		p["c"]["b"].setValue( IECore.BoolVectorData( [ True, False, True ] ) )

		v = p.parameters().getValue().copy()

		preset = IECore.BasicPreset( p, p.parameters() )

		self.failUnless( preset.applicableTo( p, p.parameters() ) )

		p.parameters().setValue( p.parameters().defaultValue )
		self.assertNotEqual( p.parameters().getValue(), v )

		preset( p, p.parameters() )

		self.assertEqual( p.parameters().getValue(), v )

	def tearDown( self ) :

		savePath = os.path.abspath( "%s/%s" % ( os.path.dirname( __file__ ), "data/basicPreset" ) )
		paths = (
			savePath+"/basicPresetTest",
			savePath+"/basicPresetTest.cob",
			savePath+"/basicPresetTest2.cob",
			savePath+"/basicPresetTestClassLoader",
		)

		for p in paths :
			if os.path.isdir( p ) :
				shutil.rmtree( p )
			elif os.path.isfile( p ) :
				os.remove( p )

if __name__ == "__main__":
	unittest.main()
