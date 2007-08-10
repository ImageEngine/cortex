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

"""Unit test for FloatVectorData binding"""

import math
import unittest
from IECore import *


class BaseVectorDataTest:

	def __init__(self, vectorFactory, valueFactory):
		self.vectorFactory = vectorFactory
		self.valueFactory = valueFactory

	def testConstructors(self):
		"""Test constructors"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector()
		v2 = _vector(v1)
		v3 = _vector([_c(23), _c(4), _c(34), _c(12)])
		v4 = _vector(10)
		self.assertEqual( len( v4 ), 10 )
		
	def testResize(self):
		"""Test resizing"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v = self.vectorFactory()
		v.append(_c(2))
		self.assertEqual(v[0], _c(2))
		v.append(_c(0))
		v.append(_c(3))
		v.append(_c(2))
		v.append(_c(5))
		self.assertEqual(v[4], _c(5))
		self.assertEqual(len(v), 5)
		del(v[0])
		self.assertEqual(len(v), 4)
		self.assert_(v[0] == _c(0))
		del(v[-2])
		self.assertEqual(len(v), 3)
		self.assert_(v[-2] == _c(3))
		
		v.resize( 10 )
		self.assertEqual( len( v ), 10 )
		v2 = v.copy()
		v.resize( 2 )
		self.assertEqual( len( v ), 2 )
		self.assertEqual( v[0], v2[0] )
		self.assertEqual( v[1], v2[1] )
		
	def testAssignment(self):
		"""Test assignment"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector()
		v1.append(_c(0))
		v1.append(_c(0))
		v1[0] = _c(6)
		v1[1] = _c(13)
		v2 = _vector(v1)
		v3 = v1
		self.assertEqual(len(v1), 2)
		self.assertEqual(len(v1), len(v2))
		self.assertEqual(v1[0], v2[0])
		self.assertEqual(v1[1], v2[1])
		
	def testNegativeIndexing(self):
		"""Test negative indexing"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2)])
		self.assertEqual(v1[-1], _c(2))
		self.assertEqual(v1[-2], _c(1))
		
	def testCopyOnWrite(self):
		"""Test copy-on-write behavior"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2)])
		v2 = _vector(v1)
		v1[0] = _c(5)
		self.assertEqual(v1[0], _c(5))
		self.assertEqual(v2[0], _c(1))
		v1.append(_c(5));
		self.assertEqual(len(v1), 3)
		self.assertEqual(len(v2), 2)
		
	def testContains(self):
		"""Test contains, index and count functions"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2), _c(2), _c(2), _c(4), _c(5)])
		self.assert_(_c(4) in v1)
		self.assert_(_c(0) not in v1)
		self.assertEqual(v1.count(_c(2)), 3)
		self.assertEqual(v1.count(_c(6)), 0)
		self.assertEqual(v1.index(_c(2)), 1)
		self.assertEqual(v1.index(_c(2), 2), 2)
		self.assertEqual(v1.index(_c(2), 2, 3), 2)
		
	def testExtend(self):
		"""Test extend function"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2), _c(3), _c(4), _c(5)])
		v2 = _vector(v1)
		v2.extend(v1)
		self.assertEqual(len(v2), 10)
		v2.extend([_c(9), _c(9), _c(9)])
		self.assert_(v2 == _vector([_c(1), _c(2), _c(3), _c(4), _c(5), _c(1), _c(2), _c(3), _c(4), _c(5), _c(9), _c(9), _c(9)]))
		v1.insert(0, _c(9))
		v1.insert(-1, _c(8))
		v1.insert(100, _c(7))
		v1.insert(-100, _c(6))
		v1.insert(4, _c(10))
		self.assert_(v1 == _vector([_c(6), _c(9), _c(1), _c(2), _c(10), _c(3), _c(4), _c(8), _c(5), _c(7)]))
	
	def testSlices(self):
		"""Test slicing behavior"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2), _c(3), _c(4), _c(5), _c(6), _c(7), _c(8), _c(9), _c(10)])
		v2 = v1[3:7]
		self.assertEqual(len(v2), 4)
		self.assert_(v2 == _vector([_c(4), _c(5), _c(6), _c(7)]))
		v3 = _vector(v1)
		del(v3[3:7])
		self.assertEqual(len(v3), 6)
		self.assert_(v3 == _vector([_c(1), _c(2), _c(3), _c(8), _c(9), _c(10)]))
		v4 = _vector(v3)
		v3[3:3] = [_c(4),_c(5),_c(6),_c(7)]
		self.assert_(v1 == v3)
		v1[1:9] = _vector([_c(9), _c(9), _c(9)])
		self.assertEqual(len(v1), 5)
		self.assert_(v1 == _vector([_c(1), _c(9), _c(9), _c(9), _c(10)]))

	def testEquality(self):
		"""Test equality function"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2), _c(3), _c(4), _c(5)])
		v2 = _vector([_c(1), _c(2), _c(3), _c(4), _c(5)])
		v3 = _vector([_c(1), _c(2), _c(3), _c(4)])
		self.assert_(v1 == v2)
		self.assert_(not v1 != v2)
		self.assert_(not v1 == v3)
		self.assert_(not v2 == v3)
		v2[-1] = _c(6)
		self.assert_(v1 != v2)
		self.assert_(not v1 == v2)
		del(v1[-1])
		self.assert_(v1 == v3)
	
	def testComparison(self):
		"""Test comparison function"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2), _c(3), _c(4)])
		v2 = _vector([_c(2), _c(2), _c(3), _c(4)])
		v3 = _vector([_c(1), _c(2), _c(3), _c(4), _c(5)])
		v4 = _vector([_c(2), _c(2), _c(3), _c(4), _c(5)])
		v5 = _vector([_c(1), _c(2), _c(4), _c(4), _c(5)])

		self.assert_(v1 == v1)
		self.assert_(v1 <= v1)
		self.assert_(v1 >= v1)
		self.assert_(not v1 < v1)
		self.assert_(not v1 > v1)

		self.assert_(not v1 == v2)
		self.assert_(v1 <= v2)
		self.assert_(not v1 >= v2)
		self.assert_(v1 < v2)
		self.assert_(not v1 > v2)

		self.assert_(not v1 == v3)
		self.assert_(v1 <= v3)
		self.assert_(not v1 >= v3)
		self.assert_(v1 < v3)
		self.assert_(not v1 > v3)

		self.assert_(not v1 == v4)
		self.assert_(v1 <= v4)
		self.assert_(not v1 >= v4)
		self.assert_(v1 < v4)
		self.assert_(not v1 > v4)

		self.assert_(not v1 == v5)
		self.assert_(v1 <= v5)
		self.assert_(not v1 >= v5)
		self.assert_(v1 < v5)
		self.assert_(not v1 > v5)

		self.assert_(not v2 == v1)
		self.assert_(not v2 <= v1)
		self.assert_(v2 >= v1)
		self.assert_(not v2 < v1)
		self.assert_(v2 > v1)

		self.assert_(v2 == v2)
		self.assert_(v2 <= v2)
		self.assert_(v2 >= v2)
		self.assert_(not v2 < v2)
		self.assert_(not v2 > v2)

		self.assert_(not v2 == v3)
		self.assert_(not v2 <= v3)
		self.assert_(v2 >= v3)
		self.assert_(not v2 < v3)
		self.assert_(v2 > v3)

		self.assert_(not v2 == v4)
		self.assert_(v2 <= v4)
		self.assert_(not v2 >= v4)
		self.assert_(v2 < v4)
		self.assert_(not v2 > v4)

		self.assert_(not v2 == v5)
		self.assert_(not v2 <= v5)
		self.assert_(v2 >= v5)
		self.assert_(not v2 < v5)
		self.assert_(v2 > v5)
		
		self.assert_(not v3 == v1)
		self.assert_(not v3 <= v1)
		self.assert_(v3 >= v1)
		self.assert_(not v3 < v1)
		self.assert_(v3 > v1)

		self.assert_(not v3 == v2)
		self.assert_(v3 <= v2)
		self.assert_(not v3 >= v2)
		self.assert_(v3 < v2)
		self.assert_(not v3 > v2)

		self.assert_(not v3 == v4)
		self.assert_(v3 <= v4)
		self.assert_(not v3 >= v4)
		self.assert_(v3 < v4)
		self.assert_(not v3 > v4)
		
		self.assert_(not v5 == v3)
		self.assert_(not v5 <= v3)
		self.assert_(v5 >= v3)
		self.assert_(not v5 < v3)
		self.assert_(v5 > v3)
		
	def testSumOperations(self):
		"""Test sum operations"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2), _c(3), _c(4), _c(5)])
		v2 = v1 + _c(1)
		self.assert_(v2 == _vector([_c(2), _c(3), _c(4), _c(5), _c(6)]))
		v2 = _vector(v1)
		v2 += _c(1)
		self.assert_(v2 == _vector([_c(2), _c(3), _c(4), _c(5), _c(6)]))
		v3 = v1 + v2
		self.assert_(v3 == _vector([_c(3), _c(5), _c(7), _c(9), _c(11)]))
		v3 = _vector(v1)
		v3 += v2
		self.assert_(v3 == _vector([_c(3), _c(5), _c(7), _c(9), _c(11)]))

	def testSubOperations(self):
		"""Test subtraction operations"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(2), _c(3), _c(4), _c(5), _c(6)])
		v2 = v1 - _c(1)
		self.assert_(v1 == _vector([_c(2), _c(3), _c(4), _c(5), _c(6)]))
		self.assert_(v2 == _vector([_c(1), _c(2), _c(3), _c(4), _c(5)]))
		v2 = _vector(v1)
		v2 -= _c(1)
		self.assert_(v2 == _vector([_c(1), _c(2), _c(3), _c(4), _c(5)]))
		v3 = v1 - v2
		self.assert_(v3 == _vector([_c(1), _c(1), _c(1), _c(1), _c(1)]))
		v3 = _vector(v1)
		v3 -= v2
		self.assert_(v3 == _vector([_c(1), _c(1), _c(1), _c(1), _c(1)]))

	def testMultOperations(self):
		"""Test multiplication operations"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2), _c(3), _c(4), _c(5)])
		v2 = v1 * _c(2)
		self.assert_(v1 == _vector([_c(1), _c(2), _c(3), _c(4), _c(5)]))
		self.assert_(v2 == _vector([_c(2), _c(4), _c(6), _c(8), _c(10)]))
		v2 = _vector(v1)
		v2 *= _c(2)
		self.assert_(v2 == _vector([_c(2), _c(4), _c(6), _c(8), _c(10)]))
		v3 = v1 * v2
		self.assert_(v3 == _vector([_c(2), _c(8), _c(18), _c(32), _c(50)]))
		v3 = _vector(v1)
		v3 *= v2
		self.assert_(v3 == _vector([_c(2), _c(8), _c(18), _c(32), _c(50)]))

	def testDivOperations(self):
		"""Test division operations"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(4), _c(8), _c(16), _c(32), _c(64)])
		v2 = v1 / _c(2)
		self.assert_(v1 == _vector([_c(4), _c(8), _c(16), _c(32), _c(64)]))
		self.assert_(v2 == _vector([_c(2), _c(4), _c(8), _c(16), _c(32)]))
		v2 = _vector(v1)
		v2 /= _c(2)
		self.assert_(v2 == _vector([_c(2), _c(4), _c(8), _c(16), _c(32)]))
		v3 = v1 / v2
		self.assert_(v3 == _vector([_c(2), _c(2), _c(2), _c(2), _c(2)]))
		v3 = _vector(v1)
		v3 /= v2
		self.assert_(v3 == _vector([_c(2), _c(2), _c(2), _c(2), _c(2)]))

	def testByValueItem(self):
		"""Test by value return type"""
		_vector = self.vectorFactory
		_c = self.valueFactory
		v1 = _vector([_c(1), _c(2), _c(3)])
		self.assert_(v1[0] == _c(1))
		a = v1[0]
		a = _c(255)
		self.assert_(v1[0] == _c(1))
		self.assert_(a == _c(255))
		a = v1[1:3]
		a[:] = [_c(9),_c(9),_c(9)]
		self.assertEqual(len(v1), 3)
		self.assert_(v1[1] == _c(2))
		self.assert_(v1[2] == _c(3))
		self.assertEqual(len(a), 3)
		self.assert_(a[0], _c(9))

