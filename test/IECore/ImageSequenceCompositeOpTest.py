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

import os
import math
import unittest
from IECore import *

class ImageSequenceCompositeOpTest( unittest.TestCase ) :

	def testConstruction( self ) :

		op = ImageSequenceCompositeOp()
		self.assertEqual( op.parameters()["operation"].getValue().value, ImageCompositeOp.Operation.Over )

	def testSimple( self ) :

		expectedSequence = FileSequence( "test/IECore/data/expectedResults/imageSequenceCompositeOp.####.exr", FrameRange( 1,6 ) )
		outputSequence = FileSequence( "test/IECore/imageSequenceCompositeOpTest.####.exr", FrameRange( 1,6 ) )

		op = ImageSequenceCompositeOp()
		op.parameters()['fileSequence1'].setFileSequenceValue( FileSequence( "test/IECore/data/exrFiles/checkerAnimated.####.exr", FrameRange( 1,6 ) ) )
		op.parameters()['fileSequence2'].setFileSequenceValue( FileSequence( "test/IECore/data/exrFiles/colorBarsAnimated.####.exr", FrameRange( 1,6 ) ) )
		op.parameters()['outputFileSequence'].setValue( StringData( str( outputSequence ) ) )

		op()

		for i in range( 1, 6 ) :

			result = Reader.create( outputSequence.fileNameForFrame( i ) ).read()
			expectedResult = Reader.create( expectedSequence.fileNameForFrame( i ) ).read()

			diffOp = ImageDiffOp()

			diff = diffOp(
                        	imageA = result,
                        	imageB = expectedResult
                	)

 			self.failIf( diff.value )

	def setUp( self ) :

		self.tearDown()

	def tearDown( self ) :

		outputSequence = FileSequence( "test/IECore/imageSequenceCompositeOpTest.####.exr", FrameRange( 1,6 ) )

		for i in range( 1, 7 ) :

			filename = outputSequence.fileNameForFrame( i )
			if os.path.exists( filename ):
				os.remove( filename )

if __name__ == "__main__":
    unittest.main()
