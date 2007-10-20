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

class stringParsing( Op ) :

	def __init__( self ) :
	
		Op.__init__( self,
			"stringParsing",
			"test various problem cases with string parsing.",
			IntParameter( 
				name = "result",
				description = "one if everything is ok",
				defaultValue = 0,
			)
		)

		self.parameters().addParameters(
			
			[
				StringParameter(
					name = "emptyString",
					description = "",
					defaultValue = "notEmpty",
				),
				StringParameter(
					name = "normalString",
					description = "",
					defaultValue = "",
				),
				StringParameter(
					name = "stringWithSpace",
					description = "",
					defaultValue = "noSpacesHere",
				),
				StringParameter(
					name = "stringWithManySpaces",
					description = "",
					defaultValue = "noSpacesHere",
				)
			]
		
		)
		
	def doOperation( self, args ) :

		assert args.emptyString.value == ""
		assert args.normalString.value == "hello"
		assert args.stringWithSpace.value == "hello there"
		assert args.stringWithManySpaces.value == "hello there old chap"
		
		return IntData( 1 )