class FloatVectorDataTest(BaseVectorDataTest,unittest.TestCase):

	def __init__(self, param1):
		BaseVectorDataTest.__init__(self, FloatVectorData, float)
		unittest.TestCase.__init__(self, param1)

	def testConstructors(self):
		"""Test FloatVectorData constructors"""
		BaseVectorDataTest.testConstructors(self)
		
	def testResize(self):
		"""Test FloatVectorData resizing"""
		BaseVectorDataTest.testResize(self)
				
	def testAssignment(self):
		"""Test FloatVectorData assignment"""
		BaseVectorDataTest.testAssignment(self)
				
	def testNegativeIndexing(self):
		"""Test FloatVectorData negative indexing"""
		BaseVectorDataTest.testNegativeIndexing(self)
				
	def testCopyOnWrite(self):
		"""Test FloatVectorData copy-on-write behavior"""
		BaseVectorDataTest.testCopyOnWrite(self)
				
	def testContains(self):
		"""Test FloatVectorData contains function"""
		BaseVectorDataTest.testContains(self)
				
	def testExtend(self):
		"""Test FloatVectorData extend function"""
		BaseVectorDataTest.testExtend(self)
			
	def testSlices(self):
		"""Test FloatVectorData slicing behavior"""
		BaseVectorDataTest.testSlices(self)

	def testEquality(self):
		"""Test FloatVectorData equality function"""
		BaseVectorDataTest.testEquality(self)
			
	def testComparison(self):
		"""Test FloatVectorData comparison function"""
		BaseVectorDataTest.testComparison(self)
				
	def testSumOperations(self):
		"""Test FloatVectorData sum operations"""
		BaseVectorDataTest.testSumOperations(self)
		
	def testSubOperations(self):
		"""Test FloatVectorData subtraction operations"""
		BaseVectorDataTest.testSubOperations(self)
		
	def testMultOperations(self):
		"""Test FloatVectorData multiplication operations"""
		BaseVectorDataTest.testMultOperations(self)
				
	def testDivOperations(self):
		"""Test FloatVectorData division operations"""
		BaseVectorDataTest.testDivOperations(self)
				
	def testByValueItem(self):
		"""Test FloatVectorData by value return type"""
		BaseVectorDataTest.testByValueItem(self)

