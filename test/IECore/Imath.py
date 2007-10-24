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

"""Unit test for Imath binding"""

import math
import unittest
import random
from IECore import *

class ImathV2f(unittest.TestCase):

	def testConstructors(self):
		"""Test V2f constructors"""
		v = V2f()
		v = V2f(1)
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)
		v = V2f(2, 3)
		self.assertEqual(v.x, 2)
		self.assertEqual(v.y, 3)
		
	def testDimensions(self):
		"""Test V2f dimensions"""
		
		v = V2f()
		self.assertEqual( v.dimensions(), 2 )	
	
	def testIndexing(self):
		"""Test V2f indexing via operator[]"""
		v1 = V2f(1.0, 2.0)
		
		self.assertEqual(v1[0], 1.0)
		self.assertEqual(v1[1], 2.0)	
		
		v1[0] = 12.0
		v1[1] = 15.0
		
		self.assertEqual(v1[0], 12.0)
		self.assertEqual(v1[1], 15.0)	
	
		
	def testCopyAndAssign(self):
		"""Test V2f copy construction and assignment"""
		v1 = V2f(2.0)
		v2 = V2f(3.0)
		
		v2 = v1
		# v2 should now contain contents of v1, v1 should be unchanged
		self.assertEqual(v2.x, 2)
		self.assertEqual(v2.y, 2)
		self.assertEqual(v1.x, 2)
		self.assertEqual(v1.y, 2)
		
			
		
	def testEquality(self):
		"""Test V2f comparison for equality"""
		v1 = V2f(1.00)
		v2 = V2f(1.01)
		
		self.assert_( v1.equalWithAbsError(v2, 0.01) )
		
		v1 = V2f(2.0)
		v2 = V2f(3.0)
		
		self.assert_( v1.equalWithRelError(v2, 0.5) )
		
		v1 = V2f(1.0)
		v2 = V2f(1.0)
		self.assert_( v1 == v2 )
		v1 = V2f(1.0)
		v2 = V2f(1.1)
		
		self.assert_( v1 != v2 )
		
	def testDotProduct(self):
		"""Test V2f dot product"""
		v1 = V2f(3.0)
		v2 = V2f(4.0)
		# By definition
		self.assertEqual( v1.dot(v2), 3*4 + 3*4)
		
		# Commutative
		self.assertEqual( v1 ^ v2, v2 ^ v1)
		self.assertEqual( v1.dot(v2), v2.dot(v1) )
		
		# Operator/method equivalence
		self.assertEqual( v1.dot(v2), v1 ^ v2)		
		
		# Because cos( angleBetween(v1, v2) ) == 1:
		self.assertAlmostEqual( v1 ^ v2, v1.length() * v2.length(), 3 )
		
		
	def testCrossProduct(self):
		"""Test V2f cross product"""
		v1 = V2f(2.0, 2.0)
		v2 = V2f(0.0, 2.0)
		
		# Area of parallelogram, by definition
		self.assertEqual( v1.cross(v2), 4.0 )
		
		# Operator/method equivalence
		self.assertEqual( v1.cross(v2), v1 % v2 )
				
		# ImathVec.h comment validity
		self.assertEqual( v1.cross(v2), (V3f(v1.x, v1.y, 0.0) % V3f(v2.x, v2.y, 0.0)).z )
		
	def testOperators(self):
		"""Test V2f arithmetic operators"""
		v1 = V2f(3.4,  9.2)
		v2 = V2f(5.3, -0.4)
		
		# ADDITION
		
		# By definition
		self.assertEqual( v1 + v2, V2f( v1.x + v2.x, v1.y + v2.y ) )
		
		# Commutative
		self.assertEqual( v1 + v2, v2 + v1 )
		
		# Assignment
		v1_copy = V2f(v1)
		temp = v1
		temp += v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, (v1_copy + v2))
		
		# SUBTRACTION
		
		# By definition
		self.assertEqual( v1 - v2, V2f( v1.x - v2.x, v1.y - v2.y ) )				
		self.assertEqual( v1 - v2, -v2 + v1 )
		
		# Assignment
		v1_copy = V2f(v1)
		temp = v1
		temp -= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy - v2)
		
		# NEGATION
		
		self.assertEqual( -v1, V2f( -v1.x, -v1.y) )
		self.assertEqual( -v1, v1.negate() )
		self.assertEqual( -( -v1), v1 )
		
		# MULTIPLICATION
		
		# By definition
		self.assertEqual( v1 * v2, V2f(v1.x * v2.x, v1.y * v2.y) )
		c = 3
		self.assertEqual( v1 * c, V2f(v1.x * c, v1.y * c) )		
		
		# Commutative
		self.assertEqual( v1 * v2, v2 * v1 )
		self.assertEqual( c * v1, V2f(v1.x * c, v1.y * c) )
		
		# Assignment
		v1_copy = V2f(v1)
		temp = v1
		temp *= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy * v2)
		
		v1_copy = V2f(v1)
		temp = v1
		temp *= c
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy * c)
		
		# DIVISION
		
		# By definition
		self.assertEqual( v1 / v2, V2f(v1.x / v2.x, v1.y / v2.y) )		
		self.assertEqual( v1 / c, V2f(v1.x / c, v1.y / c) )
		
		# Assignment
		v1_copy = V2f(v1)
		temp = v1
		temp /= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy / v2)
		
		v1_copy = V2f(v1)
		temp = v1
		temp /= c
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy / c)
		
		# matrix multiplication
		
		v1 = V2f( 1, 2 )
		m = M33f.createTranslated( V2f( 1, 2 ) )
		v2 = v1 * m
		v1 *= m
		self.assertEqual( v1, v2 )
		self.assertEqual( v1, V2f( 2, 4 ) )
		

	def testMiscMethods(self):
		"""Test V2f miscellaneous methods"""
		v1 = V2f(2.3, -4.98)
		
		self.assertAlmostEqual( v1.length2(), v1.dot(v1), 3 )
		self.assertAlmostEqual( v1.length(), math.sqrt(v1.dot(v1)), 3 )
		self.assertAlmostEqual( v1.length() * v1.length(), v1.length2(), 3 )
		
		v1 = V2f(10.0, 0.0)
		self.assertEqual( v1.normalized(), v1 / v1.length() )
		self.assertEqual( v1, V2f(10.0, 0.0) )
		
		v1.normalize()
		self.assertEqual( v1, V2f(1.0, 0.0) )
		

