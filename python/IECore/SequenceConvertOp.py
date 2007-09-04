#!/usr/bin/env python
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

#import VersionControl
#VersionControl.setVersion('IECore', '2')
#from IECore import *

from IECore import *

class SequenceConvertOp( Op ) :

	def __init__( self ) :
	
		Op.__init__( self, "SequenceConvertOp", "Converts file sequences.",
			FileSequenceParameter(
				name = "result",
				description = "The new file sequence.",
				defaultValue = "",
				check = FileSequenceParameter.CheckType.DontCare,
				allowEmptyString = True,
			)
		)
		
		self.parameters().addParameters(
			[
				FileSequenceParameter(
					name = "src",
					description = "The source file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
				),
				FileSequenceParameter(
					name = "dst",
					description = "The destination file sequence.",
					defaultValue = "",
					check = FileSequenceParameter.CheckType.MustNotExist,
					allowEmptyString = False,
				)
			]
		)

	def doOperation( self, operands ) :
	
		src = self.parameters()["src"].getFileSequenceValue()
		dst = src.copy() # to get the frameList
		dst.fileName = operands.dst.value

		# if extensions match, simply copy
		# \todo compare extensions
		if False:
			
			cp(src, dst)
			
		# if extensions don't match, read and write
		else:
			for (sf, df) in zip(src.fileNames(), dst.fileNames()):
				#print 'convert %s to %s' % (sf, df)
				img = Reader.create(sf).read()
				Writer.create(img, df).write()
			
		return StringData(dst.fileName)

#makeRunTimeTyped( SequenceConvertOp, 100007, Op )

# if __name__ == '__main__':
#  	scop = SequenceConvertOp()

# 	# demo: convert dpx files to cineon files
#  	p = scop.parameters()
#  	p['src'].setTypedValue('/home/blair/example/sequence/MT001_001_Element001.####.dpx 1-4')
#  	p['dst'].setTypedValue('/tmp/MT001_001_Element001.####.cin')

#  	scop.operate()
