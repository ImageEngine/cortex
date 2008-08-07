##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

class LayeredDictTest( unittest.TestCase ) :

	def testDict( self ) :
	
		dict1 = {
			"a" : 10,
			"b" : {
				"c" : 20,
				"d" : 30,
			},
			"e" : 40,
		}
		
		dict2 = {
			"a" : 20,
			"b" : {
				"c" : 100,
				"f" : {
					"g" : 1000,
				},
				"h" : 1
			},
		}
		
		d = IECore.LayeredDict( [ dict1, dict2 ] )
		
		self.assertEqual( d["a"], 10 )
		self.assertEqual( d["b"]["c"], 20 )
		self.assertEqual( d["b"]["d"], 30 )
		self.assertEqual( d["b"]["f"]["g"], 1000 )
		self.assertEqual( d["e"], 40 )
		self.assertEqual( d["b"]["h"], 1 )

		self.assertRaises( KeyError, d.__getitem__, "z" )
		
		
	def testCompoundObject( self ) :
	
		dict1 = IECore.CompoundObject(
			{
				"a" : IECore.IntData( 10 ),
				"b" : {
					"c" : IECore.IntData( 20 ),
					"d" : IECore.IntData( 30 ),
				},
				"e" : IECore.IntData( 40 ),
			}
		)
	
		dict2 = IECore.CompoundObject( 
			{
				"a" : IECore.IntData( 20 ),
				"b" : {
					"c" : IECore.IntData( 100 ),
					"f" : {
						"g" : IECore.IntData( 1000 ),
					},
					"h" : IECore.IntData( 1 )
				},
			}
		)

		d = IECore.LayeredDict( [ dict1, dict2 ] )
		
		self.assertEqual( d["a"], IECore.IntData( 10 ) )
		self.assertEqual( d["b"]["c"], IECore.IntData( 20 ) )
		self.assertEqual( d["b"]["d"], IECore.IntData( 30 ) )
		self.assertEqual( d["b"]["f"]["g"], IECore.IntData( 1000 ) )
		self.assertEqual( d["e"], IECore.IntData( 40 ) )
		self.assertEqual( d["b"]["h"], IECore.IntData( 1 ) )

		self.assertRaises( KeyError, d.__getitem__, "z" )
	
	def testKeys( self ) :
	
		dict1 = {
			"a" : 10,
			"b" : {
				"c" : 20,
				"d" : 30,
			},
			"e" : 40,
		}
		
		dict2 = IECore.CompoundObject( 
			{
				"a" : IECore.IntData( 20 ),
				"b" : {
					"c" : IECore.IntData( 100 ),
					"f" : {
						"g" : IECore.IntData( 1000 ),
					},
					"h" : IECore.IntData( 1 )
				},
				"i" : IECore.IntData( 1 )
			}
		)

		d = IECore.LayeredDict( [ dict1, dict2 ] )
		
		self.assertEqual( set( d.keys() ), set( [ "a", "b", "e", "i" ] ) )
		self.assertEqual( set( d["b"].keys() ), set( [ "c", "d", "f", "h" ] ) )
			
if __name__ == "__main__":
    unittest.main()   