class HalfVectorDataTest(BaseVectorDataTest,unittest.TestCase):

	def __init__(self, param1):
		BaseVectorDataTest.__init__(self, HalfVectorData, float)
		unittest.TestCase.__init__(self, param1)

	def testConstructors(self):
		"""Test HalfVectorData constructors"""
		BaseVectorDataTest.testConstructors(self)
		
	def testResize(self):
		"""Test HalfVectorData resizing"""
		BaseVectorDataTest.testResize(self)
				
	def testAssignment(self):
		"""Test HalfVectorData assignment"""
		BaseVectorDataTest.testAssignment(self)
				
	def testNegativeIndexing(self):
		"""Test HalfVectorData negative indexing"""
		BaseVectorDataTest.testNegativeIndexing(self)
				
	def testCopyOnWrite(self):
		"""Test HalfVectorData copy-on-write behavior"""
		BaseVectorDataTest.testCopyOnWrite(self)
				
	def testContains(self):
		"""Test HalfVectorData contains function"""
		BaseVectorDataTest.testContains(self)
				
	def testExtend(self):
		"""Test HalfVectorData extend function"""
		BaseVectorDataTest.testExtend(self)
			
	def testSlices(self):
		"""Test FloatVectorData slicing behavior"""
		BaseVectorDataTest.testSlices(self)

	def testEquality(self):
		"""Test HalfVectorData equality function"""
		BaseVectorDataTest.testEquality(self)
			
	def testComparison(self):
		"""Test HalfVectorData comparison function"""
		BaseVectorDataTest.testComparison(self)
				
	def testSumOperations(self):
		"""Test HalfVectorData sum operations"""
		BaseVectorDataTest.testSumOperations(self)
		
	def testSubOperations(self):
		"""Test HalfVectorData subtraction operations"""
		BaseVectorDataTest.testSubOperations(self)
		
	def testMultOperations(self):
		"""Test HalfVectorData multiplication operations"""
		BaseVectorDataTest.testMultOperations(self)
				
	def testDivOperations(self):
		"""Test HalfVectorData division operations"""
		BaseVectorDataTest.testDivOperations(self)
				
	def testByValueItem(self):
		"""Test HalfVectorData by value return type"""
		BaseVectorDataTest.testByValueItem(self)
						
