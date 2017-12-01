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
import imath

import IECore

class ImathV2f(unittest.TestCase):

	def testConstructors(self):
		"""Test V2f constructors"""
		v = imath.V2f()
		v = imath.V2f(1)
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)
		v = imath.V2f(2, 3)
		self.assertEqual(v.x, 2)
		self.assertEqual(v.y, 3)

		self.assertEqual( imath.V2f( imath.V2i( 1, 2 ) ), imath.V2f( 1, 2 ) )
		self.assertEqual( imath.V2f( imath.V2f( 1, 2 ) ), imath.V2f( 1, 2 ) )
		self.assertEqual( imath.V2f( imath.V2d( 1, 2 ) ), imath.V2f( 1, 2 ) )

		self.assertEqual( imath.V2d( imath.V2i( 1, 2 ) ), imath.V2d( 1, 2 ) )
		self.assertEqual( imath.V2d( imath.V2f( 1, 2 ) ), imath.V2d( 1, 2 ) )
		self.assertEqual( imath.V2d( imath.V2d( 1, 2 ) ), imath.V2d( 1, 2 ) )

		self.assertEqual( imath.V2i( imath.V2i( 1, 2 ) ), imath.V2i( 1, 2 ) )
		self.assertEqual( imath.V2i( imath.V2f( 1, 2 ) ), imath.V2i( 1, 2 ) )
		self.assertEqual( imath.V2i( imath.V2d( 1, 2 ) ), imath.V2i( 1, 2 ) )

		self.assertEqual( imath.V3f( imath.V3i( 1, 2, 3 ) ), imath.V3f( 1, 2, 3 ) )
		self.assertEqual( imath.V3f( imath.V3f( 1, 2, 3 ) ), imath.V3f( 1, 2, 3 ) )
		self.assertEqual( imath.V3f( imath.V3d( 1, 2, 3 ) ), imath.V3f( 1, 2, 3 ) )

		self.assertEqual( imath.V3d( imath.V3i( 1, 2, 3 ) ), imath.V3d( 1, 2, 3 ) )
		self.assertEqual( imath.V3d( imath.V3f( 1, 2, 3 ) ), imath.V3d( 1, 2, 3 ) )
		self.assertEqual( imath.V3d( imath.V3d( 1, 2, 3 ) ), imath.V3d( 1, 2, 3 ) )

		self.assertEqual( imath.V3i( imath.V3i( 1, 2, 3 ) ), imath.V3i( 1, 2, 3 ) )
		self.assertEqual( imath.V3i( imath.V3f( 1, 2, 3 ) ), imath.V3i( 1, 2, 3 ) )
		self.assertEqual( imath.V3i( imath.V3d( 1, 2, 3 ) ), imath.V3i( 1, 2, 3 ) )

		v = imath.V2f( [ 1, 1 ] )
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)

		self.assertRaises( RuntimeError, imath.V2f, [ 1 ] )
		self.assertRaises( RuntimeError, imath.V2f, [ 1, 2, 3 ] )

	def testDimensions(self):
		"""Test V2f dimensions"""

		v = imath.V2f()
		self.assertEqual( v.dimensions(), 2 )

	def testIndexing(self):
		"""Test V2f indexing via operator[]"""
		v1 = imath.V2f(1.0, 2.0)

		self.assertEqual(v1[0], 1.0)
		self.assertEqual(v1[1], 2.0)

		v1[0] = 12.0
		v1[1] = 15.0

		self.assertEqual(v1[0], 12.0)
		self.assertEqual(v1[1], 15.0)


	def testCopyAndAssign(self):
		"""Test V2f copy construction and assignment"""
		v1 = imath.V2f(2.0)
		v2 = imath.V2f(3.0)

		v2 = v1
		# v2 should now contain contents of v1, v1 should be unchanged
		self.assertEqual(v2.x, 2)
		self.assertEqual(v2.y, 2)
		self.assertEqual(v1.x, 2)
		self.assertEqual(v1.y, 2)



	def testEquality(self):
		"""Test V2f comparison for equality"""
		v1 = imath.V2f(1.00)
		v2 = imath.V2f(1.01)

		self.assert_( v1.equalWithAbsError(v2, 0.01) )

		v1 = imath.V2f(2.0)
		v2 = imath.V2f(3.0)

		self.assert_( v1.equalWithRelError(v2, 0.5) )

		v1 = imath.V2f(1.0)
		v2 = imath.V2f(1.0)
		self.assert_( v1 == v2 )
		v1 = imath.V2f(1.0)
		v2 = imath.V2f(1.1)

		self.assert_( v1 != v2 )

	def testDotProduct(self):
		"""Test V2f dot product"""
		v1 = imath.V2f(3.0)
		v2 = imath.V2f(4.0)
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
		v1 = imath.V2f(2.0, 2.0)
		v2 = imath.V2f(0.0, 2.0)

		# Area of parallelogram, by definition
		self.assertEqual( v1.cross(v2), 4.0 )

		# Operator/method equivalence
		self.assertEqual( v1.cross(v2), v1 % v2 )

		# ImathVec.h comment validity
		self.assertEqual( v1.cross(v2), (imath.V3f(v1.x, v1.y, 0.0) % imath.V3f(v2.x, v2.y, 0.0)).z )

	def testOperators(self):
		"""Test V2f arithmetic operators"""
		v1 = imath.V2f(3.4,  9.2)
		v2 = imath.V2f(5.3, -0.4)

		# ADDITION

		# By definition
		self.assertEqual( v1 + v2, imath.V2f( v1.x + v2.x, v1.y + v2.y ) )

		# Commutative
		self.assertEqual( v1 + v2, v2 + v1 )

		# Assignment
		v1_copy = imath.V2f(v1)
		temp = v1
		temp += v2
		self.assertEqual( temp, (v1_copy + v2))

		# SUBTRACTION

		# By definition
		self.assertEqual( v1 - v2, imath.V2f( v1.x - v2.x, v1.y - v2.y ) )
		self.assertEqual( v1 - v2, -v2 + v1 )

		# Assignment
		v1_copy = imath.V2f(v1)
		temp = v1
		temp -= v2
		self.assertEqual( temp, v1_copy - v2)

		# NEGATION

		self.assertEqual( -v1, imath.V2f( -v1.x, -v1.y) )
		self.assertEqual( -v1, v1.negate() )
		self.assertEqual( -( -v1), v1 )

		# MULTIPLICATION

		# By definition
		self.assertEqual( v1 * v2, imath.V2f(v1.x * v2.x, v1.y * v2.y) )
		c = 3
		self.assertEqual( v1 * c, imath.V2f(v1.x * c, v1.y * c) )

		# Commutative
		self.assertEqual( v1 * v2, v2 * v1 )
		self.assertEqual( c * v1, imath.V2f(v1.x * c, v1.y * c) )

		# Assignment
		v1_copy = imath.V2f(v1)
		temp = v1
		temp *= v2
		self.assertEqual( temp, v1_copy * v2)

		v1_copy = imath.V2f(v1)
		temp = v1
		temp *= c
		self.assertEqual( temp, v1_copy * c)

		# DIVISION

		# By definition
		self.assertEqual( v1 / v2, imath.V2f(v1.x / v2.x, v1.y / v2.y) )
		self.assertEqual( v1 / c, imath.V2f(v1.x / c, v1.y / c) )

		# Assignment
		v1_copy = imath.V2f(v1)
		temp = v1
		temp /= v2
		self.assertEqual( temp, v1_copy / v2)

		v1_copy = imath.V2f(v1)
		temp = v1
		temp /= c
		self.assertEqual( temp, v1_copy / c)

		# matrix multiplication

		v1 = imath.V2f( 1, 2 )
		m = imath.M33f().translate( imath.V2f( 1, 2 ) )
		v2 = v1 * m
		v1 *= m
		self.assertEqual( v1, v2 )
		self.assertEqual( v1, imath.V2f( 2, 4 ) )


	def testMiscMethods(self):
		"""Test V2f miscellaneous methods"""
		v1 = imath.V2f(2.3, -4.98)

		self.assertAlmostEqual( v1.length2(), v1.dot(v1), 3 )
		self.assertAlmostEqual( v1.length(), math.sqrt(v1.dot(v1)), 3 )
		self.assertAlmostEqual( v1.length() * v1.length(), v1.length2(), 3 )

		v1 = imath.V2f(10.0, 0.0)
		self.assertEqual( v1.normalized(), v1 / v1.length() )
		self.assertEqual( v1, imath.V2f(10.0, 0.0) )

		v1.normalize()
		self.assertEqual( v1, imath.V2f(1.0, 0.0) )


