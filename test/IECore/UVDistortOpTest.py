##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

"""Unit test for UVDistortOp binding"""
import os
import unittest
import math
import random

from IECore import *

class testUVDistortOp( unittest.TestCase ) :

	def testUVDistort( self ):
		"""Testing the UVDistort op using a known UV map against a known resulting image."""

		uvMapName = "test/IECore/data/exrFiles/undistorted_21mm_uv.exr"
		uvImg = Reader.create( uvMapName )()
		imgName = "test/IECore/data/jpg/21mm.jpg"
		img = Reader.create( imgName )()
		op = UVDistortOp()
		resultImg = op( input = img, uvMap = uvImg )
		resultImg.displayWindow = resultImg.dataWindow

		self.assert_( ImageDiffOp()( imageA = img, imageB = resultImg, maxError = 0.02 ).value )

		# reverse operation
		uvMapName = "test/IECore/data/exrFiles/distorted_21mm_uv.exr"
		uvImg = Reader.create( uvMapName )()
		resultImg2 = op( input = resultImg, uvMap = uvImg )

		self.assert_( ImageDiffOp()( imageA = resultImg, imageB = resultImg2, maxError = 0.02 ).value )
		self.assert_( not ImageDiffOp()( imageA = img, imageB = resultImg2, maxError = 0.015 ).value )

if __name__ == "__main__":
        unittest.main()


