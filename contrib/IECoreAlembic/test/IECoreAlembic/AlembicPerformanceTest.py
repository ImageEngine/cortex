##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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
import shutil
import unittest
import imath
import tempfile

import IECore
import IECoreScene
import IECoreAlembic


class Timer( object ) :

	def __init__( self, name) :
		self.name = name
		self.timer = IECore.Timer( False, IECore.Timer.WallClock )

	def __enter__( self ) :
		self.timer.start()
		return self

	def __exit__( self, type, value, traceback ) :
		t = self.timer.stop()
		print( "{0} , time: {1}s".format(self.name, t) )


class AlembicPerformanceTest( unittest.TestCase ) :

	def setUp(self):

		self.filesCreated = []

	def tearDown(self):

		for fileName in self.filesCreated:
			if os.path.exists( fileName ):
				os.unlink( fileName )

	def writeCacheFile( self, suffix = ".abc", withAttributes = False ) :

		with tempfile.NamedTemporaryFile(suffix = suffix) as tf:
			fileName = tf.name

		with Timer("write file  with attributes: '{0}', filename: '{1}'".format(withAttributes, fileName)) as t:
			root = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Write )

			# pow( 10, 4 ) = 100,000 locations
			maxDepth = 5
			numChildren = 10

			def createChildren( location, depth, withAttributes = False ) :

				if depth >= maxDepth :

					# scene interface forbids writing attributes on the root
					if withAttributes and depth != 0:
						location.writeAttribute( "testFloat", IECore.FloatData( 1.0 ), 0.0 )
						location.writeAttribute( "testInt", IECore.IntData( 2 ), 0.0 )
						location.writeAttribute( "testString", IECore.StringData( "don" ), 0.0 )
						location.writeAttribute( "testV3f", IECore.V3fData( imath.V3f(1,2,3), IECore.GeometricData.Interpretation.Vector ), 0.0 )

					return

				for i in range( numChildren ) :
					c = location.createChild( str( i ) )
					createChildren( c, depth + 1, withAttributes = withAttributes )

			createChildren( root, 0, withAttributes = withAttributes )

		self.filesCreated.append( fileName )
		return fileName


	@unittest.skipUnless( os.environ.get("IE_PERFORMANCE_TEST", False), "'IE_PERFORMANCE_TEST' env var not set" )
	def testCompareReadWithAttributes( self ) :

		extensions = ['.abc', '.scc']

		for extension in extensions:
			print( "== {0} ==".format( extension ) )

			cacheFileName = self.writeCacheFile( suffix = extension )

			root = IECoreScene.SceneInterface.create( cacheFileName, IECore.IndexedIO.OpenMode.Read )

			def readAll( location ) :

				# read all attributes
				attributeNames = location.attributeNames()

				for attributeName in attributeNames:
					location.readAttribute( attributeName, 0.0 )

				numAttributes = len( attributeNames )

				# recurse into child locations
				for childName in location.childNames() :
					numAttributes += readAll( location.child( childName ) )

				return numAttributes

			times = []
			for testRun in range( 10 ):
				t = IECore.Timer( True , IECore.Timer.WallClock )
				results = IECoreScene.SceneAlgo.parallelReadAll(root, 0, 0, 24.0, IECoreScene.SceneAlgo.Transforms | IECoreScene.SceneAlgo.Attributes )
				times.append( t.stop() )
				self.assertEqual( results["attributes"], 0 )

			print( times )

			cacheFileName = self.writeCacheFile( suffix = extension, withAttributes = True )

			root = IECoreScene.SceneInterface.create( cacheFileName, IECore.IndexedIO.OpenMode.Read )

			times = []
			for testRun in range( 10 ):
				t = IECore.Timer( True , IECore.Timer.WallClock )
				results = IECoreScene.SceneAlgo.parallelReadAll(root, 0, 0, 24.0, IECoreScene.SceneAlgo.Transforms | IECoreScene.SceneAlgo.Attributes )
				times.append( t.stop() )
				self.assertEqual( results["attributes"] , 100000 * 4 )

			print( times )


