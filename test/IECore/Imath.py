##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

import IECore

class ImathV2f(unittest.TestCase):

	def testConstructors(self):
		"""Test V2f constructors"""
		v = IECore.V2f()
		v = IECore.V2f(1)
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)
		v = IECore.V2f(2, 3)
		self.assertEqual(v.x, 2)
		self.assertEqual(v.y, 3)

		self.assertEqual( IECore.V2f( IECore.V2i( 1, 2 ) ), IECore.V2f( 1, 2 ) )
		self.assertEqual( IECore.V2f( IECore.V2f( 1, 2 ) ), IECore.V2f( 1, 2 ) )
		self.assertEqual( IECore.V2f( IECore.V2d( 1, 2 ) ), IECore.V2f( 1, 2 ) )

		self.assertEqual( IECore.V2d( IECore.V2i( 1, 2 ) ), IECore.V2d( 1, 2 ) )
		self.assertEqual( IECore.V2d( IECore.V2f( 1, 2 ) ), IECore.V2d( 1, 2 ) )
		self.assertEqual( IECore.V2d( IECore.V2d( 1, 2 ) ), IECore.V2d( 1, 2 ) )

		self.assertEqual( IECore.V2i( IECore.V2i( 1, 2 ) ), IECore.V2i( 1, 2 ) )
		self.assertEqual( IECore.V2i( IECore.V2f( 1, 2 ) ), IECore.V2i( 1, 2 ) )
		self.assertEqual( IECore.V2i( IECore.V2d( 1, 2 ) ), IECore.V2i( 1, 2 ) )

		self.assertEqual( IECore.V3f( IECore.V3i( 1, 2, 3 ) ), IECore.V3f( 1, 2, 3 ) )
		self.assertEqual( IECore.V3f( IECore.V3f( 1, 2, 3 ) ), IECore.V3f( 1, 2, 3 ) )
		self.assertEqual( IECore.V3f( IECore.V3d( 1, 2, 3 ) ), IECore.V3f( 1, 2, 3 ) )

		self.assertEqual( IECore.V3d( IECore.V3i( 1, 2, 3 ) ), IECore.V3d( 1, 2, 3 ) )
		self.assertEqual( IECore.V3d( IECore.V3f( 1, 2, 3 ) ), IECore.V3d( 1, 2, 3 ) )
		self.assertEqual( IECore.V3d( IECore.V3d( 1, 2, 3 ) ), IECore.V3d( 1, 2, 3 ) )

		self.assertEqual( IECore.V3i( IECore.V3i( 1, 2, 3 ) ), IECore.V3i( 1, 2, 3 ) )
		self.assertEqual( IECore.V3i( IECore.V3f( 1, 2, 3 ) ), IECore.V3i( 1, 2, 3 ) )
		self.assertEqual( IECore.V3i( IECore.V3d( 1, 2, 3 ) ), IECore.V3i( 1, 2, 3 ) )

		v = IECore.V2f( [ 1, 1 ] )
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)

		self.assertRaises( RuntimeError, IECore.V2f, [ 1 ] )
		self.assertRaises( RuntimeError, IECore.V2f, [ 1, 2, 3 ] )

	def testDimensions(self):
		"""Test V2f dimensions"""

		v = IECore.V2f()
		self.assertEqual( v.dimensions(), 2 )

	def testIndexing(self):
		"""Test V2f indexing via operator[]"""
		v1 = IECore.V2f(1.0, 2.0)

		self.assertEqual(v1[0], 1.0)
		self.assertEqual(v1[1], 2.0)

		v1[0] = 12.0
		v1[1] = 15.0

		self.assertEqual(v1[0], 12.0)
		self.assertEqual(v1[1], 15.0)


	def testCopyAndAssign(self):
		"""Test V2f copy construction and assignment"""
		v1 = IECore.V2f(2.0)
		v2 = IECore.V2f(3.0)

		v2 = v1
		# v2 should now contain contents of v1, v1 should be unchanged
		self.assertEqual(v2.x, 2)
		self.assertEqual(v2.y, 2)
		self.assertEqual(v1.x, 2)
		self.assertEqual(v1.y, 2)



	def testEquality(self):
		"""Test V2f comparison for equality"""
		v1 = IECore.V2f(1.00)
		v2 = IECore.V2f(1.01)

		self.assert_( v1.equalWithAbsError(v2, 0.01) )

		v1 = IECore.V2f(2.0)
		v2 = IECore.V2f(3.0)

		self.assert_( v1.equalWithRelError(v2, 0.5) )

		v1 = IECore.V2f(1.0)
		v2 = IECore.V2f(1.0)
		self.assert_( v1 == v2 )
		v1 = IECore.V2f(1.0)
		v2 = IECore.V2f(1.1)

		self.assert_( v1 != v2 )

	def testDotProduct(self):
		"""Test V2f dot product"""
		v1 = IECore.V2f(3.0)
		v2 = IECore.V2f(4.0)
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
		v1 = IECore.V2f(2.0, 2.0)
		v2 = IECore.V2f(0.0, 2.0)

		# Area of parallelogram, by definition
		self.assertEqual( v1.cross(v2), 4.0 )

		# Operator/method equivalence
		self.assertEqual( v1.cross(v2), v1 % v2 )

		# ImathVec.h comment validity
		self.assertEqual( v1.cross(v2), (IECore.V3f(v1.x, v1.y, 0.0) % IECore.V3f(v2.x, v2.y, 0.0)).z )

	def testOperators(self):
		"""Test V2f arithmetic operators"""
		v1 = IECore.V2f(3.4,  9.2)
		v2 = IECore.V2f(5.3, -0.4)

		# ADDITION

		# By definition
		self.assertEqual( v1 + v2, IECore.V2f( v1.x + v2.x, v1.y + v2.y ) )

		# Commutative
		self.assertEqual( v1 + v2, v2 + v1 )

		# Assignment
		v1_copy = IECore.V2f(v1)
		temp = v1
		temp += v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, (v1_copy + v2))

		# SUBTRACTION

		# By definition
		self.assertEqual( v1 - v2, IECore.V2f( v1.x - v2.x, v1.y - v2.y ) )
		self.assertEqual( v1 - v2, -v2 + v1 )

		# Assignment
		v1_copy = IECore.V2f(v1)
		temp = v1
		temp -= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy - v2)

		# NEGATION

		self.assertEqual( -v1, IECore.V2f( -v1.x, -v1.y) )
		self.assertEqual( -v1, v1.negate() )
		self.assertEqual( -( -v1), v1 )

		# MULTIPLICATION

		# By definition
		self.assertEqual( v1 * v2, IECore.V2f(v1.x * v2.x, v1.y * v2.y) )
		c = 3
		self.assertEqual( v1 * c, IECore.V2f(v1.x * c, v1.y * c) )

		# Commutative
		self.assertEqual( v1 * v2, v2 * v1 )
		self.assertEqual( c * v1, IECore.V2f(v1.x * c, v1.y * c) )

		# Assignment
		v1_copy = IECore.V2f(v1)
		temp = v1
		temp *= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy * v2)

		v1_copy = IECore.V2f(v1)
		temp = v1
		temp *= c
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy * c)

		# DIVISION

		# By definition
		self.assertEqual( v1 / v2, IECore.V2f(v1.x / v2.x, v1.y / v2.y) )
		self.assertEqual( v1 / c, IECore.V2f(v1.x / c, v1.y / c) )

		# Assignment
		v1_copy = IECore.V2f(v1)
		temp = v1
		temp /= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy / v2)

		v1_copy = IECore.V2f(v1)
		temp = v1
		temp /= c
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy / c)

		# matrix multiplication

		v1 = IECore.V2f( 1, 2 )
		m = IECore.M33f.createTranslated( IECore.V2f( 1, 2 ) )
		v2 = v1 * m
		v1 *= m
		self.assertEqual( v1, v2 )
		self.assertEqual( v1, IECore.V2f( 2, 4 ) )


	def testMiscMethods(self):
		"""Test V2f miscellaneous methods"""
		v1 = IECore.V2f(2.3, -4.98)

		self.assertAlmostEqual( v1.length2(), v1.dot(v1), 3 )
		self.assertAlmostEqual( v1.length(), math.sqrt(v1.dot(v1)), 3 )
		self.assertAlmostEqual( v1.length() * v1.length(), v1.length2(), 3 )

		v1 = IECore.V2f(10.0, 0.0)
		self.assertEqual( v1.normalized(), v1 / v1.length() )
		self.assertEqual( v1, IECore.V2f(10.0, 0.0) )

		v1.normalize()
		self.assertEqual( v1, IECore.V2f(1.0, 0.0) )


