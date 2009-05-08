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

import re
import os
import re
import glob
import shutil
import os.path
import _IECore as IECore

# This is here because we can't yet create a to_python converter for boost::regex
IECore.FileSequence.fileNameValidator = staticmethod( lambda : re.compile( "^([^#]*)(#+)([^#]*)$" ) )

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

__all__ = [ "mv", "cp", "rm", "cat" ]
