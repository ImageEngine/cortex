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

import os
import os.path

import IECore

class ClassLsOp( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self, "Lists installed classes which can be loaded with IECore.ClassLoader.",
			IECore.Parameter(
				name = "result",
				description = "A list of classes.",
				defaultValue = IECore.StringVectorData()
			)
		)

		self.parameters().addParameters(
			[
				IECore.StringParameter(
					name = "type",
					description = "The type of class to list.",
					defaultValue = "op",
					presets = (
						( "Op", "op" ),
						( "Other", "other" ),
					),
					presetsOnly = True,
				),
				IECore.StringParameter(
					name = "match",
					description = "A glob style match string used to list only a subset of classes.",
					defaultValue = "*",
				),
				IECore.StringParameter(
					name = "searchPath",
					description = "When type is set to \"other\", this specifies a colon separated list of paths to search for classes on.",
					defaultValue = "",
				),
				IECore.StringParameter(
					name = "searchPathEnvVar",
					description = 	"When type is set to \"other\", this specifies an environment variable "
									"specifying a list of paths to search for classes on.",
					defaultValue = "",
				),
				IECore.StringParameter(
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

		t = operands["type"].value
		if t=="op" :
			loader = IECore.ClassLoader.defaultOpLoader()
		else :
			if operands["searchPath"].value and operands["searchPathEnvVar"].value :
				raise RuntimeError( "Cannot specify both searchPath and searchPathEnvVar." )
			if not operands["searchPath"].value and not operands["searchPathEnvVar"].value :
				raise RuntimeError( "Must specify either searchPath or searchPathEnvVar." )

			if operands["searchPath"].value :
				sp = IECore.SearchPath( operands["searchPath"].value, os.pathsep )
			else :
				sp = IECore.SearchPath( os.path.expandvars( os.environ[operands["searchPathEnvVar"].value] ), os.pathsep )

			loader = IECore.ClassLoader( sp )

		classes = loader.classNames( operands["match"].value )

		if operands["resultType"].value == "string" :
			return IECore.StringData( "\n".join( classes ) )
		else :
			return IECore.StringVectorData( classes )

IECore.registerRunTimeTyped( ClassLsOp )