class ImathV3f(unittest.TestCase):

	def testConstructors(self):
		"""Test V3f constructors"""
		v = IECore.V3f()
		v = IECore.V3f(1)
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)
		self.assertEqual(v.z, 1)
		v = IECore.V3f(2, 3, 4)
		self.assertEqual(v.x, 2)
		self.assertEqual(v.y, 3)
		self.assertEqual(v.z, 4)

		v = IECore.V3f( [ 1, 1, 1 ] )
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)
		self.assertEqual(v.z, 1)

		self.assertRaises( RuntimeError, IECore.V3f, [ 1 ] )
		self.assertRaises( RuntimeError, IECore.V3f, [ 1, 2 ] )
		self.assertRaises( RuntimeError, IECore.V3f, [ 1, 2, 3, 4 ] )

	def testDimensions(self):
		"""Test V3f dimensions"""

		v = IECore.V3f()
		self.assertEqual( v.dimensions(), 3 )


	def testIndexing(self):
		"""Test V3f indexing via operator[]"""
		v1 = IECore.V3f(1.0, 2.0, 3.0)

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
		v1 = IECore.V3f(2.0)
		v2 = IECore.V3f(3.0)

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
		v1 = IECore.V3f(1.00)
		v2 = IECore.V3f(1.01)

		self.assert_( v1.equalWithAbsError(v2, 0.01) )

		v1 = IECore.V3f(2.0)
		v2 = IECore.V3f(3.0)

		self.assert_( v1.equalWithRelError(v2, 0.5) )

		v1 = IECore.V3f(1.0)
		v2 = IECore.V3f(1.0)
		self.assert_( v1 == v2 )
		v1 = IECore.V3f(1.0)
		v2 = IECore.V3f(1.1)

		self.assert_( v1 != v2 )

	def testDotProduct(self):
		"""Test V3f dot product"""
		v1 = IECore.V3f(3.0)
		v2 = IECore.V3f(4.0)
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
		v1 = IECore.V3f(1.0, 0.0, 0.0)
		v2 = IECore.V3f(0.0, 1.0, 0.0)

		# Area of "parallelogram", by definition
		self.assertEqual( v1.cross(v2), IECore.V3f(0.0, 0.0, 1.0) )

		# Operator/method equivalence
		self.assertEqual( v1.cross(v2), v1 % v2 )

	def testOperators(self):
		"""Test V3f arithmetic operators"""
		v1 = IECore.V3f(3.4,  9.2, 18.05)
		v2 = IECore.V3f(5.3, -0.4, -5.7 )

		# ADDITION

		# By definition
		self.assertEqual( v1 + v2, IECore.V3f( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z ) )

		# Commutative
		self.assertEqual( v1 + v2, v2 + v1 )

		# Assignment
		v1_copy = IECore.V3f(v1)
		temp = v1
		temp += v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, (v1_copy + v2))

		# SUBTRACTION

		# By definition
		self.assertEqual( v1 - v2, IECore.V3f( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z ) )
		self.assertEqual( v1 - v2, -v2 + v1 )

		# Assignment
		v1_copy = IECore.V3f(v1)
		temp = v1
		temp -= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy - v2)

		# NEGATION

		self.assertEqual( -v1, IECore.V3f( -v1.x, -v1.y, -v1.z) )
		self.assertEqual( -v1, v1.negate() )
		self.assertEqual( -( -v1), v1 )

		# MULTIPLICATION

		# By definition
		self.assertEqual( v1 * v2, IECore.V3f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z ) )
		c = 3
		self.assertEqual( v1 * c, IECore.V3f(v1.x * c, v1.y * c, v1.z * c) )

		# Commutative
		self.assertEqual( v1 * v2, v2 * v1 )
		self.assertEqual( c * v1, IECore.V3f(v1.x * c, v1.y * c, v1.z * c) )

		# Assignment
		v1_copy = IECore.V3f(v1)
		temp = v1
		temp *= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy * v2)

		v1_copy = IECore.V3f(v1)
		temp = v1
		temp *= c
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy * c)

		# DIVISION

		# By definition
		self.assertEqual( v1 / v2, IECore.V3f(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z) )
		self.assertEqual( v1 / c, IECore.V3f(v1.x / c, v1.y / c, v1.z / c) )

		# Assignment
		v1_copy = IECore.V3f(v1)
		temp = v1
		temp /= v2
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy / v2)

		v1_copy = IECore.V3f(v1)
		temp = v1
		temp /= c
		self.assert_( temp is v1 )
		self.assertEqual( temp, v1_copy / c)

		# matrix multiplication

		v1 = IECore.V3f( 1, 2, 3 )
		m = IECore.M44f.createTranslated( IECore.V3f( 1, 2, 3 ) )
		v2 = v1 * m
		v1 *= m
		self.assertEqual( v1, v2 )
		self.assertEqual( v1, IECore.V3f( 2, 4, 6 ) )

	def testMiscMethods(self):
		"""Test V3f miscellaneous methods"""
		v1 = IECore.V3f(41.4, 2.3, -4.98)

		self.assertAlmostEqual( v1.length2(), v1.dot(v1), 3 )
		self.assertAlmostEqual( v1.length(), math.sqrt(v1.dot(v1)), 3 )
		self.assertAlmostEqual( v1.length() * v1.length(), v1.length2(), 3 )

		v1 = IECore.V3f(10.0, 0.0, 0.0)
		self.assertEqual( v1.normalized(), v1 / v1.length() )
		self.assertEqual( v1, IECore.V3f(10.0, 0.0, 0.0) )

		v1.normalize()
		self.assertEqual( v1, IECore.V3f(1.0, 0.0, 0.0) )

	def testRepr( self ) :

		v1 = IECore.V3f( 0.091242323423 )
		v2 = eval( repr( v1 ) )
		self.assertEqual( v1, v2 )