class ImathV3f(unittest.TestCase):

	def testConstructors(self):
		"""Test V3f constructors"""
		v = V3f()
		v = V3f(1)
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)
		self.assertEqual(v.z, 1)
		v = V3f(2, 3, 4)
		self.assertEqual(v.x, 2)
		self.assertEqual(v.y, 3)
		self.assertEqual(v.z, 4)
		
	def testDimensions(self):
		"""Test V3f dimensions"""
		
		v = V3f()
		self.assertEqual( v.dimensions(), 3 )	
	
	
	def testIndexing(self):
		"""Test V3f indexing via operator[]"""
		v1 = V3f(1.0, 2.0, 3.0)
		
		self.assertEqual(v1[0], 1.0)
		self.assertEqual(v1[1], 2.0)
		self.assertEqual(v1[2], 3.0)
		
		v1[0] = 12.0
		v1[1] = 15.0
		v1[2] = -25.0
		
		self.assertEqual(v1[0], 12.0)
		self.assertEqual(v1[1], 15.0)
		self.assertEqual(v1[2], -25.0)
	
		
	def testCopyAndAssign(self):
		"""Test V3f copy construction and assignment"""
		v1 = V3f(2.0)
		v2 = V3f(3.0)
		
		v2 = v1
		# v2 should now contain contents of v1, v1 should be unchanged
		self.assertEqual(v2.x, 2)
		self.assertEqual(v2.y, 2)
		self.assertEqual(v2.z, 2)
		self.assertEqual(v1.x, 2)
		self.assertEqual(v1.y, 2)
		self.assertEqual(v1.z, 2)
		
			
		
	def testEquality(self):
		"""Test V3f comparison for equality"""
		v1 = V3f(1.00)
		v2 = V3f(1.01)
		
		self.assert_( v1.equalWithAbsError(v2, 0.01) )
		
		v1 = V3f(2.0)
		v2 = V3f(3.0)
		
		self.assert_( v1.equalWithRelError(v2, 0.5) )
		
		v1 = V3f(1.0)
		v2 = V3f(1.0)
		self.assert_( v1 == v2 )
		v1 = V3f(1.0)
		v2 = V3f(1.1)
		
		self.assert_( v1 != v2 )
		
	def testDotProduct(self):
		"""Test V3f dot product"""
		v1 = V3f(3.0)
		v2 = V3f(4.0)
		# By definition
		self.assertEqual( v1.dot(v2), 3*4 + 3*4 + 3*4)
		
		# Commutative
		self.assertEqual( v1 ^ v2, v2 ^ v1)
		self.assertEqual( v1.dot(v2), v2.dot(v1) )
		
		# Operator/method equivalence
		self.assertEqual( v1.dot(v2), v1 ^ v2)		
		
		# Because cos( angleBetween(v1, v2) ) == 1:
		self.assertAlmostEqual( v1 ^ v2, v1.length() * v2.length(), 3 )
		
		
	def testCrossProduct(self):
		"""Test V3f cross product"""
		v1 = V3f(1.0, 0.0, 0.0)
		v2 = V3f(0.0, 1.0, 0.0)
		
		# Area of "parallelogram", by definition
		self.assertEqual( v1.cross(v2), V3f(0.0, 0.0, 1.0) )
		
		# Operator/method equivalence
		self.assertEqual( v1.cross(v2), v1 % v2 )
		
	def testOperators(self):
		"""Test V3f arithmetic operators"""
		v1 = V3f(3.4,  9.2, 18.05)
		v2 = V3f(5.3, -0.4, -5.7 )
		
		# ADDITION
		
		# By definition
		self.assertEqual( v1 + v2, V3f( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z ) )
		
		# Commutative
		self.assertEqual( v1 + v2, v2 + v1 )
		
		# Assignment
		v1_copy = V3f(v1)
		temp = v1
		temp += v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, (v1_copy + v2))
		
		# SUBTRACTION
		
		# By definition
		self.assertEqual( v1 - v2, V3f( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z ) )
		self.assertEqual( v1 - v2, -v2 + v1 )
		
		# Assignment
		v1_copy = V3f(v1)
		temp = v1
		temp -= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy - v2)
		
		# NEGATION
		
		self.assertEqual( -v1, V3f( -v1.x, -v1.y, -v1.z) )
		self.assertEqual( -v1, v1.negate() )
		self.assertEqual( -( -v1), v1 )
		
		# MULTIPLICATION
		
		# By definition
		self.assertEqual( v1 * v2, V3f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z ) )
		c = 3
		self.assertEqual( v1 * c, V3f(v1.x * c, v1.y * c, v1.z * c) )		
		
		# Commutative
		self.assertEqual( v1 * v2, v2 * v1 )
		self.assertEqual( c * v1, V3f(v1.x * c, v1.y * c, v1.z * c) )
		
		# Assignment
		v1_copy = V3f(v1)
		temp = v1
		temp *= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy * v2)
		
		v1_copy = V3f(v1)
		temp = v1
		temp *= c
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy * c)
		
		# DIVISION
		
		# By definition
		self.assertEqual( v1 / v2, V3f(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z) )		
		self.assertEqual( v1 / c, V3f(v1.x / c, v1.y / c, v1.z / c) )
		
		# Assignment
		v1_copy = V3f(v1)
		temp = v1
		temp /= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy / v2)
		
		v1_copy = V3f(v1)
		temp = v1
		temp /= c
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy / c)
		
		# matrix multiplication
		
		v1 = V3f( 1, 2, 3 )
		m = M44f.createTranslated( V3f( 1, 2, 3 ) )
		v2 = v1 * m
		v1 *= m
		self.assertEqual( v1, v2 )
		self.assertEqual( v1, V3f( 2, 4, 6 ) )

	def testMiscMethods(self):
		"""Test V3f miscellaneous methods"""
		v1 = V3f(41.4, 2.3, -4.98)
		
		self.assertAlmostEqual( v1.length2(), v1.dot(v1), 3 )
		self.assertAlmostEqual( v1.length(), math.sqrt(v1.dot(v1)), 3 )
		self.assertAlmostEqual( v1.length() * v1.length(), v1.length2(), 3 )
		
		v1 = V3f(10.0, 0.0, 0.0)
		self.assertEqual( v1.normalized(), v1 / v1.length() )
		self.assertEqual( v1, V3f(10.0, 0.0, 0.0) )
		
		v1.normalize()
		self.assertEqual( v1, V3f(1.0, 0.0, 0.0) )
		