class ImathV3f(unittest.TestCase):

	def testConstructors(self):
		"""Test V3f constructors"""
		v = imath.V3f()
		v = imath.V3f(1)
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)
		self.assertEqual(v.z, 1)
		v = imath.V3f(2, 3, 4)
		self.assertEqual(v.x, 2)
		self.assertEqual(v.y, 3)
		self.assertEqual(v.z, 4)

		v = imath.V3f( [ 1, 1, 1 ] )
		self.assertEqual(v.x, 1)
		self.assertEqual(v.y, 1)
		self.assertEqual(v.z, 1)

		self.assertRaises( RuntimeError, imath.V3f, [ 1 ] )
		self.assertRaises( RuntimeError, imath.V3f, [ 1, 2 ] )
		self.assertRaises( RuntimeError, imath.V3f, [ 1, 2, 3, 4 ] )

	def testDimensions(self):
		"""Test V3f dimensions"""

		v = imath.V3f()
		self.assertEqual( v.dimensions(), 3 )


	def testIndexing(self):
		"""Test V3f indexing via operator[]"""
		v1 = imath.V3f(1.0, 2.0, 3.0)

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
		v1 = imath.V3f(2.0)
		v2 = imath.V3f(3.0)

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
		v1 = imath.V3f(1.00)
		v2 = imath.V3f(1.01)

		self.assert_( v1.equalWithAbsError(v2, 0.01) )

		v1 = imath.V3f(2.0)
		v2 = imath.V3f(3.0)

		self.assert_( v1.equalWithRelError(v2, 0.5) )

		v1 = imath.V3f(1.0)
		v2 = imath.V3f(1.0)
		self.assert_( v1 == v2 )
		v1 = imath.V3f(1.0)
		v2 = imath.V3f(1.1)

		self.assert_( v1 != v2 )

	def testDotProduct(self):
		"""Test V3f dot product"""
		v1 = imath.V3f(3.0)
		v2 = imath.V3f(4.0)
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
		v1 = imath.V3f(1.0, 0.0, 0.0)
		v2 = imath.V3f(0.0, 1.0, 0.0)

		# Area of "parallelogram", by definition
		self.assertEqual( v1.cross(v2), imath.V3f(0.0, 0.0, 1.0) )

		# Operator/method equivalence
		self.assertEqual( v1.cross(v2), v1 % v2 )

	def testOperators(self):
		"""Test V3f arithmetic operators"""
		v1 = imath.V3f(3.4,  9.2, 18.05)
		v2 = imath.V3f(5.3, -0.4, -5.7 )

		# ADDITION

		# By definition
		self.assertEqual( v1 + v2, imath.V3f( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z ) )

		# Commutative
		self.assertEqual( v1 + v2, v2 + v1 )

		# Assignment
		v1_copy = imath.V3f(v1)
		temp = v1
		temp += v2
		self.assertEqual( temp, (v1_copy + v2))

		# SUBTRACTION

		# By definition
		self.assertEqual( v1 - v2, imath.V3f( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z ) )
		self.assertEqual( v1 - v2, -v2 + v1 )

		# Assignment
		v1_copy = imath.V3f(v1)
		temp = v1
		temp -= v2
		self.assertEqual( temp, v1_copy - v2)

		# NEGATION

		self.assertEqual( -v1, imath.V3f( -v1.x, -v1.y, -v1.z) )
		self.assertEqual( -v1, v1.negate() )
		self.assertEqual( -( -v1), v1 )

		# MULTIPLICATION

		# By definition
		self.assertEqual( v1 * v2, imath.V3f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z ) )
		c = 3
		self.assertEqual( v1 * c, imath.V3f(v1.x * c, v1.y * c, v1.z * c) )

		# Commutative
		self.assertEqual( v1 * v2, v2 * v1 )
		self.assertEqual( c * v1, imath.V3f(v1.x * c, v1.y * c, v1.z * c) )

		# Assignment
		v1_copy = imath.V3f(v1)
		temp = v1
		temp *= v2
		self.assertEqual( temp, v1_copy * v2)

		v1_copy = imath.V3f(v1)
		temp = v1
		temp *= c
		self.assertEqual( temp, v1_copy * c)

		# DIVISION

		# By definition
		self.assertEqual( v1 / v2, imath.V3f(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z) )
		self.assertEqual( v1 / c, imath.V3f(v1.x / c, v1.y / c, v1.z / c) )

		# Assignment
		v1_copy = imath.V3f(v1)
		temp = v1
		temp /= v2
		self.assertEqual( temp, v1_copy / v2)

		v1_copy = imath.V3f(v1)
		temp = v1
		temp /= c
		self.assertEqual( temp, v1_copy / c)

		# matrix multiplication

		v1 = imath.V3f( 1, 2, 3 )
		m = imath.M44f().translate( imath.V3f( 1, 2, 3 ) )
		v2 = v1 * m
		v1 *= m
		self.assertEqual( v1, v2 )
		self.assertEqual( v1, imath.V3f( 2, 4, 6 ) )

	def testMiscMethods(self):
		"""Test V3f miscellaneous methods"""
		v1 = imath.V3f(41.4, 2.3, -4.98)

		self.assertAlmostEqual( v1.length2(), v1.dot(v1), 3 )
		self.assertAlmostEqual( v1.length(), math.sqrt(v1.dot(v1)), 3 )
		self.assertAlmostEqual( v1.length() * v1.length(), v1.length2(), 3 )

		v1 = imath.V3f(10.0, 0.0, 0.0)
		self.assertEqual( v1.normalized(), v1 / v1.length() )
		self.assertEqual( v1, imath.V3f(10.0, 0.0, 0.0) )

		v1.normalize()
		self.assertEqual( v1, imath.V3f(1.0, 0.0, 0.0) )

	def testRepr( self ) :

		v1 = imath.V3f( 0.091242323423 )
		v2 = eval( "imath." + repr( v1 ) )
		self.assertEqual( v1, v2 )