class ImathBox3f(unittest.TestCase):
	def testConstructors(self):
		"""Test Box3f constructors"""
		b = IECore.Box3f()
		self.assert_( b.isEmpty() )

		b = IECore.Box3f( IECore.V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.min, IECore.V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.max, IECore.V3f(1.0, 1.0, 1.0) )

		b = IECore.Box3f( IECore.V3f(-1.0, -1.0, -1.0), IECore.V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.min, IECore.V3f(-1.0, -1.0, -1.0) )
		self.assertEqual( b.max, IECore.V3f( 1.0,  1.0,  1.0) )

	def testEquality(self):
		"""Test Box3f comparison for equality"""

		b1 = IECore.Box3f( IECore.V3f(1.0, 2.0, 3.0) )
		b2 = IECore.Box3f( IECore.V3f(1.0, 2.0, 3.0) )
		self.assert_( b1 == b2 )

		b2 = IECore.Box3f( IECore.V3f(3.0, 2.0, 1.0) )
		self.assert_( b1 != b2 )

	def testMiscMethods(self):
		"""Test Box3f miscellaneous methods"""

		b1 = IECore.Box3f( IECore.V3f(-1.0, -1.0, -1.0), IECore.V3f(2.0, 2.0, 2.0) )
		self.assertEqual( b1.isEmpty(), False )
		self.assert_( b1.hasVolume() )

		b1.makeEmpty()
		self.assert_( b1.isEmpty() )
		self.assertEqual( b1.hasVolume(), False )

		b1 = IECore.Box3f( IECore.V3f(-1.0, -1.0, -1.0), IECore.V3f(10.0, 2.0, 2.0) )

		X_AXIS = 0
		self.assertEqual( b1.majorAxis(), X_AXIS )

		self.assertEqual( b1.center(), (b1.min + b1.max) / 2.0 )

		b2 = IECore.Box3f( IECore.V3f(-0.5), IECore.V3f(1.0) )
		self.assert_( b2.intersects(b1) )

		b2 = IECore.Box3f( IECore.V3f(-5.0), IECore.V3f(-2.0) )
		self.failIf( b2.intersects(b1) )

		self.assertEqual( b2.size(), b2.max - b2.min )

		b = IECore.Box3f( IECore.V3f(1), IECore.V3f(2) )
		m = IECore.M44f()
		m[0,0]=2
		m[1,1]=2
		m[2,2]=2
		self.assertEqual( b.transform( m ), IECore.Box3f( IECore.V3f(2), IECore.V3f(4) ) )
		m = IECore.M44d()
		m[0,0]=2
		m[1,1]=2
		m[2,2]=2
		self.assertEqual( b.transform( m ), IECore.Box3f( IECore.V3f(2), IECore.V3f(4) ) )

	def testContains( self ) :

		b1 = IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )
		b2 = IECore.Box3f( IECore.V3f( 0, -0.5, 0.5 ), IECore.V3f( 0.1, 0, 0.9 ) )
		b3 = IECore.Box3f( IECore.V3f( -1.2, -0.6, 0.4 ), IECore.V3f( 0.2, 0.1, 1 ) )

		self.assert_( b1.contains( b2 ) )
		self.assert_( not b2.contains( b1 ) )

		self.assert_( not b2.contains( b3 ) )
		self.assert_( b3.contains( b2 ) )

		self.assert_( not b3.contains( b1 ) )
		self.assert_( not b1.contains( b3 ) )

	def testSplit( self ) :

		r = IECore.Rand32()
		for i in range( 0, 100 ) :

			b = IECore.Box3f()
			b.extendBy( r.nextV3f() )
			b.extendBy( r.nextV3f() )

			major = b.majorAxis()

			low, high = b.split()
			low2, high2 = b.split( major )

			self.assertEqual( low, low2 )
			self.assertEqual( high, high2 )

			b2 = IECore.Box3f()
			b2.extendBy( low )
			b2.extendBy( high )

			self.assertEqual( b, b2 )

