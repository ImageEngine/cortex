##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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
import unittest

import IECore
import IECoreImage

class ColorAlgoTest( unittest.TestCase ) :

	def __verifyImageRGB( self, imgNew, imgOrig, maxError = 0.002, same=True ):

		self.assertEqual( type(imgNew), IECoreImage.ImagePrimitive )

		if "R" in imgOrig :
			self.assertTrue( "R" in imgNew )

		if "G" in imgOrig :
			self.assertTrue( "G" in imgNew )

		if "B" in imgOrig :
			self.assertTrue( "B" in imgNew )

		if "A" in imgOrig :
			self.assertTrue( "A" in imgNew )

		if "Y" in imgOrig :
			self.assertTrue( "Y" in imgNew )

		op = IECoreImage.ImageDiffOp()

		res = op(
			imageA = imgNew,
			imageB = imgOrig,
			maxError = maxError,
			skipMissingChannels = True
		)

		if same :
			self.assertFalse( res.value )
		else :
			self.assertTrue( res.value )

	def testTransformImageSRGB( self ) :

		linearImage = IECore.Reader.create( "test/IECoreImage/data/exr/uvMap.512x256.exr" ).read()
		image = linearImage.copy()

		reader = IECore.Reader.create( "test/IECoreImage/data/jpg/uvMap.512x256.jpg" )
		reader["rawChannels"].setTypedValue( True )
		srgbImage = reader.read()

		IECoreImage.ColorAlgo.transformImage( image, "linear", "sRGB" )
		self.__verifyImageRGB( image, linearImage, same=False )
		self.__verifyImageRGB( image, srgbImage, maxError = 0.004, same=True )

		IECoreImage.ColorAlgo.transformImage( image, "sRGB", "linear" )
		self.__verifyImageRGB( image, srgbImage, maxError = 0.004, same=False )
		self.__verifyImageRGB( image, linearImage, same=True )

	@unittest.skipIf( not os.path.exists( os.environ.get( "OCIO", "" ) ), "Insufficient color specification. Linear -> Cineon conversion is not possible with an OCIO config" )
	def testTransformImageLog( self ) :

		linearImage = IECore.Reader.create( "test/IECoreImage/data/exr/uvMap.512x256.exr" ).read()
		image = linearImage.copy()

		reader = IECore.Reader.create( "test/IECoreImage/data/dpx/uvMap.512x256.dpx" )
		reader["rawChannels"].setTypedValue( True )
		logImage = reader.read()

		IECoreImage.ColorAlgo.transformImage( image, "linear", "cineon" )
		self.__verifyImageRGB( image, linearImage, same=False )
		self.__verifyImageRGB( image, logImage, same=True )

		IECoreImage.ColorAlgo.transformImage( image, "cineon", "linear" )
		self.__verifyImageRGB( image, logImage, same=False )
		self.__verifyImageRGB( image, linearImage, same=True )

	@unittest.skipIf( not os.path.exists( os.environ.get( "OCIO", "" ) ), "Insufficient color specification. Linear -> Cineon conversion is not possible with an OCIO config" )
	def testRoles( self ) :

		linearImage = IECore.Reader.create( "test/IECoreImage/data/exr/uvMap.512x256.exr" ).read()
		image = linearImage.copy()

		IECoreImage.ColorAlgo.transformImage( image, "scene_linear", "color_picking" )
		self.__verifyImageRGB( image, linearImage, same=False )

		IECoreImage.ColorAlgo.transformImage( image, "color_picking", "scene_linear" )
		self.__verifyImageRGB( image, linearImage, same=True )

if __name__ == "__main__" :
	unittest.main()
