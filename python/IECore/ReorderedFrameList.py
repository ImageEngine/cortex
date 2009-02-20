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

from FrameList import FrameList
from EmptyFrameList import EmptyFrameList
import re

## The ReorderedFrameList is a base class for any FrameList classes which
# hold a child FrameList and return some reordering of that list in the
# asList() method. The child is held as an attribute
# called frameList, and can be modified directly.
# \ingroup python
class ReorderedFrameList( FrameList ) :

	def __init__( self, frameList ) :
	
		self.frameList = frameList

	## Implemented to protect the frameList attribute from being assigned
	# invalid values.
	def __setattr__( self, key, value ) :
	
		if key=="frameList" :
		
			if not isinstance( value, FrameList ) :
			
				raise TypeError( "Reordered.frameList must be a FrameList instance" )
			
		self.__dict__[key] = value	

	## Returns the frame list in a human readable form. Must be implemented in all
	# subclasses.
	def __str__( self ) :
		
		s = str( self.frameList )
		if ',' in s :
			return "(%s)%s" % ( s, self.suffix() )
		else :
			return s + self.suffix()
		
	## Each derived class should override this method		
	def __repr__( self ) :
	
		raise NotImplementedError			
	
	## Each derived class should override this method to define a suffix used in the
	# string representation of the frame list.
	@classmethod
	def suffix( self ) :
	
		raise NotImplementedError

	## This utility can be used by the parse functions in derived classes.
	# It matches strings like "(...)s" or "...s", where s is the suffix. It
	# then returns a FrameList parsed from the "..." section.
	@classmethod
	def parseForChildList( cls, s ) :
	
		if not s.endswith( cls.suffix() ) :
			return None
	
		s = s[:-len(cls.suffix())]
		
		if (s[0]=='(' and s[-1]==')') or not ',' in s :
			return FrameList.parse( s )
		
		return None