class DoubleVectorDataTest(BaseVectorDataTest,unittest.TestCase):

	def __init__(self, param1):
		BaseVectorDataTest.__init__(self, DoubleVectorData, float)
		unittest.TestCase.__init__(self, param1)

	def testConstructors(self):
		"""Test DoubleVectorData constructors"""
		BaseVectorDataTest.testConstructors(self)
		
	def testResize(self):
		"""Test DoubleVectorData resizing"""
		BaseVectorDataTest.testResize(self)
				
	def testAssignment(self):
		"""Test DoubleVectorData assignment"""
		BaseVectorDataTest.testAssignment(self)
				
	def testNegativeIndexing(self):
		"""Test DoubleVectorData negative indexing"""
		BaseVectorDataTest.testNegativeIndexing(self)
				
	def testCopyOnWrite(self):
		"""Test DoubleVectorData copy-on-write behavior"""
		BaseVectorDataTest.testCopyOnWrite(self)
				
	def testContains(self):
		"""Test DoubleVectorData contains function"""
		BaseVectorDataTest.testContains(self)
				
	def testExtend(self):
		"""Test DoubleVectorData extend function"""
		BaseVectorDataTest.testExtend(self)
			
	def testSlices(self):
		"""Test DoubleVectorData slicing behavior"""
		BaseVectorDataTest.testSlices(self)
			
	def testEquality(self):
		"""Test DoubleVectorData equality function"""
		BaseVectorDataTest.testEquality(self)

	def testComparison(self):
		"""Test DoubleVectorData comparison function"""
		BaseVectorDataTest.testComparison(self)
				
	def testSumOperations(self):
		"""Test DoubleVectorData sum operations"""
		BaseVectorDataTest.testSumOperations(self)
		
	def testSubOperations(self):
		"""Test DoubleVectorData subtraction operations"""
		BaseVectorDataTest.testSubOperations(self)
		
	def testMultOperations(self):
		"""Test DoubleVectorData multiplication operations"""
		BaseVectorDataTest.testMultOperations(self)
				
	def testDivOperations(self):
		"""Test DoubleVectorData division operations"""
		BaseVectorDataTest.testDivOperations(self)
				
	def testByValueItem(self):
		"""Test DoubleVectorData by value return type"""
		BaseVectorDataTest.testByValueItem(self)

