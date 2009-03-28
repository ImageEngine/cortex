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

import _IECore as IECore
from FileExaminer import FileExaminer
import re

## The NukeFileExaminer class implements the FileExaminer interface for
# nuke script files.
class NukeFileExaminer( FileExaminer ) :

	def __init__( self, fileName ) :
	
		FileExaminer.__init__( self, fileName )
		
	def dependencies( self ) :

		## I'd rather this was implemented as some sort of batch script
		# in nuke so it wasn't so text munging based, but it seems that
		# would mean taking a nuke gui license, which seems a bit excessive.
		# Hopefully this will do for now - it's really susceptible to changes
		# in the formatting of scripts.

		lines = []
		f = open( self.getFileName() )
		try :
			lines = f.readlines()
		finally :
			f.close()
		
		# find the frame range for the script
		rootNodes = self.__findNodes( "Root", lines )
		if len( rootNodes )==1 :
			scriptFirstFrame = int( self.__knobValue( "first_frame", rootNodes[0], "1" ) )
			scriptLastFrame = int( self.__knobValue( "last_frame", rootNodes[0], "100" ) )
		elif len( rootNodes )==0 :
			raise Exception( "No Root node found." )
		else :
			raise Exception( "More than one root node." )
		
		# find all the file references
		##################################################
		
		result = set()
		
		# first find read nodes
		readNodes = self.__findNodes( "Read", lines )
		for readNode in readNodes :
			
			fileName = self.__knobValue( "file", readNode, "" )		
			proxyFileName = self.__knobValue( "proxy", readNode, "" )	
			firstFrame = int( self.__knobValue( "first", readNode, "1" ) )
			lastFrame = int( self.__knobValue( "last", readNode, "1" ) )
			firstFrame = max( scriptFirstFrame, firstFrame )
			lastFrame = min( scriptLastFrame, lastFrame )
			
			if fileName!="" :
				result.add( self.__convertFileName( fileName, firstFrame, lastFrame ) )
			if proxyFileName!="" :
				result.add( self.__convertFileName( proxyFileName, firstFrame, lastFrame ) )
				
		# now find read geo nodes
		readGeoNodes = self.__findNodes( "ReadGeo", lines )
		for readGeoNode in readGeoNodes :
			
			fileName = self.__knobValue( "file", readGeoNode, "" )		
			if fileName!="" :
				result.add( self.__convertFileName( fileName, scriptFirstFrame, scriptLastFrame ) )
			
		# now find grain nodes
		grainNodes = self.__findNodes( "ScannedGrain", lines )
		for grainNode in grainNodes :
		
			fileName = self.__knobValue( "fullGrain", grainNode, "" )
			if fileName!="" :
				firstFrame = int( self.__knobValue( "fullGrain.first_frame", grainNode, "1" ) )
				lastFrame = int( self.__knobValue( "fullGrain.last_frame", grainNode, "50" ) )
				result.add( self.__convertFileName( fileName, firstFrame, lastFrame ) )
							
		return result
	
	def __findNodes( self, nodeType, lines ) :
	
		result = []
		startIndex = 0
		while startIndex < len( lines ) :
			r = self.__findNode( nodeType, lines, startIndex )
			if r :
				result.append( r[0] )
				startIndex = r[1]
			else :
				break
				
		return result
		
	def __findNode( self, nodeType, lines, startIndex ) :
	
		for i in range( startIndex, len( lines ) ) :
			
			words = lines[i].split()
			
			if len( words ) == 2 and words[0]==nodeType and words[1]=="{" :			
		
				node = []
				for j in range( i, len( lines ) ) :
				
					node.append( lines[j] )
					if lines[j].strip() == "}" :
						return node, j + 1
		
		return None
	
	def __knobValue( self, knobName, node, default ) :
	
		for line in node[1:-1] :
						
			words = line.split()
			if len( words )==2 and words[0]==knobName :
				return words[1]
				
		return default
	
	def __convertFileName( self, fileName, firstFrame, lastFrame ) :
	
		m = re.compile( "^(.*)%([0-9]*)d(.*)$" ).match( fileName )
		if m :
					
			padding = 1
			padder = m.group( 2 )
			if len( padder ) :
			
				if ' ' in fileName :
					raise Exception( "Sequence fileName \%s\" contains a space." % fileName )
			
				if padder[0]=="0" :
					padding = int( padder[1:] )
				else :
					# if the padding doesn't begin with 0 then
					# nuke seems to pad with spaces. we won't accept
					# spaces in a filename
					raise Exception( "Filename \"%s\" is padded with spaces." % fileName )
				
			return m.group( 1 ) + "".ljust( padding, '#' ) + m.group( 3 ) + " " + str( IECore.FrameRange( firstFrame, lastFrame ) )
		
		else :
		
			return fileName
				
FileExaminer.registerExaminer( [ "nk" ], NukeFileExaminer )