class ImathQuatf(unittest.TestCase):
	def testConstructors(self):
		"""Test Quatf constructors"""

		q = IECore.Quatf()
		q = IECore.Quatf(q)
		q = IECore.Quatf(0.1, 0.2, 0.3, 0.4)
		q = IECore.Quatf(0.1, IECore.V3f(0.2, 0.3, 0.4))
		q = IECore.Quatf.identity()
		self.assertEqual( q, IECore.Quatf(1,0,0,0) )

	def testIndexing(self):
		"""Test Quatf indexing via operator[]"""

		q = IECore.Quatf( 1, 2, 3, 4 )

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

		q1 = IECore.Quatf( 1, 2, 3, 4 )
		q2 = IECore.Quatf( 1, 2, 3, 4 )
		self.assertEqual(q1, q1)
		self.assertEqual(q1, q2)

		q2 = IECore.Quatf( 5, 2, 3, 4 )
		self.assert_( q1 != q2 )

	def testMiscMethods(self):
		"""Test Quatf miscellaneous methods"""

		q1 = IECore.Quatf( 1, 2, 3, 4 )
		self.assertAlmostEqual( q1.length(), math.sqrt(q1[0]*q1[0]+(q1.v^q1.v)), 3 )

		# axis/angle
		axis = IECore.V3f( 1, 2, 3 )
		axis.normalize()

		q1.setAxisAngle( axis, 0.5 )
		self.assertAlmostEqual( q1.axis().x, axis.x, 3 )
		self.assertAlmostEqual( q1.axis().y, axis.y, 3 )
		self.assertAlmostEqual( q1.axis().z, axis.z, 3 )

		self.assertAlmostEqual( q1.angle(), 0.5, 3 )

		# Rotate x axis onto y axis
		q1.setRotation( IECore.V3f(1,0,0), IECore.V3f(0,1,0) )

		#We should have gone 90 degrees about the +ve z-axis
		self.assertAlmostEqual( q1.angle(), 90.0 * math.pi / 180.0, 3 )

		self.assertAlmostEqual( q1.axis().x, 0.0, 3 )
		self.assertAlmostEqual( q1.axis().y, 0.0, 3 )
		self.assertAlmostEqual( q1.axis().z, 1.0, 3 )

		#inversion
		q1 = IECore.Quatf( 1, 2, 3, 4 )
		qdot = q1 ^ q1
		qi_test = IECore.Quatf( q1.r / qdot, -q1.v / qdot)
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
		q2 = IECore.Quatf( 0.5, 0.6, 0.7, 0.8 )
		qs = IECore.slerp(q1, q2, 0.5)

		# normalization
		qn = qi.normalized()
		qn_test = IECore.Quatf( qi.r / qi.length(), qi.v / qi.length() )

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
		fromDir = IECore.V3f(1,0,0)
		toDir = IECore.V3f(0,1,0)
		q1.setRotation( fromDir, toDir )
		m = q1.toMatrix33()
		m = q1.toMatrix44()

	def testOperators(self):
		"""Test Quatf operators"""

		q1 = IECore.Quatf( 1, 2, 3, 4 )
		q2 = IECore.Quatf( 5, 6, 7, 8 )
		self.assertAlmostEqual( q1 ^ q2, q1.r * q2.r + (q1.v ^ q2.v ), 3 )

	def testSlerpStability( self ) :

		q1 = IECore.Quatd( 0.60477471085951961527, 0.19082800913200048676, -0.73048263950686898038, 0.25343112163777203882, )
		q2 = IECore.Quatd( 0.6047747108595192822, 0.190828009132000459, -0.73048263950686909141,    0.25343112163777264945, )

		q3 = IECore.slerp( q1, q2, 0.5 )

		self.assert_( q1.v.equalWithAbsError( q3.v, 0.000000000000001 ) )
		self.assertAlmostEqual( q1.r, q3.r, 14 )

