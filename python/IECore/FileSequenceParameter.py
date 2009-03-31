##########################################################################
#
#  Copyright (c) 2007-2008 Image Engine Design Inc. All rights reserved.
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

import re
import os.path
import _IECore as IECore
from FileSequenceFunctions import ls
from RunTimeTypedUtil import makeRunTimeTyped

## The FileSequenceParameter class implements a Parameter
# to define FileSequences. It's cunningly named in that
# respect. As it can't store FileSequence objects as
# it's value (they're not derived from Object) it stores
# a string representing the sequence instead, but provides
# methods for turning this into a FileSequence.
# \ingroup python
# \todo Have this support the specification of frame ranges as well (in a form like "fileName.#.ext 1-20")
# This should be pretty easy to achieve as the FrameList class already defines the serialisation and parsing
# for frame ranges.
class FileSequenceParameter( IECore.PathParameter ) :

	def __init__( self, name, description, defaultValue = "", allowEmptyString = True, check = IECore.PathParameter.CheckType.DontCare,
		presets = {}, presetsOnly = False, userData = IECore.CompoundObject(), extensions = [] ) :
		
		IECore.PathParameter.__init__( self, name, description, defaultValue, allowEmptyString, check, presets, presetsOnly, userData )

		if isinstance( extensions, list ) :
			self.extensions = extensions
		else :
			self.extensions = extensions.split()
			
	## Returns true only if the value is StringData and matches the FileSequence.fileNameValidator
	# pattern. Also checks that the sequence exists or doesn't exist based on the CheckType passed to
	# the constructor.
	def valueValid( self, value ) :
		
		# we can't call PathParameter.valueValid() because that would do existence checking on
		# our path specifier with the # characters in it, and that would yield the wrong results
		# so we call StringParameter.valueValid and do the rest ourselves.
		v = IECore.StringParameter.valueValid( self, value )
		if not v[0] :
			return v
		
		if self.allowEmptyString and value.value=="" :
			return True, ""
			
		if not IECore.FileSequence.fileNameValidator().match( value.value ) :
			return False, "Value must contain one sequence of at least one # character to specify frame number."
				
		fileSequence = None
		try :
		
			fileSequence = self.__parseFileSequence( value.value )			
			
		except RuntimeError, e :
		
			return False, "Not a valid file sequence specification"
					
		assert( fileSequence )	
					
		if len( self.extensions ) :
			e = os.path.splitext( fileSequence.fileName )[1].split(".")[1]

			if not e in self.extensions :
				return False, "File sequence extension not valid."
		
		if self.mustExist :
						
			if ls( fileSequence.fileName ) is None:
				return False, "File sequence does not exist"
			
		elif self.mustNotExist :

			if ls( fileSequence.fileName ) :
				return False, "File sequence already exists"
			
		return True, ""	
	
	## Sets the internal StringData value to fileSequence.fileName
	# \todo Don't throw away fileSequence.frameList here!
	def setFileSequenceValue( self, fileSequence ) :
	
		self.setValue( IECore.StringData( fileSequence.fileName ) )
	
	## Find the longest space-delimited tail substring that is a parseable FrameList and
	# return a FileSequence instance which contains that FrameList. Everything before that is considered to 
	# be part of the filename. Previous implementations would just split on the first space character 
	# encountered, but this wouldn't allow for the filename portion of the value to include spaces itself.
	def __parseFileSequence( self, fileSequenceStr ) :
	
		fileSequenceCopy = str( fileSequenceStr )

		spaceIndex = fileSequenceCopy.find( " " )
		
		found = False
		filename = fileSequenceStr
		framelist = IECore.FrameList.parse( "" )

		while not found and spaceIndex != -1 :

			head, tail = fileSequenceStr[ 0:spaceIndex ], fileSequenceStr[ spaceIndex+1: ]
			filename = head

			assert( head + " " + tail == fileSequenceStr )

			try :

				framelist = IECore.FrameList.parse( tail )
				found = True
			except :
				fileSequenceCopy =  fileSequenceCopy[ 0:spaceIndex ] + "*" + fileSequenceCopy[ spaceIndex+1: ]				
				spaceIndex = fileSequenceCopy.find( " " )

		return IECore.FileSequence( filename, framelist )
	
	## Gets the internal StringData value and creates a FileSequence
	# from it using the ls() function. Note that this can return None
	# if check is DontCare and no matching sequence exists on disk.
	def getFileSequenceValue( self ) :
	
		fileSequenceStr = self.getValidatedValue().value
	
                if fileSequenceStr.find( " " ) == -1:
                        return ls( fileSequenceStr )
			
		return self.__parseFileSequence( fileSequenceStr )

		
makeRunTimeTyped( FileSequenceParameter, 100000, IECore.PathParameter )
