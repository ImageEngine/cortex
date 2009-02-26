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
import datetime

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
				BoolParameter(
					name = "followLinks",
					description = "When on, follow symbolic links during directory traversal.",
					defaultValue = False,
				),
				IntParameter(
					name = "maxDepth",
					description = "The maximum depth to recursion - this can be used to prevent accidental traversing of huge hierarchies.",
					defaultValue = 1000,
					minValue = 1,
				),
				StringParameter( 
					name = "type",
					description = "The file types of the sequences to classify.",
					defaultValue = "any",
					presets = {
						"files" : "files",
						"directories" : "directories",
						"any" : "any"
					},
					presetsOnly = True,
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
				BoolParameter(
					name = "contiguousSequencesOnly",
					description = "When on, only sequences without missing frames are returned.",
					defaultValue = False,
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
				),								
				CompoundParameter(
					name = "advanced",
					description = "Advanced paramaters for filtering results based on various criteria",
					members = [
						
						CompoundParameter(
							name = "modificationTime",
							description = "Controls for filtering results based on modification time",
							
							members = [
								BoolParameter(
									name = "enabled",
									description = "Enable filtering based on modification time",
									defaultValue = False
								),
								StringParameter(
									name = "mode",
									description = "Changes the mode of modified time operation, e.g. before or after",
									defaultValue = "before",
									presets = {
										"Before Start" : "before",
										"After End" : "after",
										"Between Start/End" : "between",
										"Outside Start/End" : "outside"										
									},
									presetsOnly = True
								),
								DateTimeParameter(
									name = "startTime",
									description = "The start time at which to make modification time comparisons against",
									defaultValue = datetime.datetime.now()
								),
								DateTimeParameter(
									name = "endTime",
									description = "The end time at which to make modification time comparisons against",
									defaultValue = datetime.datetime.now()
								),
									
							]
						)
					]
				)
			]
		)

	@staticmethod		
	def __walk(top, topdown=True, followlinks=False):

		from os.path import join, isdir, islink
		from os import listdir, error

		try:
			names = listdir(top)
		except error :
		        return

		dirs, nondirs = [], []
		for name in names:
			if isdir(join(top, name)):
				dirs.append(name)
			else:
				nondirs.append(name)
	
		if topdown:
			yield top, dirs, nondirs
			
		for name in dirs:
			path = join(top, name)
			if followlinks or not islink(path):
				for x in SequenceLsOp.__walk(path, topdown, followlinks):
					yield x
					
		if not topdown:
			yield top, dirs, nondirs
	

	def doOperation( self, operands ) :
	
		# recursively find sequences
		baseDirectory = operands.dir.value
		if baseDirectory[-1] == '/' :
			baseDirectory = baseDirectory[:-1]
	
		sequences = ls( baseDirectory )
		
		# If we've passed in a directory which isn't the current one it is convenient to get that included in the returned sequence names
		relDir = os.path.normpath( baseDirectory ) != "."
				
		if relDir :
			for s in sequences :
				s.fileName = os.path.join( baseDirectory, s.fileName )
					
		if operands.recurse.value :
			# \todo Can safely use os.walk here after Python 2.6, which introduced the followlinks parameter
			for root, dirs, files in SequenceLsOp.__walk( baseDirectory, topdown = True, followlinks = operands.followLinks.value ) :
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
						
							if relDir :
								s.fileName = os.path.join( baseDirectory, relRoot, d, s.fileName )
							else :	
								s.fileName = os.path.join( relRoot, d, s.fileName )
							sequences.append( s )
		
		
		# \todo This Op would benefit considerably from dynamic parameters
		# NB. Ordering of filters could have considerable impact on execution time. The most expensive filters should be specified last.
		filters = []
		
		# filter sequences based on type
		if operands.type.value != "any" :
		
			if operands.type.value == "files" :
				fileTypeTest = os.path.isfile
			else :	
				assert( operands.type.value == "directories" )
				fileTypeTest = os.path.isdir
					
			def matchType( sequence ) :						
				for sequenceFile in sequence.fileNames() :				
					if not fileTypeTest( sequenceFile ) :						
						return False
						
				return True			
				
			filters.append( matchType )
									
		# filter sequences based on extension
		if operands.extensions.size() :
							
			extensions = set( ["." + e for e in operands.extensions] )
			
			def matchExt( sequence ) :			
				return os.path.splitext( sequence.fileName )[1] in extensions
				
			filters.append( matchExt )

		# filter sequences which aren't contiguous			
		if operands.contiguousSequencesOnly.value :
		
			def isContiguous( sequence ):
			
				return len( sequence.frameList.asList() ) == max( sequence.frameList.asList() ) - min( sequence.frameList.asList() ) + 1
				
			filters.append( isContiguous )		
			
		# advanced filters
		if operands.advanced.modificationTime.enabled.value :
			
			filteredSequences = []
			
			mode = operands.advanced.modificationTime.mode.value
			startTime = operands.advanced.modificationTime.startTime.value
			endTime = operands.advanced.modificationTime.endTime.value

			matchFn = None
			if mode == "before" :												
				matchFn = lambda x : x < startTime	
					
			elif mode == "after" :				
				matchFn = lambda x : x > startTime
				
			elif mode == "between" :			
				matchFn = lambda x : x > startTime and x < endTime				
				
			else :
				assert( mode == "outside" )				
				matchFn = lambda x : x < startTime or x > endTime
				
			assert( matchFn )	
				
			def matchModifcationTime( sequence ) :	
			
				# If any file in the sequence matches, we have a match.
				for sequenceFile in sequence.fileNames() :

					st = os.stat( sequenceFile )
					modifiedTime = datetime.datetime.utcfromtimestamp( st.st_mtime )
					if matchFn( modifiedTime ) :
						return True
						
				return False	
				
			filters.append( matchModifcationTime )		
			
		def matchAllFilters( sequence ) :
				
			for f in filters :
				if not f( sequence ) : return False
				
			return True
			
		def matchAnyFilter( sequence ) :
				
			for f in filters :
				if f( sequence ) : return True
				
			return False	
			
		# \todo Allow matching of any filter, optionally	
		sequences = filter( matchAllFilters, sequences )

		# reformat the sequences into strings as requested
		
		for i in xrange( 0, len( sequences ) ) :
			
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
				for j in xrange( 1, len( frames ) ) :
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