class ImathM33f(unittest.TestCase):
	def testConstructors(self):
		"""Test M33f constructors"""
		m = IECore.M33f()

		m = IECore.M33f(2)

		m = IECore.M33f(1, 0, 0,
		              0, 1, 0,
                              0, 0, 1);

		m = IECore.M33f( [ 1, 0, 0, 0, 1, 0, 0, 0, 1 ] )

		self.assertRaises( RuntimeError, IECore.M33f, [ 1 ] )
		self.assertRaises( RuntimeError, IECore.M33f, [ 1, 2 ] )
		self.assertRaises( RuntimeError, IECore.M33f, [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ] )

	def testDimensions(self):
		"""Test M33f dimensions"""

		m1 = IECore.M33f()
		d = m1.dimensions()
		self.assertEqual( d[0], 3 )
		self.assertEqual( d[1], 3 )

	def testCopyAndAssign(self):
		"""Test M33f copy construction and assignment"""

		m1 = IECore.M33f()
		m2 = IECore.M33f(m1)
		self.failIf(m1 is m2)

	def testIndexing(self):
		"""Test M33f indexing via [] operator"""

		m = IECore.M33f()

		# test tuple indexing:
		m[(0,0)] = 10.0
		m[(0,1)] = 11.0
		m[(0,2)] = 12.0
		m[(1,0)] = 13.0
		m[(1,1)] = 14.0
		m[(1,2)] = 15.0
		m[(2,0)] = 16.0
		m[(2,1)] = 17.0
		m[(2,2)] = 18.0

		# test __getitem__( tuple ):
		self.assertEqual( m[(0,0)], 10.0 )
		self.assertEqual( m[(0,1)], 11.0 )
		self.assertEqual( m[(0,2)], 12.0 )
		self.assertEqual( m[(1,0)], 13.0 )
		self.assertEqual( m[(1,1)], 14.0 )
		self.assertEqual( m[(1,2)], 15.0 )
		self.assertEqual( m[(2,0)], 16.0 )
		self.assertEqual( m[(2,1)], 17.0 )
		self.assertEqual( m[(2,2)], 18.0 )

		self.assertEqual( m, IECore.M33f( 10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0 ) )

	def testOperators(self):
		"""Test M33f operators"""

		x = 10
		y = 2
		m1 = IECore.M33f(x)
		m2 = IECore.M33f(y)

		self.assertEqual(m1 + m2, IECore.M33f(x + y))
		self.assertEqual(m1 - m2, IECore.M33f(x - y))
		self.assertEqual(m1 * y, IECore.M33f(x * y))
		self.assertEqual(m1 / y, IECore.M33f(x / y))

	def testMiscellaneousMethods(self):
		"""Test M33f miscellaneous methods"""

		m1 = IECore.M33f()
		m1.makeIdentity()

		m1 = IECore.M33f(3)
		m2 = IECore.M33f(3.1)
		self.assert_( m1.equalWithAbsError(m2, 0.1) )

		m1 = IECore.M33f(2)
		m2 = IECore.M33f(3)
		self.assert_( m1.equalWithRelError(m2, 0.51) )

		m1 = IECore.M33f(1, 0, 0,
		               0, 2, 0,
			       0, 0, 3)
		self.assertEqual( m1.transposed().transposed(), m1)



	def testEquality(self):
		"""Test M33f comparison for equality"""
		m1 = IECore.M33f(3)
		m2 = IECore.M33f(3)

		self.assertEqual(m1, m2)

	def testCreate(self ) :

		self.assertEqual( IECore.M33f(), IECore.M33f.createScaled( IECore.V2f( 1 ) ) )

		m = IECore.M33f()
		m.scale( IECore.V2f( 2, 3 ) )
		self.assertEqual( m, IECore.M33f.createScaled( IECore.V2f( 2, 3 ) ) )

		self.assertEqual( IECore.M33f(), IECore.M33f.createTranslated( IECore.V2f( 0 ) ) )

		m = IECore.M33f()
		m.translate( IECore.V2f( 2, 3 ) )
		self.assertEqual( m, IECore.M33f.createTranslated( IECore.V2f( 2, 3 ) ) )

		self.assertEqual( IECore.M33f(), IECore.M33f.createRotated( 0 ) )

		m = IECore.M33f()
		m.rotate( 2 )
		self.assertEqual( m, IECore.M33f.createRotated( 2 ) )

	def testMultMethods( self ) :

		v = IECore.M33f.createTranslated( IECore.V2f( 1, 2 ) ).multVecMatrix( IECore.V2f( 0 ) )
		self.assertEqual( v, IECore.V2f( 1, 2 ) )

		v = IECore.M33f.createTranslated( IECore.V2f( 1, 2 ) ).multDirMatrix( IECore.V2f( 1 ) )
		self.assertEqual( v, IECore.V2f( 1 ) )

	def testDeterminant( self ) :

		m = IECore.M33f()
		self.assertAlmostEqual( m.determinant(), 1, 10 )
		m.scale( IECore.V2f( -1, 1 ) )
		self.assertAlmostEqual( m.determinant(), -1, 10 )
		m.scale( IECore.V2f( 1, -1 ) )
		self.assertAlmostEqual( m.determinant(), 1, 10 )
		m.scale( IECore.V2f( 3, -1 ) )
		self.assertAlmostEqual( m.determinant(), -3, 10 )
		m.scale( IECore.V2f( 3, 3 ) )
		self.assertAlmostEqual( m.determinant(), -27, 10 )

		r = IECore.curry( random.uniform, -10, 10 )
		for i in range( 0, 1000 ) :

			m = IECore.M33f( r(), r(), r(), r(), r(), r(), r(), r(), r() )
			d = m.determinant()

			if math.fabs( d ) > 0.00001 :

				mi = m.inverse()
				di = mi.determinant()

				self.assertAlmostEqual( d, 1/di, 1 )

			mt = m.transposed()
			self.assertAlmostEqual( d, mt.determinant(), 3 )

	def testConstructFromOtherType( self ) :

		md = IECore.M33d( 1, 2, 3, 4, 5, 6, 7, 8, 9 )
		mf = IECore.M33f( 1, 2, 3, 4, 5, 6, 7, 8, 9 )

		mf2 = IECore.M33f( md )
		self.assertEqual( mf2, mf )

		md2 = IECore.M33d( mf )
		self.assertEqual( md2, md )