class IntVectorDataTest(BaseVectorDataTest,unittest.TestCase):

	def __init__(self, param1):
		BaseVectorDataTest.__init__(self, IntVectorData, int)
		unittest.TestCase.__init__(self, param1)

	def testConstructors(self):
		"""Test IntVectorData constructors"""
		BaseVectorDataTest.testConstructors(self)
		
	def testResize(self):
		"""Test IntVectorData resizing"""
		BaseVectorDataTest.testResize(self)
				
	def testAssignment(self):
		"""Test IntVectorData assignment"""
		BaseVectorDataTest.testAssignment(self)
				
	def testNegativeIndexing(self):
		"""Test IntVectorData negative indexing"""
		BaseVectorDataTest.testNegativeIndexing(self)
				
	def testCopyOnWrite(self):
		"""Test IntVectorData copy-on-write behavior"""
		BaseVectorDataTest.testCopyOnWrite(self)
				
	def testContains(self):
		"""Test IntVectorData contains function"""
		BaseVectorDataTest.testContains(self)
				
	def testExtend(self):
		"""Test IntVectorData extend function"""
		BaseVectorDataTest.testExtend(self)
			
	def testSlices(self):
		"""Test IntVectorData slicing behavior"""
		BaseVectorDataTest.testSlices(self)
			
	def testEquality(self):
		"""Test IntVectorData equality function"""
		BaseVectorDataTest.testEquality(self)

	def testComparison(self):
		"""Test IntVectorData comparison function"""
		BaseVectorDataTest.testComparison(self)
				
	def testSumOperations(self):
		"""Test IntVectorData sum operations"""
		BaseVectorDataTest.testSumOperations(self)
		
	def testSubOperations(self):
		"""Test IntVectorData subtraction operations"""
		BaseVectorDataTest.testSubOperations(self)
		
	def testMultOperations(self):
		"""Test IntVectorData multiplication operations"""
		BaseVectorDataTest.testMultOperations(self)
				
	def testDivOperations(self):
		"""Test IntVectorData division operations"""
		BaseVectorDataTest.testDivOperations(self)
				
	def testByValueItem(self):
		"""Test IntVectorData by value return type"""
		BaseVectorDataTest.testByValueItem(self)