class ImathBox3f(unittest.TestCase):
	def testConstructors(self):
		"""Test Box3f constructors"""
		b = Box3f()
		self.assert_( b.isEmpty() )
		
		b = Box3f( V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.min, V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.max, V3f(1.0, 1.0, 1.0) )
		
		b = Box3f( V3f(-1.0, -1.0, -1.0), V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.min, V3f(-1.0, -1.0, -1.0) )
		self.assertEqual( b.max, V3f( 1.0,  1.0,  1.0) )
		
	def testEquality(self):
		"""Test Box3f comparison for equality"""
		
		b1 = Box3f( V3f(1.0, 2.0, 3.0) )
		b2 = Box3f( V3f(1.0, 2.0, 3.0) )
		self.assert_( b1 == b2 )
		
		b2 = Box3f( V3f(3.0, 2.0, 1.0) )
		self.assert_( b1 != b2 )
		
	def testMiscMethods(self):
		"""Test Box3f miscellaneous methods"""
		
		b1 = Box3f( V3f(-1.0, -1.0, -1.0), V3f(2.0, 2.0, 2.0) )
		self.assertEqual( b1.isEmpty(), False )
		self.assert_( b1.hasVolume() )
		
		b1.makeEmpty()
		self.assert_( b1.isEmpty() )
		self.assertEqual( b1.hasVolume(), False )
		
		b1 = Box3f( V3f(-1.0, -1.0, -1.0), V3f(10.0, 2.0, 2.0) )
		
		X_AXIS = 0
		self.assertEqual( b1.majorAxis(), X_AXIS )
		
		self.assertEqual( b1.center(), (b1.min + b1.max) / 2.0 )
		
		b2 = Box3f( V3f(-0.5), V3f(1.0) )
		self.assert_( b2.intersects(b1) )
		
		b2 = Box3f( V3f(-5.0), V3f(-2.0) )
		self.failIf( b2.intersects(b1) )
				
		self.assertEqual( b2.size(), b2.max - b2.min )

		b = Box3f( V3f(1), V3f(2) )
		m = M44f()
		m[0,0]=2
		m[1,1]=2
		m[2,2]=2
		self.assertEqual( b.transform( m ), Box3f( V3f(2), V3f(4) ) )
		m = M44d()
		m[0,0]=2
		m[1,1]=2
		m[2,2]=2
		self.assertEqual( b.transform( m ), Box3f( V3f(2), V3f(4) ) )
		