class ImathBox3f(unittest.TestCase):
	def testConstructors(self):
		"""Test Box3f constructors"""
		b = imath.Box3f()
		self.assert_( b.isEmpty() )

		b = imath.Box3f( imath.V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.min(), imath.V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.max(), imath.V3f(1.0, 1.0, 1.0) )

		b = imath.Box3f( imath.V3f(-1.0, -1.0, -1.0), imath.V3f(1.0, 1.0, 1.0) )
		self.assertEqual( b.min(), imath.V3f(-1.0, -1.0, -1.0) )
		self.assertEqual( b.max(), imath.V3f( 1.0,  1.0,  1.0) )

	def testEquality(self):
		"""Test Box3f comparison for equality"""

		b1 = imath.Box3f( imath.V3f(1.0, 2.0, 3.0) )
		b2 = imath.Box3f( imath.V3f(1.0, 2.0, 3.0) )
		self.assert_( b1 == b2 )

		b2 = imath.Box3f( imath.V3f(3.0, 2.0, 1.0) )
		self.assert_( b1 != b2 )

	def testMiscMethods(self):
		"""Test Box3f miscellaneous methods"""

		b1 = imath.Box3f( imath.V3f(-1.0, -1.0, -1.0), imath.V3f(2.0, 2.0, 2.0) )
		self.assertEqual( b1.isEmpty(), False )
		self.assert_( b1.hasVolume() )

		b1.makeEmpty()
		self.assert_( b1.isEmpty() )
		self.assertEqual( b1.hasVolume(), False )

		b1 = imath.Box3f( imath.V3f(-1.0, -1.0, -1.0), imath.V3f(10.0, 2.0, 2.0) )

		X_AXIS = 0
		self.assertEqual( b1.majorAxis(), X_AXIS )

		self.assertEqual( b1.center(), (b1.min() + b1.max()) / 2.0 )

		b2 = imath.Box3f( imath.V3f(-0.5), imath.V3f(1.0) )
		self.assert_( b2.intersects(b1) )

		b2 = imath.Box3f( imath.V3f(-5.0), imath.V3f(-2.0) )
		self.failIf( b2.intersects(b1) )

		self.assertEqual( b2.size(), b2.max() - b2.min() )

		b = imath.Box3f( imath.V3f(1), imath.V3f(2) )
		m = imath.M44f()
		m[0][0]=2
		m[1][1]=2
		m[2][2]=2
		self.assertEqual( b * m, imath.Box3f( imath.V3f(2), imath.V3f(4) ) )
		m = imath.M44d()
		m[0][0]=2
		m[1][1]=2
		m[2][2]=2
		self.assertEqual( b * m, imath.Box3f( imath.V3f(2), imath.V3f(4) ) )

