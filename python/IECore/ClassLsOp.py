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
import os
import os.path

class ClassLsOp( Op ) :

	def __init__( self ) :
	
		Op.__init__( self, "ClassLsOp", "Lists installed classes which can be loaded with IECore.ClassLoader.",
			Parameter(
				name = "result",
				description = "A list of classes.",
				defaultValue = StringVectorData()
			)
		)
		
		self.parameters().addParameters(
			[
				StringParameter(
					name = "type",
					description = "The type of class to list.",
					defaultValue = "procedural",
					presets = {
						"Procedural" : "procedural",
						"Op" : "op",
						"Other" : "other",
					},
					presetsOnly = True,
				),
				StringParameter(
					name = "match",
					description = "A glob style match string used to list only a subset of classes.",
					defaultValue = "*",
				),
				StringParameter(
					name = "searchPath",
					description = "When type is set to \"other\", this specifies a colon separated list of paths to search for classes on.",
					defaultValue = "",
				),
				StringParameter(
					name = "searchPathEnvVar",
					description = 	"When type is set to \"other\", this specifies an environment variable "
									"specifying a list of paths to search for classes on.",
					defaultValue = "",
				),
				StringParameter(
					name = "resultType",
					description = "The format of the result",
					defaultValue = "string",
					presets = {
						"string" : "string",
						"stringVector" : "stringVector",
					},
					presetsOnly = True,
				)
			]
		)

	def doOperation( self, operands ) :
	
		t = operands.type.value
		if t=="procedural" :
			loader = ClassLoader.defaultProceduralLoader()
		elif t=="op" :
			loader = ClassLoader.defaultOpLoader()
		else :
			if operands.searchPath.value and operands.searchPathEnvVar.value :
				raise RuntimeError( "Cannot specify both searchPath and searchPathEnvVar." )
			if not operands.searchPath.value and not operands.searchPathEnvVar.value :
				raise RuntimeError( "Must specify either searchPath or searchPathEnvVar." )
				
			if operands.searchPath.value :
				sp = SearchPath( operands.searchPath.value, ":" )
			else :
				sp = SearchPath( os.path.expandvars( os.environ[operands.searchPathEnvVar.value] ), ":" )
				
			loader = ClassLoader( sp )
			
		classes = loader.classNames( operands.match.value )	
							
		if operands.resultType.value == "string" :
			return StringData( "\n".join( classes ) )
		else :
			return StringVectorData( classes )

makeRunTimeTyped( ClassLsOp, 100008, Op )