class ImathQuatf(unittest.TestCase):
	def testConstructors(self):
		"""Test Quatf constructors"""
		
		q = Quatf()
		q = Quatf(q)
		q = Quatf(0.1, 0.2, 0.3, 0.4)
		q = Quatf(0.1, V3f(0.2, 0.3, 0.4))
		q = Quatf.identity()
		self.assertEqual( q, Quatf(1,0,0,0) )
		
	def testIndexing(self):
		"""Test Quatf indexing via operator[]"""
		
		q = Quatf( 1, 2, 3, 4 )
		
		self.assertEqual( q[0], 1 )
		self.assertEqual( q[1], 2 )
		self.assertEqual( q[2], 3 )
		self.assertEqual( q[3], 4 )
		
		self.assertEqual( q[0], q.r )
		self.assertEqual( q[1], q.v.x )
		self.assertEqual( q[2], q.v.y )
		self.assertEqual( q[3], q.v.z )
		
	def testEquality(self):
		"""Test Quatf comparison for equality"""
		
		q1 = Quatf( 1, 2, 3, 4 )
		q2 = Quatf( 1, 2, 3, 4 )		
		self.assertEqual(q1, q1)
		self.assertEqual(q1, q2)
		
		q2 = Quatf( 5, 2, 3, 4 )
		self.assert_( q1 != q2 )
			
	def testMiscMethods(self):
		"""Test Quatf miscellaneous methods"""
		
		q1 = Quatf( 1, 2, 3, 4 )
		self.assertAlmostEqual( q1.length(), math.sqrt(q1[0]*q1[0]+(q1.v^q1.v)), 3 )
		
		# axis/angle
		axis = V3f( 1, 2, 3 )
		axis.normalize()
				
		q1.setAxisAngle( axis, 0.5 )
		self.assertAlmostEqual( q1.axis().x, axis.x, 3 )
		self.assertAlmostEqual( q1.axis().y, axis.y, 3 )
		self.assertAlmostEqual( q1.axis().z, axis.z, 3 )
		
		self.assertAlmostEqual( q1.angle(), 0.5, 3 )
		
		# Rotate x axis onto y axis
		q1.setRotation( V3f(1,0,0), V3f(0,1,0) )
		
		#We should have gone 90 degrees about the +ve z-axis
		self.assertAlmostEqual( q1.angle(), 90.0 * math.pi / 180.0, 3 )
		
		self.assertAlmostEqual( q1.axis().x, 0.0, 3 )
		self.assertAlmostEqual( q1.axis().y, 0.0, 3 )
		self.assertAlmostEqual( q1.axis().z, 1.0, 3 )
		
		#inversion
		q1 = Quatf( 1, 2, 3, 4 )
		qdot = q1 ^ q1
		qi_test = Quatf( q1.r / qdot, -q1.v / qdot)
		qi = q1.inverse()
		
		self.assertAlmostEqual(qi[0], qi_test[0], 3)
		self.assertAlmostEqual(qi[1], qi_test[1], 3)
		self.assertAlmostEqual(qi[2], qi_test[2], 3)
		self.assertAlmostEqual(qi[3], qi_test[3], 3)
		
		q1.invert()
		self.assertAlmostEqual(qi[0], qi_test[0], 3)
		self.assertAlmostEqual(qi[1], qi_test[1], 3)
		self.assertAlmostEqual(qi[2], qi_test[2], 3)
		self.assertAlmostEqual(qi[3], qi_test[3], 3)
		
		#slerp 
		q2 = Quatf( 0.5, 0.6, 0.7, 0.8 )
		qs = slerp(q1, q2, 0.5)
		
		# normalization
		qn = qi.normalized()
		qn_test = Quatf( qi.r / qi.length(), qi.v / qi.length() )
		
		self.assertAlmostEqual(qn[0], qn_test[0], 3)
		self.assertAlmostEqual(qn[1], qn_test[1], 3)
		self.assertAlmostEqual(qn[2], qn_test[2], 3)
		self.assertAlmostEqual(qn[3], qn_test[3], 3)
		
		qn = qi.normalize()
		self.assertAlmostEqual(qn[0], qn_test[0], 3)
		self.assertAlmostEqual(qn[1], qn_test[1], 3)
		self.assertAlmostEqual(qn[2], qn_test[2], 3)
		self.assertAlmostEqual(qn[3], qn_test[3], 3)
		
		#matrix conversion
		fromDir = V3f(1,0,0)
		toDir = V3f(0,1,0) 
		q1.setRotation( fromDir, toDir )
		m = q1.toMatrix33()				
		m = q1.toMatrix44()
		
	def testOperators(self):
		"""Test Quatf operators"""
		
		q1 = Quatf( 1, 2, 3, 4 )
		q2 = Quatf( 5, 6, 7, 8 )
		self.assertAlmostEqual( q1 ^ q2, q1.r * q2.r + (q1.v ^ q2.v ), 3 )