class ImathQuatf(unittest.TestCase):
	def testConstructors(self):
		"""Test Quatf constructors"""

		q = imath.Quatf()
		q = imath.Quatf(q)
		q = imath.Quatf(0.1, 0.2, 0.3, 0.4)
		q = imath.Quatf(0.1, imath.V3f(0.2, 0.3, 0.4))

	def testEquality(self):
		"""Test Quatf comparison for equality"""

		q1 = imath.Quatf( 1, 2, 3, 4 )
		q2 = imath.Quatf( 1, 2, 3, 4 )
		self.assertEqual(q1, q1)
		self.assertEqual(q1, q2)

		q2 = imath.Quatf( 5, 2, 3, 4 )
		self.assert_( q1 != q2 )

	def testMiscMethods(self):
		"""Test Quatf miscellaneous methods"""

		q1 = imath.Quatf( 1, 2, 3, 4 )
		self.assertAlmostEqual(
			q1.length(),
			math.sqrt(
				q1.r()*q1.r()+(q1.v()^q1.v())
			),
			3
		)

		# axis/angle
		axis = imath.V3f( 1, 2, 3 )
		axis.normalize()

		q1.setAxisAngle( axis, 0.5 )
		self.assertAlmostEqual( q1.axis().x, axis.x, 3 )
		self.assertAlmostEqual( q1.axis().y, axis.y, 3 )
		self.assertAlmostEqual( q1.axis().z, axis.z, 3 )

		self.assertAlmostEqual( q1.angle(), 0.5, 3 )

		# Rotate x axis onto y axis
		q1.setRotation( imath.V3f(1,0,0), imath.V3f(0,1,0) )

		#We should have gone 90 degrees about the +ve z-axis
		self.assertAlmostEqual( q1.angle(), 90.0 * math.pi / 180.0, 3 )

		self.assertAlmostEqual( q1.axis().x, 0.0, 3 )
		self.assertAlmostEqual( q1.axis().y, 0.0, 3 )
		self.assertAlmostEqual( q1.axis().z, 1.0, 3 )

		#inversion
		q1 = imath.Quatf( 1, 2, 3, 4 )
		qdot = q1 ^ q1
		qi_test = imath.Quatf( q1.r() / qdot, -q1.v() / qdot)
		qi = q1.inverse()

		self.assertAlmostEqual(qi.r(), qi_test.r(), 3)
		self.assertAlmostEqual(qi.v()[0], qi_test.v()[0], 3)
		self.assertAlmostEqual(qi.v()[1], qi_test.v()[1], 3)
		self.assertAlmostEqual(qi.v()[2], qi_test.v()[2], 3)

		q1.invert()
		self.assertAlmostEqual(qi.r(), qi_test.r(), 3)
		self.assertAlmostEqual(qi.v()[0], qi_test.v()[0], 3)
		self.assertAlmostEqual(qi.v()[1], qi_test.v()[1], 3)
		self.assertAlmostEqual(qi.v()[2], qi_test.v()[2], 3)

		#slerp
		q2 = imath.Quatf( 0.5, 0.6, 0.7, 0.8 )
		qs = q1.slerp(q2, 0.5)

		# normalization
		qn = qi.normalized()
		qn_test = imath.Quatf( qi.r() / qi.length(), qi.v() / qi.length() )

		self.assertAlmostEqual(qn.r(), qn_test.r(), 3)
		self.assertAlmostEqual(qn.v()[0], qn_test.v()[0], 3)
		self.assertAlmostEqual(qn.v()[1], qn_test.v()[1], 3)
		self.assertAlmostEqual(qn.v()[2], qn_test.v()[2], 3)

		qn = qi.normalize()
		self.assertAlmostEqual(qn.r(), qn_test.r(), 3)
		self.assertAlmostEqual(qn.v()[0], qn_test.v()[0], 3)
		self.assertAlmostEqual(qn.v()[1], qn_test.v()[1], 3)
		self.assertAlmostEqual(qn.v()[2], qn_test.v()[2], 3)

		#matrix conversion
		fromDir = imath.V3f(1,0,0)
		toDir = imath.V3f(0,1,0)
		q1.setRotation( fromDir, toDir )
		m = q1.toMatrix33()
		m = q1.toMatrix44()

	def testOperators(self):
		"""Test Quatf operators"""

		q1 = imath.Quatf( 1, 2, 3, 4 )
		q2 = imath.Quatf( 5, 6, 7, 8 )
		self.assertAlmostEqual( q1 ^ q2, q1.r() * q2.r() + (q1.v() ^ q2.v() ), 3 )

	def testSlerpStability( self ) :

		q1 = imath.Quatd( 0.60477471085951961527, 0.19082800913200048676, -0.73048263950686898038, 0.25343112163777203882, )
		q2 = imath.Quatd( 0.6047747108595192822, 0.190828009132000459, -0.73048263950686909141,    0.25343112163777264945, )

		q3 = q1.slerp( q2, 0.5 )

		self.assert_( q1.v().equalWithAbsError( q3.v(), 0.000000000000001 ) )
		self.assertAlmostEqual( q1.r(), q3.r(), 14 )

