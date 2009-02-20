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

import FrameList

## The CompoundFrameList class implements the FrameList interface by storing a
# set of FrameList objects, and returning an order preserving union of all the frames they represent
# in its asList() method.
# 
# \ingroup python
class CompoundFrameList( FrameList.FrameList ) :

	## Constructs a CompoundFrameList object given an optional list
	# of FrameList objects. These can subsequently be accessed and modified
	# via the .frameLists instance attribute.
	def __init__( self, frameLists = [] ) :
		
		self.frameLists = frameLists
	
	## Implemented to protect the frameLists attribute from being assigned
	# invalid values.
	def __setattr__( self, key, value ) :
	
		if key=="frameLists" :
		
			self.__checkList( value )
			
		self.__dict__[key] = value	

	
	def __checkList( self, value ) :
	
		if not type( value ) is list :

			raise TypeError( "CompoundFrameList.frameLists must be a list" )

		for f in value :

			if not isinstance( f, FrameList.FrameList ) :

				raise TypeError( "CompoundFrameList.frameLists must contain only FrameList objects" )
				
	def __str__( self ) :
	
		self.__checkList( self.frameLists )
		return ",".join( [ str( l ) for l in self.frameLists ] )
		
	def __repr__( self ) :
	
		return "IECore.CompoundFrameList( %s )" % ( repr( self.frameLists ) )	
		
	def __eq__( self, other ) :
	
		return self.frameLists == other.frameLists	

	## Returns all the frames represented by the FrameLists in self.frameLists.
	# Frames are returned in the order specified by self.frameLists, but duplicate
	# frames will be omitted.
	def asList( self ) :
	
		self.__checkList( self.frameLists )
		
		result = []
		frameSet = set()
		for l in self.frameLists :
			for f in l.asList() :
				if not f in frameSet :
					result.append( f )
					frameSet.add( f )
		
		return result

	@staticmethod
	def parse( s ) :
	
		if s.count( "," ) :
			ss = s.split( "," )
			try :
				l = [ FrameList.FrameList.parse( x ) for x in ss ]
				return CompoundFrameList( l )
			except :
				return None
			
		return None	
		
FrameList.FrameList.registerParser( CompoundFrameList.parse )
