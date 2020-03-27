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

"""Unit test for SimpleTypedData binding"""

import os
import math
import unittest
import random
import six
import imath

import IECore

class SimpleTypedDataTest(unittest.TestCase):

	def __toLong( self, o ) :

		if six.PY3 :
			return int( o )
		else :
			return long( o )

	def testIntData(self):
		"""Test IntData"""
		a = IECore.IntData()
		self.assert_(type(a) is IECore.IntData)
		b = IECore.IntData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(int(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.IntData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.IntData() ), "0" )
		self.assertEqual( repr( IECore.IntData() ), "IECore.IntData( 0 )" )
		self.failUnless( IECore.IntData.hasBase() )

	def testUIntData(self):
		"""Test UIntData"""
		a = IECore.UIntData()
		self.assert_(type(a) is IECore.UIntData)
		b = IECore.UIntData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual( self.__toLong( b ), 1 )
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.UIntData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.UIntData() ), "0" )
		self.assertEqual( repr( IECore.UIntData() ), "IECore.UIntData( 0 )" )
		self.failUnless( IECore.UIntData.hasBase() )

	def testFloatData(self):
		"""Test FloatData"""
		a = IECore.FloatData()
		self.assert_(type(a) is IECore.FloatData)
		b = IECore.FloatData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(float(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.FloatData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.FloatData() ), "0" )
		self.assertEqual( repr( IECore.FloatData() ), "IECore.FloatData( 0 )" )
		self.failUnless( IECore.FloatData.hasBase() )

	def testDoubleData(self):
		"""Test DoubleData"""
		a = IECore.DoubleData()
		self.assert_(type(a) is IECore.DoubleData)
		b = IECore.DoubleData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(float(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.DoubleData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.DoubleData() ), "0" )
		self.assertEqual( repr( IECore.DoubleData() ), "IECore.DoubleData( 0 )" )
		self.failUnless( IECore.DoubleData.hasBase() )

	def testCharData(self):
		"""Test CharData"""
		a = IECore.CharData()
		self.assert_(type(a) is IECore.CharData)
		b = IECore.CharData('1')
		c = b.copy()
		self.assertEqual(b.value, '1')
		b.value = '2'
		self.assertEqual(b.value, '2')
		a = IECore.CharData('2')
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.CharData() ), "0" )
		self.assertEqual( repr( IECore.CharData() ), "IECore.CharData( 0 )" )
		self.failUnless( IECore.CharData.hasBase() )

	def testUCharData(self):
		"""Test UCharData"""
		a = IECore.UCharData()
		self.assert_(type(a) is IECore.UCharData)
		b = IECore.UCharData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(int(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.UCharData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.UCharData() ), "0" )
		self.assertEqual( repr( IECore.UCharData() ), "IECore.UCharData( 0 )" )
		self.failUnless( IECore.UCharData.hasBase() )

	def testHalfData(self):
		"""Test HalfData"""
		a = IECore.HalfData()
		self.assert_(type(a) is IECore.HalfData)
		b = IECore.HalfData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(float(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.HalfData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.HalfData() ), "0" )
		self.assertEqual( repr( IECore.HalfData() ), "IECore.HalfData( 0 )" )
		self.failUnless( IECore.HalfData.hasBase() )

	def testShortData(self):
		"""Test ShortData"""
		a = IECore.ShortData()
		self.assert_(type(a) is IECore.ShortData)
		b = IECore.ShortData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(int(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.ShortData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.ShortData() ), "0" )
		self.assertEqual( repr( IECore.ShortData() ), "IECore.ShortData( 0 )" )
		self.failUnless( IECore.ShortData.hasBase() )

	def testUShortData(self):
		"""Test UShortData"""
		a = IECore.UShortData()
		self.assert_(type(a) is IECore.UShortData)
		b = IECore.UShortData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(int(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.UShortData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.UShortData() ), "0" )
		self.assertEqual( repr( IECore.UShortData() ), "IECore.UShortData( 0 )" )
		self.failUnless( IECore.UShortData.hasBase() )

	def testInt64Data(self):
		"""Test Int64Data"""
		a = IECore.Int64Data()
		self.assert_(type(a) is IECore.Int64Data)
		b = IECore.Int64Data(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual( self.__toLong( b ), 1 )
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.Int64Data(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.Int64Data() ), "0" )
		self.assertEqual( repr( IECore.Int64Data() ), "IECore.Int64Data( 0 )" )
		self.failUnless( IECore.Int64Data.hasBase() )

	def testUInt64Data(self):
		"""Test UInt64Data"""
		a = IECore.UInt64Data()
		self.assert_(type(a) is IECore.UInt64Data)
		b = IECore.UInt64Data(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual( self.__toLong( b ), 1 )
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IECore.UInt64Data(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IECore.UInt64Data() ), "0" )
		self.assertEqual( repr( IECore.UInt64Data() ), "IECore.UInt64Data( 0 )" )
		self.failUnless( IECore.UInt64Data.hasBase() )

	def testImathVecTypes(self):

		types = [
			[IECore.V2fData, imath.V2f],
			[IECore.V3fData, imath.V3f],
			[IECore.V2dData, imath.V2d],
			[IECore.V3dData, imath.V3d],
			[IECore.Color3fData, imath.Color3f],
			[IECore.Color4fData, imath.Color4f],
		]

		for t, vt in types :

			self.assertEqual( t(), t( vt( 0 ) ) )

			v = vt( 10 )
			self.assertEqual( " ".join( ["10"] * vt.dimensions() ), str( t( v ) ) )
			self.assertEqual( "IECore." + t.__name__ + "( " + "imath." + vt.__name__ + "( " + ", ".join( ["10"] * vt.dimensions() ) + " ) )", repr( t( v ) ) )
			self.failUnless( t.hasBase() )

			for i in range( 0, 1000 ) :
				v = vt( random.uniform( -100, 100 ) )
				d = t( v )
				self.assertEqual( d.value, v )
				v2 = vt( random.uniform( -100, 100 ) )
				d.value = v2
				self.assertEqual( d.value, v2 )

	def testImathBoxTypes(self):

		types = [
			[IECore.Box2fData, imath.Box2f, imath.V2f],
			[IECore.Box3fData, imath.Box3f, imath.V3f],
			[IECore.Box2dData, imath.Box2d, imath.V2d],
			[IECore.Box3dData, imath.Box3d, imath.V3d] ]

		for t, bt, vt in types :

			v = vt( 1 )
			b = bt( v, v )
			self.assertEqual( " ".join( ["1"]*vt.dimensions()*2 ), str( t( b ) ) )
			vr = "imath." + vt.__name__ + "( " + ", ".join( ["1"]*vt.dimensions() ) + " )"
			br = "imath." + bt.__name__ + "( " + vr + ", " + vr + " )"
			self.assertEqual( "IECore." + t.__name__ + "( " + br + " )", repr( t( b ) ) )
			self.failUnless( t.hasBase() )

			for i in range( 0, 1000 ) :

				b = bt()
				for j in range( 0, 10 ) :

					b.extendBy( vt( random.uniform( -100, 100 ) ) )

				v = t( b )
				self.assertEqual( v.value, b )
				b2 = bt()
				v.value = b2
				self.assertEqual( v.value, b2 )

	def testInternedStringData( self ) :

		s = IECore.InternedStringData( "i" )
		self.assertEqual( str( s ), "i" )
		self.assertEqual( repr(s ), 'IECore.InternedStringData( "i" )' )

		m = IECore.MemoryIndexedIO( IECore.CharVectorData(), [], IECore.IndexedIO.OpenMode.Append )
		s.save( m, "o" )

		s2 = IECore.Object.load( m, "o" )

		self.assertEqual( s2.value.value(), "i" )
		self.assertEqual( s, s2 )

	def testStringDataRepr( self ) :

		for s in [
			"a\nb",
			"a'",
			'"',
		] :
			d = IECore.StringData( s )
			self.assertEqual( eval( repr( d ) ), d )

	def testComparison( self ) :

		for i in range( 0, 1000 ) :

			self.assertTrue( IECore.IntData( 10 ) < IECore.IntData( 20 ) )
			self.assertFalse( IECore.IntData( 20 ) < IECore.IntData( 10 ) )

			self.assertTrue( IECore.IntData( 11 ) > IECore.IntData( 10 ) )
			self.assertFalse( IECore.IntData( 10 ) > IECore.IntData( 10 ) )

			self.assertTrue( IECore.IntData( 11 ) >= IECore.IntData( 11 ) )
			self.assertTrue( IECore.IntData( 12 ) >= IECore.IntData( 11 ) )
			self.assertFalse( IECore.IntData( 9 ) >= IECore.IntData( 10 ) )

			self.assertTrue( IECore.IntData( 11 ) <= IECore.IntData( 11 ) )
			self.assertTrue( IECore.IntData( 10 ) <= IECore.IntData( 11 ) )
			self.assertFalse( IECore.IntData( 11 ) <= IECore.IntData( 10 ) )

	def testEqualityDoesntThrow( self ) :

		self.assertFalse( IECore.IntData( 100 ) == None )
		self.assertTrue( IECore.IntData( 100 ) != None )