class ImathM33f(unittest.TestCase):
	def testConstructors(self):
		"""Test M33f constructors"""
		m = imath.M33f()

		m = imath.M33f(2)

		m = imath.M33f(1, 0, 0,
		              0, 1, 0,
                              0, 0, 1);

		m = imath.M33f( 1, 0, 0, 0, 1, 0, 0, 0, 1 )

	def testCopyAndAssign(self):
		"""Test M33f copy construction and assignment"""

		m1 = imath.M33f()
		m2 = imath.M33f(m1)
		self.failIf(m1 is m2)

	def testIndexing(self):
		"""Test M33f indexing via [] operator"""

		m = imath.M33f()

		# test __setitem__
		m[0][0] = 10.0
		m[0][1] = 11.0
		m[0][2] = 12.0
		m[1][0] = 13.0
		m[1][1] = 14.0
		m[1][2] = 15.0
		m[2][0] = 16.0
		m[2][1] = 17.0
		m[2][2] = 18.0

		# test __getitem__
		self.assertEqual( m[0][0], 10.0 )
		self.assertEqual( m[0][1], 11.0 )
		self.assertEqual( m[0][2], 12.0 )
		self.assertEqual( m[1][0], 13.0 )
		self.assertEqual( m[1][1], 14.0 )
		self.assertEqual( m[1][2], 15.0 )
		self.assertEqual( m[2][0], 16.0 )
		self.assertEqual( m[2][1], 17.0 )
		self.assertEqual( m[2][2], 18.0 )

		self.assertEqual( m, imath.M33f( 10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0 ) )

	def testOperators(self):
		"""Test M33f operators"""

		x = 10
		y = 2
		m1 = imath.M33f(x)
		m2 = imath.M33f(y)

		self.assertEqual(m1 + m2, imath.M33f(x + y))
		self.assertEqual(m1 - m2, imath.M33f(x - y))
		self.assertEqual(m1 * y, imath.M33f(x * y))
		self.assertEqual(m1 / y, imath.M33f(x / y))

	def testMiscellaneousMethods(self):
		"""Test M33f miscellaneous methods"""

		m1 = imath.M33f()
		m1.makeIdentity()

		m1 = imath.M33f(3)
		m2 = imath.M33f(3.1)
		self.assert_( m1.equalWithAbsError(m2, 0.1) )

		m1 = imath.M33f(2)
		m2 = imath.M33f(3)
		self.assert_( m1.equalWithRelError(m2, 0.51) )

		m1 = imath.M33f(1, 0, 0,
		               0, 2, 0,
			       0, 0, 3)
		self.assertEqual( m1.transposed().transposed(), m1)



	def testEquality(self):
		"""Test M33f comparison for equality"""
		m1 = imath.M33f(3)
		m2 = imath.M33f(3)

		self.assertEqual(m1, m2)

	def testMultMethods( self ) :

		v = imath.M33f().translate( imath.V2f( 1, 2 ) ).multVecMatrix( imath.V2f( 0 ) )
		self.assertEqual( v, imath.V2f( 1, 2 ) )

		v = imath.M33f().translate( imath.V2f( 1, 2 ) ).multDirMatrix( imath.V2f( 1 ) )
		self.assertEqual( v, imath.V2f( 1 ) )

	def testDeterminant( self ) :

		m = imath.M33f()
		self.assertAlmostEqual( m.determinant(), 1, 10 )
		m.scale( imath.V2f( -1, 1 ) )
		self.assertAlmostEqual( m.determinant(), -1, 10 )
		m.scale( imath.V2f( 1, -1 ) )
		self.assertAlmostEqual( m.determinant(), 1, 10 )
		m.scale( imath.V2f( 3, -1 ) )
		self.assertAlmostEqual( m.determinant(), -3, 10 )
		m.scale( imath.V2f( 3, 3 ) )
		self.assertAlmostEqual( m.determinant(), -27, 10 )

	def testConstructFromOtherType( self ) :

		md = imath.M33d( 1, 2, 3, 4, 5, 6, 7, 8, 9 )
		mf = imath.M33f( 1, 2, 3, 4, 5, 6, 7, 8, 9 )

		mf2 = imath.M33f( md )
		self.assertEqual( mf2, mf )

		md2 = imath.M33d( mf )
		self.assertEqual( md2, md )