class UIntVectorDataTest(BaseVectorDataTest,unittest.TestCase):

	def __init__(self, param1):
		BaseVectorDataTest.__init__(self, UIntVectorData, int)
		unittest.TestCase.__init__(self, param1)

	def testConstructors(self):
		"""Test UIntVectorData constructors"""
		BaseVectorDataTest.testConstructors(self)
		
	def testResize(self):
		"""Test UIntVectorData resizing"""
		BaseVectorDataTest.testResize(self)
				
	def testAssignment(self):
		"""Test UIntVectorData assignment"""
		BaseVectorDataTest.testAssignment(self)
				
	def testNegativeIndexing(self):
		"""Test UIntVectorData negative indexing"""
		BaseVectorDataTest.testNegativeIndexing(self)
				
	def testCopyOnWrite(self):
		"""Test UIntVectorData copy-on-write behavior"""
		BaseVectorDataTest.testCopyOnWrite(self)
				
	def testContains(self):
		"""Test UIntVectorData contains function"""
		BaseVectorDataTest.testContains(self)
				
	def testExtend(self):
		"""Test UIntVectorData extend function"""
		BaseVectorDataTest.testExtend(self)
			
	def testSlices(self):
		"""Test UIntVectorData slicing behavior"""
		BaseVectorDataTest.testSlices(self)

	def testEquality(self):
		"""Test UIntVectorData equality function"""
		BaseVectorDataTest.testEquality(self)
			
	def testComparison(self):
		"""Test UIntVectorData comparison function"""
		BaseVectorDataTest.testComparison(self)
				
	def testSumOperations(self):
		"""Test UIntVectorData sum operations"""
		BaseVectorDataTest.testSumOperations(self)
		
	def testSubOperations(self):
		"""Test UIntVectorData subtraction operations"""
		BaseVectorDataTest.testSubOperations(self)
		
	def testMultOperations(self):
		"""Test UIntVectorData multiplication operations"""
		BaseVectorDataTest.testMultOperations(self)
				
	def testDivOperations(self):
		"""Test UIntVectorData division operations"""
		BaseVectorDataTest.testDivOperations(self)
				
	def testByValueItem(self):
		"""Test UIntVectorData by value return type"""
		BaseVectorDataTest.testByValueItem(self)

