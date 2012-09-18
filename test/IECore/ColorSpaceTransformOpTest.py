##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

import unittest
from IECore import *

class ColorSpaceTransformOpTest( unittest.TestCase ) :

	def test( self ) :

		self.assertEqual(  set( ColorSpaceTransformOp.colorSpaces() ), set( ['cineon', 'linear', 'rec709', 'srgb', 'panalog', 'alexaLogC'] ) )
		self.assertEqual(  set( ColorSpaceTransformOp.inputColorSpaces() ), set( ['cineon', 'linear', 'rec709', 'srgb', 'panalog', 'alexaLogC'] ) )
		self.assertEqual(  set( ColorSpaceTransformOp.outputColorSpaces() ), set( ['cineon', 'linear', 'rec709', 'srgb', 'panalog', 'alexaLogC'] ) )

	def testConversion( self ) :

		op = ColorSpaceTransformOp()
		result = op(
			inputColorSpace = "linear",
			outputColorSpace = "srgb",

			input = Reader.create( "test/IECore/data/exrFiles/uvMap.256x256.exr" ).read()
		)

		# SRGB To Rec709 isn't directly available, we have to go SRGB->Linear->Rec709
		result = op(
			inputColorSpace = "srgb",
			outputColorSpace = "rec709",

			input = result
		)

		# Result verified by eye
		diff = ImageDiffOp()
		diffResult = diff(
			imageA = result,
			imageB = Reader.create( "test/IECore/data/expectedResults/colorSpaceTransformOp1.exr" ).read(),
			maxError = 0.0001
		)
		self.failIf( diffResult.value )
		
	def testPremultConversion( self ) :
		# Test the colourspace conversion of an image that has been premultipled with an alpha.
		# The desired behaviour is to unpremult the image, apply the transform and premult it.
		op = ColorSpaceTransformOp()
		result = op(
			inputColorSpace = "linear",
			outputColorSpace = "srgb",

			input = Reader.create( "test/IECore/data/exrFiles/checker2Premult.exr" ).read()
		)
		
		# Result verified by eye
		diff = ImageDiffOp()
		diffResult = diff(
			imageA = result,
			imageB = Reader.create( "test/IECore/data/exrFiles/checker2PremultSRGB.exr" ).read(),
			maxError = 0.0001
		)
		self.failIf( diffResult.value )
		
	def testLinearToPanalog( self ):
		op = ColorSpaceTransformOp()
		result = op(
			inputColorSpace = "panalog",
			outputColorSpace = "linear",

			input = Reader.create( "test/IECore/data/exrFiles/uvMap.256x256.exr" ).read()
		)

		# Result verified by eye
		diff = ImageDiffOp()
		diffResult = diff(
			imageA = result,
			imageB = Reader.create( "test/IECore/data/expectedResults/colorSpaceTransformOpPanalog.exr" ).read(),
			maxError = 0.001
		)
		self.failIf( diffResult.value )
		
	def testPanalog( self ):
		# testing if I can transform back and forth
		op = ColorSpaceTransformOp()
		result = op(
			inputColorSpace = "linear",
			outputColorSpace = "panalog",

			input = Reader.create( "test/IECore/data/exrFiles/uvMap.256x256.exr" ).read()
		)
		
		result = op(
			inputColorSpace = "panalog",
			outputColorSpace = "linear",

			input = result
		)
		
		# Result verified by eye
		diff = ImageDiffOp()
		diffResult = diff(
			imageA = result,
			imageB = Reader.create( "test/IECore/data/exrFiles/uvMap.256x256.exr" ).read(),
			maxError = 0.0001
		)
		self.failIf( diffResult.value )


if __name__ == "__main__":
	unittest.main()

