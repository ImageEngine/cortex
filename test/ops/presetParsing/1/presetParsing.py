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

class presetParsing( Op ) :
	
	def __init__( self ) :
	
		Op.__init__( self,
			"presetParsing",
			"An Op to test the parsing of parameter presets.",
			IntParameter( 
				name = "result",
				description = "d",
				defaultValue = 2,
			)
		)
		
		self.parameters().addParameters(
		
			[	
			
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
					defaultValue = V2dData( V2d( 0 ) ),
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

				)
					
			]
		)
	
	def doOperation( self, operands ) :
		
		assert operands.h == V3fData( V3f( 1, 0, 0 ) )
		
		assert operands.i == V2dData( V2d( 0 ) )
	
		compoundPreset = CompoundObject()
		compoundPreset.j = V3dData( V3d( 1 ) )
		compoundPreset.k = M44fData( M44f( 1 ) )
		assert operands.compound == compoundPreset
	
		return IntData( 1 )

