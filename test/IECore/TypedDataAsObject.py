##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
from IECore import *

class TestTypedDataAsObject( unittest.TestCase ) :

	def testSimpleCopy( self ) :
	
		o = IntData( 1 )
		self.assertEqual( o.value, 1 )
		self.assertEqual( o, o )
		o.value = 2
		self.assertEqual( o.value, 2 )

		oo = o.copy()
		self.assertEqual( oo.value, 2 )
		self.assertEqual( o, oo )
		
		o.value = 3
		self.assertEqual( o.value, 3 )
		self.assertEqual( oo.value, 2 )
		self.assertNotEqual( o, oo )
		
		oo.value = 4
		self.assertEqual( o.value, 3 )
		self.assertEqual( oo.value, 4 )
		self.assertNotEqual( o, oo )
		
	def testCompoundCopy( self ) :
	
		"""CompoundData must specialise the copyFrom() method
		to ensure that a genuine deep copy of the data is produced.
		This tests that."""
	
		a = IntData( 2 )
		self.assertEqual( a.value, 2 )
		self.assertEqual( a, a )
		
		c = CompoundData()
		c["A"] = a
		self.assertEqual( c["A"].value, 2 )
		
		a.value = 3
		self.assertEqual( a.value, 3 )
		self.assertEqual( c["A"].value, 3 )
		
		self.assert_( a.isSame( c["A"] ) )
		
		cc = c.copy()
		
		self.assert_( a.isSame( c["A"] ) )
		self.assert_( not a.isSame( cc["A"] ) )
		self.assertEqual( c, cc )
		
		a.value = 10
		self.assertEqual( a.value, 10 )
		self.assertEqual( c["A"].value, 10 )
		self.assertEqual( cc["A"].value, 3 )
		self.assertNotEqual( c, cc )
		
		cc["A"].value = 100
		self.assertEqual( cc["A"].value, 100 )
		self.assertEqual( c["A"].value, 10 )
		self.assertEqual( a.value, 10 )
		self.assertNotEqual( c, cc )
		
		a.value = 100
		self.assertEqual( c, cc )
		

if __name__ == "__main__":
    unittest.main()   
