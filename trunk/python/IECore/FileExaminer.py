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

import os.path
from FileSequence import FileSequence
from FileSequenceFunctions import ls

## The FileExaminer class is an abstract base class for classes which
# can perform some query on a file.
class FileExaminer :

	## Accepts a single argument, the name of the file to be
	# examined.
	def __init__( self, fileName ) :
	
		if type( fileName )!=str :
			raise TypeError( "FileName parameter must be a string." )

		self.__fileName = fileName
	
	## Sets the name of the file to be examined.	
	def setFileName( self, fileName ) :
	
		self.__fileName = fileName
	
	## Returns the name of the file to be examined.
	def getFileName( self ) :
	
		return self.__fileName	
	
	## Returns a set of dependencies for the file being examined.
	# The set can contain both fileName strings and also strings
	# specifying FileSequence objects in the form "sequence.#.ext frameRange".
	def dependencies( self ) :
	
		return set()
	
	## Recursively declares dependencies for all
	# files starting with the specified file, returning a set of
	# strings.		
	@staticmethod
	def allDependencies( fileName ) :
	
		examiner = FileExaminer.create( fileName )
		if not examiner :
			return set()
		else :
			dependencies = examiner.dependencies()
			result = dependencies
			for dependency in dependencies :
				if FileSequence.fileNameValidator().match( dependency ) :
					ext = os.path.splitext( dependency )
					if ext!="" :
						ext = ext[1:]
						if ext in FileExaminer.__examiners :
							sequence = ls( dependency )
							if sequence :
								for f in sequence.fileNames() :
									result.update( FileExaminer.__allDependencies( f ) )
				else :
					result.update( allDependencies( dependency ) )

			return result			
			
	## Creates an appropriate FileExaminer subclass for
	# the given fileName, returning None if no such
	# implementation exists.
	@staticmethod
	def create( fileName ) :

		ext = os.path.splitext( fileName )[1]
		if ext=="" :
			return None
		else :
			# strip the dot
			ext = ext[1:]

		if not ext in FileExaminer.__examiners :
			return None
			
		return FileExaminer.__examiners[ext]( fileName )

	## Returns a list of extensions for which FileExaminer
	# implementations have been registered
	@staticmethod
	def supportedExtensions() :
	
		return FileExaminer.__examiners.keys()
		
	## Registers a class which implements the FileExaminer
	# interface for files specified in the list of extensions.
	@staticmethod 
	def registerExaminer( extensions, examinerClass ) :
	
		for ext in extensions :
		
			FileExaminer.__examiners[ext] = examinerClass
		
	__examiners = {}