class ImathM44f(unittest.TestCase):
	def testConstructors(self):
		"""Test M44f constructors"""
		m = imath.M44f(1., 0., 0., 0.,
		              0., 1., 0., 0.,
			      0., 0., 1., 0.,
			      0., 0., 0., 1.);

		m3 = imath.M33f(1., 0., 0.,
		               0., 1., 0.,
			       0., 0., 1.)

		t = imath.V3f(5., 5., 5.)

		m = imath.M44f( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 )

	def testCopyAndAssign(self):
		"""Test M44f copy construction and assignment"""
		m1 = imath.M44f()
		m2 = imath.M44f(m1)
		self.failIf(m1 is m2)

		m1 = m2

	def testIndexing(self):
		"""Test M44f indexing via [] operator"""
		m = imath.M44f()

		# test tuple indexing:
		m[0][0] = 10.0
		m[0][1] = 11.0
		m[0][2] = 12.0
		m[0][3] = 13.0
		m[1][0] = 14.0
		m[1][1] = 15.0
		m[1][2] = 16.0
		m[1][3] = 17.0
		m[2][0] = 18.0
		m[2][1] = 19.0
		m[2][2] = 20.0
		m[2][3] = 21.0
		m[3][0] = 22.0
		m[3][1] = 23.0
		m[3][2] = 24.0
		m[3][3] = 25.0

		# test __getitem__( tuple ):
		self.assertEqual( m[0][0], 10.0 )
		self.assertEqual( m[0][1], 11.0 )
		self.assertEqual( m[0][2], 12.0 )
		self.assertEqual( m[0][3], 13.0 )
		self.assertEqual( m[1][0], 14.0 )
		self.assertEqual( m[1][1], 15.0 )
		self.assertEqual( m[1][2], 16.0 )
		self.assertEqual( m[1][3], 17.0 )
		self.assertEqual( m[2][0], 18.0 )
		self.assertEqual( m[2][1], 19.0 )
		self.assertEqual( m[2][2], 20.0 )
		self.assertEqual( m[2][3], 21.0 )
		self.assertEqual( m[3][0], 22.0 )
		self.assertEqual( m[3][1], 23.0 )
		self.assertEqual( m[3][2], 24.0 )
		self.assertEqual( m[3][3], 25.0 )

		self.assertEqual( m, imath.M44f( 10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,25.0 ) )

	def testOperators(self):
		"""Test M44f operators"""
		x = 10
		y = 2
		m1 = imath.M44f(x)
		m2 = imath.M44f(y)

		self.assertEqual(m1 + m2, imath.M44f(x + y))
		self.assertEqual(m1 - m2, imath.M44f(x - y))
		self.assertEqual(m1 * y, imath.M44f(x * y))
		self.assertEqual(m1 / y, imath.M44f(x / y))

	def testMiscellaneousMethods(self):
		"""Test M44f miscellaneous methods"""
		m1 = imath.M44f()
		m1.makeIdentity()

		m1 = imath.M44f(3)
		m2 = imath.M44f(3.1)
		self.assert_( m1.equalWithAbsError(m2, 0.1) )

		m1 = imath.M44f(2)
		m2 = imath.M44f(3)
		self.assert_( m1.equalWithRelError(m2, 0.51) )

		m1 = imath.M44f(1, 0, 0, 0,
		               0, 2, 0, 0,
			       0, 0, 3, 0,
			       0, 0, 0, 4)
		self.assertEqual( m1.transposed().transposed(), m1)

	def testEquality(self):
		"""Test M44f comparison for equality"""

		m1 = imath.M44f(3)
		m2 = imath.M44f(3)

		self.assertEqual(m1, m2)

	def testMultMethods( self ) :

		v = imath.M44f().translate( imath.V3f( 1, 2, 3 ) ).multVecMatrix( imath.V3f( 0 ) )
		self.assertEqual( v, imath.V3f( 1, 2, 3 ) )

		v = imath.M44f().translate( imath.V3f( 1, 2, 3 ) ).multDirMatrix( imath.V3f( 1 ) )
		self.assertEqual( v, imath.V3f( 1 ) )

	def testDeterminant( self ) :

		m = imath.M44f()
		self.assertAlmostEqual( m.determinant(), 1, 10 )
		m.scale( imath.V3f( -1, 1, 1 ) )
		self.assertAlmostEqual( m.determinant(), -1, 10 )
		m.scale( imath.V3f( 1, -1, 1 ) )
		self.assertAlmostEqual( m.determinant(), 1, 10 )
		m.scale( imath.V3f( 3, -1, 1 ) )
		self.assertAlmostEqual( m.determinant(), -3, 10 )
		m.scale( imath.V3f( 3, 3, 1 ) )
		self.assertAlmostEqual( m.determinant(), -27, 10 )

		random.seed( 42 )

		r = IECore.curry( random.uniform, -2, 2 )
		for i in range( 0, 1000 ) :

			m = imath.M44f( r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r(), r() )
			d = m.determinant()

			if math.fabs( d ) > 0.00001 :

				mi = m.inverse()
				di = mi.determinant()

				self.assertAlmostEqual( d, 1/di, 3 )

			mt = m.transposed()
			self.assertAlmostEqual( d, mt.determinant(), 4 )

		for i in range( 0, 1000 ) :

			m = imath.M44f()
			m.translate( imath.V3f( r(), r(), r() ) )
			self.assertAlmostEqual( m.determinant(), 1, 10 )

	def testConstructFromOtherType( self ) :

		md = imath.M44d( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 )
		mf = imath.M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 )

		mf2 = imath.M44f( md )
		self.assertEqual( mf2, mf )

		md2 = imath.M44d( mf )
		self.assertEqual( md2, md )

