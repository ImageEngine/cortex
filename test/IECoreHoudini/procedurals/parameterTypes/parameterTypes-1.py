##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

from IECore import *
import os

class parameterTypes( ParameterisedProcedural ) :

	def __init__( self ) :

		ParameterisedProcedural.__init__( self, "test parameter types" )

		self.parameters().addParameters(

			[

				IntParameter(
					name = "a",
					description = "An int which has a very long description to help test the help formatting. i wonder if there's anything more interesting i could write here.",
					defaultValue = 1,
					userData = { 'UI': { "label" : StringData( "Int" ), "update" : BoolData( True ), "visible" : BoolData( False ) } }
				),

				FloatParameter(
					name = "b",
					description = "A float which has a very long description to help test the help formatting. i wonder if there's anything more interesting i could write here.",
					defaultValue = 2,
					userData = { 'UI': { "update" : BoolData( True ) } }
				),

				DoubleParameter(
					name = "c",
					description = "A double",
					defaultValue = 3,
					userData = { 'UI': { "label" : StringData( "Double" ), "update" : BoolData( True ), "visible" : BoolData( False ) } }
				),

				StringParameter(
					name = "d",
					description = "A string",
					defaultValue = "ssss",
					userData = { 'UI': { "update" : BoolData( True ) } }
				),

				IntVectorParameter(
					name = "e",
					description = "An array of ints",
					defaultValue = IntVectorData( [ 4, -1, 2 ] ),
				),

				StringVectorParameter(
					name = "f",
					description = "An array of strings",
					defaultValue = StringVectorData( ["one", "two", "three" ]),
				),

				V2fParameter(
					name = "g",
					description = "A v2f",
					defaultValue = V2fData( V2f( 1,2 ) ),
					userData = { 'UI': { "label" : StringData( "V2f" ), "update" : BoolData( True ) } }
				),

				V3fParameter(
					name = "h",
					description = "a v3f",
					defaultValue = V3fData( V3f( 1, 1, 1 ) ),
					presets = (
						( "x", V3f( 1, 0, 0 ) ),
						( "y", V3f( 0, 1, 0 ) ),
						( "z", V3f( 0, 0, 1 ) )
					),
					userData = { 'UI': { "label" : StringData( "V3f" ), "update" : BoolData( True ) } }
				),

				V2dParameter(
					name = "i",
					description = "a v2d",
					defaultValue = V2dData( V2d( 1, 1 ) ),
					userData = { 'UI': { "label" : StringData( "V2d" ), "update" : BoolData( True ) } }
				),

				V3dParameter(
					name = "i_2",
					description = "a v3d",
					defaultValue = V3dData( V3d( 1, 1, 0 ) ),
					userData = { 'UI': { "label" : StringData( "V2d" ), "update" : BoolData( True ) } }
				),

				CompoundParameter(

					name = "compound",
					description = "a compound parameter",
					userData = { 'UI': { "label" : StringData( "My Compound" ) } },
					members = [

						V3dParameter(
							name = "j",
							description = "a v3d",
							defaultValue = V3dData( V3d( 8, 16, 32 ) ),
							presets = (
								( "one", V3d( 1 ) ),
								( "two", V3d( 2 ) )
							),
							userData = { "label":StringData("Compound->V3d") }
						),

						M44fParameter(
							name = "k",
							description = "an m44f",
							defaultValue = M44fData( ),
							presets = (
								( "one", M44f( 1 ) ),
								( "two", M44f( 2 ) )
							)
						),

					]

				),

				Color3fParameter(
					name = "l",
					description = "a color3f",
					defaultValue = Color3fData( Color3f( 1, 0, 1 )),
					userData = { 'UI': { "label" : StringData( "Colour 3" ), "update" : BoolData( True ) } }
				),

				Color4fParameter(
					name = "m",
					description = "a color4f",
					defaultValue = Color4fData( Color4f( 1, 0, 1, 0.5 ) ),
					userData = { 'UI': { "label" : StringData( "Colour 4" ), "update" : BoolData( True ) } }
				),

				FileNameParameter(
					name = "o",
					description = "tif file please!",
					defaultValue = "test.tif",
					extensions = "tif",
					allowEmptyString = True,
					userData = { 'UI': { "label" : StringData( "File Name" ), "update" : BoolData( True ) } }
				),

				DirNameParameter(
					name = "p",
					description = "directory please!",
					defaultValue = os.getcwd(),
					check = DirNameParameter.CheckType.MustExist,
					allowEmptyString = True,
					userData = { 'UI': { "label" : StringData( "Dir Name" ), "update" : BoolData( True ) } }
				),

				BoolParameter(
					name = "q",
					description = "blah",
					defaultValue = True,
					userData = { 'UI': { "label" : StringData( "Boolean" ), "update" : BoolData( True ) } }
				),

				FileSequenceParameter(
					name = "r",
					description = "File sequence please!",
					defaultValue = "/path/to/sequence.####.tif",
					userData = { 'UI': { "label" : StringData( "File Seq" ), "update" : BoolData( True ) } }
				),

				Box2dParameter(
					name = "s",
					description = "boxboxbox",
					defaultValue = Box2d( V2d( -1 ), V2d( 1 ) )
				),

				Box2iParameter(
					name = "s_1",
					description = "boxboxbox2i",
					defaultValue = Box2i( V2i( -1 ), V2i( 1 ) )
				),

				Box3iParameter(
					name = "s_2",
					description = "boxboxbox3i",
					defaultValue = Box3i( V3i( -1 ), V3i( 1 ) )
				),

				Box3fParameter(
					name = "t",
					description = "boxboxbox",
					defaultValue = Box3f( V3f( -1 ), V3f( 1 ) )
				),

				V2iParameter(
					name = "u",
					description = "A v2i",
					defaultValue = V2iData( V2i( 2, 2 ) ),
					userData = { 'UI': { "label" : StringData( "V2i" ), "update" : BoolData( True ) } }
				),


				V3iParameter(
					name = "v",
					description = "A v3i",
					defaultValue = V3iData( V3i( 5, 5, 5 ) ),
					userData = { 'UI': { "label" : StringData( "V3i" ), "update" : BoolData( True ) } }
				),

				FrameListParameter(
					name = "w",
					description = "A FrameList",
					defaultValue = "",
				),

				M44fParameter(
					name = "i_3",
					description = "an m44f",
					defaultValue = M44fData( ),
					presets = (
						( "one", M44f( 1 ) ),
						( "two", M44f( 2 ) )
					),
					userData = { 'UI': { "label" : StringData( "M44f" ), "update" : BoolData( True ) } }
				),

				M44dParameter(
					name = "i_4",
					description = "an m44d",
					defaultValue = M44dData( ),
					presets = (
						( "one", M44d( 1 ) ),
						( "two", M44d( 2 ) )
					),
					userData = { 'UI': { "label" : StringData( "M44d" ), "update" : BoolData( True ) } }
				),
			]
		)

	def doBound(self, args) :
		assert args["a"].value==123
		assert args["b"].value > 1.9999
		assert args["c"].value==3
		assert args["d"].value=="hello"
		#assert args["e"] == IntVectorData( [2, 4, 5] )
		#assert args["f"] == StringVectorData( ["one", "two", "three"] )
		assert args["g"] == V2fData( V2f( 2, 4 ) )
		assert args["h"] == V3fData( V3f( 1, 4, 8 ) )
		assert args["i"] == V2dData( V2d( 2, 4 ) )
		assert args["compound"]["j"] == V3dData( V3d( 1, 4, 8 ) )
		assert args["compound"]["k"] == M44fData( M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) )
		assert args["i_3"] == M44fData( M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) )
		assert args["i_4"] == M44dData( M44d( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) )
		assert args["l"] == Color3fData( Color3f( 1, 0, 0 ) )
		assert args["m"] == Color4fData( Color4f( 1, 1, 0, 1 ) )
		assert args["o"] == StringData( "myFile.tif" )
		assert args["p"] == StringData( os.getcwd() )
		assert args["q"] == BoolData( True )
		assert args["r"] == StringData( "mySequence.####.tif" )
		assert args["s"] == Box2dData( Box2d( V2d( -1, -2 ), V2d( 10, 20 ) ) )
		assert args["s_1"] == Box2iData( Box2i( V2i( -1, -2 ), V2i( 10, 20 ) ) )
		assert args["s_2"] == Box3iData( Box3i( V3i( -1, -2, -3 ), V3i( 10, 20, 30 ) ) )
		assert args["t"] == Box3fData( Box3f( V3f( -1, -2, -3), V3f( 10, 20, 30) ) )
		assert args["u"] == V2iData( V2i( 64, 128 ) )
		assert args["v"] == V3iData( V3i( 25, 26, 27 ) )
		#assert self["w"].getFrameListValue().asList() == FrameRange( 0, 500, 250 ).asList()
		box = MeshPrimitive.createBox( Box3f( V3f(0,0,0), V3f(1,1,1) ) )
		return box.bound()

	def doRenderState(self, renderer, args) :
		pass

	def doRender(self, renderer, args) :
		box = MeshPrimitive.createBox( Box3f( V3f(0,0,0), V3f(1,1,1) ) )
		box.render( renderer )
		return IntData( 1 )

registerRunTimeTyped( parameterTypes )
