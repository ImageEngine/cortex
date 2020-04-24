##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

class ClassVectorParameterTest( unittest.TestCase ) :

	def testConstructor( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		self.assertEqual( c.name, "n" )
		self.assertEqual( c.description, "d" )
		self.assertEqual( c.userData(), IECore.CompoundObject() )
		self.assertEqual( c.getClasses(), [] )
		self.assertEqual( c.getClasses( True ), [] )
		self.assertEqual( c.searchPathEnvVar(), "IECORE_OP_PATHS" )

		c = IECore.ClassVectorParameter(
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
		self.assertEqual( c.getClasses(), [] )
		self.assertEqual( c.getClasses( True ), [] )
		self.assertEqual( c.searchPathEnvVar(), "IECORE_OP_PATHS" )

	def testSetAndGetClasses( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		c.setClasses(

			[
				( "mult", "maths/multiply", 2 ),
				( "coIO", "compoundObjectInOut", 1 ),
			]

		)

		cl = c.getClasses()
		self.assertTrue( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0].typeName(), "multiply" )
		self.assertEqual( cl[1].typeName(), "compoundObjectInOut" )

		self.assertEqual( len( c ), 2 )
		self.assertEqual( c.keys(), [ "mult", "coIO" ] )
		self.assertEqual( c["mult"].keys(), [ "a", "b" ] )
		self.assertEqual( c["coIO"].keys(), [ "input" ] )

		cl = c.getClasses( True )
		self.assertTrue( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[1][0].typeName(), "compoundObjectInOut" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[1][1], "coIO" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[1][2], "compoundObjectInOut" )
		self.assertEqual( cl[0][3], 2 )
		self.assertEqual( cl[1][3], 1 )

	def testInsertClasses( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		c.setClasses(

			[
				( "mult", "maths/multiply", 2 ),
				( "coIO", "compoundObjectInOut", 1 ),
			]

		)

		cl = c.getClasses( True )
		self.assertTrue( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[1][0].typeName(), "compoundObjectInOut" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[1][1], "coIO" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[1][2], "compoundObjectInOut" )
		self.assertEqual( cl[0][3], 2 )
		self.assertEqual( cl[1][3], 1 )

		c.setClasses(

			[
				( "pp", "presetParsing", 1 ),
				( "mult", "maths/multiply", 2 ),
				( "si", "splineInput", 1 ),
				( "coIO", "compoundObjectInOut", 1 ),
			]

		)

		cl2 = c.getClasses( True )

		self.assertTrue( isinstance( cl2, list ) )
		self.assertEqual( len( cl2 ), 4 )

		self.assertTrue( cl[0][0] is cl2[1][0] )
		self.assertTrue( cl[1][0] is cl2[3][0] )

		self.assertEqual( cl2[0][0].typeName(), "presetParsing" )
		self.assertEqual( cl2[1][0].typeName(), "multiply" )
		self.assertEqual( cl2[2][0].typeName(), "splineInput" )
		self.assertEqual( cl2[3][0].typeName(), "compoundObjectInOut" )

		self.assertEqual( cl2[0][1], "pp" )
		self.assertEqual( cl2[1][1], "mult" )
		self.assertEqual( cl2[2][1], "si" )
		self.assertEqual( cl2[3][1], "coIO" )

		self.assertEqual( cl2[0][2], "presetParsing" )
		self.assertEqual( cl2[1][2], "maths/multiply" )
		self.assertEqual( cl2[2][2], "splineInput" )
		self.assertEqual( cl2[3][2], "compoundObjectInOut" )

		self.assertEqual( cl2[0][3], 1 )
		self.assertEqual( cl2[1][3], 2 )
		self.assertEqual( cl2[2][3], 1 )
		self.assertEqual( cl2[3][3], 1 )

		self.assertEqual( c.keys(), [ "pp", "mult", "si", "coIO" ] )

	def testRemoveClasses( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		c.setClasses(

			[
				( "mult", "maths/multiply", 2 ),
				( "coIO", "compoundObjectInOut", 1 ),
			]

		)

		cl = c.getClasses( True )
		self.assertTrue( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[1][0].typeName(), "compoundObjectInOut" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[1][1], "coIO" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[1][2], "compoundObjectInOut" )
		self.assertEqual( cl[0][3], 2 )
		self.assertEqual( cl[1][3], 1 )

		c.setClasses(

			[
				( "coIO", "compoundObjectInOut", 1 ),
			]

		)

		cl2 = c.getClasses( True )
		self.assertTrue( isinstance( cl2, list ) )
		self.assertEqual( len( cl2 ), 1 )
		self.assertEqual( cl2[0][0].typeName(), "compoundObjectInOut" )
		self.assertEqual( cl2[0][1], "coIO" )
		self.assertEqual( cl2[0][2], "compoundObjectInOut" )
		self.assertEqual( cl2[0][3], 1 )

		self.assertTrue( cl2[0][0] is cl[1][0] )

		self.assertEqual( c.keys(), [ "coIO" ] )


	def testReorderClasses( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		c.setClasses(

			[
				( "mult", "maths/multiply", 2 ),
				( "coIO", "compoundObjectInOut", 1 ),
			]

		)

		cl = c.getClasses( True )
		self.assertTrue( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[1][0].typeName(), "compoundObjectInOut" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[1][1], "coIO" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[1][2], "compoundObjectInOut" )
		self.assertEqual( cl[0][3], 2 )
		self.assertEqual( cl[1][3], 1 )

		c.setClasses(

			[
				( "coIO", "compoundObjectInOut", 1 ),
				( "mult", "maths/multiply", 2 ),
			]

		)

		cl2 = c.getClasses( True )
		self.assertTrue( isinstance( cl2, list ) )
		self.assertEqual( len( cl2 ), 2 )
		self.assertEqual( cl2[1][0].typeName(), "multiply" )
		self.assertEqual( cl2[0][0].typeName(), "compoundObjectInOut" )
		self.assertEqual( cl2[1][1], "mult" )
		self.assertEqual( cl2[0][1], "coIO" )
		self.assertEqual( cl2[1][2], "maths/multiply" )
		self.assertEqual( cl2[0][2], "compoundObjectInOut" )
		self.assertEqual( cl2[1][3], 2 )
		self.assertEqual( cl2[0][3], 1 )

		self.assertTrue( cl[0][0] is cl2[1][0] )
		self.assertTrue( cl[1][0] is cl2[0][0] )

		self.assertEqual( c.keys(), [ "coIO", "mult" ] )


	def testChangeClasses( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		c.setClasses(

			[
				( "mult", "maths/multiply", 2 ),
			]

		)

		cl = c.getClasses( True )
		self.assertTrue( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 1 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[0][3], 2 )

		self.assertEqual( c["mult"].keys(), [ "a", "b" ] )

		c.setClasses(

			[
				( "mult", "compoundObjectInOut", 1 )
			]

		)

		cl2 = c.getClasses( True )

		self.assertTrue( isinstance( cl2, list ) )
		self.assertEqual( len( cl2 ), 1 )
		self.assertEqual( cl2[0][0].typeName(), "compoundObjectInOut" )
		self.assertEqual( cl2[0][1], "mult" )
		self.assertEqual( cl2[0][2], "compoundObjectInOut" )
		self.assertEqual( cl2[0][3], 1 )

		self.assertEqual( c["mult"].keys(), [ "input" ] )

	def testSerialiseAndParse( self ) :

		p = IECore.CompoundParameter()

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		p.addParameter( c )

		c.setClasses(

			[
				( "mult", "maths/multiply", 2 ),
			]

		)

		p["n"]["mult"]["a"].setTypedValue( 10 )
		p["n"]["mult"]["b"].setTypedValue( 20 )

		s = IECore.ParameterParser().serialise( p )

		c.setClasses( [] )
		self.assertEqual( len( c.getClasses() ), 0 )
		self.assertEqual( len( c.keys() ), 0 )

		IECore.ParameterParser().parse( s, p )

		cl = c.getClasses( True )
		self.assertEqual( len( cl ), 1 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[0][3], 2 )

		self.assertEqual( len( c.keys() ), 1 )
		self.assertEqual( len( c["mult"].keys() ), 2 )

		self.assertEqual( p["n"]["mult"]["a"].getTypedValue(), 10 )
		self.assertEqual( p["n"]["mult"]["b"].getTypedValue(), 20 )

	def testDuplicateParameterNames( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS",
		)

		self.assertRaises(

			ValueError,
			c.setClasses,
			[
				( "p1", "maths/multiply", 1 ),
				( "p2", "stringParsing", 1 ),
				( "p1", "maths/multiply", 1 ),
			]

		)

		self.assertEqual( len( c.getClasses() ), 0 )

	def testSetGetAndRemoveIndividualClasses( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS",
		)

		c.setClass( "new", "maths/multiply", 1 )

		cl = c.getClasses( True )

		self.assertEqual( len( cl ), 1 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[0][1], "new" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[0][3], 1 )

		self.assertEqual( len( c.keys() ), 1 )
		self.assertEqual( len( c["new"].keys() ), 2 )

		cl = c.getClass( "new" )
		self.assertEqual( cl.typeName(), "multiply" )

		cl = c.getClass( "new", True )
		self.assertEqual( cl[0].typeName(), "multiply" )
		self.assertEqual( cl[1], "maths/multiply" )
		self.assertEqual( cl[2], 1 )

		c.setClass( "new", "stringParsing", 1 )

		cl = c.getClasses( True )

		self.assertEqual( len( cl ), 1 )
		self.assertEqual( cl[0][0].typeName(), "stringParsing" )
		self.assertEqual( cl[0][1], "new" )
		self.assertEqual( cl[0][2], "stringParsing" )
		self.assertEqual( cl[0][3], 1 )

		c.removeClass( "new" )

		cl = c.getClasses( True )
		self.assertEqual( len( cl ), 0 )
		self.assertEqual( len( c ), 0 )

	def testNewParameterName( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS",
		)

		self.assertEqual( c.newParameterName(), "p0" )

		c.setClasses(

			[
				( "p0", "compoundObjectInOut", 1 ),
			]

		)

		self.assertEqual( c.newParameterName(), "p1" )

		c.setClasses(

			[
				( "p0", "compoundObjectInOut", 1 ),
				( "p1", "compoundObjectInOut", 1 ),
			]

		)

		self.assertEqual( c.newParameterName(), "p2" )

		c.setClasses(

		[
			( "p1", "compoundObjectInOut", 1 ),
		]

		)

		self.assertEqual( c.newParameterName(), "p0" )

	def testUserData( self ) :

		c = IECore.ClassVectorParameter(
			"n",
			"d",
			"IECORE_OP_PATHS"
		)

		c.setClasses(

			[
				( "p0", "classVectorParameterTest", 1 ),
			]

		)

		loader = IECore.ClassLoader.defaultLoader( "IECORE_OP_PATHS" )
		instance = loader.load( "classVectorParameterTest", 1 )()

		self.assertEqual( c["p0"].userData(), instance.parameters().userData() )

		c.setClasses(

			[
				( "p0", "compoundObjectInOut", 1 ),
			]

		)

		self.assertEqual( c["p0"].userData(), IECore.CompoundObject() )


if __name__ == "__main__" :
	unittest.main()