class ImathColor3Test( unittest.TestCase ) :

	def test( self ) :

		c = imath.Color3f( 1 )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 1 )
		self.assertEqual( c.b, 1 )

		c = imath.Color3f( 1, 2, 3 )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 2 )
		self.assertEqual( c.b, 3 )

		cc = imath.Color3f( c )
		self.assertEqual( c, cc )

		c = imath.Color3f( imath.V3f(1,2,3) )
		self.assertEqual( c.r, 1 )
		self.assertEqual( c.g, 2 )
		self.assertEqual( c.b, 3 )

		c = imath.Color3f( imath.V3d(1,2,3) )
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

		cm -= imath.Color3f( 2 )
		self.assertEqual( cm, imath.Color3f( -4, -10, -20 ) )

		self.assertEqual( c.dimensions(), 3 )

	def testHSVTransforms( self ) :

		c = imath.Color3f( 0.1, 0.2, 0.3 )

		chsv = c.rgb2hsv()
		self.assertEqual( c, imath.Color3f( 0.1, 0.2, 0.3 ) )
		self.failUnless( isinstance( chsv, imath.Color3f ) )
		self.failUnless( chsv.equalWithAbsError( imath.Color3f( 0.5833, 0.6667, 0.3 ), 0.001 ) )

		crgb = chsv.hsv2rgb()
		self.failUnless( chsv.equalWithAbsError( imath.Color3f( 0.5833, 0.6667, 0.3 ), 0.001 ) )
		self.failUnless( crgb.equalWithAbsError( c, 0.001 ) )

	def testRepr( self ) :

		c1 = imath.Color3f( 0.091242323423 )
		c2 = eval( "imath." + repr( c1 ) )
		self.assertEqual( c1, c2 )

