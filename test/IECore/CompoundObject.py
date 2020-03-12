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

import unittest
import sys
import subprocess

import IECore

class testCompoundObject( unittest.TestCase ) :

	def test( self ) :

		o = IECore.CompoundObject()
		self.assertEqual( len( o ), 0 )
		self.assertEqual( len( o.keys() ), 0 )
		self.assertEqual( len( o.values() ), 0 )

		o["a"] = IECore.IntData( 1 )
		self.assertEqual( len( o ), 1 )
		self.assertEqual( o["a"], IECore.IntData( 1 ) )
		self.assertEqual( len( o.keys() ), 1 )
		self.assertEqual( len( o.values() ), 1 )
		self.assertTrue( "a" in o.keys() )

		o["b"] = IECore.IntData( 2 )
		self.assertEqual( len( o ), 2 )
		self.assertEqual( o["b"], IECore.IntData( 2 ) )
		self.assertEqual( len( o.keys() ), 2 )
		self.assertEqual( len( o.values() ), 2 )
		self.assertTrue( "a" in o.keys() )
		self.assertTrue( "b" in o.keys() )

	def testDictConstructor( self ):
		o = IECore.CompoundObject( { "a": IECore.IntData( 1 ), "b": IECore.FloatData( 1.0 ) } )
		self.assertEqual( len( o ), 2 )
		self.assertEqual( o["a"], IECore.IntData( 1 ) )
		self.assertEqual( o["b"], IECore.FloatData( 1.0 ) )
		self.assertEqual( len( o.keys() ), 2 )
		self.assertEqual( len( o.values() ), 2 )
		self.assertTrue( "a" in o.keys() )
		# also test has_key
		self.assertTrue( o.has_key("a") )
		self.assertTrue( o.has_key("b") )
		self.assertTrue( not o.has_key("c") )
		# also test get
		self.assertTrue(o.get("a") == IECore.IntData( 1 ))
		self.assertTrue(o.get("b", IECore.IntData( 1 )) == IECore.FloatData( 1.0 ))
		self.assertTrue(o.get("xx", IECore.IntData(10)) == IECore.IntData(10))
		self.assertTrue(o.get("xx") == None)
		self.assertTrue(o.get("xx", None ) == None)

	def testDictConstructorRecursion( self ) :

		o = IECore.CompoundObject( {
				"a" : IECore.IntData( 1 ),
				"b" : IECore.IntData( 2 ),
				"c" : {
					"d" : IECore.IntData( 3 )
				}
			}
		)

		self.assertEqual( o["a"], IECore.IntData( 1 ) )
		self.assertEqual( o["b"], IECore.IntData( 2 ) )
		self.assertEqual( o["c"], IECore.CompoundObject( { "d" : IECore.IntData( 3 ) } ) )
		self.assertEqual( o["c"]["d"], IECore.IntData( 3 ) )

	def testRepr(self):
		"""Test repr"""

		v1 = IECore.CompoundObject( {
				"a" : IECore.IntData( 1 ),
				"b" : IECore.IntData( 2 ),
				"c" : {
					"d" : IECore.IntData( 3 )
				}
			}
		)

		r1 = repr(v1)

		self.assertEqual( eval(repr(v1)), v1 )

	def testAttributeAccessRemoval( self ) :

		# we used to support access to the children as attributes as well
		# as as items, but as of version 4 this was deprecated and in version
		# 5 it was removed.

		v = IECore.CompoundObject( { "a" : IECore.IntData( 1 ) } )

		self.assertRaises( AttributeError, getattr, v, "a" )

	def testKeyErrors( self ) :

		def get( c ) :

			c["iDontExist"]

		def delete( c ) :

			del c["iDontExist"]

		c = IECore.CompoundObject()

		self.assertRaises( KeyError, get, c )
		self.assertRaises( KeyError, delete, c )

	def testItems( self ) :

		o = IECore.CompoundObject( { "a": IECore.IntData( 1 ) } )
		self.assertEqual( o.items(), [ ( "a", IECore.IntData( 1 ) ) ] )

	def testDefaultInstance( self ) :

		o = IECore.CompoundObject.defaultInstance()

		o["a"] = IECore.IntData( 10 )

		self.assertTrue( o["a"].isSame( IECore.CompoundObject.defaultInstance()["a"] ) )

		del o["a"]

		self.assertTrue( "a" not in IECore.CompoundObject.defaultInstance() )

	def testHash( self ) :

		o1 = IECore.CompoundObject()
		o2 = IECore.CompoundObject()

		o1["a"] = IECore.StringData( "a" )
		o1["b"] = IECore.StringData( "b" )

		o2["b"] = IECore.StringData( "b" )
		o2["a"] = IECore.StringData( "a" )

		self.assertEqual( o1.hash(), o2.hash() )

		o2["c"] = IECore.StringData( "c" )

		self.assertNotEqual( o1.hash(), o2.hash() )

	def testHashIndependentFromOrderOfConstruction( self ) :

		# CompoundObject internally uses a map from InternedString to Data.
		# a naive iteration over this might yield a different order in each
		# process as it's dependent on the addresses of the InternedStrings.
		# we need to keep hashes consistent between processes.

		commands = [
			"import IECore; IECore.InternedString( 'a' ); print( IECore.CompoundObject( { 'a' : IECore.IntData( 10 ), 'b' : IECore.IntData( 20 ) } ).hash() )",
			"import IECore; IECore.InternedString( 'b' ); print( IECore.CompoundObject( { 'a' : IECore.IntData( 10 ), 'b' : IECore.IntData( 20 ) } ).hash() )",
		]

		hashes = set()
		for command in commands :
			p = subprocess.Popen( [ sys.executable, "-c", command ], stdout=subprocess.PIPE )
			hash, nothing = p.communicate()
			hashes.add( hash )

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

		o = IECore.CompoundObject()

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

if __name__ == "__main__":
        unittest.main()

