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

from FileExaminer import FileExaminer
from IECore import findSequences
import os

## The RIBFileExaminer class implements the FileExaminer interface for
# RIB files. It uses the ribdepends utility distributed with 3delight
# to do the work.
class RIBFileExaminer( FileExaminer ) :

	def __init__( self, fileName ) :

		FileExaminer.__init__( self, fileName )

	def dependencies( self ) :

		pipe = os.popen( "ribdepends \"%s\"" % self.getFileName(), 'r' )
		lines = pipe.readlines()
		status = pipe.close()
		if status :
			raise RuntimeError( "Error running ribdepends" )

		goodIdentifiers = [ 's', 't', 'x', 'u', 'c' ]
		files = []
		for line in lines :
			if len( line ) > 4 :
				if line[0]=='[' and line[2:4]=="] " and line[1] in goodIdentifiers :

					files.append( line[4:].strip() )

		result = set()
		for f in files :
			result.add( f )

		return result

FileExaminer.registerExaminer( [ "rib" ], RIBFileExaminer )