class ImathEulerfTest( unittest.TestCase ) :

	def testConstructors(self):
		"""Test Eulerf constructors"""

		#
		e = imath.Eulerf()
		self.assertEqual( e.x, 0 )
		self.assertEqual( e.y, 0 )
		self.assertEqual( e.z, 0 )

		self.assertEqual( e.order(), imath.Eulerf.Order.XYZ )

		#
		ecopy = imath.Eulerf(e)
		self.assertEqual( ecopy.x, 0 )
		self.assertEqual( ecopy.y, 0 )
		self.assertEqual( ecopy.z, 0 )

		self.assertEqual( ecopy.order(), imath.Eulerf.Order.XYZ )

		#
		e = imath.Eulerf( imath.Eulerf.Order.ZYX )
		self.assertEqual( e.order(), imath.Eulerf.Order.ZYX )

		#
		e = imath.Eulerf( imath.V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), imath.Eulerf.Order.XYZ )

		e = imath.Eulerf( imath.V3f( 0, 0, 0 ), imath.Eulerf.Order.ZYX )
		self.assertEqual( e.order(), imath.Eulerf.Order.ZYX )

		#
		e = imath.Eulerf( 0, 0, 0 )
		e = imath.Eulerf( imath.V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), imath.Eulerf.Order.XYZ )

		e = imath.Eulerf( 0, 0, 0, imath.Eulerf.Order.ZYX  )
		self.assertEqual( e.order(), imath.Eulerf.Order.ZYX )

		#
		e = imath.Eulerf( imath.M33f() )
		e = imath.Eulerf( imath.V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), imath.Eulerf.Order.XYZ )

		e = imath.Eulerf( imath.M33f(), imath.Eulerf.Order.ZYX )
		self.assertEqual( e.order(), imath.Eulerf.Order.ZYX )

		#
		e = imath.Eulerf( imath.M44f() )
		e = imath.Eulerf( imath.V3f( 0, 0, 0 ) )
		self.assertEqual( e.order(), imath.Eulerf.Order.XYZ )

		e = imath.Eulerf( imath.M44f(), imath.Eulerf.Order.ZYX )
		self.assertEqual( e.order(), imath.Eulerf.Order.ZYX )


	def testOrder(self):
		"""Test Eulerf order"""

		self.assertEqual( len( imath.Eulerf.Order.values ), 24 )

		e = imath.Eulerf()

		for order in imath.Eulerf.Order.values.values():
			e.setOrder( order )
			self.assertEqual( e.order(), order )

	def testMisc(self):
		"""Test Eulerf miscellaneous"""

		self.assertEqual( len(imath.Eulerf.Axis.values), 3 )
		self.assertEqual( len(imath.Eulerf.InputLayout.values), 2 )

		self.assert_( imath.V3f in imath.Eulerf.__bases__ )

	def testExtract(self):

		"""Test Eulerf extract"""

		e = imath.Eulerf()
		e.extract( imath.M33f() )

		e.extract( imath.M44f() )

		e.extract( imath.Quatf() )

		m = e.toMatrix33()
		m = e.toMatrix44()
		q = e.toQuat()
		v = e.toXYZVector()

	def testAngleOrder(self):

		"""Test Eulerf angleOrder"""

		e = imath.Eulerf()

		o = e.angleOrder()

		self.assertIsInstance( o, imath.V3i )
		self.assertEqual( len(o), 3 )

	def testStr(self):
		"""Test Eulerf str"""

		e = imath.Eulerf()
		self.assertEqual( str(e), "Eulerf(0, 0, 0, EULER_XYZ)" )

	def testRepr(self):
		"""Test Eulerf repr"""

		e = imath.Eulerf()
		self.assertEqual( repr(e), "Eulerf(0, 0, 0, EULER_XYZ)" )

class ImathPlane3fTest( unittest.TestCase ) :

	def testConstructors( self ) :

		p = imath.Plane3f( imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 ) )
		self.assertEqual( p.normal(), imath.V3f( 1, 0, 0 ) )
		self.assertEqual( p.distance(), 0 )

		p = imath.Plane3f( imath.V3f( 0, 0, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 0, 1 ) )
		self.assertEqual( p.normal(), imath.V3f( 1, 0, 0 ) )
		self.assertEqual( p.distance(), 0 )

		p = imath.Plane3f( imath.V3f( 2, 2, 2 ), imath.V3f( 2, 3, 2 ), imath.V3f( 2, 2, 3 ) )
		self.assertEqual( p.normal(), imath.V3f( 1, 0, 0 ) )
		self.assertEqual( p.distance(), 2 )


if __name__ == "__main__":
    unittest.main()

