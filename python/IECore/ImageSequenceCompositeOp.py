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

# The ImageSequenceCompositeOp does a simple A-over-B composite of two input sequences of image files
class ImageSequenceCompositeOp( SequenceMergeOp ) :

	def __init__( self ) :

		SequenceMergeOp.__init__(
			self,
			"ImageSequenceCompositeOp",
			"The ImageSequenceCompositeOp does a simple A-over-B composite of two input sequences of image files",
			extensions = [ "tif", "tiff", "exr", "cin", "dpx", "jpg" ]
		)

		self.parameters().addParameters(
			[
				IntParameter(
					name = "operation",
					description = "The compositing operation to apply",
					defaultValue = ImageCompositeOp.Operation.Over,
					presets = (
						( "Over", ImageCompositeOp.Operation.Over ),
						( "Min", ImageCompositeOp.Operation.Min ),
						( "Max", ImageCompositeOp.Operation.Max ),
					),
					presetsOnly = True
				),
			]
		)

	def _merge( self, fileName1, fileName2, outputFileName ) :

		image1 = Reader.create( fileName1 ).read()
		if not image1.isInstanceOf( "ImagePrimitive" ) :
			raise RuntimeError( "ImageSequenceCompositeOp: Could not load image from from '%s'" % ( fileName1 ) )

		image2 = Reader.create( fileName2 ).read()
		if not image2.isInstanceOf( "ImagePrimitive" ) :
			raise RuntimeError( "ImageSequenceCompositeOp: Could not load image from from '%s'" % ( fileName2 ) )

		op = ImageCompositeOp()

		resultImage = op(
			input = image2,
			imageA = image1,
			operation = self.parameters()["operation"].getValue().value,
			inputMode = ImageCompositeOp.InputMode.Unpremultiplied,
		)
		Writer.create( resultImage, outputFileName ).write()

		return True

registerRunTimeTyped( ImageSequenceCompositeOp, 100025, SequenceMergeOp )
