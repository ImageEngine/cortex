##########################################################################
#
#  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

class SubstitutedDictTest( unittest.TestCase ) :

	def test( self ) :

		d = {
			"a" : "hello ${name}",
			"b" : IECore.CompoundObject(
				{
					"c" : IECore.StringData( "goodbye ${place}" )
				}
			)
		}

		ds = IECore.SubstitutedDict( d, { "name" : "john", "place" : "london" } )

		self.assertEqual( ds["a"], "hello john" )
		self.assertEqual( ds["b"]["c"], IECore.StringData( "goodbye london" ) )
		self.assertTrue( isinstance( ds["b"], IECore.SubstitutedDict ) )
		self.assertEqual( ds.get( "a" ), "hello john" )
		self.assertEqual( ds.get( "notThere" ), None )
		self.assertEqual( ds.get( "notThere", 10 ), 10 )
		self.assertEqual( ds.get( "a", substituted=False ), "hello ${name}" )
		self.assertEqual( ds.get( "b", substituted=False )["c"], IECore.StringData( "goodbye ${place}" ) )
		self.assertTrue( ds.get( "b", substituted=False ).isInstanceOf( IECore.CompoundObject.staticTypeId() ) )
		self.assertEqual( ds.get( "notThere", substituted=False ), None )

		self.assertEqual( ds, ds )

		keys = ds.keys()
		self.assertEqual( len( keys ), 2 )
		self.assertTrue( "a" in keys )
		self.assertTrue( "b" in keys )

		values = ds.values()
		self.assertEqual( len( values ), len( keys ) )
		self.assertEqual( values[keys.index( "a" )], "hello john" )
		self.assertTrue( isinstance( values[keys.index( "b" )], IECore.SubstitutedDict ) )

		values = ds.values( substituted=False )
		self.assertEqual( len( values ), len( keys ) )
		self.assertEqual( values[keys.index( "a" )], "hello ${name}" )
		self.assertTrue( isinstance( values[keys.index( "b" )], IECore.CompoundObject ) )

		self.assertEqual( zip( *(ds.items()) ), [ tuple( ds.keys() ), tuple( ds.values() ) ] )

	def testEquality( self ) :

		d = IECore.SubstitutedDict(
			{
				"a" : "aa",
				"b" : "${b}",
			},
			{
				"b" : "x",
			}
		)

		d2 = IECore.SubstitutedDict(
			{
				"a" : "aa",
				"b" : "${b}",
			},
			{
				"b" : "x",
			}
		)

		d3 = IECore.SubstitutedDict(
			{
				"a" : "aa",
				"b" : "different ${b}",
			},
			{
				"b" : "x",
			}
		)

		d4 = IECore.SubstitutedDict(
			{
				"a" : "aa",
				"b" : "${b}",
			},
			{
				"b" : "xxx",
			}
		)

		self.assertEqual( d, d )
		self.assertEqual( d, d2 )
		self.assertNotEqual( d, d3 )
		self.assertNotEqual( d, d4 )

if __name__ == "__main__":
    unittest.main()

