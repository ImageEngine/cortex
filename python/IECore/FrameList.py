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

## The FrameList class defines an abstract base class for classes which are
# able to generate a list of frame numbers.
#
# \ingroup python
class FrameList :

	## Returns the frame list in a human readable form. Must be implemented in all
	# subclasses.
	def __str__( self ) :
	
		raise NotImplementedError

	## This method returns a simple list of frames. They are not guaranteed to
	# be in ascending order, but they are guaranteed to be non repeating - make
	# sure you honour these requirements when creating subclasses.
	def asList( self ) :
	
		raise NotImplementedError

	## This method takes the list of frames returned by asList() and returns a list
	# of lists of frames, where each sublist contains no more than clumpSize frames.
	def asClumpedList( self, clumpSize ) :
	
		frames = self.asList()
		result = []
		while len( frames ) :

			thisClumpSize = min( clumpSize, len( frames ) )
			result.append( frames[:clumpSize] )
			frames = frames[clumpSize:]

		return result

	## Parses a string and returns the FrameList object that it represents.
	# Strings may be in any of the forms returned by str( SomeFrameListSubclass ).
	# Subclasses must register a suitable parser for the form that they return
	# using the registerParser method. Raises RuntimeError if the string is in an unrecognised form.
	@staticmethod
	def parse( s ) :
		
		for p in FrameList.__parsers :
		
			f = p( s )
			if f :
				return f
				
		raise RuntimeError( "\"%s\" does not define a valid frame list." % s )

	## Registers a parser for turning a string into a FrameList object of some sort.
	# Each subclass must register a parser that handles the form returns by that class'
	# __str__ method. When parsing, each parser is called in turn until one returns a FrameList -
	# a parsing function should therefore return None if it doesn't recognise the form
	# of the string it is passed.
	@staticmethod
	def registerParser( parser ) :
		
		FrameList.__parsers.append( parser )

	__parsers = []
