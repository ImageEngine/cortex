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

import unittest
import sys, os
import IECore

class TestImageCropOp(unittest.TestCase):

	testoutTifffile = "test/IECore/data/tiff/bluegreen_noise.testoutput.tiff"

	def testCropTiff(self):

		inputfile =    "test/IECore/data/tiff/bluegreen_noise.400x300.tif"

		tests = [
			{ 
				"cropBox": IECore.Box2i( IECore.V2i( 0, 75 ), IECore.V2i( 399, 224 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.400x150.tif",
			},

			{ 
				"cropBox": IECore.Box2i( IECore.V2i( 100, 75 ), IECore.V2i( 299, 224 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.200x150.tif",
			},

			{ 
				"cropBox": IECore.Box2i( IECore.V2i( -100, -75 ), IECore.V2i( 499, 374 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.400x300.tif",
			},
		
		]
		self.__testCrop( inputfile, self.testoutTifffile, tests )

	def __testCrop( self, inputfile, outputfile, tests ):
		r = IECore.Reader.create(inputfile)
		img = r.read()
		self.assertEqual(type(img), IECore.ImagePrimitive)

		cropOp = IECore.ImageCropOp()
		errors = []

		for testCase in tests:

			cropOp['copyInput'] = True
			cropOp['cropBox'] = testCase['cropBox']
			cropOp['matchDataWindow'] = testCase.get( 'matchDataWindow', True )

			for resetOrigin in [ True, False ]:

				testCase['resetOrigin'] = resetOrigin

				r = None

				cropOp['resetOrigin'] = resetOrigin
				cropOp['object'] = img
				croppedImg = cropOp()

				if not croppedImg.arePrimitiveVariablesValid():
					raise Exception, "Invalid cropped image in test case: " + str(testCase) + ". Image info (displayWindow, dataWindow,bufferSize): " + str( croppedImg.displayWindow) + ", " + str( croppedImg.dataWindow ) + ", " + str( len(croppedImg["R"].data) )

				w = IECore.Writer.create(croppedImg, outputfile)
				w.write()
				w = None

				r = IECore.Reader.create(outputfile)
				testImg = r.read()

				if not resetOrigin:
					dyw = croppedImg.displayWindow
					dtw = croppedImg.dataWindow
					dyw.max -= dyw.min
					dtw.max -= dyw.min
					dtw.min -= dyw.min
					dyw.min = IECore.V2i( 0,0 )
					croppedImg.displayWindow = dyw
					croppedImg.dataWindow = dtw

				if croppedImg == testImg:
					continue

				errors.append( "Crop test case failed:" + str(testCase) + ". Cropped image: " + str(croppedImg.displayWindow) + " " + str(croppedImg.dataWindow) + " Loaded image: " + str(testImg.displayWindow) + " " + str(testImg.dataWindow) )

		if len(errors):
			raise Exception, "\n".join( errors )

	def tearDown( self ) :
		if os.path.isfile( self.testoutTifffile ) :
			os.remove( self.testoutTifffile )

if __name__ == "__main__":
	unittest.main()   
	
