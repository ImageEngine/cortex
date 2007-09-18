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

class SequenceLsOp( Op ) :

	def __init__( self ) :
	
		Op.__init__( self, "SequenceLsOp", "Lists file sequences.",
			Parameter(
				name = "result",
				description = "A list of matching sequences.",
				defaultValue = StringVectorData()
			)
		)
		
		self.parameters().addParameters(
			[
				DirNameParameter(
					name = "dir",
					description = "The directory to look for sequences in.",
					defaultValue = "./",
					check = DirNameParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				BoolParameter(
					name = "recurse",
					description = "When on, recursively searches all subdirectories for sequences.",
					defaultValue = False,
				),
				IntParameter(
					name = "maxDepth",
					description = "The maximum depth to recursion - this can be used to prevent accidental traversing of huge hierarchies.",
					defaultValue = 1000,
					minValue = 1,
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
				),
				StringVectorParameter(
					name = "extensions",
					description = "A list of file extensions which the sequences must have if they are to be listed. An empty list"
						"means that any sequence will be listed. The . character should be omitted from the extension.",
					defaultValue = StringVectorData(),
					presets = {
						"images" : StringVectorData( [ "tif", "tiff", "jpg", "jpeg", "exr", "cin", "dpx", "ppm", "png", "gif", "iff" ] )
					}
				)
			]
		)

	def doOperation( self, operands ) :
	
		# recursively find sequences
		baseDirectory = operands.dir.value
		if baseDirectory[-1] == '/' :
			baseDirectory = baseDirectory[:-1]
	
		sequences = ls( baseDirectory )
		if operands.recurse.value :
			for root, dirs, files in os.walk( baseDirectory, True ) :
				
				relRoot = root[len(baseDirectory)+1:]
				if relRoot!="" :
					depth = len( relRoot.split( "/" ) )
				else :
					depth = 0
				
				if depth>=operands.maxDepth.value :
					dirs[:] = []
				
				for d in dirs :
					ss = ls( os.path.join( root, d ) )
					if ss :
						for s in ss :
							s.fileName = os.path.join( relRoot, d, s.fileName )
							sequences.append( s )
							
		# filter sequences based on extension
		if operands.extensions.size() :
		
			filteredSequences = []
			extensions = set( ["." + e for e in operands.extensions] )
			print extensions	
			for sequence in sequences :
				root, ext = os.path.splitext( sequence.fileName )
				if ext in extensions :
					filteredSequences.append( sequence )
		
			sequences = filteredSequences
			
		# return the result in the requested format					
		if operands.resultType.value == "string" :
			return StringData( "\n".join( [str(s) for s in sequences] ) )
		else :
			return StringVectorData( [str(s) for s in sequences] )

makeRunTimeTyped( SequenceLsOp, 100006, Op )
