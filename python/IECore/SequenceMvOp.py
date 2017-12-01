##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

import IECore

class SequenceMvOp( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self, "Moves file sequences.",
			IECore.FileSequenceParameter(
				name = "result",
				description = "The new file sequence.",
				defaultValue = "",
				check = IECore.FileSequenceParameter.CheckType.DontCare,
				allowEmptyString = True,
				minSequenceSize = 1,
			)
		)

		self.parameters().addParameters(
			[
				IECore.FileSequenceParameter(
					name = "src",
					description = "The source file sequence.",
					defaultValue = "",
					check = IECore.FileSequenceParameter.CheckType.MustExist,
					allowEmptyString = False,
					minSequenceSize = 1,
				),
				IECore.FileSequenceParameter(
					name = "dst",
					description = "The destination file sequence.",
					defaultValue = "",
					check = IECore.FileSequenceParameter.CheckType.MustNotExist,
					allowEmptyString = False,
					minSequenceSize = 1,
				)
			]
		)

	def doOperation( self, operands ) :

		src = self.parameters()["src"].getFileSequenceValue()
		dst = self.parameters()["dst"].getFileSequenceValue()
		# if no frame list is specified on the dst parameter, then we use the same as src parameter.
		if isinstance( dst.frameList, IECore.EmptyFrameList ):
			dst.frameList = src.frameList

		IECore.mv( src, dst )

		return IECore.StringData( str(dst) )

IECore.registerRunTimeTyped( SequenceMvOp )