class ImathM33f(unittest.TestCase):
	def testConstructors(self):
		"""Test M33f constructors"""
		m = M33f()
		
		m = M33f(2)
		
		m = M33f(1, 0, 0,
		              0, 1, 0,
                              0, 0, 1);

	def testDimensions(self):
		"""Test M33f dimensions"""
		
		m1 = M33f()
		d = m1.dimensions()	
		self.assertEqual( d[0], 3 )
		self.assertEqual( d[1], 3 )
		
	def testCopyAndAssign(self):
		"""Test M33f copy construction and assignment"""
		
		m1 = M33f()
		m2 = M33f(m1)
		self.failIf(m1 is m2)
		
	def testIndexing(self):
		"""Test M33f indexing via [] operator"""
		
	def testOperators(self):
		"""Test M33f operators"""
		
		x = 10
		y = 2
		m1 = M33f(x)
		m2 = M33f(y)
		
		self.assertEqual(m1 + m2, M33f(x + y))
		self.assertEqual(m1 - m2, M33f(x - y))
		self.assertEqual(m1 * y, M33f(x * y))
		self.assertEqual(m1 / y, M33f(x / y))
		
	def testMiscellaneousMethods(self):
		"""Test M33f miscellaneous methods"""
		
		m1 = M33f()
		m1.makeIdentity()
		
		m1 = M33f(3)
		m2 = M33f(3.1)
		self.assert_( m1.equalWithAbsError(m2, 0.1) )
		
		m1 = M33f(2)
		m2 = M33f(3)
		self.assert_( m1.equalWithRelError(m2, 0.51) )
		
		m1 = M33f(1, 0, 0,
		               0, 2, 0,
			       0, 0, 3)
		self.assertEqual( m1.transposed().transposed(), m1)
		
		
		
	def testEquality(self):
		"""Test M33f comparison for equality"""
		m1 = M33f(3)
		m2 = M33f(3)
		
		self.assertEqual(m1, m2)
		
	def testCreate(self ) :
	
		self.assertEqual( M33f(), M33f.createScaled( V2f( 1 ) ) )
		
		m = M33f()
		m.scale( V2f( 2, 3 ) )
		self.assertEqual( m, M33f.createScaled( V2f( 2, 3 ) ) )
		
		self.assertEqual( M33f(), M33f.createTranslated( V2f( 0 ) ) )
		
		m = M33f()
		m.translate( V2f( 2, 3 ) )
		self.assertEqual( m, M33f.createTranslated( V2f( 2, 3 ) ) )
		
		self.assertEqual( M33f(), M33f.createRotated( 0 ) )

		m = M33f()
		m.rotate( 2 )
		self.assertEqual( m, M33f.createRotated( 2 ) )
		
	def testMultMethods( self ) :
	
		v = M33f.createTranslated( V2f( 1, 2 ) ).multVecMatrix( V2f( 0 ) )
		self.assertEqual( v, V2f( 1, 2 ) )
		
		v = M33f.createTranslated( V2f( 1, 2 ) ).multDirMatrix( V2f( 1 ) )
		self.assertEqual( v, V2f( 1 ) )	
		
