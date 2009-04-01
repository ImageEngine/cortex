##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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
from IECore import ls
from RunTimeTypedUtil import makeRunTimeTyped

## The FileSequenceVectorParameter class implements a Parameter
# to define a vector (list) of FileSequences. It's cunningly named in that
# respect. As it can't store FileSequence objects as
# it's value (they're not derived from Object) it stores
# a vector of strings representing those sequences instead, but provides
# methods for turning this into a list FileSequence objects.
# \ingroup python
# \todo Have this support the specification of frame ranges as well (in a form like "fileName.#.ext 1-20")
# This should be pretty easy to achieve as the FrameList class already defines the serialisation and parsing
# for frame ranges.
class FileSequenceVectorParameter( IECore.PathVectorParameter ) :

	def __init__( self, name, description, defaultValue = IECore.StringVectorData(), allowEmptyList = True, check = IECore.PathVectorParameter.CheckType.DontCare,
		presets = {}, presetsOnly = False, userData = IECore.CompoundObject(), extensions = [] ) :
		
		IECore.PathVectorParameter.__init__( self, name, description, defaultValue , allowEmptyList, check )#, presets, presetsOnly, userData )

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
		v = IECore.StringVectorParameter.valueValid( self, value )
		if not v[0] :
			return v
		
		if self.allowEmptyList and len( value ) == 0 :
			return True, ""
		
		for v in value :
			
			if not IECore.FileSequence.fileNameValidator().match( v ) :
				return False, "Value must contain one sequence of at least one # character to specify frame number."

			if len( self.extensions ) :
				e = os.path.splitext( v )[1].lstrip( "." ).split(' ')[0]
				if not e in self.extensions :
					return False, "File sequence extension not valid."

			if self.mustExist :
				parts = v.split(' ')
				if len(parts) == 0 or ls(parts[0]) is None:
					return False, "File sequence does not exist"

			elif self.mustNotExist :
				parts = v.split(' ')
				if ls( parts[0] ) :
					return False, "File sequence already exists"
			
		return True, ""	
	
	## Sets the internal StringVectorData value to fileSequence.fileName for each fileSequence in the passed list
	def setFileSequenceValues( self, fileSequences ) :
	
		data = IECore.StringVectorData()
		
		for fileSequence in fileSequences:
		
			data.append( fileSequence.fileName )
	
		self.setValue( data )
	
	## Gets the internal StringVectorData value and creates a Python list of FileSequence objects
	# from it using the ls() function. Note that this can return the empty list
	# if check is DontCare and no matching sequences exist on disk.
	def getFileSequenceValues( self ) :
	
		values = []
		fileSequences = self.getValidatedValue()
		
		for fileSequence in fileSequences :
		
			value = None
		
			parts = fileSequence.split(' ')
			if len(parts) == 1:
				value = ls(fileSequence)
			else:
				filename = parts[0]
				framelist = IECore.FrameList.parse(parts[1])
				value = IECore.FileSequence(filename, framelist)
				
			if value :
			
				values.append( value )	
			
		return values	

		
makeRunTimeTyped( FileSequenceVectorParameter, 100018, IECore.PathVectorParameter )
