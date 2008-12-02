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

# \ingroup python

import os
from IECore import *

# The SequenceMergeOp is a base class for Ops which perform merging of two file sequences into a single file sequence.
class SequenceMergeOp( Op ) :

	def __init__( self, name, description, extensions = [] ) :
	
		assert( type( extensions ) is list )
	
		Op.__init__(
			self, 
			name, 
			description, 
			StringVectorParameter( 
				name = "result",
				description = "The names of the files created",
				defaultValue = StringVectorData([])
			)
		)
				
		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "fileSequence1",
					description = "The first input sequence",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					extensions = extensions,
				),
				FileSequenceParameter(
					name = "fileSequence2",
					description = "The second input sequence",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					extensions = extensions,
				),
				FileSequenceParameter(
					name = "outputFileSequence",
					description = "The output file sequence to generate. For each frame in this sequence, the corresponding inputs for that frame are merged",
					defaultValue = "",					
					allowEmptyString = False,
					check = FileSequenceParameter.CheckType.MustNotExist,
				),
			]
		)
	
	# Derived classes should override this method, and merge the files given in "fileName1" and "fileName2" into "outputFileName",
	# returning True on success or False on failure.
	def _merge( self, fileName1, fileName2, outputFileName ) :
	
		pass	
		
	def doOperation( self, args ) :
	
		fileSequence1 = self.parameters()['fileSequence1'].getFileSequenceValue()
		fileSequence2 = self.parameters()['fileSequence2'].getFileSequenceValue()
		outputFileSequence = self.parameters()['outputFileSequence'].getFileSequenceValue()
		
		resultFiles = []
				
		for frame in outputFileSequence.frameList.asList() :
		
			fileName1 = fileSequence1.fileNameForFrame( frame )
			fileName2 = fileSequence2.fileNameForFrame( frame )
			outputFileName = outputFileSequence.fileNameForFrame( frame )
		
			if self._merge( fileName1, fileName2, outputFileName ) :
			
				resultFiles.append( outputFileName )
			
		return StringVectorData( resultFiles )	

makeRunTimeTyped( SequenceMergeOp, 100024, Op )