class ImathM44f(unittest.TestCase):		
	def testConstructors(self):
		"""Test M44f constructors"""
		m = M44f(1., 0., 0., 0.,
		              0., 1., 0., 0.,
			      0., 0., 1., 0.,
			      0., 0., 0., 1.);
			      
		m3 = M33f(1., 0., 0.,
		               0., 1., 0.,
			       0., 0., 1.)
			      
		t = V3f(5., 5., 5.)
		
		m = M44f(m3, t)
		
	def testDimensions(self):
		"""Test M44f dimensions"""
		
		m1 = M44f()
		d = m1.dimensions()	
		self.assertEqual( d[0], 4 )
		self.assertEqual( d[1], 4 )
		
	def testCopyAndAssign(self):
		"""Test M44f copy construction and assignment"""
		m1 = M44f()
		m2 = M44f(m1)
		self.failIf(m1 is m2)
		
		m1 = m2
				
	def testIndexing(self):
		"""Test M44f indexing via [] operator"""
		pass
		
	def testOperators(self):
		"""Test M44f operators"""
		x = 10
		y = 2
		m1 = M44f(x)
		m2 = M44f(y)
		
		self.assertEqual(m1 + m2, M44f(x + y))
		self.assertEqual(m1 - m2, M44f(x - y))
		self.assertEqual(m1 * y, M44f(x * y))
		self.assertEqual(m1 / y, M44f(x / y))
		
	def testMiscellaneousMethods(self):
		"""Test M44f miscellaneous methods"""
		m1 = M44f()
		m1.makeIdentity()
		
		m1 = M44f(3)
		m2 = M44f(3.1)
		self.assert_( m1.equalWithAbsError(m2, 0.1) )
		
		m1 = M44f(2)
		m2 = M44f(3)
		self.assert_( m1.equalWithRelError(m2, 0.51) )
		
		m1 = M44f(1, 0, 0, 0,
		               0, 2, 0, 0,
			       0, 0, 3, 0,
			       0, 0, 0, 4)
		self.assertEqual( m1.transposed().transposed(), m1)
		
	def testEquality(self):
		"""Test M44f comparison for equality"""
		
		m1 = M44f(3)
		m2 = M44f(3)
		
		self.assertEqual(m1, m2)
		
	def testCreate(self ) :
	
		self.assertEqual( M44f(), M44f.createScaled( V3f( 1 ) ) )
		
		m = M44f()
		m.scale( V3f( 2, 3, 4 ) )
		self.assertEqual( m, M44f.createScaled( V3f( 2, 3, 4 ) ) )
		
		self.assertEqual( M44f(), M44f.createTranslated( V3f( 0 ) ) )
		
		m = M44f()
		m.translate( V3f( 2, 3, 4 ) )
		self.assertEqual( m, M44f.createTranslated( V3f( 2, 3, 4 ) ) )
		
		self.assertEqual( M44f(), M44f.createRotated( V3f( 0 ) ) )

		m = M44f()
		m.rotate( V3f( 1, 2, 3 ) )
		self.assertEqual( m, M44f.createRotated( V3f( 1, 2, 3 ) ) )
		
		m = M44f.createAimed( V3f( 1, 0, 0  ), V3f( 0, 1, 0 ) )
		self.assert_( V3f( 0, 1, 0 ).equalWithAbsError( V3f( 1, 0, 0 ) * m, 0.0000001 ) )
		
		m = M44f.createAimed( V3f( 1, 0, 0 ), V3f( 0, 0, 1 ), V3f( 0, 1, 0 ) )
		self.assert_( V3f( 0, 0, 1 ).equalWithAbsError( V3f( 1, 0, 0 ) * m, 0.0000001 ) )
		self.assert_( V3f( 0, 1, 0 ).equalWithAbsError( V3f( 0, 1, 0 ) * m, 0.0000001 ) )
		
	def testMultMethods( self ) :
	
		v = M44f.createTranslated( V3f( 1, 2, 3 ) ).multVecMatrix( V3f( 0 ) )
		self.assertEqual( v, V3f( 1, 2, 3 ) )
		
		v = M44f.createTranslated( V3f( 1, 2, 3 ) ).multDirMatrix( V3f( 1 ) )
		self.assertEqual( v, V3f( 1 ) )
		
	def testFromBasis( self ) :
	
		for i in range( 0, 10000 ) :
		
			m = M44f()
			m.translate( V3f( random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ) ) )
			m.rotate( V3f( random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ) ) )
			m.scale( V3f( random.uniform( -100, 100 ), random.uniform( -100, 100 ), random.uniform( -100, 100 ) ) )

			x = m.multDirMatrix( V3f( 1, 0, 0 ) )
			y = m.multDirMatrix( V3f( 0, 1, 0 ) )
			z = m.multDirMatrix( V3f( 0, 0, 1 ) )
			o = V3f( 0, 0, 0 ) * m
			
			self.assertEqual( matrixFromBasis( x, y, z, o ), m )

