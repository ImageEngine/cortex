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


"""Unit test for AttributeCache binding"""
import os
import unittest
import math
import random

from IECore import *


class TestAttributeCache(unittest.TestCase):

	cachedObjectNames = [ "pSphere1", "pSphere2", "pSphere3", "pCube1" ]
	cachedHeaderNames = [ "header1", "header2" ]
	uncachedObjectNames = [ "pPlane14", "nurbsCurve12" ]

	def testConstructors(self):
		"""Test AttributeCache constructors"""
		
		cache = AttributeCache("./test/AttributeCache.fio", IndexedIOOpenMode.Write) 
		
	def testReadWrite(self):
		"""Test AttributeCache read/write"""
		
		cache = AttributeCache("./test/AttributeCache.fio", IndexedIOOpenMode.Write) 		
		
		for obj in self.cachedObjectNames:
			# Make some random vertex data
			
			dataWritten = V3fVectorData()
			
			numPts = int(random.random())
			numPts = numPts * numPts * 100
			
			for i in range(0, numPts):
				dataWritten.append( V3f( random.random(), random.random(), random.random() ) )
			
			cache.write(obj, "P", dataWritten)
			
			dataRead = cache.read(obj, "P")
			
			self.assertEqual( dataWritten, dataRead )

			dataRead = cache.read(obj)
			
			self.assertEqual( dataWritten, dataRead["P"] )

		self.assertEqual( set( self.cachedObjectNames ), set( cache.objects() ) )
			
	def testReadWriteHeaders(self):
		"""Test AttributeCache read/write headers"""
		
		cache = AttributeCache("./test/AttributeCache.fio", IndexedIOOpenMode.Write) 		
		
		for obj in self.cachedHeaderNames:
			# Make some random data
			
			dataWritten = V3fVectorData()
			
			numPts = int(random.random())
			numPts = numPts * numPts * 100
			
			for i in range(0, numPts):
				dataWritten.append( V3f( random.random(), random.random(), random.random() ) )
			
			cache.writeHeader(obj, dataWritten)
			
			dataRead = cache.readHeader(obj)
			
			self.assertEqual( dataWritten, dataRead )

			dataRead = cache.readHeader()
			
			self.assertEqual( dataWritten, dataRead[ obj ] )

		self.assertEqual( set( self.cachedHeaderNames ).intersection( cache.headers() ), set( self.cachedHeaderNames ) )

	def testAttributes(self):
		"""Test AttributeCache attributes"""
		cache = AttributeCache("./test/AttributeCache.fio", IndexedIOOpenMode.Write) 		
		cache.write( self.cachedObjectNames[0], "attrib1", V3fData( V3f( 1 ) ) )
		cache.write( self.cachedObjectNames[0], "attrib2", IntData( 1 ) )
		cache.write( self.cachedObjectNames[0], "other", IntData(0) )
		
		self.assertEqual( len( cache.attributes( self.cachedObjectNames[0] )), 3 )
		self.assertEqual( len( cache.attributes( self.cachedObjectNames[0], "attrib[12]" )), 2 )
		self.assertEqual( len( cache.attributes( self.cachedObjectNames[0], "other" )), 1 )

	def testRemove(self):
		"""Test AttributeCache remove"""

		cache = AttributeCache("./test/AttributeCache.fio", IndexedIOOpenMode.Write) 
		cache.write( self.cachedObjectNames[0], "attrib1", V3fData( V3f( 1 ) ) )
		cache.write( self.cachedObjectNames[0], "attrib2", IntData( 1 ) )
		cache.write( self.cachedObjectNames[1], "attrib3", IntData(0) )
		cache.writeHeader( self.cachedHeaderNames[0], IntData(2) )
		cache.writeHeader( self.cachedHeaderNames[1], IntData(52) )

		cache.remove( self.cachedObjectNames[0], "attrib1" )
		cache.remove( self.cachedObjectNames[1] )
		cache.removeHeader( self.cachedHeaderNames[0] )

		self.assertEqual( cache.attributes( self.cachedObjectNames[0] ), [ "attrib2" ] )
		self.assertEqual( cache.objects(), [ self.cachedObjectNames[0] ] )
		self.assert_( self.cachedHeaderNames[1] in cache.headers() )
		self.assert_( not self.cachedHeaderNames[0] in cache.headers() )
		
		
	def testContains(self):
		"""Test AttributeCache contains"""
		cache = AttributeCache("./test/AttributeCache.fio", IndexedIOOpenMode.Write) 		
		
		for obj in self.cachedObjectNames:
			# Create some dummy data (contents not important for this test)
			
			dataWritten = V3fVectorData()
			dataWritten.append( V3f( 1, 1, 1) )
			
			cache.write(obj, "P", dataWritten)
						
			self.assert_( cache.contains( obj ) )
			self.assert_( cache.contains( obj, "P" ) )
			
		for obj in self.uncachedObjectNames:
			self.failIf( cache.contains( obj ) )
			self.failIf( cache.contains( obj ), "not_in_cache" )
			
	def testOverwriting( self ):
		"""Test AttributeCache overwriting"""
		cache = None
		cache = AttributeCache("./test/AttributeCache.fio", IndexedIOOpenMode.Write)
		cache.write("Object1", "Attribute1", IntData(1) )
		cache.write("Object2", "Attribute1", IntData(1) )
		cache.write("Object3", "Attribute1", IntData(1) )
		
		
		cache = None
		cache = AttributeCache( "./test/AttributeCache.fio", IndexedIOOpenMode.Read)
		self.assertEqual( len( cache.attributes( "Object1" ) ), 1 )
		
		cache = None
		cache = AttributeCache("./test/AttributeCache.fio", IndexedIOOpenMode.Append)
		cache.write("Object1", "Attribute1", IntData(1) )
		cache.write("Object2", "Attribute1", IntData(1) )
		cache.write("Object3", "Attribute1", IntData(1) )
		cache.write("Object1", "Attribute2", IntData(2) )		
		cache.write("Object2", "Attribute2", IntData(2) )
		cache.write("Object3", "Attribute2", IntData(2) )
		
		cache = None
		cache = AttributeCache( "./test/AttributeCache.fio", IndexedIOOpenMode.Read)
		self.assertEqual( len( cache.attributes( "Object1" ) ), 2 )
										
	def tearDown(self):
		
		# cleanup
	
		for f in [ "./test/AttributeCache.fio", "test/IECore/data/attributeCaches/lotsOfV3fData.fio" ] :
			if os.path.isfile( f ) :	
				os.remove( f )				
		
if __name__ == "__main__":
	unittest.main()   
	
