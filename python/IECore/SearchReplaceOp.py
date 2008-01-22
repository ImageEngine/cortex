##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
from IECore import *

class SearchReplaceOp( Op ) :

	def __init__( self ) :
	
		Op.__init__( self, "SearchReplaceOp", "Performs a search and replace on ASCII text files.",
			FileNameParameter(
				name = "result",
				description = "The resulting file. Maya be the same as the input file.",
				defaultValue = "",
				allowEmptyString = True,
			)
		)
		
		self.parameters().addParameters(
			[
				FileNameParameter(
					name = "source",
					description = "The source file.",
					defaultValue = "",
					extensions = "ma rib shk nuk txt doc",
					check = FileNameParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				FileNameParameter(
					name = "destination",
					description = "The destination file.",
					defaultValue = "",
					allowEmptyString = False,
				),
				StringParameter(
					name = "searchFor",
					description = "The pattern to search for",
					defaultValue = "",
				),
				BoolParameter(
					name = "regexpSearch",
					description = "Enable to perform searching based on regular expressions",
					defaultValue = False
				),
				StringParameter(
					name = "replaceWith",
					description = "The string with which to replace patterns which match the search criteria",
					defaultValue = "",
				)
			]
		)

	def doOperation( self, operands ) :
	
		source = operands.source.value
		destination = operands.destination.value		
		
		searchFor = operands.searchFor.value		
		if not operands.regexpSearch :
			searchFor = re.escape( searchFor )
			
		replaceWith = operands.replaceWith.value	
		
		inFileStat = os.stat( source ).st_mode
		
		inFile = open( source, "r" )		
		
		if source == destination :
			
			inPlace = True
			fd = tempfile.mkstemp()
			
		else :
		
			inPlace = False
			fd = ( os.open( destination, os.O_WRONLY | os.O_TRUNC | os.O_CREAT ), destination )
				
		outFile = os.fdopen( fd[0], "w" )	
		
		
		inLine = inFile.readline()				
		while inLine :
			
			outLine = re.sub( searchFor, replaceWith, inLine )
			os.write( fd[0], outLine )
			
			inLine = inFile.readline()
								
		inFile.close()
		outFile.close()
		
		if inPlace :
									
			shutil.move( destination, destination + ".bak" )
			shutil.move( fd[1], destination )
										
		os.chmod( destination, inFileStat )
					
		return StringData( destination )

makeRunTimeTyped( SearchReplaceOp, 100015, Op )
