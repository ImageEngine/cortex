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

"""Unit test for BlindDataHolder binding"""
import os
import unittest

from IECore import *

class TestBlindDataHolder(unittest.TestCase):

	def testConstructors(self):
		"""Test BlindDataHolder constructors"""
		b = BlindDataHolder()

		c = CompoundData()
		c["floatData"] = FloatData(3.0)

		b = BlindDataHolder(c)

		self.assertEqual( b.typeName(), "BlindDataHolder" )
		self.failIf( Object.isAbstractType( "BlindDataHolder") )

	def testBlindData(self):
		"""Test BlindDataHolder blindData"""

		b = BlindDataHolder()

		b.blindData()["floatData"] = FloatData(1.0)
		b.blindData()["intData"] = IntData(-5)

		self.assertEqual( b.blindData()["floatData"].value, 1.0 )
		self.assertEqual( b.blindData()["intData"].value, -5 )

		self.assertEqual( len(b.blindData()), 2 )

	def testComparison(self):

		# test the empty case (where it doesn't allocate the compound data)
		a = BlindDataHolder( )
		b = BlindDataHolder( CompoundData() )
		c = BlindDataHolder( )
		c.blindData()

		self.assertEqual( a, a )
		self.assertEqual( a, a )
		self.assertEqual( a, b )
		self.assertEqual( b, a )
		self.assertEqual( a, c )
		self.assertEqual( c, a )
		self.assertEqual( b, c )
		self.assertEqual( c, b )
		
		c.blindData()['a'] = IntData(10)
		self.assertNotEqual( a, c )
		self.assertNotEqual( c, a )
		self.assertNotEqual( b, c )
		self.assertNotEqual( c, b )

	def testLoadSave(self):

		"""Test BlindDataHolder load/save"""

		iface = IndexedIO.create( "test/BlindDataHolder.fio", IndexedIO.OpenMode.Write )

		# first simple test: saving with some blind data
		b1 = BlindDataHolder()
		b1.blindData()["floatData"] = FloatData(1.0)
		b1.blindData()["intData"] = IntData(-5)
		b1.save( iface, "test" )

		b2 = Object.load( iface, "test" )
		self.assertEqual( b1, b2 )
		
		# should have written a "blindData" entry into the indexed io hierarchy
		self.failUnless( isinstance( iface.directory( ["test","data","BlindDataHolder", "data", "blindData"], IndexedIO.MissingBehaviour.NullIfMissing ), IndexedIO ) )

		# second test: overriding with no blind data
		b1 = BlindDataHolder()
		b1.save( iface, "test" )
		b1 = Object.load( iface, "test" )
		self.assertEqual( b1, BlindDataHolder() )

		# thirt test: saving a derived class with some blind data
		g1 = Group()
		g1.blindData()["floatData"] = FloatData(1.0)
		g1.blindData()["intData"] = IntData(-5)
		g1.save( iface, "test" )
		g2 = Object.load( iface, "test" )
		self.assertEqual( g1, g2 )
		
		# fourth test: overriding with no blind data
		g1 = Group()
		g1.blindData()
		g1.save( iface, "test" )
		g2 = Object.load( iface, "test" )
		self.assertEqual( g1, g2 )
		
		# "blindData" entry should be excluded from the IndexedIO hierarchy
		self.assertEqual( iface.directory( ["test","data","BlindDataHolder"], IndexedIO.MissingBehaviour.NullIfMissing ), None )
		
	def testHash( self ) :
	
		b1 = BlindDataHolder()
		b2 = BlindDataHolder()
		
		self.assertEqual( b1.hash(), b2.hash() )
		
		b2.blindData()["a"] = FloatData( 1 )
		self.assertNotEqual( b1.hash(), b2.hash() )
		
	def tearDown(self):

		if os.path.isfile("./test/BlindDataHolder.fio") :
			os.remove("./test/BlindDataHolder.fio")

if __name__ == "__main__":
        unittest.main()
