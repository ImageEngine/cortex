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

import os
import unittest

import IECore

class ParameterAlgoTest( unittest.TestCase ) :

	def testFindClasses( self ) :

		p = IECore.CompoundParameter(

			name = "p",
			description = "",

			members = [

				IECore.CompoundParameter(

					name = "q",
					description = "",
					members = [

						IECore.ClassVectorParameter(

							"cv",
							"d",
							"IECORE_OP_PATHS",
							[
								( "mult", os.path.join( "maths", "multiply" ), 2 ),
								( "coIO", "compoundObjectInOut", 2 ),
							]

						),

						IECore.ClassParameter(

							"c",
							"d",
							"IECORE_OP_PATHS",
							"classParameterTest", 1

						)

					]

				),


			]

		)

		p["q"]["c"]["cp"].setClass( "classVectorParameterTest", 1 )

		p["q"]["c"]["cp"]["cv"].setClasses( [
			( "mult", os.path.join( "maths", "multiply" ), 2 ),
			( "coIO", "compoundObjectInOut", 2 )
		] )

		c = IECore.ParameterAlgo.findClasses( p )

		expected = [

			{
				"parent" : p["q"]["cv"],
				"parameterPath" : [ "q", "cv", "mult" ],
				"uiPath" : [ "q", "cv", "mult" ],
				"classInstance" : p["q"]["cv"].getClasses()[0],
			},

			{
				"parent" : p["q"]["cv"],
				"parameterPath" : [ "q", "cv", "coIO" ],
				"uiPath" : [ "q", "cv", "iAmALabel" ],
				"classInstance" : p["q"]["cv"].getClasses()[1],
			},

			{
				"parent" : p["q"]["c"],
				"parameterPath" : [ "q", "c" ],
				"uiPath" : [ "q", "c" ],
				"classInstance" : p["q"]["c"].getClass(),
			},

			{
				"parent" : p["q"]["c"]["cp"],
				"parameterPath" : [ "q", "c", "cp" ],
				"uiPath" : [ "q", "c", "cp" ],
				"classInstance" : p["q"]["c"]["cp"].getClass(),
			},

			{
				"parent" : p["q"]["c"]["cp"]["cv"],
				"parameterPath" : [ "q", "c", "cp", "cv", "mult" ],
				"uiPath" : [ "q", "c", "cp", "cv", "mult" ],
				"classInstance" : p["q"]["c"]["cp"]["cv"].getClasses()[0],
			},

			{
				"parent" : p["q"]["c"]["cp"]["cv"],
				"parameterPath" : [ "q", "c", "cp", "cv", "coIO" ],
				"uiPath" : [ "q", "c", "cp", "cv", "iAmALabel" ],
				"classInstance" : p["q"]["c"]["cp"]["cv"].getClasses()[1],
			},

		]

		self.assertEqual( expected, c )

		filteredExpected = [ expected[0], expected[4] ]

		c = IECore.ParameterAlgo.findClasses( p, classNameFilter=os.path.join( "maths", "*" ) )

		self.assertEqual( filteredExpected, c )

	def testCopyClasses( self ) :

		p = IECore.CompoundParameter(

			name = "q",
			description = "",
			members = [

				IECore.ClassVectorParameter(

					"cv",
					"d",
					"IECORE_OP_PATHS",
					[
						( "mult", os.path.join( "maths", "multiply" ), 2 ),
						( "coIO", "compoundObjectInOut", 2 ),
					]

				),

				IECore.ClassParameter(

					"c",
					"d",
					"IECORE_OP_PATHS",
					"classParameterTest", 1

				)

			]

		)

		p["c"]["cp"].setClass( "classVectorParameterTest", 1 )

		p["c"]["cp"]["cv"].setClasses( [
			( "mult", os.path.join( "maths", "multiply" ), 2 ),
			( "coIO", "compoundObjectInOut", 2 )
		] )

		p2 = IECore.CompoundParameter(

			name = "q",
			description = "",
			members = [

				IECore.ClassVectorParameter(

					"cv",
					"d",
					"IECORE_OP_PATHS",

				),

				IECore.ClassParameter(

					"c",
					"d",
					"IECORE_OP_PATHS",

				)

			]

		)

		IECore.ParameterAlgo.copyClasses( p, p2 )

		cl = [ c[1:] for c in p2["cv"].getClasses( True ) ]
		self.assertEqual( cl, [
				( "mult", os.path.join( "maths", "multiply" ), 2 ),
				( "coIO", "compoundObjectInOut", 2 )
			]
		)

		cl = [ c[1:] for c in p2["c"]["cp"]["cv"].getClasses( True ) ]
		self.assertEqual( cl, [
				( "mult", os.path.join( "maths", "multiply" ), 2 ),
				( "coIO", "compoundObjectInOut", 2 )
			]
		)

if __name__ == "__main__":
	unittest.main()
