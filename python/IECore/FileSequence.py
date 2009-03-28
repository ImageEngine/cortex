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

import re
import _IECore as IECore

## The FileSequence class represents a sequence of files on disk. It provides
# methods for manipulating the names of such files so that renumbering and renaming
# become easy operations. Note that the FileSequence class never modifies or moves
# actual files - it merely provides a means of generating the names required to do
# so in a meaningful way.
# \ingroup python
# \todo file sequences like 0, 100,101,102,...400 are represented as: 0-100x100,101-400. Should be 0,100-400
class FileSequence :

	## Constructs a new FileSequence object with the specified fileName, padding and frameList.
	# fileName must be a string containing a single sequence of at least # character - this
	# represents the location and padding of the frame numbers. frameList must be a FrameList instance.
	# These arguments are subsequently accessible and modifiable as instance attributes of the same name.
	def __init__( self, fileName, frameList ) :
	
		self.fileName = fileName
		self.frameList = frameList
		
	def __setattr__( self, key, value ) :
	
		if key=="fileName" :
		
			if not type( value ) is str :
			
				raise TypeError( "FileSequence fileName must be a string" )
				
			if not FileSequence.fileNameValidator().match( value ) :
			
				raise ValueError( "FileSequence fileName must contain a single sequence of at least one # to denote frame number." )
				
		elif key=="frameList" :
		
			if not isinstance( value, IECore.FrameList ) :
			
				raise ValueError( "FileSequence frameList must be an instance of FrameList." )
		
		self.__dict__[key] = value
	
	## Returns a useful human readable string.
	def __str__( self ) :
	
		return "%s %s" % ( self.fileName, str( self.frameList ) )
		
	def __repr__( self ) :
		
		return "IECore.FileSequence( %s, %s )" % ( repr( self.fileName ), repr( self.frameList ) )
		
	## Returns true if other is a FileSequence with the same fileName and FrameList
	def __eq__( self, other ) :
	
		if not isinstance( other, FileSequence ) :
			return False
			
		return self.fileName==other.fileName and self.frameList==other.frameList
	
	## Returns -1, 0, or 1 depending on whether this file sequence is less than, equal to, or greater than the other. The fileName is considered
	# more important than the frameList when determining ordering.
	def __cmp__( self, other ) :

		c = cmp( self.fileName, other.fileName )		
		if c != 0 : return c
		else :return cmp( self.frameList, other.frameList )	
	
	## Returns the frame number padding - this is calculated by looking at the
	# number of # characters in self.fileName."""	
	def getPadding( self ) :
		
		return self.fileName.count( "#" )
	
	## Sets the frame number padding - this is achieved by modifying the number of # characters
	# in self.fileName."""	
	def setPadding( self, padding ) :

		oldPadding = "".ljust( self.getPadding(), "#" )
		newPadding = "".ljust( padding, "#" )
		
		self.fileName = self.fileName.replace( oldPadding, newPadding )
		
	## Returns the part of self.fileName before the # sequence representing the
	# frame number.
	def getPrefix( self ) :
	
		return self.fileName[:self.fileName.index("#")]
	
	## Sets the part of self.fileName before the # sequence to the string specified
	# by prefix.
	def setPrefix( self, prefix ) :
	
		self.fileName = prefix + self.fileName[self.fileName.index("#"):]
	
	## Returns the part of self.fileName following the # sequence representing the
	# frame number.
	def getSuffix( self ) :
		
		return self.fileName[self.fileName.rindex("#")+1:]
	
	## Sets the part of self.fileName following the # sequence to the string specified
	# by suffix.
	def setSuffix( self, suffix ) :
		
		self.fileName = self.fileName[:self.fileName.rindex("#")+1] + suffix
	
	## Returns a filename for the sequence represented by this object, with the frame number
	# specified by the frameNumber parameter, which must be an integer
	def fileNameForFrame( self, frameNumber ) :
	
		fileNameTemplate = self.__fileNameTemplate()
		return fileNameTemplate % frameNumber
		
	## Returns the frame number of a filename from this sequence.
	def frameForFileName( self, fileName ) :
	
		prefix = self.getPrefix()
		suffix = self.getSuffix()
		
		if not fileName.startswith( prefix ) or not fileName.endswith( suffix ) :
		
			raise RuntimeError( "Filename \"%s\" is not a part of sequence \"%s\".", ( fileName, str( self ) ) )
			
		frame = fileName[len(prefix):-len(suffix)]
		return int( frame )

	## Returns a list of all filenames represented by this object, ordered according to
	# the ordering of frames in self.frameList.
	def fileNames( self ) :
	
		fileNameTemplate = self.__fileNameTemplate()
		
		result = []
		for frame in self.frameList.asList() :
		
			result.append( fileNameTemplate % frame )
			
		return result
			
	## Returns a list of lists of filenames represented by this object, with no more
	# than clumpSize filenames in each sublist.
	def clumpedFileNames( self, clumpSize ) :
		
		fileNameTemplate = self.__fileNameTemplate()
	
		result = []
		for clump in self.frameList.asClumpedList( clumpSize ) :
			c = []
			for frame in clump :
				c.append( fileNameTemplate % frame )
			result.append( c )
			
		return result
		
	# Returns a deep copy of this object.
	def copy( self ) :
	
		return FileSequence( str( self.fileName ), self.frameList.copy() )
	
	## Returns a mapping from the filenames represented by this object to
	# the filenames represented by another FileSequence object. Throws an exception if
	# the number of files represented by other is not the same as the number of files
	# represented by self. This can be used for generating a map to inform the renaming
	# of files from one form to another. If asList is True then a list of tuples of the
	# form (src, dst) is returned, otherwise a dictionary with source files as the keys
	# and destination files as values is returned. The former is useful in that it
	# preserves the order of the files.
	def mapTo( self, other, asList = False ) :
		
		selfNames = self.fileNames()
		otherNames = other.fileNames()
		
		if len( selfNames )!=len( otherNames ) :
			raise IndexError( "FileSequence.mapTo() : FileSequence objects contain different numbers of frames." )
		
		if asList :
			result = []
			for i in xrange( 0, len( selfNames ) ) :
				result.append( (selfNames[i], otherNames[i]) )
			return result
		else :
			result = {}
			for i in xrange( 0, len( selfNames ) ) :
				result[selfNames[i]] = otherNames[i]
			return result
			
	## Returns a regular expression that matches only valid filenames. Group 1, 2, and 3 of any
	# resulting matches are the prefix, padding and suffix of the matched filename.
	@staticmethod
	def fileNameValidator() :
		
		return re.compile( "^([^#]*)(#+)([^#]*)$" )			
	
	# Returns a string with appropriate format specifier for expanding out the frame number with the
	# % operator.
	def __fileNameTemplate( self ) :
	
		padding = self.getPadding()
		paddingStr = "".ljust( padding, "#" )
		
		return self.fileName.replace( "%", "%%").replace( paddingStr, "%." + str(padding) + "d" )
