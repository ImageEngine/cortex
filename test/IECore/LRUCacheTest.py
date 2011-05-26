##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

class LRUCacheTest( unittest.TestCase ) :

	def test( self ) :

		self.numGetterCalls = 0
		
		def getter( key ) :
		
			self.numGetterCalls += 1
		
			return (
				# value
				{
					"same" : key,
					"times2" : key * 2,
					"times4" : key * 4,
				},
				# cost
				1
			)

		c = IECore.LRUCache( getter, 10 )
		self.assertEqual( c.getMaxCost(), 10 )
		c.setMaxCost( 20 )
		self.assertEqual( c.getMaxCost(), 20 )
		c.setMaxCost( 10 )
		self.assertEqual( c.getMaxCost(), 10 )
				
		v = c.get( 10 )
		self.assertEqual( v,
			{ 
				"same" : 10,
				"times2" : 20,
				"times4" : 40,
			}
		)
		
		self.assertEqual( c.currentCost(), 1 )
		self.assertEqual( self.numGetterCalls, 1 )
				
		v2 = c.get( 10 )
		self.failUnless( v2 is v )
		
		self.assertEqual( c.currentCost(), 1 )
		self.assertEqual( self.numGetterCalls, 1 )
				
		for k in range( 11, 10000 ) :
		
			v = c.get( k )
			self.assertEqual( v,
				{ 
					"same" : k,
					"times2" : k * 2,
					"times4" : k * 4,
				}
			)
			
			self.failIf( c.currentCost() > 10 )
				
if __name__ == "__main__":
    unittest.main()
