##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

import datetime
import imath

import IECore

class parameterTypes( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"test all parameter types.",
			IECore.IntParameter(
				name = "result",
				description = "one if everything is ok",
				defaultValue = 0,
			)
		)

		self.parameters().addParameters(

			[

				IECore.IntParameter(
					name = "a",
					description = "An int which has a very long description to help test the help formatting. i wonder if there's anything more interesting i could write here.",
					defaultValue = 1,
				),

				IECore.FloatParameter(
					name = "b",
					description = "A float which has a very long description to help test the help formatting. i wonder if there's anything more interesting i could write here.",
					defaultValue = 2,
				),

				IECore.DoubleParameter(
					name = "c",
					description = "A double",
					defaultValue = 3,
				),

				IECore.StringParameter(
					name = "d",
					description = "A string",
					defaultValue = "ssss",
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
					defaultValue = IECore.V2fData( imath.V2f( 1,2 ) ),
				),

				IECore.V3fParameter(
					name = "h",
					description = "a v3f",
					defaultValue = IECore.V3fData( imath.V3f( 1, 1, 1 ) ),
					presets = (
						( "x", imath.V3f( 1, 0, 0 ) ),
						( "y", imath.V3f( 0, 1, 0 ) ),
						( "z", imath.V3f( 0, 0, 1 ) )
					)
				),

				IECore.V2dParameter(
					name = "i",
					description = "a v2d",
					defaultValue = IECore.V2dData( imath.V2d( 1, 1 ) ),
				),

				IECore.CompoundParameter(

					name = "compound",
					description = "a compound parameter",

					members = [

						IECore.V3dParameter(
							name = "j",
							description = "a v3d",
							defaultValue = IECore.V3dData( imath.V3d( 8, 16, 32 ) ),
							presets = (
								( "one", imath.V3d( 1 ) ),
								( "two", imath.V3d( 2 ) )
							)
						),

						IECore.M44fParameter(
							name = "k",
							description = "an m44f",
							defaultValue = IECore.M44fData( ),
							presets = (
								( "one", imath.M44f( 1 ) ),
								( "two", imath.M44f( 2 ) )
							)
						),

					]

				),

				IECore.Color3fParameter(
					name = "l",
					description = "a color3f",
					defaultValue = IECore.Color3fData( imath.Color3f( 1, 0, 1 )),
				),

				IECore.Color4fParameter(
					name = "m",
					description = "a color4f",
					defaultValue = IECore.Color4fData( imath.Color4f( 1, 0, 1, 0.5 ) ),
				),

				IECore.FileNameParameter(
					name = "o",
					description = "tif file please!",
					defaultValue = "",
					extensions = "tif",
					allowEmptyString = True,
				),

				IECore.DirNameParameter(
					name = "p",
					description = "directory please!",
					defaultValue = "",
					check = IECore.DirNameParameter.CheckType.MustExist,
					allowEmptyString = True,
				),

				IECore.BoolParameter(
					name = "q",
					description = "blah",
					defaultValue = False,
				),

				IECore.FileSequenceParameter(
					name = "r",
					description = "File sequence please!",
					defaultValue = ""
				),

				IECore.Box2dParameter(
					name = "s",
					description = "boxboxbox",
					defaultValue = imath.Box2d( imath.V2d( -1 ), imath.V2d( 1 ) )
				),

				IECore.Box3fParameter(
					name = "t",
					description = "boxboxbox",
					defaultValue = imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) )
				),

				IECore.V2iParameter(
					name = "u",
					description = "A v2i",
					defaultValue = IECore.V2iData( imath.V2i( 2, 2 ) ),
				),


				IECore.V3iParameter(
					name = "v",
					description = "A v3i",
					defaultValue = IECore.V3iData( imath.V3i( 5, 5, 5 ) ),
				),

				IECore.FrameListParameter(
					name = "w",
					description = "A FrameList",
					defaultValue = "",
				),

				IECore.TransformationMatrixfParameter(
					name = "x",
					description = "",
					defaultValue = IECore.TransformationMatrixf(),
				),

				# We'd like to have one with a non-standard rotation order
				# here, but they're not currently supported in Maya.
				IECore.TransformationMatrixdParameter(
					name = "y",
					description = "",
					defaultValue = IECore.TransformationMatrixd(),
				),

				IECore.ObjectParameter(
					name = "p1",
					description = "",
					defaultValue = IECore.CompoundObject(),
					types = [ IECore.TypeId.CompoundObject ]
				),

				IECore.LineSegment3fParameter(
					name = "p2",
					description = "",
					defaultValue = IECore.LineSegment3f( imath.V3f( 1 ), imath.V3f( 2 ) )
				),

				IECore.LineSegment3dParameter(
					name = "p3",
					description = "",
					defaultValue = IECore.LineSegment3d( imath.V3d( 1 ), imath.V3d( 2 ) )
				),

				IECore.DateTimeParameter(
					name = "p4",
					description = "",
					defaultValue = datetime.datetime.now()
				),
			]
		)

	def doOperation( self, args ) :

		assert args["a"].value==10
		assert abs(args["b"].value-20.2) < 0.0001
		assert args["c"].value==40.5
		assert args["d"].value=="hello"
		assert args["e"] == IECore.IntVectorData( [2, 4, 5] )
		assert args["f"] == IECore.StringVectorData( ["one", "two", "three", "-1", "-dash", "\\-slashDash", "\\\\-slashSlashDash", "inline-dash"] )
		assert args["g"] == IECore.V2fData( imath.V2f( 2, 4 ) )
		assert args["h"] == IECore.V3fData( imath.V3f( 1, 4, 8 ) )
		assert args["i"] == IECore.V2dData( imath.V2d( 2, 4 ) )
		assert args["compound"]["j"] == IECore.V3dData( imath.V3d( 1, 4, 8 ) )
		assert args["compound"]["k"] == IECore.M44fData( imath.M44f( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) )
		assert args["l"] == IECore.Color3fData( imath.Color3f( 1, 0, 0 ) )
		assert args["m"] == IECore.Color4fData( imath.Color4f( 1, 1, 0, 1 ) )
		assert args["o"] == IECore.StringData( "myFile.tif" )
		assert args["p"] == IECore.StringData( "test" )
		assert args["q"] == IECore.BoolData( True )
		assert args["r"] == IECore.StringData( "mySequence.####.tif" )
		assert args["s"] == IECore.Box2dData( imath.Box2d( imath.V2d( -1, -2 ), imath.V2d( 10, 20 ) ) )
		assert args["t"] == IECore.Box3fData( imath.Box3f( imath.V3f( -1, -2, -3), imath.V3f( 10, 20, 30) ) )
		assert args["u"] == IECore.V2iData( imath.V2i( 64, 128 ) )
		assert args["v"] == IECore.V3iData( imath.V3i( 25, 26, 27 ) )
		assert self["w"].getFrameListValue().asList() == IECore.FrameRange( 0, 500, 250 ).asList()
		assert args["x"] == IECore.TransformationMatrixfData()
		assert args["y"] == IECore.TransformationMatrixdData()

		return IECore.IntData( 1 )

IECore.registerRunTimeTyped( parameterTypes )
