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

from IECore import *

class parameterTypes( Op ) :
	
	def __init__( self ) :
	
		Op.__init__( self,
			"parameterTypes",
			"test all parameter types.",
			IntParameter( 
				name = "result",
				description = "one if everything is ok",
				defaultValue = 0,
			)
		)
		
		self.parameters().addParameters(
		
			[

				IntParameter(
					name = "a",
					description = "An int which has a very long description to help test the help formatting. i wonder if there's anything more interesting i could write here.",
					defaultValue = 1,
				),

				FloatParameter(
					name = "b",
					description = "A float which has a very long description to help test the help formatting. i wonder if there's anything more interesting i could write here.",
					defaultValue = 2,
				),

				DoubleParameter(
					name = "c",
					description = "A double",
					defaultValue = 3,
				),

				StringParameter(
					name = "d",
					description = "A string",
					defaultValue = "ssss",
				),

				IntVectorParameter(
					name = "e",
					description = "An array of ints",
					defaultValue = IntVectorData(),
				),

				StringVectorParameter(
					name = "f",
					description = "An array of strings",
					defaultValue = StringVectorData(),
				),

				V2fParameter(
					name = "g",
					description = "A v2f",
					defaultValue = V2fData(),
				),

				V3fParameter(
					name = "h",
					description = "a v3f",
					defaultValue = V3fData(),
					presets = {
						"x" : V3f( 1, 0, 0 ),
						"y" : V3f( 0, 1, 0 ),
						"z" : V3f( 0, 0, 1 )
					}
				),

				V2dParameter(
					name = "i",
					description = "a v2d",
					defaultValue = V2dData(),
				),

				CompoundParameter(

					name = "compound",
					description = "a compound parameter",

					members = [

						V3dParameter(
							name = "j",
							description = "a v3d",
							defaultValue = V3dData(),
							presets = {
								"one" : V3d( 1 ),
								"two" : V3d( 2 )
							}
						),

						M44fParameter(
							name = "k",
							description = "an m44f",
							defaultValue = M44fData(),
							presets = {
								"one" : M44f( 1 ),
								"two" : M44f( 2 )
							}
						),

					]

				),

				Color3fParameter(
					name = "l",
					description = "a color3f",
					defaultValue = Color3fData(),
				),

				Color4fParameter(
					name = "m",
					description = "a color4f",
					defaultValue = Color4fData(),
				),

				FileNameParameter(
					name = "o",
					description = "tif file please!",
					defaultValue = "",
					extensions = "tif",
					allowEmptyString = True,
				),

				DirNameParameter(
					name = "p",
					description = "directory please!",
					defaultValue = "",
					check = DirNameParameter.CheckType.MustExist,
					allowEmptyString = True,
				),

				BoolParameter(
					name = "q",
					description = "blah",
					defaultValue = False,
				),

				FileSequenceParameter(
					name = "r",
					description = "File sequence please!",
					defaultValue = ""
				),
				
				Box2dParameter(
					name = "s",
					description = "boxboxbox",
					defaultValue = Box2d( V2d( -1 ), V2d( 1 ) )
				),
				
				Box3fParameter(
					name = "t",
					description = "boxboxbox",
					defaultValue = Box3f( V3f( -1 ), V3f( 1 ) )
				),
				
				V2iParameter(
					name = "u",
					description = "A v2i",
					defaultValue = V2iData(),
				),
				
				
				V3iParameter(
					name = "v",
					description = "A v3i",
					defaultValue = V3iData(),
				),
			]
		)
	
	def doOperation( self, args ) :
	
		assert args.a.value==10
		assert abs(args.b.value-20.2) < 0.0001
		assert args.c.value==40.5
		assert args.d.value=="hello"
		assert args.e == IntVectorData( [2, 4, 5] )
		assert args.f == StringVectorData( ["one", "two", "three"] )
		assert args.g == V2fData( V2f( 2, 4 ) )   
		assert args.h == V3fData( V3f( 1, 4, 8 ) )
		assert args.i == V2dData( V2d( 2, 4 ) )   
		assert args.compound.j == V3dData( V3d( 1, 4, 8 ) )
		assert args.compound.k == M44fData( M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) )
		assert args.l == Color3fData( Color3f( 1, 0, 0 ) )	 
		assert args.m == Color4fData( Color4f( 1, 1, 0, 1 ) ) 
		assert args.o == StringData( "myFile.tif" )  
		assert args.p == StringData( "test" )	 
		assert args.q == BoolData( True )	 
		assert args.r == StringData( "mySequence.####.tif" ) 
		assert args.s == Box2dData( Box2d( V2d( -1, -2 ), V2d( 10, 20 ) ) ) 
		assert args.t == Box3fData( Box3f( V3f( -1, -2, -3), V3f( 10, 20, 30) ) ) 		
		assert args.u == V2iData( V2i( 64, 128 ) )
		assert args.v == V3iData( V3i( 25, 26, 27 ) )		

		return IntData( 1 )

