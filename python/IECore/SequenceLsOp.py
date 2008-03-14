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

		self.userData()["UI"] = CompoundObject(
			{
				"showResult": BoolData( True ),
			}
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
					description = "The type of the result returned.",
					defaultValue = "string",
					presets = {
						"string" : "string",
						"stringVector" : "stringVector",
					},
					presetsOnly = True,
				),
				StringParameter(
					name = "format",
					description = "The format of the result. This can be used to return strings suitable for viewing a sequence in a particular package.",
					defaultValue = "<PREFIX><#PADDING><SUFFIX> <FRAMES>",
					presets = {
						"shake" : "shake -t <FRAMES> <PREFIX>#<SUFFIX>",
						"nuke" : "nuke -v <PREFIX>%0<PADDINGSIZE>d<SUFFIX> <FIRST>,<LAST>",
						"fcheck" : "fcheck -n <FIRST> <LAST> <STEP> <PREFIX><@PADDING><SUFFIX>",
						"frameCycler" : "framecycler <PREFIX><#PADDING><SUFFIX>",
					}
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
			for sequence in sequences :
				root, ext = os.path.splitext( sequence.fileName )
				if ext in extensions :
					filteredSequences.append( sequence )
		
			sequences = filteredSequences
		
		# reformat the sequences into strings as requested
		
		for i in range( 0, len( sequences ) ) :
			
			s = operands.format.value
			s = s.replace( "<PREFIX>", sequences[i].getPrefix() )
			
			pi = s.find( "PADDING>" )
			if pi > 1 and s[pi-2]=='<' :
				s = s[:pi-2] + "".ljust( sequences[i].getPadding(), s[pi-1] ) + s[pi+8:]
			
			s = s.replace( "<PADDINGSIZE>", str( sequences[i].getPadding() ) )
			s = s.replace( "<SUFFIX>", sequences[i].getSuffix() )
			s = s.replace( "<FRAMES>", str( sequences[i].frameList ) )
			
			frames = sequences[i].frameList.asList()
			frames.sort()
			s = s.replace( "<FIRST>", str( frames[0] ) )
			s = s.replace( "<LAST>", str( frames[-1] ) )
			if s.find( "<STEP>" )!=-1 :
				stepCounts = {}
				for j in range( 1, len( frames ) ) :
					step = frames[j] - frames[j-1]
					stepCounts[step] = stepCounts.setdefault( step, 0 ) + 1
				m = 0
				step = 1
				for k, v in stepCounts.items() :
					if v > m :
						step = k
						m = v
				s = s.replace( "<STEP>", str( step ) )
			
			sequences[i] = s
			
		# return the result as the requested type					
		if operands.resultType.value == "string" :
			return StringData( "\n".join( sequences ) )
		else :
			return StringVectorData( sequences )

makeRunTimeTyped( SequenceLsOp, 100006, Op )
