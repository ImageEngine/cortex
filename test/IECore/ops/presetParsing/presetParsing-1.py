##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

import imath

import IECore

class presetParsing( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self,
			"An Op to test the parsing of parameter presets.",
			IECore.IntParameter(
				name = "result",
				description = "d",
				defaultValue = 2,
			)
		)

		self.parameters().addParameters(

			[

				IECore.V3fParameter(
					name = "h",
					description = "a v3f",
					defaultValue = IECore.V3fData(),
					presets = (
						( "x", imath.V3f( 1, 0, 0 ) ),
						( "y", imath.V3f( 0, 1, 0 ) ),
						( "z", imath.V3f( 0, 0, 1 ) )
					)
				),

				IECore.V2dParameter(
					name = "i",
					description = "a v2d",
					defaultValue = IECore.V2dData( imath.V2d( 0 ) ),
				),

				IECore.CompoundParameter(

					name = "compound",
					description = "a compound parameter",

					members = [

						IECore.V3dParameter(
							name = "j",
							description = "a v3d",
							defaultValue = IECore.V3dData(),
							presets = (
								( "one", imath.V3d( 1 ) ),
								( "two", imath.V3d( 2 ) )
							)
						),

						IECore.M44fParameter(
							name = "k",
							description = "an m44f",
							defaultValue = IECore.M44fData(),
							presets = (
								( "one", imath.M44f( 1 ) ),
								( "two", imath.M44f( 2 ) )
							)
						),

					]

				)

			]
		)

	def doOperation( self, operands ) :

		assert operands["h"] == IECore.V3fData( imath.V3f( 1, 0, 0 ) )

		assert operands["i"] == IECore.V2dData( imath.V2d( 0 ) )

		compoundPreset = IECore.CompoundObject()
		compoundPreset["j"] = IECore.V3dData( imath.V3d( 1 ) )
		compoundPreset["k"] = IECore.M44fData( imath.M44f( 1 ) )
		assert operands["compound"] == compoundPreset

		return IECore.IntData( 1 )

IECore.registerRunTimeTyped( presetParsing )
