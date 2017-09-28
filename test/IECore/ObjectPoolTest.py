##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

from IECore import *
import os

class ObjectPoolTest( unittest.TestCase ) :

	def testConstructor( self ) :

		p = ObjectPool(500)
		self.assertTrue( isinstance( p, ObjectPool ) )
		self.assertEqual( p.memoryUsage(), 0 )
		self.assertEqual( p.getMaxMemoryUsage(), 500 )

		p2 = ObjectPool.defaultObjectPool()
		self.assertTrue( isinstance( p2, ObjectPool ) )

	def testUniqueness( self ) :

		p = ObjectPool(500)
		self.assertEqual( p.memoryUsage(), 0 )

		a = p.store( IntData(1), ObjectPool.StoreReference )
		self.assertEqual( p.memoryUsage(), a.memoryUsage() )
		self.assertTrue( p.contains(a.hash()) )

		self.assertTrue( a.isSame( p.retrieve(a.hash(),_copy=False) ) )
		self.assertFalse( a.isSame( p.retrieve(a.hash() ) ) )
		self.assertEqual( a, p.retrieve(a.hash() ) )
		self.assertTrue( a.isSame( p.store(a, ObjectPool.StoreCopy) ) )
		self.assertTrue( a.isSame( p.store(a, ObjectPool.StoreReference) ) )
		self.assertTrue( a.isSame( p.store(IntData(1), ObjectPool.StoreReference) ) )

		self.assertEqual( p.memoryUsage(), a.memoryUsage() )

	def testStoreMode( self ) :

		b = IntData(10)

		p = ObjectPool(500)
		self.assertTrue( b.isSame( p.store( b, ObjectPool.StoreReference ) ) )
		p.clear()
		self.assertFalse( b.isSame( p.store( b, ObjectPool.StoreCopy ) ) )
		self.assertEqual( b, p.retrieve(b.hash(),_copy=False) )

	def testRemoval( self ) :

		p = ObjectPool(500)
		self.assertEqual( p.memoryUsage(), 0 )
		a = p.store( IntData(1), ObjectPool.StoreReference )
		self.assertTrue( p.contains(a.hash()) )
		self.assertEqual( p.memoryUsage(), a.memoryUsage() )
		p.clear()
		self.assertEqual( p.memoryUsage(), 0 )
		self.assertFalse( p.contains(a.hash()) )

		a = p.store( StringData("abc"), ObjectPool.StoreReference )
		self.assertTrue( p.contains(a.hash()) )
		self.assertEqual( p.memoryUsage(), a.memoryUsage() )
		p.erase( a.hash() )
		self.assertEqual( p.memoryUsage(), 0 )
		self.assertFalse( p.contains(a.hash()) )

	def testMaxMemoryUsage( self ) :

		p = ObjectPool(500)
		self.assertEqual( p.memoryUsage(), 0 )
		a = p.store( IntData(1), ObjectPool.StoreReference )
		self.assertEqual( p.memoryUsage(), a.memoryUsage() )
		b = p.store( StringData("abc"), ObjectPool.StoreReference )
		self.assertEqual( p.memoryUsage(), a.memoryUsage()+b.memoryUsage() )

		p.setMaxMemoryUsage( p.memoryUsage() - 1 )
		self.assertLess( p.getMaxMemoryUsage(), a.memoryUsage() + b.memoryUsage() )
		self.assertFalse(
			p.contains( a.hash() ) and
			p.contains( b.hash() )
		)

if __name__ == "__main__":
    unittest.main()
