##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
		self.assert_( "a" in o.keys() )

		o["b"] = IECore.IntData( 2 )
		self.assertEqual( len( o ), 2 )
		self.assertEqual( o["b"], IECore.IntData( 2 ) )
		self.assertEqual( len( o.keys() ), 2 )
		self.assertEqual( len( o.values() ), 2 )
		self.assert_( "a" in o.keys() )
		self.assert_( "b" in o.keys() )

	def testDictConstructor( self ):
		o = IECore.CompoundObject( { "a": IECore.IntData( 1 ), "b": IECore.FloatData( 1.0 ) } )
		self.assertEqual( len( o ), 2 )
		self.assertEqual( o["a"], IECore.IntData( 1 ) )
		self.assertEqual( o["b"], IECore.FloatData( 1.0 ) )
		self.assertEqual( len( o.keys() ), 2 )
		self.assertEqual( len( o.values() ), 2 )
		self.assert_( "a" in o.keys() )
		# also test has_key
		self.assert_( o.has_key("a") )
		self.assert_( o.has_key("b") )
		self.assert_( not o.has_key("c") )
		# also test get
		self.assert_(o.get("a") == IECore.IntData( 1 ))
		self.assert_(o.get("b", IECore.IntData( 1 )) == IECore.FloatData( 1.0 ))
		self.assert_(o.get("xx", IECore.IntData(10)) == IECore.IntData(10))
		self.assert_(o.get("xx") == None)
		self.assert_(o.get("xx", None ) == None)

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
		
		self.failUnless( o["a"].isSame( IECore.CompoundObject.defaultInstance()["a"] ) )
		
		del o["a"]
		
		self.failUnless( "a" not in IECore.CompoundObject.defaultInstance() )
		
if __name__ == "__main__":
        unittest.main()