class ImathM44f(unittest.TestCase):
	def testConstructors(self):
		"""Test M44f constructors"""
		m = IECore.M44f(1., 0., 0., 0.,
		              0., 1., 0., 0.,
			      0., 0., 1., 0.,
			      0., 0., 0., 1.);

		m3 = IECore.M33f(1., 0., 0.,
		               0., 1., 0.,
			       0., 0., 1.)

		t = IECore.V3f(5., 5., 5.)

		m = IECore.M44f(m3, t)

		m = IECore.M44f( [ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 ] )

		self.assertRaises( RuntimeError, IECore.M44f, [ 1 ] )
		self.assertRaises( RuntimeError, IECore.M44f, [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ] )
		self.assertRaises( RuntimeError, IECore.M44f, [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 ] )

	def testDimensions(self):
		"""Test M44f dimensions"""

		m1 = IECore.M44f()
		d = m1.dimensions()
		self.assertEqual( d[0], 4 )
		self.assertEqual( d[1], 4 )

	def testCopyAndAssign(self):
		"""Test M44f copy construction and assignment"""
		m1 = IECore.M44f()
		m2 = IECore.M44f(m1)
		self.failIf(m1 is m2)

		m1 = m2

	def testIndexing(self):
		"""Test M44f indexing via [] operator"""
		m = IECore.M44f()

		# test tuple indexing:
		m[(0,0)] = 10.0
		m[(0,1)] = 11.0
		m[(0,2)] = 12.0
		m[(0,3)] = 13.0
		m[(1,0)] = 14.0
		m[(1,1)] = 15.0
		m[(1,2)] = 16.0
		m[(1,3)] = 17.0
		m[(2,0)] = 18.0
		m[(2,1)] = 19.0
		m[(2,2)] = 20.0
		m[(2,3)] = 21.0
		m[(3,0)] = 22.0
		m[(3,1)] = 23.0
		m[(3,2)] = 24.0
		m[(3,3)] = 25.0

		# test __getitem__( tuple ):
		self.assertEqual( m[(0,0)], 10.0 )
		self.assertEqual( m[(0,1)], 11.0 )
		self.assertEqual( m[(0,2)], 12.0 )
		self.assertEqual( m[(0,3)], 13.0 )
		self.assertEqual( m[(1,0)], 14.0 )
		self.assertEqual( m[(1,1)], 15.0 )
		self.assertEqual( m[(1,2)], 16.0 )
		self.assertEqual( m[(1,3)], 17.0 )
		self.assertEqual( m[(2,0)], 18.0 )
		self.assertEqual( m[(2,1)], 19.0 )
		self.assertEqual( m[(2,2)], 20.0 )
		self.assertEqual( m[(2,3)], 21.0 )
		self.assertEqual( m[(3,0)], 22.0 )
		self.assertEqual( m[(3,1)], 23.0 )
		self.assertEqual( m[(3,2)], 24.0 )
		self.assertEqual( m[(3,3)], 25.0 )

		self.assertEqual( m, IECore.M44f( 10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,25.0 ) )

	def testOperators(self):
		"""Test M44f operators"""
		x = 10
		y = 2
		m1 = IECore.M44f(x)
		m2 = IECore.M44f(y)

		self.assertEqual(m1 + m2, IECore.M44f(x + y))
		self.assertEqual(m1 - m2, IECore.M44f(x - y))
		self.assertEqual(m1 * y, IECore.M44f(x * y))
		self.assertEqual(m1 / y, IECore.M44f(x / y))

	def testMiscellaneousMethods(self):
		"""Test M44f miscellaneous methods"""
		m1 = IECore.M44f()
		m1.makeIdentity()

		m1 = IECore.M44f(3)
		m2 = IECore.M44f(3.1)
		self.assert_( m1.equalWithAbsError(m2, 0.1) )

		m1 = IECore.M44f(2)
		m2 = IECore.M44f(3)
		self.assert_( m1.equalWithRelError(m2, 0.51) )

		m1 = IECore.M44f(1, 0, 0, 0,
		               0, 2, 0, 0,
			       0, 0, 3, 0,
			       0, 0, 0, 4)
		self.assertEqual( m1.transposed().transposed(), m1)

	def testEquality(self):
		"""Test M44f comparison for equality"""

		m1 = IECore.M44f(3)
		m2 = IECore.M44f(3)

		self.assertEqual(m1, m2)

	def testCreate(self ) :

		self.assertEqual( IECore.M44f(), IECore.M44f.createScaled( IECore.V3f( 1 ) ) )

		m = IECore.M44f()
		m.scale( IECore.V3f( 2, 3, 4 ) )
		self.assertEqual( m, IECore.M44f.createScaled( IECore.V3f( 2, 3, 4 ) ) )

		self.assertEqual( IECore.M44f(), IECore.M44f.createTranslated( IECore.V3f( 0 ) ) )

		m = IECore.M44f()
		m.translate( IECore.V3f( 2, 3, 4 ) )
		self.assertEqual( m, IECore.M44f.createTranslated( IECore.V3f( 2, 3, 4 ) ) )

		self.assertEqual( IECore.M44f(), IECore.M44f.createRotated( IECore.V3f( 0 ) ) )

		m = IECore.M44f()
		m.rotate( IECore.V3f( 1, 2, 3 ) )
		self.assertEqual( m, IECore.M44f.createRotated( IECore.V3f( 1, 2, 3 ) ) )

		m = IECore.M44f.createAimed( IECore.V3f( 1, 0, 0  ), IECore.V3f( 0, 1, 0 ) )
		self.assert_( IECore.V3f( 0, 1, 0 ).equalWithAbsError( IECore.V3f( 1, 0, 0 ) * m, 0.0000001 ) )

		m = IECore.M44f.createAimed( IECore.V3f( 1, 0, 0 ), IECore.V3f( 0, 0, 1 ), IECore.V3f( 0, 1, 0 ) )
		self.assert_( IECore.V3f( 0, 0, 1 ).equalWithAbsError( IECore.V3f( 1, 0, 0 ) * m, 0.0000001 ) )
		self.assert_( IECore.V3f( 0, 1, 0 ).equalWithAbsError( IECore.V3f( 0, 1, 0 ) * m, 0.0000001 ) )

	def testMultMethods( self ) :

		v = IECore.M44f.createTranslated( IECore.V3f( 1, 2, 3 ) ).multVecMatrix( IECore.V3f( 0 ) )
		self.assertEqual( v, IECore.V3f( 1, 2, 3 ) )

		v = IECore.M44f.createTranslated( IECore.V3f( 1, 2, 3 ) ).multDirMatrix( IECore.V3f( 1 ) )
		self.assertEqual( v, IECore.V3f( 1 ) )

	def testFromBasis( self ) :

		for i in range( 0, 10000 ) :

			m = IECore.M44f()
			m.translate( IECore.V3f( random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ) ) )
			m.rotate( IECore.V3f( random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ) ) )
			m.scale( IECore.V3f( random.uniform( -100, 100 ), random.uniform( -100, 100 ), random.uniform( -100, 100 ) ) )

			x = m.multDirMatrix( IECore.V3f( 1, 0, 0 ) )
			y = m.multDirMatrix( IECore.V3f( 0, 1, 0 ) )
			z = m.multDirMatrix( IECore.V3f( 0, 0, 1 ) )
			o = IECore.V3f( 0, 0, 0 ) * m

			self.assertEqual( IECore.M44f.createFromBasis( x, y, z, o ), m )

	def testDeterminant( self ) :

		m = IECore.M44f()
		self.assertAlmostEqual( m.determinant(), 1, 10 )
		m.scale( IECore.V3f( -1, 1, 1 ) )
		self.assertAlmostEqual( m.determinant(), -1, 10 )
		m.scale( IECore.V3f( 1, -1, 1 ) )
		self.assertAlmostEqual( m.determinant(), 1, 10 )
		m.scale( IECore.V3f( 3, -1, 1 ) )
		self.assertAlmostEqual( m.determinant(), -3, 10 )
		m.scale( IECore.V3f( 3, 3, 1 ) )
		self.assertAlmostEqual( m.determinant(), -27, 10 )

		random.seed( 42 )

		r = IECore.curry( random.uniform, -2, 2 )
		for i in range( 0, 1000 ) :

			m = IECore.M44f( r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r() )
			d = m.determinant()

			if math.fabs( d ) > 0.00001 :

				mi = m.inverse()
				di = mi.determinant()

				self.assertAlmostEqual( d, 1/di, 4 )

			mt = m.transposed()
			self.assertAlmostEqual( d, mt.determinant(), 4 )

		for i in range( 0, 1000 ) :

			m = IECore.M44f()
			m.translate( IECore.V3f( r(), r(), r() ) )
			self.assertAlmostEqual( m.determinant(), 1, 10 )

	def testConstructFromOtherType( self ) :

		md = IECore.M44d( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 )
		mf = IECore.M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 )

		mf2 = IECore.M44f( md )
		self.assertEqual( mf2, mf )

		md2 = IECore.M44d( mf )
		self.assertEqual( md2, md )

