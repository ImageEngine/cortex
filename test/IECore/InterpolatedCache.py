##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
import os, glob
import unittest
import math

from IECore import *


class TestInterpolatedCache(unittest.TestCase):

	pathTemplate = "./test/cache_%003d.fio"

	def __createV3f( self, seed ):
		dataWritten = V3fVectorData()
		for i in range(0, 3):
			dataWritten.append( V3f( seed, seed, seed ) )
		return dataWritten

	def __createCache( self, frame, seed ):

		cache = AttributeCache( self.pathTemplate % frame, IndexedIOOpenMode.Write)

		dataWritten = self.__createV3f( seed )
		cache.write( "obj1", "v3fVec", dataWritten)
		cache.write( "obj2", "i", IntData( seed ) )
		cache.write( "obj2", "d", DoubleData( seed ) )
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

	def setUp( self ):
		"""Create some cache files do play with..."""
		self.__createCache( self.__time( 0 ), 0 )
		self.__createCache( self.__time( 1 ), 1 )
		self.__createCache( self.__time( 2 ), 2 )
		self.__createCache( self.__time( 3 ), 4 )
		self.__createCache( self.__time( 4 ), 8 )
		self.__createCache( self.__time( 5 ), 16 )

	def testConstructors(self):
		"""Test InterpolatedCache constructors"""
		cache = InterpolatedCache(self.pathTemplate, frame = 0, interpolation = InterpolatedCache.Interpolation.None )
		self.assertEqual( cache.getFrameRate(), 24. )
		self.assertEqual( cache.getInterpolation(), InterpolatedCache.Interpolation.None )
		self.assertEqual( cache.getPathTemplate(), self.pathTemplate )
		self.assertEqual( cache.getFrame(), 0 )
		self.assertEqual( cache.getOversamples(), 1 )

		cache = InterpolatedCache(self.pathTemplate, frame = 0, interpolation = InterpolatedCache.Interpolation.Linear, oversamples = 1, frameRate = 20.0 ) 
		self.assertEqual( cache.getFrameRate(), 20. )
		self.assertEqual( cache.getInterpolation(), InterpolatedCache.Interpolation.Linear )

		cache = InterpolatedCache(self.pathTemplate, frame = 10, interpolation = InterpolatedCache.Interpolation.Cosine, oversamples = 3, frameRate = 24.0 ) 
		self.assertEqual( cache.getOversamples(), 3 )
		self.assertEqual( cache.getFrame(), 10 )

		cache = InterpolatedCache(self.pathTemplate, frame = 0, interpolation = InterpolatedCache.Interpolation.Cubic, oversamples = 1, frameRate = 24.0 ) 

	def testHeaders( self ):
		"""Test InterpolatedCache headers"""

		cache = InterpolatedCache(self.pathTemplate, frame = 1.5, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assert_( "testCache" in cache.headers() )
		h = cache.readHeader( "testCache" )
		self.assertEqual( h, self.__headerCompound( ( self.__time( 1 ) + self.__time( 2 ) ) / 2 ) )

	def testAttributes(self):
		"""Test InterpolatedCache attributes"""

		cache = InterpolatedCache(self.pathTemplate, frame = 1.2, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assertEqual( len( cache.attributes( "obj2" )), 2 )
		self.assertEqual( len( cache.attributes( "obj2", "i.*" )), 1 )
		self.assertEqual( len( cache.attributes( "obj1" )), 1 )

	def testContains(self):
		"""Test InterpolatedCache contains"""
		cache = InterpolatedCache(self.pathTemplate, frame = 1.2, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assert_( cache.contains( "obj1" ) )
		self.assert_( cache.contains( "obj2", "i" ) )
		self.failIf( cache.contains( "obj3" ) )
		self.failIf( cache.contains( "obj2", "not_in_cache" ) )

	def testReading(self):
		"""Test InterpolatedCache read"""
		cache = InterpolatedCache(self.pathTemplate, frame = 1.5, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assertEqual( cache.read( "obj2", "i" ), IntData( 1 ) )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 1.5 ) )
		self.assertEqual( cache.read( "obj1" ), CompoundObject( { "v3fVec": self.__createV3f( 1.5 ) } ) )

	def testChangingParameters( self ):
		"""Test InterpolatedCache reading and changing frame, interpolation, ... """
		cache = InterpolatedCache(self.pathTemplate, frame = 0, interpolation = InterpolatedCache.Interpolation.None )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 0 ) )
		cache.setFrame( 0.8 )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 0 ) )
		cache.setFrame( 1 )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 1 ) )
		cache.setFrame( 0.5 )
		cache.setInterpolation( InterpolatedCache.Interpolation.Linear )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 0.5 ) )
		cache.setFrame( 2.25 )
		self.assertAlmostEqual( cache.read( "obj2", "d" ).value, 2.496, 3 )
		cache.setInterpolation( InterpolatedCache.Interpolation.Cosine )
		self.assertAlmostEqual( cache.read( "obj2", "d" ).value, 2.28846, 3 )
		cache.setInterpolation( InterpolatedCache.Interpolation.Cubic )
		self.assertAlmostEqual( cache.read( "obj2", "d" ).value, 2.4512, 3 )
		cache.setFrame( 5 )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 16 ) )
		cache.setInterpolation( InterpolatedCache.Interpolation.Linear )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 16 ) )
		cache.setInterpolation( InterpolatedCache.Interpolation.Cosine )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 16 ) )
		cache.setInterpolation( InterpolatedCache.Interpolation.None )
		self.assertEqual( cache.read( "obj2", "d" ), DoubleData( 16 ) )

	def testOldTransformationMatrixData( self ):
		cache = InterpolatedCache( "test/IECore/data/attributeCaches/transform.old.%04d.fio", frame = 4, interpolation = InterpolatedCache.Interpolation.Linear )
		self.assertEqual( cache.read( ".parent", "transformCache.transform" ).value.rotate, Eulerd() )
		self.assertAlmostEqual( (cache.read( ".pSphere1", "transformCache.transform" ).value.rotate - Eulerd( 0.244525, 0, 0 )).length(), 0, 2 )
		cache.setFrame( 4.5 )
		self.assertEqual( cache.read( ".parent", "transformCache.transform" ).value.rotate, Eulerd() )
		self.assertAlmostEqual( (cache.read( ".pSphere1", "transformCache.transform" ).value.rotate - Eulerd(0.283422, 0, 0)).length(), 0, 2 )

	def testNewTransformationMatrixData( self ):

		# Code that created the cache files for this test:
		#cache1 = AttributeCache( "test/IECore/data/attributeCaches/transform.new.0250.fio", IndexedIOOpenMode.Write )
		#cache2 = AttributeCache( "test/IECore/data/attributeCaches/transform.new.0500.fio", IndexedIOOpenMode.Write )
		#for order in [ Eulerd.Order.XYZ, Eulerd.Order.XZY, Eulerd.Order.YZX, Eulerd.Order.YXZ, Eulerd.Order.ZXY, Eulerd.Order.ZYX ]:
		#	t = TransformationMatrixd()
		#	t.rotate = Eulerd( 1000, -10, 100, order )
		#	transform = TransformationMatrixdData( t )
		#	cache1.write( "test%d" % order, "transform", transform )
		#	t = TransformationMatrixd()
		#	t.rotate = Eulerd( 1002, -12, 102, order )
		#	transform = TransformationMatrixdData( t )
		#	cache2.write( "test%d" % order, "transform", transform )
		#cache1 = None
		#cache2 = None
		
		cache = InterpolatedCache( "test/IECore/data/attributeCaches/transform.new.%04d.fio", frame = 1.5, interpolation = InterpolatedCache.Interpolation.Linear )
		for order in [ Eulerd.Order.XYZ, Eulerd.Order.XZY, Eulerd.Order.YZX, Eulerd.Order.YXZ, Eulerd.Order.ZXY, Eulerd.Order.ZYX ]:
			self.assertEqual( cache.read( "test%d" % order, "transform" ).value.rotate, Eulerd( 1001, -11, 101, order ) )
		cache = None

	def tearDown(self):

		# cleanup
		allFiles = glob.glob( self.pathTemplate.replace( "%003d", "*" ) )
		for f in allFiles:	
			os.remove( f )

if __name__ == "__main__":
	unittest.main()   
	
