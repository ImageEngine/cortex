##########################################################################
#
#  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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
import threading
import time

import IECore

class LRUCacheTest( unittest.TestCase ) :

	def test( self ) :

		self.numGetterCalls = 0
		
		def getter( key ) :
		
			self.numGetterCalls += 1
		
			return (
				# value
				{
					"same" : key,
					"times2" : key * 2,
					"times4" : key * 4,
				},
				# cost
				1
			)

		c = IECore.LRUCache( getter, 10 )
		self.assertEqual( c.getMaxCost(), 10 )
		c.setMaxCost( 20 )
		self.assertEqual( c.getMaxCost(), 20 )
		c.setMaxCost( 10 )
		self.assertEqual( c.getMaxCost(), 10 )
				
		v = c.get( 10 )
		self.assertEqual( v,
			{ 
				"same" : 10,
				"times2" : 20,
				"times4" : 40,
			}
		)
		
		self.assertEqual( c.currentCost(), 1 )
		self.assertEqual( self.numGetterCalls, 1 )
				
		v2 = c.get( 10 )
		self.failUnless( v2 is v )
		
		self.assertEqual( c.currentCost(), 1 )
		self.assertEqual( self.numGetterCalls, 1 )
				
		for k in range( 11, 10000 ) :
		
			v = c.get( k )
			self.assertEqual( v,
				{ 
					"same" : k,
					"times2" : k * 2,
					"times4" : k * 4,
				}
			)
			
			self.failIf( c.currentCost() > 10 )
	
	def testClearCausesReloads( self ) :
	
		self.numGetterCalls = 0
		self.multiplier = 2
		
		def getter( key ) :
		
			self.numGetterCalls += 1
			return ( key * self.multiplier, 1 )
			
		c = IECore.LRUCache( getter, 10 )
		
		v = c.get( 10 )
		self.assertEqual( v, 20 )
		self.assertEqual( self.numGetterCalls, 1 )
		
		v = c.get( 10 )
		self.assertEqual( v, 20 )
		self.assertEqual( self.numGetterCalls, 1 )
		
		c.clear()
		self.multiplier = 4
		v = c.get( 10 )
		self.assertEqual( v, 40 )
		self.assertEqual( self.numGetterCalls, 2 )
		
	def testThreadingAndLimitCost( self ) :
	
		def getter( key ) :
		
			return ( key * 2, 1 )
			
		c = IECore.LRUCache( getter, 10 )
		
		def thrash() :
		
			for i in range( 0, 10000 ) :
				v = c.get( i )
				self.assertEqual( v, i * 2 )
		
		threads = []
		for i in range( 0, 10 ) :
			thread = threading.Thread( target=thrash )
			threads.append( thread )
			thread.start()
			
		for thread in threads :
			thread.join()

	def testThreadingAndClear( self ) :
	
		def getter( key ) :
		
			return ( key * 2, 1 )
	
		c = IECore.LRUCache( getter, 100000 )
	
		def f1() :
			for i in range( 0, 10000 ) :
				v = c.get( i )
				self.assertEqual( v, i * 2 )
				
		def f2() :
			for i in range( 0, 10000 ) :
				c.clear()
	
		t1 = threading.Thread( target=f1 )
		t2 = threading.Thread( target=f1 )
		t3 = threading.Thread( target=f2 )
		
		t1.start()
		t2.start()
		t3.start()
		
		t1.join()
		t2.join()
		t3.join()
		
		c.clear()
		self.assertEqual( c.currentCost(), 0 )

	def testYieldGILInGetter( self ) :
	
		def getter( key ) :
		
			# this call simulates the gil getting
			# yielded for some reason - in the real world
			# perhaps an Op call or just the python interpreter
			# deciding to switch threads.
			time.sleep( 0.1 )
			return ( key, 1 )
			
		c = IECore.LRUCache( getter, 100000 )
		
		def f() :
		
			c.get( 0 )
		
		t1 = threading.Thread( target=f )
		t2 = threading.Thread( target=f )
		
		t1.start()
		t2.start()
		t1.join()
		t2.join()
		
	def testRemovalCallback( self ) :
	
		def getter( key ) :
		
			return ( key * 2, 1 )
		
		removed = []
		def removalCallback( key, value ) :
		
			removed.append( ( key, value ) )
		
		c = IECore.LRUCache( getter, removalCallback, 5 )
		
		self.assertEqual( c.get( 1 ), 2 )
		self.assertEqual( removed, [] )
		
		self.assertEqual( c.get( 2 ), 4 )
		self.assertEqual( removed, [] )
		
		self.assertEqual( c.get( 3 ), 6 )
		self.assertEqual( removed, [] )
		
		self.assertEqual( c.get( 4 ), 8 )
		self.assertEqual( removed, [] )
		
		self.assertEqual( c.get( 5 ), 10 )
		self.assertEqual( removed, [] )
		
		self.assertEqual( c.get( 6 ), 12 )
		self.assertEqual( removed, [ ( 1, 2 ) ] )
		
		self.assertEqual( c.get( 7 ), 14 )
		self.assertEqual( removed, [ ( 1, 2 ), ( 2, 4 ) ] )

		c.clear()
		
		self.assertEqual( len( removed ), 7 )
		
		keys = [ x[0] for x in removed ]
		for i in range( 1, 8 ) :
			self.failUnless( i in keys )
	
	def testSet( self ) :
	
		def getter( key ) :
			return ( None, 1 )
			
		c = IECore.LRUCache( getter, 1000 )
		
		c.set( 5, 10, 1 )
		self.assertEqual( c.currentCost(), 1 )
		self.assertEqual( c.get( 5 ), 10 )
		self.assertEqual( c.currentCost(), 1 )
		
		c.set( 5, 20, 100000 )
		self.assertEqual( c.currentCost(), 0 )
		self.assertEqual( c.get( 5 ), None )
		self.assertEqual( c.currentCost(), 1 )
	
	def testCPPThreading( self ) :
		
		# arguments are :
		# iterations, number of unique values, maximum cost, clear frequency
		
		# cache exactly the right size
		IECore.testLRUCacheThreading( 100000, 100, 100 )
		
		# cache not quite big enough
		IECore.testLRUCacheThreading( 100000, 100, 90 )
		
		# cache thrashing like crazy
		IECore.testLRUCacheThreading( 100000, 1000, 2 )
		
		# clearing all the time while doing concurrent lookups
		IECore.testLRUCacheThreading( 100000, 1000, 90, 20 )
		
if __name__ == "__main__":
    unittest.main()
