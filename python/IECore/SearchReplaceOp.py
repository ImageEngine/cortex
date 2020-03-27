##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

import shutil
import tempfile
import os
import re
import IECore

class SearchReplaceOp( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self, "Performs a search and replace on ASCII text files.",
			IECore.FileNameParameter(
				name = "result",
				description = "The resulting file. Maya be the same as the input file.",
				defaultValue = "",
				allowEmptyString = True,
			)
		)

		self.parameters().addParameters(
			[
				IECore.FileNameParameter(
					name = "source",
					description = "The source file.",
					defaultValue = "",
					extensions = "ma rib shk nk",
					check = IECore.FileNameParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				IECore.FileNameParameter(
					name = "destination",
					description = "The destination file.",
					defaultValue = "",
					allowEmptyString = False,
				),
				IECore.StringParameter(
					name = "searchFor",
					description = "The pattern to search for",
					defaultValue = "",
				),
				IECore.BoolParameter(
					name = "regexpSearch",
					description = "Enable to perform searching based on regular expressions",
					defaultValue = False
				),
				IECore.StringParameter(
					name = "replaceWith",
					description = "The string with which to replace patterns which match the search criteria",
					defaultValue = "",
				)
			]
		)

	def doOperation( self, operands ) :

		source = operands["source"].value
		destination = operands["destination"].value

		searchFor = operands["searchFor"].value
		if not operands["regexpSearch"] :
			searchFor = re.escape( searchFor )

		replaceWith = operands["replaceWith"].value

		inFileStat = os.stat( source ).st_mode

		inFile = open( source, "r" )

		tmpDestination = None
		if source == destination :
			fd, tmpDestination = tempfile.mkstemp()
			outFile = os.fdopen( fd, "w" )
		else :
			outFile = open( destination, "w" )

		inLine = inFile.readline()
		while inLine :

			outLine = re.sub( searchFor, replaceWith, inLine )
			outFile.write( outLine )

			inLine = inFile.readline()

		inFile.close()
		outFile.close()

		if tmpDestination :

			shutil.move( destination, destination + ".bak" )
			shutil.move( tmpDestination, destination )

		os.chmod( destination, inFileStat )

		return IECore.StringData( destination )

IECore.registerRunTimeTyped( SearchReplaceOp )
