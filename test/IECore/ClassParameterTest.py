##########################################################################
#
#  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

class ClassParameterTest( unittest.TestCase ) :

	def testConstructor( self ) :

		c = IECore.ClassParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		self.assertEqual( c.name, "n" )
		self.assertEqual( c.description, "d" )
		self.assertEqual( c.userData(), IECore.CompoundObject() )
		self.assertEqual( c.getClass(), None )
		self.assertEqual( c.getClass( True ), ( None, "", 0, "IECORE_OP_PATHS" ) )

		c = IECore.ClassParameter(
			"n",
			"d",
			"IECORE_OP_PATHS",
			userData = {
				"a" : IECore.IntData( 10 ),
			}
		)

		self.assertEqual( c.name, "n" )
		self.assertEqual( c.description, "d" )
		self.assertEqual( c.userData(), IECore.CompoundObject( { "a" : IECore.IntData( 10 ) } ) )
		self.assertEqual( c.getClass(), None )
		self.assertEqual( c.getClass( True ), ( None, "", 0, "IECORE_OP_PATHS" ) )

	def testGetClass( self ) :

		c = IECore.ClassParameter(
			"n",
			"d",
			"IECORE_OP_PATHS",
			"maths/multiply",
			2
		)

		cl = c.getClass()
		self.assertEqual( c.getClass( True ), ( cl, "maths/multiply", 2, "IECORE_OP_PATHS" ) )

		self.assertEqual( c.keys(), [ "a", "b" ] )

		self.assertTrue( c["a"].isInstanceOf( IECore.IntParameter.staticTypeId() ) )
		self.assertTrue( c["b"].isInstanceOf( IECore.IntParameter.staticTypeId() ) )

		c["a"].setNumericValue( 10 )
		c["b"].setNumericValue( 20 )

		self.assertEqual( cl(), IECore.IntData( 200 ) )

	def testSetClass( self ) :

		c = IECore.ClassParameter(
			"n",
			"d",
			"IECORE_OP_PATHS",
			"maths/multiply",
			2
		)

		self.assertEqual( c.keys(), [ "a", "b" ] )

		c.setClass( "stringParsing", 1 )

		self.assertEqual( c.keys(), [ "emptyString", "normalString", "stringWithSpace", "stringWithManySpaces" ] )

	def testSetNoClass( self ) :

		c = IECore.ClassParameter(
			"n",
			"d",
			"IECORE_OP_PATHS",
			"maths/multiply",
			2
		)

		self.assertEqual( c.keys(), [ "a", "b" ] )

		c.setClass( "", 0 )

		self.assertEqual( c.keys(), [] )
		self.assertEqual( c.getClass( True ), ( None, "", 0, "IECORE_OP_PATHS" ) )

	def testSerialiseAndParse( self ) :

		p = IECore.CompoundParameter( "", "d" )
		c = IECore.ClassParameter(
			"n",
			"d",
			"IECORE_OP_PATHS",
			"maths/multiply",
			2
		)
		c["a"] = 10
		c["b"] = 20
		p.addParameter( c )

		s = IECore.ParameterParser().serialise( p )

		c.setClass( "stringParsing", 1 )

		IECore.ParameterParser().parse( s, p )

		self.assertEqual( c.keys(), [ "a", "b" ] )
		self.assertEqual( c["a"].getNumericValue(), 10 )
		self.assertEqual( c["b"].getNumericValue(), 20 )

if __name__ == "__main__" :
	unittest.main()
