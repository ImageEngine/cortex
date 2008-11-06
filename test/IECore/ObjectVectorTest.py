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

import os
import unittest

import IECore

class ObjectVectorTest( unittest.TestCase ) :

	def test( self ) :
	
		o = IECore.ObjectVector()
	
		self.assertEqual( len( o ), 0 )
		
		m1 = IECore.FloatData()
		m2 = IECore.FloatData()
		m3 = IECore.FloatData()
		
		o.append( m1 )
		self.assertEqual( len( o ), 1 )
		self.assert_( o[0].isSame( m1 ) )
		self.assert_( o[-1].isSame( m1 ) )
		self.assertRaises( IndexError, o.__getitem__, 1 )		
		self.assertRaises( IndexError, o.__getitem__, -2 )
		
		o.append( m2 )
		self.assertEqual( len( o ), 2 )
		self.assert_( o[0].isSame( m1 ) )
		self.assert_( o[1].isSame( m2 ) )
		self.assert_( o[-1].isSame( m2 ) )
		self.assert_( o[-2].isSame( m1 ) )
		self.assertRaises( IndexError, o.__getitem__, 2 )		
		self.assertRaises( IndexError, o.__getitem__, -3 )
		
		o.append( m3 )
		self.assertEqual( len( o ), 3 )
		self.assert_( o[0].isSame( m1 ) )
		self.assert_( o[1].isSame( m2 ) )
		self.assert_( o[2].isSame( m3 ) )
		self.assert_( o[-1].isSame( m3 ) )
		self.assert_( o[-2].isSame( m2 ) )
		self.assert_( o[-3].isSame( m1 ) )
		self.assertRaises( IndexError, o.__getitem__, 3 )		
		self.assertRaises( IndexError, o.__getitem__, -4 )
	
		l = [ m1, m2, m3 ]
		i = 0
		for v in o :
			self.assert_( l[i].isSame( v ) )
			i += 1
			
		del o[1]
		self.assertEqual( len( o ), 2 )
		self.assert_( o[0].isSame( m1 ) )
		self.assert_( o[1].isSame( m3 ) )
		self.assert_( o[-1].isSame( m3 ) )
		self.assert_( o[-2].isSame( m1 ) )
		self.assertRaises( IndexError, o.__getitem__, 2 )		
		self.assertRaises( IndexError, o.__getitem__, -3 )
	
		oo = o.copy()
		self.assertEqual( o, oo )
		oo.append( m2 )
		self.assertNotEqual( o, oo )
		
		IECore.ObjectWriter( o, "test/IECore/objectVector.cob" ).write()
		ooo = IECore.ObjectReader( "test/IECore/objectVector.cob" ).read()
		
		self.assertEqual( o, ooo )
		
	def tearDown( self ) :
	
		if os.path.exists( "test/IECore/objectVector.cob" ) :
			os.remove( "test/IECore/objectVector.cob" )
		
	
if __name__ == "__main__":
        unittest.main()