class ImathColor3Test( unittest.TestCase ) :

	def test( self ) :

		c = IECore.Color3f( 1 )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 1 )
		self.assertEqual( c.b, 1 )

		c = IECore.Color3f( 1, 2, 3 )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 2 )
		self.assertEqual( c.b, 3 )

		cc = IECore.Color3f( c )
		self.assertEqual( c, cc )

		c = IECore.Color3f( IECore.V3f(1,2,3) )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 2 )
		self.assertEqual( c.b, 3 )

		c = IECore.Color3f( IECore.V3d(1,2,3) )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 2 )
		self.assertEqual( c.b, 3 )

		cm = -c * 2
		self.assertEqual( cm.r, -2 )
		self.assertEqual( cm.g, -4 )
		self.assertEqual( cm.b, -6 )

		cm *= c
		self.assertEqual( cm.r, -2 )
		self.assertEqual( cm.g, -8 )
		self.assertEqual( cm.b, -18 )

		cm -= IECore.Color3f( 2 )
		self.assertEqual( cm, IECore.Color3f( -4, -10, -20 ) )

		self.assertEqual( c.dimensions(), 3 )

	def testHSVTransforms( self ) :

		c = IECore.Color3f( 0.1, 0.2, 0.3 )

		chsv = c.rgbToHSV()
		self.assertEqual( c, IECore.Color3f( 0.1, 0.2, 0.3 ) )
		self.failUnless( isinstance( chsv, IECore.Color3f ) )
		self.failUnless( chsv.equalWithAbsError( IECore.Color3f( 0.5833, 0.6667, 0.3 ), 0.001 ) )

		crgb = chsv.hsvToRGB()
		self.failUnless( chsv.equalWithAbsError( IECore.Color3f( 0.5833, 0.6667, 0.3 ), 0.001 ) )
		self.failUnless( crgb.equalWithAbsError( c, 0.001 ) )

	def testRepr( self ) :

		c1 = IECore.Color3f( 0.091242323423 )
		c2 = eval( repr( c1 ) )
		self.assertEqual( c1, c2 )

