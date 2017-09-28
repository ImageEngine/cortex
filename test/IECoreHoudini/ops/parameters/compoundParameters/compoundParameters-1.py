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

class compoundParameters( Op ) :

	def __init__( self ) :

		Op.__init__( self,
			"Op with some compound parameters.",
			ObjectParameter(
				name = "result",
				description = "Dummy.",
				defaultValue = PointsPrimitive(V3fVectorData()),
				type = TypeId.PointsPrimitive
			)
		)

		self.parameters().addParameters( [

			CompoundParameter(
				name = "compound_1",
				description = "a compound parameter",
				userData = { "UI" : { "label" : StringData( "My Compound 1" ) } },
				members = [
					V3dParameter(
						name = "j",
						description = "a v3d",
						defaultValue = V3dData( V3d( 8, 16, 32 ) ),
						userData = { "UI" : { "label" : StringData( "A Vector" ) } },
					),
					Color3fParameter(
						name = "k",
						description = "an m44f",
						defaultValue = Color3f(1,0.5,0),
						userData = { "UI" : { "label" : StringData( "A Colour" ) } },
					),
				]
			),

			CompoundParameter(
				name = "compound_2",
				description = "a compound parameter",
				userData = { "UI" : { "label" : StringData( "My Compound 2" ) } },
				members = [
					V3dParameter(
						name = "j",
						description = "a v3d",
						defaultValue = V3dData( V3d( 8, 16, 32 ) ),
						presets = (
							( "one", V3d( 1 ) ),
							( "two", V3d( 2 ) )
						),
						userData = { "UI" : { "label" : StringData( "Compound->V3d" ) } },
					),
					V2fParameter(
						name = "k",
						description = "an v2f",
						defaultValue = V2f(1,1)
					),
				]
			),

			CompoundParameter(
				name = "compound_3",
				description = "a compound parameter",
				userData ={ "UI" : { "label" : StringData( "My Compound 3" ) } },
				members = [
					CompoundParameter(
						name = "compound_4",
						description = "a compound parameter",
						userData = { "UI" : { "label" : StringData( "My Compound 4" ) } },
						members = [
							IntParameter(
								name = "some_int",
								description = "Int",
								defaultValue = 123,
								userData = { "UI" : { "label" : StringData( "Int" ) } },
							),
						]
					)
				]
			),

			FloatParameter(
				name="blah",
				description="blah",
				defaultValue = 123.0
			),

			CompoundParameter(
				name = "compound_5",
				description = "a compound parameter",
				userData = { "UI" : { "label" : StringData( "Another Compound Parameter" ) } },
				members = [
					BoolParameter(
						name = "bool_1",
						description = "a boolean parameter",
						defaultValue = True
					)
				]
			)
		] )

	def doOperation( self, args ) :
		return PointsPrimitive(V3fVectorData())

registerRunTimeTyped( compoundParameters )
