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

"""Unit test for SimpleTypedData binding"""

import os
import math
import unittest
from IECore import *
import random

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
		self.assertEqual( repr( IntData() ), "IntData( 0 )" )

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
		self.assertEqual( repr( UIntData() ), "UIntData( 0 )" )

	def testLongData(self):
		"""Test LongData"""
		a = LongData()
		self.assert_(type(a) is LongData)
		b = LongData(1)
		c = b.copy()
		self.assertEqual(b.value, 1)
		self.assertEqual(long(b), 1)
		b.value = 2
		self.assertEqual(b.value, 2)
		a = LongData(2)
		self.assert_(c < b)
		self.assert_(not c > b)
		self.assert_(a == b)
		self.assert_(not a == c)
		self.assertEqual( str( LongData() ), "0" )
		self.assertEqual( repr( LongData() ), "LongData( 0 )" )

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
		self.assertEqual( repr( FloatData() ), "FloatData( 0 )" )

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
		self.assertEqual( repr( DoubleData() ), "DoubleData( 0 )" )
		
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
		self.assertEqual( repr( CharData() ), "CharData( 0 )" )

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
		self.assertEqual( repr( UCharData() ), "UCharData( 0 )" )
		
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
		self.assertEqual( repr( HalfData() ), "HalfData( 0 )" )
		
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
		self.assertEqual( repr( ShortData() ), "ShortData( 0 )" )
		
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
		self.assertEqual( repr( UShortData() ), "UShortData( 0 )" )	
		
			
		
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
		
			v = vt( 10 )
			self.assertEqual( " ".join( ["10"] * vt.dimensions() ), str( t( v ) ) )
			self.assertEqual( t.__name__ + "( " + vt.__name__ + "( " + ", ".join( ["10"] * vt.dimensions() ) + " ) )", repr( t( v ) ) )
			
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
			vr = vt.__name__ + "( " + ", ".join( ["1"]*vt.dimensions() ) + " )"
			br = bt.__name__ + "( " + vr + ", " + vr + " )"
			self.assertEqual( t.__name__ + "( " + br + " )", repr( t( b ) ) )
			
			for i in range( 0, 1000 ) :
			
				b = bt()
				for j in range( 0, 10 ) :
				
					b.extendBy( vt( random.uniform( -100, 100 ) ) )
					
				v = t( b )
				self.assertEqual( v.value, b )
				b2 = bt()
				v.value = b2
				self.assertEqual( v.value, b2 )

class BoolDataTest( unittest.TestCase ) :

	def test( self ) :
	
		a = BoolData()
		self.assert_(type(a) is BoolData)
		b = LongData(True)
		c = b.copy()
		self.assertEqual(b.value, True)
		b.value = False
		self.assertEqual(b.value, False)
		self.assertEqual(c.value, True)
		
	def testStreaming( self ) :
	
		o = BoolData( True )
		self.assertEqual( o.value, True )
		
		o.save( "test/o.fio" )
		oo = Object.load( "test/o.fio" )
		self.assertEqual( o, oo )
		
		o = BoolData( False )
		self.assertEqual( o.value, False )
		
		o.save( "test/o.fio" )
		oo = Object.load( "test/o.fio" )
		self.assertEqual( o, oo )
		
	def tearDown( self ) :
	
		if os.path.isfile("test/o.fio"):
			os.remove("test/o.fio")		
			
if __name__ == "__main__":
    unittest.main()   