class ImathColor3Test( unittest.TestCase ) :

	def test( self ) :
	
		c = Color3f( 1 )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 1 )
		self.assertEqual( c.b, 1 )
		
		c = Color3f( 1, 2, 3 )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 2 )
		self.assertEqual( c.b, 3 )
		
		cc = Color3f( c )
		self.assertEqual( c, cc )
		
		cm = -c * 2
		self.assertEqual( cm.r, -2 )
		self.assertEqual( cm.g, -4 )
		self.assertEqual( cm.b, -6 )
		
		cm *= c
		self.assertEqual( cm.r, -2 )
		self.assertEqual( cm.g, -8 )
		self.assertEqual( cm.b, -18 )
		
		cm -= Color3f( 2 )
		self.assertEqual( cm, Color3f( -4, -10, -20 ) )
		
		self.assertEqual( c.dimensions(), 3 )
		
class ImathEulerfTest( unittest.TestCase ) :

	def testConstructors(self):
		"""Test Eulerf constructors"""
		
		#
		e = Eulerf()
		self.assertEqual( e.x, 0 )
		self.assertEqual( e.y, 0 )
		self.assertEqual( e.z, 0 )
		
		self.assertEqual( e.order(), Eulerf.Order.Default )
		self.assertEqual( e.order(), Eulerf.Order.XYZ )
		
		#
		ecopy = Eulerf(e)
		self.assertEqual( ecopy.x, 0 )
		self.assertEqual( ecopy.y, 0 )
		self.assertEqual( ecopy.z, 0 )
		
		self.assertEqual( ecopy.order(), Eulerf.Order.Default )
		self.assertEqual( ecopy.order(), Eulerf.Order.XYZ )
		
		#
		e = Eulerf( Eulerf.Order.ZYX )
		self.assertEqual( e.order(), Eulerf.Order.ZYX )
		
		#
		e = Eulerf( V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), Eulerf.Order.Default )
		self.assertEqual( e.order(), Eulerf.Order.XYZ )
		
		e = Eulerf( V3f( 0, 0, 0 ), Eulerf.Order.ZYX )
		self.assertEqual( e.order(), Eulerf.Order.ZYX )
		
		#		
		e = Eulerf( 0, 0, 0 )
		e = Eulerf( V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), Eulerf.Order.Default )
		self.assertEqual( e.order(), Eulerf.Order.XYZ )
		
		e = Eulerf( 0, 0, 0, Eulerf.Order.ZYX  )
		self.assertEqual( e.order(), Eulerf.Order.ZYX )	
		
		e = Eulerf( 0, 0, 0, Eulerf.Order.ZYX, Eulerf.InputLayout.XYZLayout )						
		self.assertEqual( e.order(), Eulerf.Order.ZYX )	
		
		#
		e = Eulerf( M33f() )
		e = Eulerf( V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), Eulerf.Order.Default )
		self.assertEqual( e.order(), Eulerf.Order.XYZ )
		
		e = Eulerf( M33f(), Eulerf.Order.ZYX )
		self.assertEqual( e.order(), Eulerf.Order.ZYX )		
		
		#
		e = Eulerf( M44f() )
		e = Eulerf( V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), Eulerf.Order.Default )
		self.assertEqual( e.order(), Eulerf.Order.XYZ )
		
		e = Eulerf( M44f(), Eulerf.Order.ZYX )
		self.assertEqual( e.order(), Eulerf.Order.ZYX )	
							
		
	def testOrder(self):
		"""Test Eulerf order"""
	
		self.assertEqual( len( Eulerf.Order.values ), 24 )
	
		e = Eulerf()
		
		for order in Eulerf.Order.values.values():
			self.assert_( Eulerf.legal( order ) )
			
			e.setOrder( order )
			
			self.assertEqual( e.order(), order )
			
	def testMisc(self):
		"""Test Eulerf miscellaneous"""
	
		self.assertEqual( len(Eulerf.Axis.values), 3 )		
		self.assertEqual( len(Eulerf.InputLayout.values), 2 )
		
		self.assert_( V3f in Eulerf.__bases__ )
		
	def testExtract(self):
	
		"""Test Eulerf extract"""
	
		e = Eulerf()
		e.extract( M33f() )
		
		e.extract( M44f() )
		
		e.extract( Quatf() )
		
		m = e.toMatrix33()
		m = e.toMatrix44()
		q = e.toQuat()
		v = e.toXYZVector()					
		
	def testAngleOrder(self):
	
		"""Test Eulerf angleOrder"""
	
		e = Eulerf()
		
		o = e.angleOrder()
		
		self.assert_( type(o) is tuple )
		self.assertEqual( len(o), 3 )
		
	def testAngleMapping(self):
	
		"""Test Eulerf angleMapping"""
	
		e = Eulerf()
		
		m = e.angleMapping()
		
		self.assert_( type(m) is tuple )
		self.assertEqual( len(m), 3 )			
			
		
	def testStr(self):
		"""Test Eulerf str"""
		
		e = Eulerf()
		self.assertEqual( str(e), "0 0 0" )
		
	def testRepr(self):
		"""Test Eulerf repr"""	
		
		e = Eulerf()
		self.assertEqual( repr(e), "Eulerf( 0, 0, 0 )" )
		
if __name__ == "__main__":
    unittest.main()   

