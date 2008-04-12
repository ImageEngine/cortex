##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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


"""Unit test for HierarchicalCache binding"""
import os
import unittest
import math
import random

from IECore import *


class TestHierarchicalCache(unittest.TestCase):

	cachedObjectNames = [ "/pSphere1", "/pSphere2", "/pSphere3", "/pSphere1/pCube1", "/pSphere1/pCube2", "/pSphere1/pCube1/pShape1", "/NewBase1/NewBase2/pShape2" ]
	undirectlyCachedObjects = [ '/NewBase1', '/NewBase1/NewBase2' ]
	objectNodes = [ M44f(), M44f(), PointsPrimitive( V3fVectorData() ), Group(), M44f(), MeshPrimitive(), MeshPrimitive() ]
	cachedHeaderNames = [ "header1", "header2" ]
	uncachedObjectNames = [ "/pPlane14", "/nurbsCurve12" ]

	def testConstructors(self):
		"""Test HierarchicalCache constructors"""
		
		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Write) 
		
	def testReadWrite(self):
		"""Test HierarchicalCache read/write"""
		
		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Write) 		
		
		for (objIndex,obj) in enumerate(self.cachedObjectNames):
			# Make some random vertex data
			
			dataWritten = V3fVectorData()
			
			numPts = int(random.random())
			numPts = numPts * numPts * 100
			
			for i in range(0, numPts):
				dataWritten.append( V3f( random.random(), random.random(), random.random() ) )
			
			cache.write(obj, "P", dataWritten)
			
			dataRead = cache.read(obj, "P")
			
			self.assertEqual( dataWritten, dataRead )
			self.failIf( cache.isTransform( obj ) )
			self.failIf( cache.isShape( obj ) )

			cache.write(obj, self.objectNodes[ objIndex ] )
			if isinstance( self.objectNodes[ objIndex ], VisibleRenderable ):
				self.assert_( cache.isShape( obj ) )
				self.failIf( cache.isTransform( obj ) )
				self.assertEqual( cache.shape( obj ), self.objectNodes[ objIndex ] )
			else:
				self.assert_( cache.isTransform( obj ) )
				self.failIf( cache.isShape( obj ) )
				self.assertEqual( cache.transformMatrix( obj ), self.objectNodes[ objIndex ] )

			dataRead = cache.read(obj)
			
			self.assertEqual( dataWritten, dataRead["P"] )

		self.assertEqual( set( self.cachedObjectNames ).union( self.undirectlyCachedObjects ), set( cache.objects() ) )
		for obj in self.undirectlyCachedObjects:
			self.failIf( cache.isTransform( obj ) )
			self.failIf( cache.isShape( obj ) )
			self.assertEqual( cache.attributes( obj ), [] )

		self.failIf( cache.isTransform( "/" ) )
		self.failIf( cache.isShape( "/" ) )

		self.assertEqual( len( cache.children( self.cachedObjectNames[0] ) ), 2 )
		self.assertEqual( len( cache.children( self.undirectlyCachedObjects[0] ) ), 1 )
		
		self.assertRaises( RuntimeError, cache.children, self.uncachedObjectNames[0] )

	def testNameFunctions(self):
		"""Test HierarchicalCache absolute/relative name functions."""
		self.assertEqual( HierarchicalCache.absoluteName( "child", "/" ), "/child" )
		self.assertEqual( HierarchicalCache.absoluteName( "child", "/parent" ), "/parent/child" )
		self.assertEqual( HierarchicalCache.absoluteName( "child", "/parent1/parent2/" ), "/parent1/parent2/child" )
		self.assertEqual( HierarchicalCache.absoluteName( "child/", "/parent1/parent2/" ), "/parent1/parent2/child" )

		self.assertRaises( RuntimeError, HierarchicalCache.absoluteName, "/child", "/parent" )
		self.assertRaises( RuntimeError, HierarchicalCache.absoluteName, "child", "parent" )
		self.assertRaises( RuntimeError, HierarchicalCache.absoluteName, "child", "" )
		self.assertRaises( RuntimeError, HierarchicalCache.absoluteName, "child", "." )		

		self.assertEqual( HierarchicalCache.relativeName( "/test" ), "test" )
		self.assertEqual( HierarchicalCache.relativeName( "/test/" ), "test" )
		self.assertEqual( HierarchicalCache.relativeName( "/base/test" ), "test" )
		self.assertEqual( HierarchicalCache.relativeName( "/base/test/" ), "test" )

		self.assertRaises( RuntimeError, HierarchicalCache.relativeName, "child" )		
		self.assertRaises( RuntimeError, HierarchicalCache.parentName, "/" )				

		self.assertEqual( HierarchicalCache.parentName( "/t" ), "/" )
		self.assertEqual( HierarchicalCache.parentName( "/base/" ), "/" )
		self.assertEqual( HierarchicalCache.parentName( "/base/test" ), "/base" )
		self.assertEqual( HierarchicalCache.parentName( "/base/test/" ), "/base" )

		self.assertRaises( RuntimeError, HierarchicalCache.parentName, "base" )	
		self.assertRaises( RuntimeError, HierarchicalCache.parentName, "" )	

		self.assertEqual( HierarchicalCache.rootName(), "/" )

	def testReadWriteHeaders(self):
		"""Test HierarchicalCache read/write headers"""
		
		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Write) 		
		
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
		"""Test HierarchicalCache attributes"""
		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Write) 		
		cache.write( self.cachedObjectNames[0], "attrib1", V3fData( V3f( 1 ) ) )
		cache.write( self.cachedObjectNames[0], "attrib2", IntData( 1 ) )
		cache.write( self.cachedObjectNames[3], "other", IntData(0) )
		
		self.assertEqual( len( cache.attributes( self.cachedObjectNames[0] )), 2 )
		self.assertEqual( len( cache.attributes( self.cachedObjectNames[0], "attrib[12]" )), 2 )
		self.assertEqual( len( cache.attributes( self.cachedObjectNames[3], "other" )), 1 )

	def testRemove(self):
		"""Test HierarchicalCache remove"""

		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Write) 
		cache.write( self.cachedObjectNames[0], "attrib1", V3fData( V3f( 1 ) ) )
		cache.write( self.cachedObjectNames[0], "attrib2", IntData( 1 ) )
		cache.write( self.cachedObjectNames[1], "attrib3", IntData(0) )
		cache.write( self.cachedObjectNames[3], "attrib4", IntData(3) )
		cache.write( self.cachedObjectNames[3], "attrib5", IntData(5) )
		cache.writeHeader( self.cachedHeaderNames[0], IntData(2) )
		cache.writeHeader( self.cachedHeaderNames[1], IntData(52) )

		cache.remove( self.cachedObjectNames[0], "attrib1" )
		cache.remove( self.cachedObjectNames[1] )
		cache.remove( self.cachedObjectNames[3], "attrib4" )
		cache.removeHeader( self.cachedHeaderNames[0] )

		self.assertEqual( cache.attributes( self.cachedObjectNames[0] ), [ "attrib2" ] )
		self.assertEqual( cache.attributes( self.cachedObjectNames[3] ), [ "attrib5" ] )
		self.assertEqual( cache.objects(), [ self.cachedObjectNames[0], self.cachedObjectNames[3] ] )
		self.assert_( self.cachedHeaderNames[1] in cache.headers() )
		self.assert_( not self.cachedHeaderNames[0] in cache.headers() )
		
	def testContains(self):
		"""Test HierarchicalCache contains"""
		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Write) 		
		
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

		self.failIf( cache.contains( self.cachedObjectNames[0], "not_in_cache" ) )
		try:
			cache.contains( "relativeName" )
		except:
			pass
		else:
			raise Exception, "Should not accept relative paths!"
			
	def testOverwriting( self ):
		"""Test HierarchicalCache overwriting"""
		cache = None
		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Write)
		cache.write("/Object1", "Attribute1", IntData(1) )
		cache.write("/Object2", "Attribute1", IntData(1) )
		cache.write("/Object1/Object3", "Attribute1", IntData(1) )
		cache.write("/Object1/Object3", MeshPrimitive() )
		cache.write("/Object1", M44f() )
		
		cache = None
		cache = HierarchicalCache( "./test/HierarchicalCache.fio", IndexedIOOpenMode.Read)
		self.assertEqual( len( cache.attributes( "/Object1" ) ), 1 )
		self.assertEqual( len( cache.attributes( "/Object1/Object3" ) ), 1 )
		self.assert_( cache.isTransform( "/Object1" ) )
		self.assert_( cache.isShape( "/Object1/Object3" ) )
		 
		cache = None
		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Append)
		cache.write("/Object1", "Attribute1", IntData(1) )
		cache.write("/Object2", "Attribute1", IntData(1) )
		cache.write("/Object1/Object3", "Attribute1", IntData(1) )
		cache.write("/Object1", "Attribute2", IntData(2) )		
		cache.write("/Object2", "Attribute2", IntData(2) )
		cache.write("/Object1/Object3", "Attribute2", IntData(2) )
		cache.write("/Object1/", MeshPrimitive() )
		cache.write("/Object1/Object3/", M44f() )
		
		cache = None
		cache = HierarchicalCache( "./test/HierarchicalCache.fio", IndexedIOOpenMode.Read)
		self.assertEqual( len( cache.attributes( "/Object1" ) ), 2 )
		self.assertEqual( len( cache.attributes( "/Object1/Object3" ) ), 2 )
		self.assert_( cache.isTransform( "/Object1/Object3" ) )
		self.failIf( cache.isShape( "/Object1/Object3" ) )
		self.assert_( cache.isShape( "/Object1/" ) )
		self.failIf( cache.isTransform( "/Object1" ) )

	def testGlobalTransformMatrix( self ):
		"""Test HierarchicalCache globalTransformMatrix"""
		cache = HierarchicalCache( "./test/HierarchicalCache.fio", IndexedIOOpenMode.Append)
		m = M44f()
		m[ 3, 2 ] = 1
		cache.write( "/t", m )
		cache.write( "/t/t", m )
		cache.write( "/t/t/t", m )
		cache.write( "/t/t/t/t", m )
		self.assertEqual( cache.globalTransformMatrix( "/t" ), m )
		m[ 3, 2 ] = 2
		self.assertEqual( cache.globalTransformMatrix( "/t/t" ), m )
		m[ 3, 2 ] = 3
		self.assertEqual( cache.globalTransformMatrix( "/t/t/t" ), m )
		m[ 3, 2 ] = 4
		self.assertEqual( cache.globalTransformMatrix( "/t/t/t/t" ), m )

	def testBoundingBox( self ):
		"""Test HierarchicalCache bound method."""
		cache = HierarchicalCache( "./test/HierarchicalCache.fio", IndexedIOOpenMode.Append)
		t = M44f()
		t[ 3,2 ] = 1
		self.assertEqual( cache.bound( "/" ), Box3f() )
		cache.write( "/s", PointsPrimitive( V3fVectorData( [ V3f(0,0,1) ] ) ) )
		self.assertEqual( cache.bound( "/s" ), Box3f( V3f( 0,0,1 ), V3f( 0,0,1 ) ) )
		self.assertEqual( cache.bound( "/" ), Box3f( V3f( 0,0,1 ), V3f( 0,0,1 ) ) )
		cache.write( "/t/s", PointsPrimitive( V3fVectorData( [ V3f(0,0,1) ] ) ) )
		self.assertEqual( cache.bound( "/t/s" ), Box3f( V3f( 0,0,1 ), V3f( 0,0,1 ) ) )
		self.assertEqual( cache.bound( "/" ), Box3f( V3f( 0,0,1 ), V3f( 0,0,1 ) ) )
		cache.write( "/t/", t )
		self.assertEqual( cache.bound( "/t/s" ), Box3f( V3f( 0,0,1 ), V3f( 0,0,1 ) ) )
		self.assertEqual( cache.bound( "/" ), Box3f( V3f( 0,0,1 ), V3f( 0,0,2 ) ) )
		cache.write( "/t/t1/s", PointsPrimitive( V3fVectorData( [ V3f(1,0,0) ] ) ) )
		cache.write( "/t/t2/s", PointsPrimitive( V3fVectorData( [ V3f(0,1,0) ] ) ) )
		cache.write( "/t/t1/t/s", PointsPrimitive( V3fVectorData( [ V3f(0,0,1) ] ) ) )
		cache.write( "/t/t1", t )
		cache.write( "/t/t1/t", t )
		self.assertEqual( cache.bound( "/" ), Box3f( V3f( 0,0,1 ), V3f( 1,1,4 ) ) )
		cache.remove( "/t/t1/t/s" )
		self.assertEqual( cache.bound( "/" ), Box3f( V3f( 0,0,1 ), V3f( 1,1,2 ) ) )
		cache.remove( "/t/t1/s" )
		self.assertEqual( cache.bound( "/t" ), Box3f( V3f( 0,0,1 ), V3f( 0,1,2 ) ) )
		cache.remove( "/t" )
		self.assertEqual( cache.bound( "/" ), Box3f( V3f( 0,0,1 ), V3f( 0,0,1 ) ) )
	
	def testOverwriteFailure( self ) :
	
		cache = HierarchicalCache("./test/HierarchicalCache.fio", IndexedIOOpenMode.Append)
		
		# write an object
		m = MeshPrimitive.createPlane( Box2f( V2f( -1 ), V2f( 1 ) ) )
		cache.write( "/o1", m )
		self.assertEqual( cache.shape( "/o1" ), m )

		# make a new and different object
		mm = m.copy()
		self.assertEqual( mm, m )
		del mm["P"]
		self.assertNotEqual( mm, m )
		
		# write that over the old one
		cache.write( "/o1", mm )
		
		# and check that the contents of the cache
		# is the new object and not the old one
		
		self.assertEqual( cache.shape( "/o1" ), mm )
		self.assertNotEqual( cache.shape( "/o1" ), m )
		
	def setUp(self):
		
		# cleanup
		if os.path.isfile("./test/HierarchicalCache.fio") :	
			os.remove("./test/HierarchicalCache.fio")	

	def tearDown(self):
		
		# cleanup
		if os.path.isfile("./test/HierarchicalCache.fio") :	
			os.remove("./test/HierarchicalCache.fio")				
				
		
if __name__ == "__main__":
	unittest.main()   
	
