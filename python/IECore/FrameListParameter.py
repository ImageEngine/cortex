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

import _IECore as IECore
from RunTimeTypedUtil import makeRunTimeTyped

## The FrameListParameter stores a string as its value but uses the
# FrameList.parse() function to validate that it represents a FrameList.
# It provides utility function for retrieving the value as a FrameList and
# setting the value from a FrameList.
# \ingroup python
class FrameListParameter( IECore.StringParameter ) :

	def __init__( self, name, description, defaultValue = "", allowEmptyList = True,
		presets = {}, presetsOnly = False, userData = IECore.CompoundObject() ) :
		
		if isinstance( defaultValue, IECore.FrameList ) :
			defaultValue = str( defaultValue )
		
		IECore.StringParameter.__init__( self, name, description, defaultValue, presets, presetsOnly, userData )
		
		self.__allowEmptyList = allowEmptyList
		
	## Returns true only if the value is StringData and parses into a valid FrameList.
	def valueValid( self, value ) :
		
		if not value.isInstanceOf( IECore.StringData.staticTypeId() ) :
		
			return False, "Value must be of type StringData"
				
		try :
			f = IECore.FrameList.parse( value.value )
			if not self.__allowEmptyList and isinstance( f, IECore.EmptyFrameList ) :
				return False, "Value must not be empty."
		except Exception, e :
			return False, str( e )
			
		return True, ""
	
	## Sets the internal StringData value to str( frameList )
	def setFrameListValue( self, frameList ) :
	
		self.setValue( IECore.StringData( str( frameList ) ) )
	
	## Gets the internal StringData value and returns a FrameList
	# creating using it.
	def getFrameListValue( self ) :
		s = self.getValidatedValue().value
		return IECore.FrameList.parse( s )
		
makeRunTimeTyped( FrameListParameter, 100012, IECore.StringParameter )