class ImathEulerfTest( unittest.TestCase ) :

	def testConstructors(self):
		"""Test Eulerf constructors"""

		#
		e = IECore.Eulerf()
		self.assertEqual( e.x, 0 )
		self.assertEqual( e.y, 0 )
		self.assertEqual( e.z, 0 )

		self.assertEqual( e.order(), IECore.Eulerf.Order.Default )
		self.assertEqual( e.order(), IECore.Eulerf.Order.XYZ )

		#
		ecopy = IECore.Eulerf(e)
		self.assertEqual( ecopy.x, 0 )
		self.assertEqual( ecopy.y, 0 )
		self.assertEqual( ecopy.z, 0 )

		self.assertEqual( ecopy.order(), IECore.Eulerf.Order.Default )
		self.assertEqual( ecopy.order(), IECore.Eulerf.Order.XYZ )

		#
		e = IECore.Eulerf( IECore.Eulerf.Order.ZYX )
		self.assertEqual( e.order(), IECore.Eulerf.Order.ZYX )

		#
		e = IECore.Eulerf( IECore.V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), IECore.Eulerf.Order.Default )
		self.assertEqual( e.order(), IECore.Eulerf.Order.XYZ )

		e = IECore.Eulerf( IECore.V3f( 0, 0, 0 ), IECore.Eulerf.Order.ZYX )
		self.assertEqual( e.order(), IECore.Eulerf.Order.ZYX )

		#
		e = IECore.Eulerf( 0, 0, 0 )
		e = IECore.Eulerf( IECore.V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), IECore.Eulerf.Order.Default )
		self.assertEqual( e.order(), IECore.Eulerf.Order.XYZ )

		e = IECore.Eulerf( 0, 0, 0, IECore.Eulerf.Order.ZYX  )
		self.assertEqual( e.order(), IECore.Eulerf.Order.ZYX )

		e = IECore.Eulerf( 0, 0, 0, IECore.Eulerf.Order.ZYX, IECore.Eulerf.InputLayout.XYZLayout )
		self.assertEqual( e.order(), IECore.Eulerf.Order.ZYX )

		#
		e = IECore.Eulerf( IECore.M33f() )
		e = IECore.Eulerf( IECore.V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), IECore.Eulerf.Order.Default )
		self.assertEqual( e.order(), IECore.Eulerf.Order.XYZ )

		e = IECore.Eulerf( IECore.M33f(), IECore.Eulerf.Order.ZYX )
		self.assertEqual( e.order(), IECore.Eulerf.Order.ZYX )

		#
		e = IECore.Eulerf( IECore.M44f() )
		e = IECore.Eulerf( IECore.V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), IECore.Eulerf.Order.Default )
		self.assertEqual( e.order(), IECore.Eulerf.Order.XYZ )

		e = IECore.Eulerf( IECore.M44f(), IECore.Eulerf.Order.ZYX )
		self.assertEqual( e.order(), IECore.Eulerf.Order.ZYX )


	def testOrder(self):
		"""Test Eulerf order"""

		self.assertEqual( len( IECore.Eulerf.Order.values ), 24 )

		e = IECore.Eulerf()

		for order in IECore.Eulerf.Order.values.values():
			self.assert_( IECore.Eulerf.legal( order ) )

			e.setOrder( order )

			self.assertEqual( e.order(), order )

	def testMisc(self):
		"""Test Eulerf miscellaneous"""

		self.assertEqual( len(IECore.Eulerf.Axis.values), 3 )
		self.assertEqual( len(IECore.Eulerf.InputLayout.values), 2 )

		self.assert_( IECore.V3f in IECore.Eulerf.__bases__ )

	def testExtract(self):

		"""Test Eulerf extract"""

		e = IECore.Eulerf()
		e.extract( IECore.M33f() )

		e.extract( IECore.M44f() )

		e.extract( IECore.Quatf() )

		m = e.toMatrix33()
		m = e.toMatrix44()
		q = e.toQuat()
		v = e.toXYZVector()

	def testAngleOrder(self):

		"""Test Eulerf angleOrder"""

		e = IECore.Eulerf()

		o = e.angleOrder()

		self.assert_( type(o) is tuple )
		self.assertEqual( len(o), 3 )

	def testAngleMapping(self):

		"""Test Eulerf angleMapping"""

		e = IECore.Eulerf()

		m = e.angleMapping()

		self.assert_( type(m) is tuple )
		self.assertEqual( len(m), 3 )


	def testStr(self):
		"""Test Eulerf str"""

		e = IECore.Eulerf()
		self.assertEqual( str(e), "0 0 0" )

	def testRepr(self):
		"""Test Eulerf repr"""

		e = IECore.Eulerf()
		self.assertEqual( repr(e), "IECore.Eulerf( 0, 0, 0 )" )

	def testSimpleXYZRotation(self):

		e = IECore.Eulerf( math.pi * 6, math.pi * 10, -math.pi * 20 )
		ee = IECore.Eulerf( e )
		t = IECore.Eulerf( 0, 0, 0 )

		es = IECore.Eulerf.simpleXYZRotation( e, t )

		# check that the simple rotations are in an appropriate range
		for r in es :
			self.assert_( math.fabs( r ) <= math.pi )

		# and that the original vector isn't modified in place
		self.assertEqual( ee, e )

	def testNearestRotation(self):

		e = IECore.Eulerf( math.pi * 6, math.pi * 10, -math.pi * 20 )
		ee = IECore.Eulerf( e )
		t = IECore.Eulerf( 0, 0, 0 )

		en = IECore.Eulerf.nearestRotation( e, t )

		# check that the original vector isn't modified in place
		self.assertEqual( ee, e )

class ImathPlane3fTest( unittest.TestCase ) :

	def testConstructors( self ) :

		p = IECore.Plane3f( IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ) )
		self.assertEqual( p.normal, IECore.V3f( 1, 0, 0 ) )
		self.assertEqual( p.distance, 0 )

		p = IECore.Plane3f( IECore.V3f( 0, 0, 0 ), IECore.V3f( 0, 1, 0 ), IECore.V3f( 0, 0, 1 ) )
		self.assertEqual( p.normal, IECore.V3f( 1, 0, 0 ) )
		self.assertEqual( p.distance, 0 )

		p = IECore.Plane3f( IECore.V3f( 2, 2, 2 ), IECore.V3f( 2, 3, 2 ), IECore.V3f( 2, 2, 3 ) )
		self.assertEqual( p.normal, IECore.V3f( 1, 0, 0 ) )
		self.assertEqual( p.distance, 2 )


if __name__ == "__main__":
    unittest.main()