class CharVectorDataTest(BaseVectorDataTest,unittest.TestCase):

	def __init__(self, param1):
		BaseVectorDataTest.__init__(self, CharVectorData, chr)
		unittest.TestCase.__init__(self, param1)

	def testConstructors(self):
		"""Test CharVectorData constructors"""
		BaseVectorDataTest.testConstructors(self)
		
	def testResize(self):
		"""Test CharVectorData resizing"""
		BaseVectorDataTest.testResize(self)
				
	def testAssignment(self):
		"""Test CharVectorData assignment"""
		BaseVectorDataTest.testAssignment(self)
				
	def testNegativeIndexing(self):
		"""Test CharVectorData negative indexing"""
		BaseVectorDataTest.testNegativeIndexing(self)
				
	def testCopyOnWrite(self):
		"""Test CharVectorData copy-on-write behavior"""
		BaseVectorDataTest.testCopyOnWrite(self)
				
	def testContains(self):
		"""Test CharVectorData contains function"""
		BaseVectorDataTest.testContains(self)
				
	def testExtend(self):
		"""Test CharVectorData extend function"""
		BaseVectorDataTest.testExtend(self)
			
	def testSlices(self):
		"""Test CharVectorData slicing behavior"""
		BaseVectorDataTest.testSlices(self)

	def testEquality(self):
		"""Test CharVectorData equality function"""
		BaseVectorDataTest.testEquality(self)
			
	def testComparison(self):
		"""Test CharVectorData comparison function"""
		BaseVectorDataTest.testComparison(self)
				
	def testSumOperations(self):
		"""Test CharVectorData sum operations"""
		BaseVectorDataTest.testSumOperations(self)
		
	def testSubOperations(self):
		"""Test CharVectorData subtraction operations"""
		BaseVectorDataTest.testSubOperations(self)
		
	def testMultOperations(self):
		"""Test CharVectorData multiplication operations"""
		BaseVectorDataTest.testMultOperations(self)
				
	def testDivOperations(self):
		"""Test CharVectorData division operations"""
		BaseVectorDataTest.testDivOperations(self)
				
	def testByValueItem(self):
		"""Test CharVectorData by value return type"""
		BaseVectorDataTest.testByValueItem(self)


class UCharVectorDataTest(BaseVectorDataTest,unittest.TestCase):

	def __init__(self, param1):
		BaseVectorDataTest.__init__(self, UCharVectorData, int)
		unittest.TestCase.__init__(self, param1)

	def testConstructors(self):
		"""Test UCharVectorData constructors"""
		BaseVectorDataTest.testConstructors(self)
		
	def testResize(self):
		"""Test UCharVectorData resizing"""
		BaseVectorDataTest.testResize(self)
				
	def testAssignment(self):
		"""Test UCharVectorData assignment"""
		BaseVectorDataTest.testAssignment(self)
				
	def testNegativeIndexing(self):
		"""Test UCharVectorData negative indexing"""
		BaseVectorDataTest.testNegativeIndexing(self)
				
	def testCopyOnWrite(self):
		"""Test UCharVectorData copy-on-write behavior"""
		BaseVectorDataTest.testCopyOnWrite(self)
				
	def testContains(self):
		"""Test UCharVectorData contains function"""
		BaseVectorDataTest.testContains(self)
				
	def testExtend(self):
		"""Test UCharVectorData extend function"""
		BaseVectorDataTest.testExtend(self)
			
	def testSlices(self):
		"""Test UCharVectorData slicing behavior"""
		BaseVectorDataTest.testSlices(self)
			
	def testEquality(self):
		"""Test UCharVectorData equality function"""
		BaseVectorDataTest.testEquality(self)

	def testComparison(self):
		"""Test UCharVectorData comparison function"""
		BaseVectorDataTest.testComparison(self)
				
	def testSumOperations(self):
		"""Test UCharVectorData sum operations"""
		BaseVectorDataTest.testSumOperations(self)
		
	def testSubOperations(self):
		"""Test UCharVectorData subtraction operations"""
		BaseVectorDataTest.testSubOperations(self)
		
	def testMultOperations(self):
		"""Test UCharVectorData multiplication operations"""
		BaseVectorDataTest.testMultOperations(self)
				
	def testDivOperations(self):
		"""Test UCharVectorData division operations"""
		BaseVectorDataTest.testDivOperations(self)
				
	def testByValueItem(self):
		"""Test UCharVectorData by value return type"""
		BaseVectorDataTest.testByValueItem(self)

