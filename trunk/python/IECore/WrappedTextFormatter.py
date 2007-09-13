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

import Formatter
import StringUtil
import sys

## Implements the Formatter interface in a very simple form, just outputting
# text with word wrapping to a file object.
#
# \ingroup python
class WrappedTextFormatter( Formatter.Formatter ) :
	
	def __init__( self, outputFile, wrapWidth = 80 ) :
	
		self.__file = outputFile
		self.__wrapWidth = wrapWidth
		self.__indentation = 0
		self.__numNewLines = -1
				
	def heading( self, name ) :
	
		name = name.strip()
	
		self.__blankLine()
		self.__indent()
		self.__output( str( name ) + "\n" )
		self.__indent()
		self.__output( "".rjust( len( name ), "-" ) + "\n\n" )
		
		return self
		
	def paragraph( self, text ) :
	
		self.__blankLine()
		lines = StringUtil.wrap( str( text ).rstrip(), self.__wrapWidth ).split( "\n" )
		for line in lines :
			self.__indent()
			self.__output( line + "\n" )
		
	def indent( self ) :
	
		self.__indentation += 1
		return self
		
	def unindent( self ) :
	
		self.__indentation -= 1
		return self

	def __output( self, text ) :
	
		self.__file.write( text )
		self.__numNewLines = 0
		while self.__numNewLines < len( text ) :
			#sys.stdout.write( "("+text[(-1-self.__numNewLines)]+")" )
			if text[(-1-self.__numNewLines)]=='\n' :
				self.__numNewLines += 1
			else :
				break
				
		#sys.stdout.write( "OUTPUT " + str( self.__numNewLines ) + "\n" )
					
	def __indent( self ) :
	
		self.__output( "".rjust( self.__indentation * 4 ) )

	def __blankLine( self ) :
	
		if self.__numNewLines == -1 :
			return
							
		for i in range( self.__numNewLines, 2 ) :
			self.__file.write( "\n" )
