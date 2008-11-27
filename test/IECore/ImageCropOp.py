##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
	testoutEXRfile = "test/IECore/data/exrFiles/colorBarsWithDataWindow.testoutput.exr"
	
	def testCropEXR(self):
	
		inputfile = "test/IECore/data/exrFiles/colorBarsWithDataWindow.exr"
		
		tests = [
			{ 
				"cropBox": IECore.Box2i( IECore.V2i( 855, 170 ), IECore.V2i( 1460, 1465 ) ),
				"checkFile": "test/IECore/data/expectedResults/imageCropDataWindow.exr",
				"matchDataWindow" : False,
				"resetOrigin" : True				
			},
			{ 
				"cropBox": IECore.Box2i( IECore.V2i( 855, 170 ), IECore.V2i( 1460, 1465 ) ),
				"checkFile": "test/IECore/data/expectedResults/imageCropDataWindowMatched.exr",
				"matchDataWindow" : True,
				"resetOrigin" : True				
			},
		]
		
		self.__testCrop( inputfile, self.testoutEXRfile, tests )

	def testCropTiff(self):

		inputfile =    "test/IECore/data/tiff/bluegreen_noise.400x300.tif"

		tests = [
			{ 
				"cropBox": IECore.Box2i( IECore.V2i( 0, 75 ), IECore.V2i( 399, 224 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.400x150.tif",
				"resetOrigin" : False				
			},

			{ 
				"cropBox": IECore.Box2i( IECore.V2i( 100, 75 ), IECore.V2i( 299, 224 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.200x150.tif",
				"resetOrigin" : False
			},

			{ 
				"cropBox": IECore.Box2i( IECore.V2i( -100, -75 ), IECore.V2i( 499, 374 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.400x300.tif",
				"resetOrigin" : False				
			},
			{ 
				"cropBox": IECore.Box2i( IECore.V2i( 0, 75 ), IECore.V2i( 399, 224 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.400x150.tif",
				"resetOrigin" : True
			},

			{ 
				"cropBox": IECore.Box2i( IECore.V2i( 100, 75 ), IECore.V2i( 299, 224 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.200x150.tif",
				"resetOrigin" : True
			},

			{ 
				"cropBox": IECore.Box2i( IECore.V2i( -100, -75 ), IECore.V2i( 499, 374 ) ),
				"checkFile": "test/IECore/data/tiff/bluegreen_noise.400x300.tif",
				"resetOrigin" : True
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
			resetOrigin = testCase.get( 'resetOrigin', True )
			cropOp['resetOrigin'] = resetOrigin
				
			cropOp['object'] = img
			croppedImg = cropOp()

			if not croppedImg.arePrimitiveVariablesValid():
				raise Exception, "Invalid cropped image in test case: " + str(testCase) + ". Image info (displayWindow, dataWindow,bufferSize): " + str( croppedImg.displayWindow) + ", " + str( croppedImg.dataWindow ) + ", " + str( len(croppedImg["R"].data) )

			IECore.Writer.create(croppedImg, outputfile).write()

			testImg = IECore.Reader.create(outputfile).read()

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
	
	def testDefaults( self ) :
	
		op = IECore.ImageCropOp()
		
		self.assert_( op['cropBox'].getValue().value.isEmpty() )
		self.assertEqual( op['resetOrigin'].getValue().value, True )
		self.assertEqual( op['matchDataWindow'].getValue().value, True )		
		
	
			
	def testEmptyCropBox( self ) :	
			
		image = IECore.Reader.create( "test/IECore/data/exrFiles/checker2.exr" ).read()
		
		op = IECore.ImageCropOp()
		executeOp = IECore.curry( op, object = image )
		self.assertRaises( RuntimeError, executeOp )

	def setUp( self ) :
		
		self.tearDown()
							
	def tearDown( self ) :
	
		if os.path.isfile( self.testoutTifffile ) :
			os.remove( self.testoutTifffile )
			
		if os.path.isfile( self.testoutEXRfile ) :
			os.remove( self.testoutEXRfile )	

if __name__ == "__main__":
	unittest.main()   
	
