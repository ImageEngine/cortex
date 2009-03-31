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

## \file FileSequenceFunctions.py
# Defines functions useful for creating and manipulating FileSequence objects.
#
# \ingroup python

import os
import re
import glob
import shutil
import os.path
from FileSequence import FileSequence
import _IECore as IECore

## Returns a list of FileSequence objects representing all the sequences in names.
# names is just a list of arbitrary strings, which may or may not represent files
# on disk.
def findSequences( names ) :

	# this matches names of the form $prefix$frameNumber$suffix
	# placing each of those in a group of the resulting match.
	# both $prefix and $suffix may be the empty string and $frameNumber
	# may be preceded by a minus sign.
	matchExpression = re.compile( "^([^#]*?)(-{,1}[0-9]+)([^0-9#]*)$" )
	# build a mapping from ($prefix, $suffix) to a list of $frameNumbers
	sequenceMap = {}
	for f in names :
	
		match = matchExpression.match( f )
		if match :
		
			fixes = match.group( 1 ), match.group( 3 )
			frames = sequenceMap.setdefault( fixes, [] )
			frames.append( match.group( 2 ) )
	
	# build a list of FileSequence objects to return
	result = []	
	for fixes, frames in sequenceMap.items() :
	
		# in diabolical cases the elements of frames may not all have the same padding
		# so we'll sort them out into padded and unpadded frame sequences here, by creating
		# a map of padding->list of frames. unpadded things will be considered to have a padding
		# of 1.
		frames.sort()
		paddingToFrames = {}
		for frame in frames :
			sign = 1
			if frame[0]=="-" :
				frame = frame[1:]
				sign = -1
			if frame[0]=="0" or len(frame) in paddingToFrames :
				numericFrames = paddingToFrames.setdefault( len(frame), [] )
			else :
				numericFrames = paddingToFrames.setdefault( 1, [] )
			numericFrames.append( sign * int(frame) )
		
		for padding, numericFrames in paddingToFrames.items() :
			
			numericFrames.sort()	
			frameList = frameListFromList( numericFrames )
			result.append( FileSequence( fixes[0] + "".ljust( padding, "#" ) + fixes[1], frameList ) )
	
	# remove any sequences with less than two files
	result = [ x for x in result if len( x.frameList.asList() ) > 1 ]
		
	return result

## If path is a directory then returns all sequences
# residing in that directory in the form of a list of FileSequences, returning
# the empty list if none are found. If path is a sequence style filename with a
# single sequence of at least one embedded # then tries to find a sequence
# matching that specification, returning None if one isn't found.
def ls( path ) :
	
	r = FileSequence.fileNameValidator()
	if r.match( path ) :
	
		padding = len( r.match( path ).group( 2 ) )
		globPath = re.sub( "#+", "*", path )
				
		files = glob.glob( globPath )
				
		sequences = findSequences( files )
		for sequence in sequences :
			if sequence.getPadding()==padding :
				return sequence
				
		return None
				
	elif os.path.isdir( path ) :
		
		allFiles = os.listdir( path )
		return findSequences( allFiles )			

## Moves the set of files specified by sequence1 to the set of files
# specified by sequence2, where sequence1 and sequence2 are
# FileSequence objects of equal length. This function is safe even if the
# files specified by each sequence overlap.				
def mv( sequence1, sequence2 ) :

	if __sequencesClash( sequence1, sequence2 ) :
		sTmp = sequence1.copy()
		sTmp.setPrefix( os.path.join( os.path.dirname( sTmp.getPrefix() ), __tmpPrefix() ) )
		for src, dst in sequence1.mapTo( sTmp, True ) :
			shutil.move( src, dst )
		for src, dst in sTmp.mapTo( sequence2, True ) :
			shutil.move( src, dst )
	else :
		for src, dst in sequence1.mapTo( sequence2, True ) :
			shutil.move( src, dst )

## Copies the set of files specified by sequence1 to the set of files
# specified by sequence2, where sequence1 and sequence2 are
# FileSequence objects of equal length. This function is safe even if the
# files specified by each sequence overlap.	
def cp( sequence1, sequence2 ) :
	
	if __sequencesClash( sequence1, sequence2 ) :
		raise RuntimeError( "Attempt to copy sequences with common filenames." )
		
	for src, dst in sequence1.mapTo( sequence2, True ) :
		shutil.copy( src, dst )

## Removes all the files specified by the sequence.
def rm( sequence ) :
	
	for f in sequence.fileNames() :
	
		os.remove( f )

## Concetenates all the files specified by the sequence to stdout
# \todo Allow destination file to be specified
def cat( sequence ) :	

	for f in sequence.fileNames() :
	
		ret = os.system( 'cat "%s"' % ( f ) )	
		
		

## Returns a FrameList instance that "best" represents the specified list of integer
# frame numbers. This function attempts to be intelligent and uses a CompoundFrameList
# of FrameRange objects to represent the specified frames compactly.
def frameListFromList( frames ) :

	if len( frames )==0 :
		return IECore.EmptyFrameList()
		
	if len( frames )==1 :
		return IECore.FrameRange( frames[0], frames[0] )
		
	frameLists = []
	
	rangeStart = 0
	rangeEnd = 1
	rangeStep = frames[rangeEnd] - frames[rangeStart]
	assert( rangeStep > 0 )
	
	while rangeEnd<=len( frames ) :
		
		if rangeEnd==len( frames ) or frames[rangeEnd] - frames[rangeEnd-1] != rangeStep :
			# we've come to the end of a run
			if rangeEnd-1==rangeStart :
				frameLists.append( IECore.FrameRange( frames[rangeStart], frames[rangeStart] ) )
			else :
				frameLists.append( IECore.FrameRange( frames[rangeStart], frames[rangeEnd-1], rangeStep ) )
			rangeStart = rangeEnd
			rangeEnd = rangeStart + 1
				
			if rangeEnd<len( frames ) :
				rangeStep = frames[rangeEnd] - frames[rangeStart]
		else :
			# continue the current run
			rangeEnd += 1

	if len( frameLists )==1 :
		return frameLists[0]
	else :
		return IECore.CompoundFrameList( frameLists )

# private utility functions

def __sequencesClash( sequence1, sequence2 ) :

	s = set()
	for f in sequence1.fileNames() :
		s.add( f )
		
	for f in sequence2.fileNames() :
		if f in s :
			return True

	return False
	
def __tmpPrefix() :

	"""Returns a hopefully unique string suitable for use as the temporary
	sequence prefix when moving a sequence."""
	
	import hashlib
	import platform
	import time
	
	h = hashlib.md5()
	h.update( platform.node() ) # computer name
	h.update( str( os.getpid() ) )
	h.update( str( time.time() ) )
	return "ieSequenceTmp" + h.hexdigest() + "."

__all__ = [ "findSequences", "ls", "mv", "cp", "rm", "cat", "frameListFromList" ]