class BoolDataTest( unittest.TestCase ) :

	def test( self ) :

		a = IECore.BoolData()
		self.assert_(type(a) is IECore.BoolData)
		b = IECore.IntData(True)
		c = b.copy()
		self.assertEqual(b.value, True)
		b.value = False
		self.assertEqual(b.value, False)
		self.assertEqual(c.value, True)

		self.failUnless( IECore.BoolData.hasBase() )

	def testStreaming( self ) :

		o = IECore.BoolData( True )
		self.assertEqual( o.value, True )

		iface = IECore.IndexedIO.create( "test/IECore/o.fio", IECore.IndexedIO.OpenMode.Write )

		o.save( iface, "test" )
		oo = IECore.Object.load( iface, "test" )
		self.assertEqual( o, oo )

		o = IECore.BoolData( False )
		self.assertEqual( o.value, False )

		o.save( iface, "test" )
		oo = IECore.Object.load( iface, "test" )
		self.assertEqual( o, oo )

	def testLineSegmentData( self ) :

		for vt, dt in [
			( imath.V3f, IECore.LineSegment3fData ),
			( imath.V3d, IECore.LineSegment3dData ),
		] :

			d = dt()
			self.assertEqual( d.value.p0, vt( 0, 0, 0 ) )
			self.assertEqual( d.value.p1, vt( 1, 0, 0 ) )

	def tearDown( self ) :

		if os.path.isfile("test/IECore/o.fio"):
			os.remove("test/IECore/o.fio")

if __name__ == "__main__":
    unittest.main()

