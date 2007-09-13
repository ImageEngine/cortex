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

import FrameList
import re

## The FrameRange class simply stores an integer range consisting of a start, end and step.
# These are accessed simply as attributes on the FrameRange instance. Exceptions are thrown
# if the attributes are modified in such a way as to create an invalid range.
#
# \ingroup python
class FrameRange( FrameList.FrameList ) :

	## Constructs a new FrameRange object.
	def __init__( self, start, end, step = 1 ) :
	
		self.start = start
		self.end = end
		self.step = step
	
	## Implemented to protect start, end and step attributes from receiving invalid
	# values.
	def __setattr__( self, key, value ) :
	
		if key in ("start", "end", "step" ) :
		
			if not ( type( value ) is int or type( value ) is long )  :
			
				raise TypeError( "FrameRange elements must be integers" )
				
			if key=="start" :
			
				if "end" in self.__dict__ and value > self.end :
				
					raise ValueError( "FrameRange start must be less than or equal to end." )
			
			elif key=="end" :
			
				if value < self.start :
				
					raise ValueError( "FrameRange end must be greater than or equal to start." )
				
			else :
			
				if (self.end - self.start) % value != 0 :
				
					raise ValueError( "FrameRange step must divide perfectly into end-start" )
							
		self.__dict__[key] = value

	def __str__( self ) :
	
		if self.step!=1 :
			return "%d-%dx%d" % (self.start, self.end, self.step)
		elif self.start!=self.end :
			return "%d-%d" % (self.start, self.end)
		else :
			return "%d" % self.start

	## Returns a list containing all the frames from start to end inclusive,
	# ascending by step each time.
	def asList( self ) :

		return range( self.start, self.end+1, self.step )
		
	# Same as asList() but returns an xrange object instead.
	def asXRange( self ) :

		return xrange( self.start, self.end+1, self.step )

	@staticmethod
	def parse( s ) :
	
		# is it a single frame?
		try :
			i = int(s)
			return FrameRange( i, i )
		except :
			pass
		
		m = re.compile( "^(-?[0-9]+)(-)(-?[0-9]+)(x-?[0-9]+)?$" ).match( s )
		if m :
					
			start = int( m.group( 1 ) )
			end = int( m.group( 3 ) )
			step = 1
			if m.group( 4 ) :
				step = int( m.group( 4 )[1:] )
				
			return FrameRange( start, end, step )
			
		return None
	

FrameList.FrameList.registerParser( FrameRange.parse )
