##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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


"""Unit test for InterpolatedCache binding"""
import os, glob, random
import unittest
import math
import threading

from IECore import *


class TestInterpolatedCache(unittest.TestCase):

	pathTemplate = "./test/cache_####.fio"

	def mark( self ) :

		self.__createCacheFile( 250, 0 )
		self.__createCacheFile( 333, 1 )
		self.__createCacheFile( 416, 2 )
		self.__createCacheFile( 500, 3 )
		self.__createCacheFile( 583, 4 )
		self.__createCacheFile( 666, 5 )
		self.__createCacheFile( 750, 6 )

	def __createV3f( self, value ):
		dataWritten = V3fVectorData()
		for i in range(0, 3):
			dataWritten.append( V3f( value, value, value ) )
		return dataWritten

	def __createCacheFile( self, frame, value ):

		fs = FileSequence( self.pathTemplate, EmptyFrameList() )

		cache = AttributeCache( fs.fileNameForFrame( frame ), IndexedIO.OpenMode.Write)
	
		dataWritten = self.__createV3f( value )
		cache.write( "obj1", "v3fVec", dataWritten)
		cache.write( "obj2", "i", IntData( value ) )
		cache.write( "obj2", "d", DoubleData( value ) )
		cache.writeHeader( "testCache", self.__headerCompound( frame ) )

	def __headerCompound( self, frame ):
		return CompoundObject(
			{
				"a": IntData( 1 ),
				"b": StringData( "HEllo!" ),
				"frame": FloatData( frame ),
			}
		)

	def __time( self, frame ):
		return int( frame * 6000. / 24. )

	def __createCache( self ):
		"""Create some cache files do play with..."""

		self.__createCacheFile( self.__time( 0 ), 0 )
		self.__createCacheFile( self.__time( 1 ), 1 )
		self.__createCacheFile( self.__time( 2 ), 2 )
		self.__createCacheFile( self.__time( 3 ), 4 )
		self.__createCacheFile( self.__time( 4 ), 8 )
		self.__createCacheFile( self.__time( 5 ), 16 )

	def testConstructors(self):
		"""Test InterpolatedCache constructors"""
		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.None )
		self.assertEqual( cache.getInterpolation(), InterpolatedCache.Interpolation.None )
		self.assertEqual( cache.getPathTemplate(), self.pathTemplate )

		os = OversamplesCalculator( frameRate = 20, samplesPerFrame = 1 )
		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.Linear, oversamplesCalculator = os )
		self.assertEqual( cache.getInterpolation(), InterpolatedCache.Interpolation.Linear )

		os = OversamplesCalculator( frameRate = 24, samplesPerFrame = 1 )
		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.Cubic, oversamplesCalculator = os )

	def testHeaders( self ):
		"""Test InterpolatedCache headers"""

		self.__createCache()

		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assert_( "testCache" in cache.headers( 1.5 ) )
		h = cache.readHeader( 1.5, "testCache" )
		self.assertEqual( h, self.__headerCompound( ( self.__time( 1 ) + self.__time( 2 ) ) / 2 ) )

	def testAttributes(self):
		"""Test InterpolatedCache attributes"""

		self.__createCache()

		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assertEqual( len( cache.attributes( 1.2, "obj2" )), 2 )
		self.assertEqual( len( cache.attributes( 1.2, "obj2", "i.*" )), 1 )
		self.assertEqual( len( cache.attributes( 1.2, "obj1" )), 1 )

	def testContains(self):
		"""Test InterpolatedCache contains"""

		self.__createCache()

		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assert_( cache.contains( 1.2, "obj1" ) )
		self.assert_( cache.contains( 1.2, "obj2", "i" ) )
		self.failIf( cache.contains( 1.2, "obj3" ) )
		self.failIf( cache.contains( 1.2, "obj2", "not_in_cache" ) )

	def testReading(self):
		"""Test InterpolatedCache read"""

		self.__createCache()

		cache = InterpolatedCache( self.pathTemplate, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assertEqual( cache.read( 1.5, "obj2", "i" ), IntData( 1 ) )
		self.assertEqual( cache.read( 1.5, "obj2", "d" ), DoubleData( 1.5 ) )
		self.assertEqual( cache.read( 1.5, "obj1" ), CompoundObject( { "v3fVec": self.__createV3f( 1.5 ) } ) )

	def testOversampledReading( self ) :

		self.mark()

		os = OversamplesCalculator( frameRate = 24, samplesPerFrame = 3 )
		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.Linear, oversamplesCalculator = os )
		self.assertAlmostEqual( cache.read( 2, "obj2", "d" ).value, 3, 4 )


	def testChangingParameters( self ):
		"""Test InterpolatedCache reading and changing frame, interpolation, ... """

		self.__createCache()

		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.None )
		self.assertEqual( cache.read( 0, "obj2", "d" ), DoubleData( 0 ) )
		self.assertEqual( cache.read( 0.8, "obj2", "d" ), DoubleData( 0 ) )
		self.assertEqual( cache.read( 1, "obj2", "d" ), DoubleData( 1 ) )
		cache.setInterpolation( InterpolatedCache.Interpolation.Linear )
		self.assertEqual( cache.read( 0.5, "obj2", "d" ), DoubleData( 0.5 ) )
		self.assertAlmostEqual( cache.read( 2.25, "obj2", "d" ).value, 2.5, 3 )
		cache.setInterpolation( InterpolatedCache.Interpolation.Cubic )
		self.assertAlmostEqual( cache.read( 2.25, "obj2", "d" ).value, 2.4531, 3 )
		self.assertEqual( cache.read( 5, "obj2", "d" ), DoubleData( 16 ) )
		cache.setInterpolation( InterpolatedCache.Interpolation.Linear )
		self.assertEqual( cache.read( 5, "obj2", "d" ), DoubleData( 16 ) )
		cache.setInterpolation( InterpolatedCache.Interpolation.None )
		self.assertEqual( cache.read( 5, "obj2", "d" ), DoubleData( 16 ) )

	def testOldTransformationMatrixData( self ):

		cache = InterpolatedCache( "test/IECore/data/attributeCaches/transform.old.####.fio", interpolation = InterpolatedCache.Interpolation.Linear )
		self.assertEqual( cache.read( 4, ".parent", "transformCache.transform" ).value.rotate, Eulerd() )
		self.assertAlmostEqual( (cache.read( 4, ".pSphere1", "transformCache.transform" ).value.rotate - Eulerd( 0.244525, 0, 0 )).length(), 0, 2 )
		self.assertEqual( cache.read( 4.5, ".parent", "transformCache.transform" ).value.rotate, Eulerd() )
		self.assertAlmostEqual( (cache.read( 4.5, ".pSphere1", "transformCache.transform" ).value.rotate - Eulerd(0.283422, 0, 0)).length(), 0, 2 )

	def testThreading( self ) :
	
		def check( cache, frame ) :
		
			objects = cache.objects( frame )
			self.assertEqual( len( objects ), 2 )
			self.failUnless( "obj1" in objects )
			self.failUnless( "obj2" in objects )
			
			self.assertEqual( cache.attributes( frame, "obj1" ), [ "v3fVec" ] )
			
			attributes = cache.attributes( frame, "obj2" )
			self.assertEqual( len( attributes ), 2 )
			self.failUnless( "i" in attributes )
			self.failUnless( "d" in attributes )
			
			v = cache.read( frame, "obj1", "v3fVec" )
			self.failUnless( v[0].equalWithAbsError( V3f( frame ), 0.001 ) )
			self.failUnless( v[1].equalWithAbsError( V3f( frame ), 0.001 ) )
			self.failUnless( v[2].equalWithAbsError( V3f( frame ), 0.001 ) )
			self.assertEqual( cache.read( frame, "obj2", "i" ), IntData( int( frame ) ) )
			self.assertAlmostEqual( cache.read( frame, "obj2", "d" ).value, frame, 6 )
	
			cache.read( frame, "obj1" )
	
			self.failUnless( "testCache" in cache.headers( frame ) )
			
			h = cache.readHeader( frame, "testCache" )
			self.assertEqual( cache.readHeader( frame )["testCache"], h )
			
			self.failUnless( cache.contains( frame, "obj1" ) )
			self.failUnless( cache.contains( frame, "obj2" ) )
			self.failIf( cache.contains( frame, "ofsdfsdbj2" ) )

		self.__createCache()
		
		cache = InterpolatedCache(self.pathTemplate, interpolation = InterpolatedCache.Interpolation.Linear )
					
		threads = []
		for i in range( 0, 1000 ) :
			t = threading.Thread( target=check, args = ( cache, random.uniform( 0, 2 ) ) )
			t.start()
			threads.append( t )

		for t in threads :
			t.join()
	
	def testDefaultConstructor( self ) :
	
		self.__createCache()

		# If we provide a default constructor then it really oughn't to throw an exception
		cache = InterpolatedCache()
		# But if we try to do something with it before setting the template then it should complain
		self.assertRaises( RuntimeError, cache.readHeader, 0 )
		# And it should work as usual if we subsequently set the template
		cache.setPathTemplate( self.pathTemplate )
		cache.readHeader( 0 )
		self.assertEqual( cache.read( 0, "obj2", "d" ), DoubleData( 0 ) )

	def testReadMissing( self ) :

		# check that trying to read missing caches fails
		
		cache = InterpolatedCache( "iDontExist.######.fio" )
		self.assertRaises( RuntimeError, cache.read, 1.5, "a", "b" )
	
		# check that trying to read missing objects or attributes fails
	
		self.__createCache()
	
		cache = InterpolatedCache( self.pathTemplate )
		cache.read( 1.5, "obj2", "d" )
		
		self.assertRaises( RuntimeError, cache.read, 1.5, "iDontExist", "a" )
		self.assertRaises( RuntimeError, cache.read, 1.5, "obj2", "iDontExist" )

	def testMaxOpenFiles( self ) :

		cache = InterpolatedCache( self.pathTemplate, maxOpenFiles=20 )
		self.assertEqual( cache.getMaxOpenFiles(), 20 )
		
		cache.setMaxOpenFiles( 10 )
		self.assertEqual( cache.getMaxOpenFiles(), 10 )
		
	def tearDown(self):

		# cleanup
		allFiles = glob.glob( self.pathTemplate.replace( "####", "*" ) )
		for f in allFiles:
			os.remove( f )

if __name__ == "__main__":
	unittest.main()

