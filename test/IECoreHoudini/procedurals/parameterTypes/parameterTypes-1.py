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

import IECore
import IECoreScene

import os

class parameterTypes( IECoreScene.ParameterisedProcedural ) :

	def __init__( self ) :

		IECoreScene.ParameterisedProcedural.__init__( self, "test parameter types" )

		self.parameters().addParameters(

			[

				IECore.IntParameter(
					name = "a",
					description = "An int which has a very long description to help test the help formatting. i wonder if there's anything more interesting i could write here.",
					defaultValue = 1,
					userData = { 'UI': { "label" : IECore.StringData( "Int" ), "update" : IECore.BoolData( True ), "visible" : IECore.BoolData( False ) } }
				),

				IECore.FloatParameter(
					name = "b",
					description = "A float which has a very long description to help test the help formatting. i wonder if there's anything more interesting i could write here.",
					defaultValue = 2,
					userData = { 'UI': { "update" : IECore.BoolData( True ) } }
				),

				IECore.DoubleParameter(
					name = "c",
					description = "A double",
					defaultValue = 3,
					userData = { 'UI': { "label" : IECore.StringData( "Double" ), "update" : IECore.BoolData( True ), "visible" : IECore.BoolData( False ) } }
				),

				IECore.StringParameter(
					name = "d",
					description = "A string",
					defaultValue = "ssss",
					userData = { 'UI': { "update" : IECore.BoolData( True ) } }
				),

				IECore.IntVectorParameter(
					name = "e",
					description = "An array of ints",
					defaultValue = IECore.IntVectorData( [ 4, -1, 2 ] ),
				),

				IECore.StringVectorParameter(
					name = "f",
					description = "An array of strings",
					defaultValue = IECore.StringVectorData( ["one", "two", "three" ]),
				),

				IECore.V2fParameter(
					name = "g",
					description = "A v2f",
					defaultValue = IECore.V2fData( IECore.V2f( 1,2 ) ),
					userData = { 'UI': { "label" : IECore.StringData( "V2f" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.V3fParameter(
					name = "h",
					description = "a v3f",
					defaultValue = IECore.V3fData( IECore.V3f( 1, 1, 1 ) ),
					presets = (
						( "x", IECore.V3f( 1, 0, 0 ) ),
						( "y", IECore.V3f( 0, 1, 0 ) ),
						( "z", IECore.V3f( 0, 0, 1 ) )
					),
					userData = { 'UI': { "label" : IECore.StringData( "V3f" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.V2dParameter(
					name = "i",
					description = "a v2d",
					defaultValue = IECore.V2dData( IECore.V2d( 1, 1 ) ),
					userData = { 'UI': { "label" : IECore.StringData( "V2d" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.V3dParameter(
					name = "i_2",
					description = "a v3d",
					defaultValue = IECore.V3dData( IECore.V3d( 1, 1, 0 ) ),
					userData = { 'UI': { "label" : IECore.StringData( "V2d" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.CompoundParameter(

					name = "compound",
					description = "a compound parameter",
					userData = { 'UI': { "label" : IECore.StringData( "My Compound" ) } },
					members = [

						IECore.V3dParameter(
							name = "j",
							description = "a v3d",
							defaultValue = IECore.V3dData( IECore.V3d( 8, 16, 32 ) ),
							presets = (
								( "one", IECore.V3d( 1 ) ),
								( "two", IECore.V3d( 2 ) )
							),
							userData = { "label":IECore.StringData("Compound->V3d") }
						),

						IECore.M44fParameter(
							name = "k",
							description = "an m44f",
							defaultValue = IECore.M44fData( ),
							presets = (
								( "one", IECore.M44f( 1 ) ),
								( "two", IECore.M44f( 2 ) )
							)
						),

					]

				),

				IECore.Color3fParameter(
					name = "l",
					description = "a color3f",
					defaultValue = IECore.Color3fData( IECore.Color3f( 1, 0, 1 )),
					userData = { 'UI': { "label" : IECore.StringData( "Colour 3" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.Color4fParameter(
					name = "m",
					description = "a color4f",
					defaultValue = IECore.Color4fData( IECore.Color4f( 1, 0, 1, 0.5 ) ),
					userData = { 'UI': { "label" : IECore.StringData( "Colour 4" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.FileNameParameter(
					name = "o",
					description = "tif file please!",
					defaultValue = "test.tif",
					extensions = "tif",
					allowEmptyString = True,
					userData = { 'UI': { "label" : IECore.StringData( "File Name" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.DirNameParameter(
					name = "p",
					description = "directory please!",
					defaultValue = os.getcwd(),
					check = IECore.DirNameParameter.CheckType.MustExist,
					allowEmptyString = True,
					userData = { 'UI': { "label" : IECore.StringData( "Dir Name" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.BoolParameter(
					name = "q",
					description = "blah",
					defaultValue = True,
					userData = { 'UI': { "label" : IECore.StringData( "Boolean" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.FileSequenceParameter(
					name = "r",
					description = "File sequence please!",
					defaultValue = "/path/to/sequence.####.tif",
					userData = { 'UI': { "label" : IECore.StringData( "File Seq" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.Box2dParameter(
					name = "s",
					description = "boxboxbox",
					defaultValue = IECore.Box2d( IECore.V2d( -1 ), IECore.V2d( 1 ) )
				),

				IECore.Box2iParameter(
					name = "s_1",
					description = "boxboxbox2i",
					defaultValue = IECore.Box2i( IECore.V2i( -1 ), IECore.V2i( 1 ) )
				),

				IECore.Box3iParameter(
					name = "s_2",
					description = "boxboxbox3i",
					defaultValue = IECore.Box3i( IECore.V3i( -1 ), IECore.V3i( 1 ) )
				),

				IECore.Box3fParameter(
					name = "t",
					description = "boxboxbox",
					defaultValue = IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )
				),

				IECore.V2iParameter(
					name = "u",
					description = "A v2i",
					defaultValue = IECore.V2iData( IECore.V2i( 2, 2 ) ),
					userData = { 'UI': { "label" : IECore.StringData( "V2i" ), "update" : IECore.BoolData( True ) } }
				),


				IECore.V3iParameter(
					name = "v",
					description = "A v3i",
					defaultValue = IECore.V3iData( IECore.V3i( 5, 5, 5 ) ),
					userData = { 'UI': { "label" : IECore.StringData( "V3i" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.FrameListParameter(
					name = "w",
					description = "A FrameList",
					defaultValue = "",
				),

				IECore.M44fParameter(
					name = "i_3",
					description = "an m44f",
					defaultValue = IECore.M44fData( ),
					presets = (
						( "one", IECore.M44f( 1 ) ),
						( "two", IECore.M44f( 2 ) )
					),
					userData = { 'UI': { "label" : IECore.StringData( "M44f" ), "update" : IECore.BoolData( True ) } }
				),

				IECore.M44dParameter(
					name = "i_4",
					description = "an m44d",
					defaultValue = IECore.M44dData( ),
					presets = (
						( "one", IECore.M44d( 1 ) ),
						( "two", IECore.M44d( 2 ) )
					),
					userData = { 'UI': { "label" : IECore.StringData( "M44d" ), "update" : IECore.BoolData( True ) } }
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
		assert args["g"] == IECore.V2fData( IECore.V2f( 2, 4 ) )
		assert args["h"] == IECore.V3fData( IECore.V3f( 1, 4, 8 ) )
		assert args["i"] == IECore.V2dData( IECore.V2d( 2, 4 ) )
		assert args["compound"]["j"] == IECore.V3dData( IECore.V3d( 1, 4, 8 ) )
		assert args["compound"]["k"] == IECore.M44fData( IECore.M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) )
		assert args["i_3"] == IECore.M44fData( IECore.M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) )
		assert args["i_4"] == IECore.M44dData( IECore.M44d( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) )
		assert args["l"] == IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) )
		assert args["m"] == IECore.Color4fData( IECore.Color4f( 1, 1, 0, 1 ) )
		assert args["o"] == IECore.StringData( "myFile.tif" )
		assert args["p"] == IECore.StringData( os.getcwd() )
		assert args["q"] == IECore.BoolData( True )
		assert args["r"] == IECore.StringData( "mySequence.####.tif" )
		assert args["s"] == IECore.Box2dData( IECore.Box2d( IECore.V2d( -1, -2 ), IECore.V2d( 10, 20 ) ) )
		assert args["s_1"] == IECore.Box2iData( IECore.Box2i( IECore.V2i( -1, -2 ), IECore.V2i( 10, 20 ) ) )
		assert args["s_2"] == IECore.Box3iData( IECore.Box3i( IECore.V3i( -1, -2, -3 ), IECore.V3i( 10, 20, 30 ) ) )
		assert args["t"] == IECore.Box3fData( IECore.Box3f( IECore.V3f( -1, -2, -3), IECore.V3f( 10, 20, 30) ) )
		assert args["u"] == IECore.V2iData( IECore.V2i( 64, 128 ) )
		assert args["v"] == IECore.V3iData( IECore.V3i( 25, 26, 27 ) )
		#assert self["w"].getFrameListValue().asList() == FrameRange( 0, 500, 250 ).asList()

		return IECore.Box3f( IECore.V3f(0,0,0), IECore.V3f(1,1,1) )

	def doRenderState(self, renderer, args) :
		pass

	def doRender(self, renderer, args) :
		box = IECoreScene.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(0,0,0), IECore.V3f(1,1,1) ) )
		box.render( renderer )
		return IECore.IntData( 1 )

IECore.registerRunTimeTyped( parameterTypes )
