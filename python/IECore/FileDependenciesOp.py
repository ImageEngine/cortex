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
import os
import os.path

class FileDependenciesOp( Op ) :

	def __init__( self ) :

		Op.__init__( self, "FileDependenciesOp", "Lists the dependencies of a file.",
			Parameter(
				name = "result",
				description = "A list of required files and file sequences.",
				defaultValue = StringVectorData()
			)
		)

		self.parameters().addParameters(
			[
				FileNameParameter(
					name = "file",
					description = "The file to list dependencies for.",
					defaultValue = "",
					check = DirNameParameter.CheckType.MustExist,
					extensions = " ".join( FileExaminer.supportedExtensions() ),
					allowEmptyString = False,
				),
				BoolParameter(
					name = "recurse",
					description = "When on, recursively searches the file dependency tree and lists all results.",
					defaultValue = False,
				),
				StringParameter(
					name = "resultType",
					description = "The format of the result",
					defaultValue = "string",
					presets = (
						( "string", "string" ),
						( "stringVector", "stringVector" ),
					),
					presetsOnly = True,
				)
			]
		)

	def doOperation( self, operands ) :

		files = set()
		if operands["recurse"].value :

			files = FileExaminer.allDependencies( operands["file"].value )

		else :

			files = FileExaminer.create( operands["file"].value ).dependencies()

		if operands["resultType"].value == "string" :
			return StringData( "\n".join( [str(s) for s in files] ) )
		else :
			return StringVectorData( [str(s) for s in files] )

registerRunTimeTyped( FileDependenciesOp, 100010, Op )
