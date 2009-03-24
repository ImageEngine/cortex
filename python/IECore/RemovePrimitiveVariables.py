##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
from fnmatch import fnmatchcase
from RunTimeTypedUtil import makeRunTimeTyped

class RemovePrimitiveVariables( PrimitiveOp ) :

	def __init__( self ) :
	
		PrimitiveOp.__init__( self, "RemovePrimitiveVariables", "Removes variables from primitives" )
		
		self.parameters().addParameters(
			[
				StringParameter( 
					name = "mode",
					description = """This chooses whether or not the names parameter specifies the names of
						variables to keep or the names of variables to remove.""",				
					defaultValue = "remove",
					presets = {
						"keep" : "keep",
						"remove" : "remove"
					},
					presetsOnly = True
				),
				StringVectorParameter( 
					name = "names",
					description = "The names of variables. These can include * or ? characters to match many names.",
					defaultValue = StringVectorData()
				)
			]
		)
		
	def modifyPrimitive( self, primitive, args ) :
	
		keep = args["mode"].value == "keep"
	
		for key in primitive.keys() :

			for n in args["names"] :
			
				m = fnmatchcase( key, n )
				if (m and not keep) or (not m and keep) :
					del primitive[key]

makeRunTimeTyped( RemovePrimitiveVariables, 100001, PrimitiveOp )
