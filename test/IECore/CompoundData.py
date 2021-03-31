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

"""Unit test for CompoundData binding"""

import os
import math
import unittest
import sys
import subprocess
import imath

import IECore

class CompoundDataTest(unittest.TestCase):

	def testConstructors(self):
		"""Test constructors"""
		v1 = IECore.CompoundData()
		a = dict()
		a["1"] = IECore.IntData(1)
		v3 = IECore.CompoundData(a)
		self.assertEqual(v3.size(), 1)

	def testResize(self):
		"""Test resizing"""
		v = IECore.CompoundData()
		v["0"] = IECore.FloatData(2)
		self.assertEqual(v["0"], IECore.FloatData(2))
		v["1"] = IECore.FloatData(0)
		v["2"] = IECore.FloatData(3)
		v["3"] = IECore.FloatData(2)
		v["4"] = IECore.FloatData(5)
		self.assertEqual(v["4"], IECore.FloatData(5))
		self.assertEqual(len(v), 5)
		del(v["0"])
		self.assertEqual(len(v), 4)
		self.assertTrue(v.has_key("0") == False)
		v.clear()
		self.assertEqual(len(v), 0)

	def testAssignment(self):
		"""Test assignment"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v2 = v1.copy()
		v3 = v1
		v4 = v1.copy()
		self.assertEqual(len(v1), 2)
		self.assertEqual(len(v1), len(v2))
		self.assertEqual(v1["0"], v2["0"])
		self.assertEqual(v1["1"], v2["1"])
		self.assertEqual(v1["0"], v4["0"])
		self.assertEqual(v1["1"], v4["1"])
		self.assertRaises( TypeError, v1.__setitem__, "2", None )	# should prevent setting None as value.

	def testCopyOnWrite(self):
		"""Test copy-on-write behavior"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v2 = v1.copy()
		v3 = v1.copy()
		v3["0"] = IECore.UIntData(5)
		self.assertTrue(v3["0"] == IECore.UIntData(5))
		self.assertTrue(v2["0"] == IECore.FloatData(1.2))
		v1["2"] = IECore.FloatData(5);
		self.assertEqual(len(v1), 3)
		self.assertEqual(len(v2), 2)

	def testSearch(self):
		"""Test search functions"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		self.assertTrue("0" in v1)
		self.assertTrue("3" not in v1)
		self.assertTrue(v1.has_key("1"))
		self.assertTrue(not v1.has_key("3"))
		self.assertTrue(v1.get("0") == IECore.FloatData(1.2))
		self.assertTrue(v1.get("0", IECore.IntData(10)) == IECore.FloatData(1.2))
		self.assertTrue(v1.get("xx", IECore.IntData(10)) == IECore.IntData(10))
		self.assertTrue(v1.get("xx") == None)
		self.assertTrue(v1.get("xx", None ) == None)
		self.assertEqual(len(v1), 3)

	def testUpdate(self):
		"""Test update function"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v2 = IECore.CompoundData()
		v2["0"] = IECore.UIntData(5)
		v2["3"] = IECore.UIntData(6)
		v2.update(v1)
		self.assertEqual(len(v2), 4)
		self.assertTrue(v2["0"] == IECore.FloatData(1.2))
		self.assertTrue(v2["3"] == IECore.UIntData(6))
		v3 = dict()
		v3["1"] = IECore.CharData("a")
		v3["4"] = IECore.UCharData(9)
		v2.update(v3)
		self.assertEqual(len(v2), 5)
		self.assertTrue(v2["1"] == IECore.CharData("a"))
		self.assertTrue(v2["4"] == IECore.UCharData(9))

	def testSetDefault(self):
		"""Test setdefault function"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v2 = v1.copy()
		self.assertEqual(len(v1), 3)
		self.assertTrue(v1.setdefault("2", IECore.UIntData(10)) == IECore.FloatData(3))
		self.assertEqual(len(v1), 3)
		self.assertTrue(v1.setdefault("x", IECore.UIntData(10)) == IECore.UIntData(10))
		self.assertEqual(len(v1), 4)

	def testPop(self):
		"""Test pop functions"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v1["3"] = IECore.FloatData(4)
		self.assertEqual(len(v1), 4)
		prev = v1.popitem()
		self.assertEqual(len(v1), 3)

		self.assertEqual(v1.pop("x", IECore.UIntData(10)), IECore.UIntData(10))
		self.assertEqual(len(v1), 3)

	def testKeyValues(self):
		"""Test keys/values listing"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1)
		v1["1"] = IECore.FloatData(2)
		v1["2"] = IECore.FloatData(3)
		self.assertEqual( set( v1.keys() ), set( ['0', '1', '2'] ) )
		vals = v1.values()
		self.assertEqual( set( [ x.value for x in vals ] ), set( [ 1, 2, 3 ] ) )
		items = v1.items()
		self.assertEqual( set( [ ( x[0], x[1].value ) for x in items ] ), set( [ ( "0", 1 ), ( "1", 2 ), ( "2", 3 ) ] ) )

	def testEquality(self):
		"""Test equality function"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v2 = IECore.CompoundData()
		v2["0"] = IECore.FloatData(1.2)
		v2["1"] = IECore.FloatData(2.3)
		v2["2"] = IECore.FloatData(3)
		v3 = v2.copy()
		del v3["2"]
		self.assertTrue(v1 == v2)
		self.assertTrue(not v1 != v2)
		self.assertTrue(not v1 == v3)
		self.assertTrue(not v2 == v3)
		v2["-1"] = IECore.FloatData(6)
		self.assertTrue(v1 != v2)
		self.assertTrue(not v1 == v2)
		del(v1["2"])
		self.assertTrue(v1 == v3)

	def testByValueItem(self):
		"""Test by value return type"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		self.assertTrue(v1["0"] == IECore.FloatData(1.2))
		a = v1["0"]
		a = IECore.UIntData(255)
		self.assertTrue(v1["0"] == IECore.FloatData(1.2))
		self.assertTrue(a == IECore.UIntData(255))

	def testLoadSave(self):
		"""Test load/save"""

		iface = IECore.IndexedIO.create( "test/CompoundData.fio", IECore.IndexedIO.OpenMode.Write )

		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v1["some:data"] = IECore.FloatData(3)
		self.assertTrue(v1["0"] == IECore.FloatData(1.2))

		v1.save( iface, "test" )

		v2 = IECore.Object.load( iface, "test" )
		self.assertEqual( v1, v2 )

	def testRepr(self):
		"""Test repr"""

		v1 = IECore.CompoundData()

		r1 = repr(v1)

		self.assertEqual( eval(repr(v1)), v1 )

		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)

		self.assertEqual( eval(repr(v1)), v1 )

		v1 = IECore.CompoundData()
		v1["0"] = IECore.StringData( "test" )
		v1["1"] = IECore.CompoundData(
			{ "0" : IECore.StringData( "test" ),
			  "1" : IECore.M33fData()
			}
		)
		v1["someMoreData"] = IECore.V3fVectorData()
		v1["A"] = IECore.Color4fVectorData()

		self.assertEqual( eval(repr(v1)), v1 )

	def testConstructionFromNestedDict( self ) :

		c = IECore.CompoundData( {
			"a" : 10,
			"b" : IECore.BoolData( True ),
			"c" : {
				"cc" : IECore.IntData( 20 ),
			},
			"d" : IECore.CompoundData( {
				"dd" : IECore.IntData( 5 ),
			} )
		} )

		self.assertEqual( len( c ), 4 )
		self.assertEqual( c["a"], IECore.IntData( 10 ) )
		self.assertEqual( c["b"], IECore.BoolData( True ) )
		self.assertEqual( len( c["c"] ), 1 )
		self.assertEqual( c["c"]["cc"], IECore.IntData( 20 ) )
		self.assertEqual( len( c["d"] ), 1 )
		self.assertEqual( c["d"]["dd"], IECore.IntData( 5 ) )

	def testUpdateFromNestedDict( self ) :

		c = IECore.CompoundData( {
			"a" : IECore.IntData( 30 )
			}
		)

		d = {
			"a" : 10,
			"b" : IECore.BoolData( True ),
			"c" : {
				"cc" : IECore.IntData( 20 ),
			},
			"d" : IECore.CompoundData( {
				"dd" : IECore.IntData( 5 ),
			} )
		}

		c.update( d )

		self.assertEqual( len( c ), 4 )
		self.assertEqual( c["a"], IECore.IntData( 10 ) )
		self.assertEqual( c["b"], IECore.BoolData( True ) )
		self.assertEqual( len( c["c"] ), 1 )
		self.assertEqual( c["c"]["cc"], IECore.IntData( 20 ) )
		self.assertEqual( len( c["d"] ), 1 )
		self.assertEqual( c["d"]["dd"], IECore.IntData( 5 ) )

	def testHash( self ) :

		o1 = IECore.CompoundData()
		o2 = IECore.CompoundData()

		o1["a"] = IECore.StringData( "a" )
		o1["b"] = IECore.StringData( "b" )

		o2["b"] = IECore.StringData( "b" )
		o2["a"] = IECore.StringData( "a" )

		self.assertEqual( o1.hash(), o2.hash() )

		o2["c"] = IECore.StringData( "c" )

		self.assertNotEqual( o1.hash(), o2.hash() )

	def testHashIndependentFromOrderOfConstruction( self ) :

		# CompoundData internally uses a map from InternedString to Data.
		# a naive iteration over this might yield a different order in each
		# process as it's dependent on the addresses of the InternedStrings.
		# we need to keep hashes consistent between processes.

		commands = [
			"import IECore; IECore.InternedString( 'a' ); print( IECore.CompoundData( { 'a' : IECore.IntData( 10 ), 'b' : IECore.IntData( 20 ) } ).hash() )",
			"import IECore; IECore.InternedString( 'b' ); print( IECore.CompoundData( { 'a' : IECore.IntData( 10 ), 'b' : IECore.IntData( 20 ) } ).hash() )",
		]

		hashes = { subprocess.check_output( [ sys.executable, "-c", command ] ) for command in commands }
		self.assertEqual( len( hashes ), 1 )

	def testHash( self ) :

		thingsToAdd = [
			( "a", IECore.IntData( 1 ), True ),
			( "a", IECore.UIntData( 1 ), True ),
			( "a", IECore.IntData( 1 ), True ),
			( "a", IECore.IntData( 1 ), False ),
			( "b", IECore.StringVectorData( [ "a", "b", "c" ] ), True ),
			( "b", IECore.StringVectorData( [ "a", "b" ] ), True ),
			( "b", IECore.StringVectorData( [ "a", "c" ] ), True ),
			( "b", IECore.StringVectorData( [ "a", "c" ] ), False ),
			( "d", IECore.StringVectorData( [ "a", "c" ] ), True ),
			( "d", None, True ),
		]

		o = IECore.CompoundData()

		for t in thingsToAdd :
			h = o.hash()
			for i in range( 0, 10 ) :
				self.assertEqual( h, o.hash() )
			if t[1] is not None :
				o[t[0]] = t[1]
			else :
				del o[t[0]]
			if t[2] :
				self.assertNotEqual( h, o.hash() )
			else :
				self.assertEqual( h, o.hash() )
			h = o.hash()

	def tearDown(self):

		if os.path.isfile("./test/CompoundData.fio") :
			os.remove("./test/CompoundData.fio")


if __name__ == "__main__":
    unittest.main()

