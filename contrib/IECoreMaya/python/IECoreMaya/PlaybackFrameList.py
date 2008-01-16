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

from IECore import FrameList
from IECore import Enum
from  maya.cmds import playbackOptions

## The PlaybackFrameList implements an IECore.FrameList which
# represents the playback ranges queryable using the maya
# playbackOptions command.
#
# \ingroup mayaPython
class PlaybackFrameList( FrameList ) :

	Range = Enum.create( "Animation", "Playback" )
	
	## Takes a single parameter specifying the Range - either
	# Range.Animation for the scene frame range or Range.Playback
	# for the current playback settings. Both these are evaluated
	# dynamically, so the result of asList() will change if the
	# settings in maya are changed.
	def __init__( self, r ) :
	
		self.range = r
	
	def __str__( self ) :
	
		if self.range==self.Range.Animation :
			return "animation"
		elif self.range==self.Range.Playback :
			return "playback"
		
		assert( False ) # should never get here

	def asList( self ) :
	
		if self.range==self.Range.Animation :
			first = playbackOptions( query=True, animationStartTime=True )
			last = playbackOptions( query=True, animationEndTime=True )
		elif self.range==self.Range.Playback :
			first = playbackOptions( query=True, minTime=True )
			last = playbackOptions( query=True, maxTime=True )
			
		return range( first, last+1 )
			
	@staticmethod
	def parse( s ) :
	
		s = s.strip()
		if s=="animation" :
			return PlaybackFrameList( PlaybackFrameList.Range.Animation )
		elif s=="playback" :
			return PlaybackFrameList( PlaybackFrameList.Range.Playback )
		
		return None		
	
FrameList.registerParser( PlaybackFrameList.parse )
