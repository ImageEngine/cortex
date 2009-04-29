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

# \ingroup python

import os
from IECore import *

#This Op checks an image file sequence for corrupted and missing files. It also warns of abrupt file size changes.
# The Op will raise an error if there's any missing or corrupt file.
# Otherwise it will return the number of suspicious frames (strange file sizes).
# This Op is targeted for users only.
class CheckImagesOp( FileSequenceAnalyzerOp ) :

	def __init__( self ) :
	
		FileSequenceAnalyzerOp.__init__( self, "CheckImagesOp",
"""This Op checks an image file sequence for corrupted and missing files. It also warns of abrupt file size changes. 
The Op will raise an error if there's any missing or corrupt file.
Otherwise it will return the number of suspicious frames (strange file sizes).""",
			IntParameter(
				name = "result",
				description = "Returns the number of suspicious frames.",
				defaultValue = 0,
			),
			extensions = "dpx exr cin tif tiff jpeg jpg"
		)
		
		self.userData()["UI"] = CompoundObject(
			{
				"infoMessages": BoolData( True ),
			}
		)

	def doOperation( self, args ) :

		suspicious = self.suspiciousFrames()
		suspicious.sort()
		corrupted =  self.corruptedFrames()
		corrupted.sort()
		missing = self.missingFrames()
		missing.sort()

		info( "Checking sequence:", args.fileSequence.value, "..." )

		if len(missing):
			error( "Missing frames:", ','.join( map( str, missing ) ) )
		if len(corrupted):
			error( "Corrupted frames:", ','.join( map( str, corrupted ) ) )
		if len(suspicious):
			warning("Suspicious frames:", ','.join( map( str, suspicious ) ) )

		if len(missing) + len(corrupted) + len(suspicious) == 0:
			info( "File sequence is ok." )

		if (len(missing) + len(corrupted)) > 0:
			raise Exception, "The file sequence did not pass the test."

		return IntData( len(suspicious) )

registerRunTimeTyped( CheckImagesOp, 100017, FileSequenceAnalyzerOp )
