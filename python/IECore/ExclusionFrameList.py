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

from FrameList import FrameList

## The ExclusionFrameList class implements a FrameList which will return
# all the frames in one list which are not in another. This allows
# strings such as 1-100!1-100x10 to specify all the inbetween frames
# to fill in a previous partial render.
# \ingroup python
class ExclusionFrameList( FrameList ) :

	def __init__( self, frameList, exclusionFrameList ) :
		
		self.frameList = frameList
		self.exclusionFrameList = exclusionFrameList

	## Implemented to protect the frameList and exclusionFrameList
	# attributes from the assignment of invalid values
	def __setattr__( self, key, value ) :
	
		if key=="frameList" or key=="exclusionFrameList" :
		
			if not isinstance( value, FrameList ) :
			
				raise TypeError( "frameList and exclusionFrameList must be of type FrameList." )

		self.__dict__[key] = value

	def __str__( self ) :
	
		s1 = str( self.frameList )
		s2 = str( self.exclusionFrameList )
		if "," in s1 :
			s1 = "(" + s1 + ")"
		if "," in s2 :
			s2 = "(" + s2 + ")"

		return s1 + "!" + s2
		
	def asList( self ) :
	
		l = self.frameList.asList()
		e = set( self.exclusionFrameList.asList() )
				
		r = []
		for f in l :
			if not f in e :
				r.append( f )
				
		return r
	
	@staticmethod
	def parse( s ) :
	
		s = s.split( "!" )
		if len( s ) != 2 :
			return None
			
		try :
			f1 = FrameList.parse( s[0] )
			f2 = FrameList.parse( s[1] )
			return ExclusionFrameList( f1, f2 )
		except :
			return None
			
		return None
	

FrameList.registerParser( ExclusionFrameList.parse )

