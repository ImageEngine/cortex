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

from FrameList import FrameList
from ReorderedFrameList import ReorderedFrameList
from EmptyFrameList import EmptyFrameList

## The BinaryFrameList class is a ReorderedFrameList which does a sort
# of binary refinement thing on the child frame list. This is useful
# when rendering a sequence of images, as you get a slow refinement of
# the whole sequence, providing earlier information about stuff going on
# in the middle and end of the sequence.
# \ingroup python
n = 0
class BinaryFrameList( ReorderedFrameList ) :

	def __init__( self, frameList = EmptyFrameList() ) :
	
		ReorderedFrameList.__init__( self, frameList )

	## Returns self.frameList.asList() in a sort of binary refined way.
	def asList( self ) :
	
		l = self.frameList.asList()
		if len( l ) <= 2 :
			return l
			
		# people wanna see the first and last straight away
		result = [ l.pop(0), l.pop(-1) ]
		
		# then we start picking out the midframes as we subdivide
		# in a breadth first manner.
		toVisit = [ l ]
		while len( toVisit ) :
			n = toVisit.pop( 0 )
			if len( n ) > 1 :
				mid = (len( n )-1)/2
				result.append( n[mid] )
				toVisit.append( n[:mid] )
				toVisit.append( n[mid+1:] )
			elif len( n ):
				result.append( n[0] )
	
		return result
		
	@classmethod
	def suffix( self ) :
	
		return "b"

	@staticmethod
	def parse( s ) :
	
		l = BinaryFrameList.parseForChildList( s )
		if l :
			return BinaryFrameList( l )
			
		return None	
		
FrameList.registerParser( BinaryFrameList.parse )