class StringVectorDataTest(BaseVectorDataTest,unittest.TestCase):

	def __init__(self, param1):
		BaseVectorDataTest.__init__(self, StringVectorData, str)
		unittest.TestCase.__init__(self, param1)

	def testConstructors(self):
		"""Test StringVectorData constructors"""
		BaseVectorDataTest.testConstructors(self)
		
	def testResize(self):
		"""Test StringVectorData resizing"""
		BaseVectorDataTest.testResize(self)
				
	def testAssignment(self):
		"""Test StringVectorData assignment"""
		BaseVectorDataTest.testAssignment(self)
				
	def testNegativeIndexing(self):
		"""Test StringVectorData negative indexing"""
		BaseVectorDataTest.testNegativeIndexing(self)
				
	def testCopyOnWrite(self):
		"""Test StringVectorData copy-on-write behavior"""
		BaseVectorDataTest.testCopyOnWrite(self)
				
	def testContains(self):
		"""Test StringVectorData contains function"""
		BaseVectorDataTest.testContains(self)
				
	def testExtend(self):
		"""Test StringVectorData extend function"""
		BaseVectorDataTest.testExtend(self)
			
	def testSlices(self):
		"""Test StringVectorData slicing behavior"""
		BaseVectorDataTest.testSlices(self)
			
	def testEquality(self):
		"""Test StringVectorData equality function"""
		BaseVectorDataTest.testEquality(self)

	def testComparison(self):
		"""Test StringVectorData comparison function"""
		pass
				
	def testSumOperations(self):
		"""Test StringVectorData sum operations"""
		pass
		
	def testSubOperations(self):
		"""Test StringVectorData subtraction operations"""
		pass
		
	def testMultOperations(self):
		"""Test StringVectorData multiplication operations"""
		pass
				
	def testDivOperations(self):
		"""Test StringVectorData division operations"""
		pass
				
	def testByValueItem(self):
		"""Test StringVectorData by value return type"""
		BaseVectorDataTest.testByValueItem(self)	

class TestVectorDataStrRepr( unittest.TestCase ) :

	def test( self ) :
	
		l = [ V3f( 2 ), V3f( 4 ), V3f( 5 ) ]
		d = V3fVectorData( l )
		self.assertEqual( " ".join( [str(x) for x in l] ), str( d ) )
		self.assertEqual( "V3fVectorData( [ " + ", ".join( [repr( x ) for x in l] ) + " ] )", repr( d ) )
		
		l = [ "one", "two", "three" ]
		d = StringVectorData( l )
		self.assertEqual( " ".join( l ), str( d ) )
		self.assertEqual( "StringVectorData( [ " + ", ".join( ["\""+x+"\"" for x in l] ) + " ] )", repr( d ) )
		
		l = [ 1, 2, 3 ]
		d = IntVectorData( l )
		self.assertEqual( " ".join( [str(x) for x in l] ), str( d ) )
		self.assertEqual( "IntVectorData( [ " + ", ".join( [str(x) for x in l] ) + " ] )", repr( d ) )
	
if __name__ == "__main__":
    unittest.main()   

