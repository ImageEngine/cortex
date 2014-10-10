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

from IECore import *
import IECore

class SimpleTypedDataTest(unittest.TestCase):

	def testIntData(self):
		"""Test IntData"""
		a = IntData()
		self.assert_(type(a) is IntData)
		b = IntData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(int(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = IntData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( IntData() ), "0" )
		self.assertEqual( repr( IntData() ), "IECore.IntData( 0 )" )
		self.failUnless( IntData.hasBase() )

	def testUIntData(self):
		"""Test UIntData"""
		a = UIntData()
		self.assert_(type(a) is UIntData)
		b = UIntData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(long(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = UIntData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( UIntData() ), "0" )
		self.assertEqual( repr( UIntData() ), "IECore.UIntData( 0 )" )
		self.failUnless( UIntData.hasBase() )

	def testFloatData(self):
		"""Test FloatData"""
		a = FloatData()
		self.assert_(type(a) is FloatData)
		b = FloatData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(float(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = FloatData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( FloatData() ), "0" )
		self.assertEqual( repr( FloatData() ), "IECore.FloatData( 0 )" )
		self.failUnless( FloatData.hasBase() )

	def testDoubleData(self):
		"""Test DoubleData"""
		a = DoubleData()
		self.assert_(type(a) is DoubleData)
		b = DoubleData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(float(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = DoubleData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( DoubleData() ), "0" )
		self.assertEqual( repr( DoubleData() ), "IECore.DoubleData( 0 )" )
		self.failUnless( DoubleData.hasBase() )

	def testCharData(self):
		"""Test CharData"""
		a = CharData()
		self.assert_(type(a) is CharData)
		b = CharData('1')
		c = b.copy()
		self.assertEqual(b.value, '1')
		b.value = '2'
		self.assertEqual(b.value, '2')
		a = CharData('2')
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( CharData() ), "0" )
		self.assertEqual( repr( CharData() ), "IECore.CharData( 0 )" )
		self.failUnless( CharData.hasBase() )

	def testUCharData(self):
		"""Test UCharData"""
		a = UCharData()
		self.assert_(type(a) is UCharData)
		b = UCharData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(int(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = UCharData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( UCharData() ), "0" )
		self.assertEqual( repr( UCharData() ), "IECore.UCharData( 0 )" )
		self.failUnless( UCharData.hasBase() )

	def testHalfData(self):
		"""Test HalfData"""
		a = HalfData()
		self.assert_(type(a) is HalfData)
		b = HalfData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(float(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = HalfData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( HalfData() ), "0" )
		self.assertEqual( repr( HalfData() ), "IECore.HalfData( 0 )" )
		self.failUnless( HalfData.hasBase() )

	def testShortData(self):
		"""Test ShortData"""
		a = ShortData()
		self.assert_(type(a) is ShortData)
		b = ShortData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(int(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = ShortData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( ShortData() ), "0" )
		self.assertEqual( repr( ShortData() ), "IECore.ShortData( 0 )" )
		self.failUnless( ShortData.hasBase() )

	def testUShortData(self):
		"""Test UShortData"""
		a = UShortData()
		self.assert_(type(a) is UShortData)
		b = UShortData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(int(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = UShortData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( UShortData() ), "0" )
		self.assertEqual( repr( UShortData() ), "IECore.UShortData( 0 )" )
		self.failUnless( UShortData.hasBase() )

	def testInt64Data(self):
		"""Test Int64Data"""
		a = Int64Data()
		self.assert_(type(a) is Int64Data)
		b = Int64Data(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(long(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = Int64Data(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( Int64Data() ), "0" )
		self.assertEqual( repr( Int64Data() ), "IECore.Int64Data( 0 )" )
		self.failUnless( Int64Data.hasBase() )

	def testUInt64Data(self):
		"""Test UInt64Data"""
		a = UInt64Data()
		self.assert_(type(a) is UInt64Data)
		b = UInt64Data(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(long(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = UInt64Data(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( UInt64Data() ), "0" )
		self.assertEqual( repr( UInt64Data() ), "IECore.UInt64Data( 0 )" )
		self.failUnless( UInt64Data.hasBase() )

	def testImathVecTypes(self):

		types = [
			[V2fData, V2f],
			[V3fData, V3f],
			[V2dData, V2d],
			[V3dData, V3d],
			[Color3fData, Color3f],
			[Color3dData, Color3d],
			[Color4fData, Color4f],
			[Color4dData, Color4d], ]

		for t, vt in types :

			self.assertEqual( t(), t( vt( 0 ) ) )

			v = vt( 10 )
			self.assertEqual( " ".join( ["10"] * vt.dimensions() ), str( t( v ) ) )
			self.assertEqual( "IECore." + t.__name__ + "( " + "IECore." + vt.__name__ + "( " + ", ".join( ["10"] * vt.dimensions() ) + " ) )", repr( t( v ) ) )
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
			[Box2fData, Box2f, V2f],
			[Box3fData, Box3f, V3f],
			[Box2dData, Box2d, V2d],
			[Box3dData, Box3d, V3d] ]

		for t, bt, vt in types :

			v = vt( 1 )
			b = bt( v, v )
			self.assertEqual( " ".join( ["1"]*vt.dimensions()*2 ), str( t( b ) ) )
			vr = "IECore." + vt.__name__ + "( " + ", ".join( ["1"]*vt.dimensions() ) + " )"
			br = "IECore." + bt.__name__ + "( " + vr + ", " + vr + " )"
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
	
		s = InternedStringData( "i" )
		self.assertEqual( str( s ), "i" )
		self.assertEqual( repr(s ), 'IECore.InternedStringData( "i" )' )

		m = MemoryIndexedIO( CharVectorData(), [], IndexedIO.OpenMode.Append )
		s.save( m, "o" )
		
		s2 = Object.load( m, "o" )
		
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
		
			self.assertTrue( IntData( 10 ) < IntData( 20 ) )
			self.assertFalse( IntData( 20 ) < IntData( 10 ) )
			
			self.assertTrue( IntData( 11 ) > IntData( 10 ) )
			self.assertFalse( IntData( 10 ) > IntData( 10 ) )
			
			self.assertTrue( IntData( 11 ) >= IntData( 11 ) )
			self.assertTrue( IntData( 12 ) >= IntData( 11 ) )
			self.assertFalse( IntData( 9 ) >= IntData( 10 ) )
			
			self.assertTrue( IntData( 11 ) <= IntData( 11 ) )
			self.assertTrue( IntData( 10 ) <= IntData( 11 ) )
			self.assertFalse( IntData( 11 ) <= IntData( 10 ) )
			
	def testEqualityDoesntThrow( self ) :
	
		self.assertFalse( IntData( 100 ) == None )
		self.assertTrue( IntData( 100 ) != None )
		
class BoolDataTest( unittest.TestCase ) :

	def test( self ) :

		a = BoolData()
		self.assert_(type(a) is BoolData)
		b = IntData(True)
		c = b.copy()
		self.assertEqual(b.value, True)
		b.value = False
		self.assertEqual(b.value, False)
		self.assertEqual(c.value, True)

		self.failUnless( BoolData.hasBase() )

	def testStreaming( self ) :

		o = BoolData( True )
		self.assertEqual( o.value, True )

		iface = IndexedIO.create( "test/IECore/o.fio", IndexedIO.OpenMode.Write )

		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( o, oo )

		o = BoolData( False )
		self.assertEqual( o.value, False )

		o.save( iface, "test" )
		oo = Object.load( iface, "test" )
		self.assertEqual( o, oo )

	def testLineSegmentData( self ) :
	
		for vt, dt in [
			( V3f, LineSegment3fData ),
			( V3d, LineSegment3dData ),
		] :

			d = dt()
			self.assertEqual( d.value.p0, vt( 0, 0, 0 ) )
			self.assertEqual( d.value.p1, vt( 1, 0, 0 ) )

	def tearDown( self ) :

		if os.path.isfile("test/IECore/o.fio"):
			os.remove("test/IECore/o.fio")

if __name__ == "__main__":
    unittest.main()

