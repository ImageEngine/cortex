##########################################################################
#
#  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

class LightTest( unittest.TestCase ) :

	def test( self ) :

		s = Light()
		self.assertEqual( s.name, "distantlight" )
		self.assert_( len(s.handle) > 0 )
		self.assertEqual( len( s.parameters ), 0 )
		self.assertEqual( s.parameters.typeName(), "CompoundData" )

		ss = Light()
		self.assertNotEqual( s.handle, ss.handle )

		s = Light( "marble", "marble001" )
		self.assertEqual( s.name, "marble" )
		self.assertEqual( s.handle, "marble001" )

		ss = s.copy()
		self.assertEqual( ss.name, s.name )
		self.assertEqual( ss.handle, s.handle )

	def testProperties( self ) :

		s = Light()
		s.handle = "myNewHandle"
		s.name = "myNewName"
		self.assertEqual( s.name, "myNewName" )
		self.assertEqual( s.handle, "myNewHandle" )

	def testConstructWithParameters( self ) :

		s = Light( "test", "test001", CompoundData( { "a" : StringData( "a" ) } ) )

		self.assertEqual( s.name, "test" )
		self.assertEqual( s.handle, "test001" )
		self.assertEqual( len( s.parameters ), 1 )
		self.assertEqual( s.parameters.typeName(), CompoundData.staticTypeName() )
		self.assertEqual( s.parameters["a"], StringData( "a" ) )

	def testCopy( self ) :

		s = Light( "test", "surface", CompoundData( { "a" : StringData( "a" ) } ) )
		ss = s.copy()

		self.assertEqual( s, ss )

	def testHash( self ) :

		s = Light( "name", "handle" )
		h = s.hash()

		s.name = "name2"
		self.assertNotEqual( s.hash(), h )
		h = s.hash()

		s.handle = "handle2"
		self.assertNotEqual( s.hash(), h )
		h = s.hash()

		s.parameters["a"] = StringData( "a" )
		self.assertNotEqual( s.hash(), h )

if __name__ == "__main__":
    unittest.main()
