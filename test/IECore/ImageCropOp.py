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

	def __fromNukeCrop( self, x, y, r, t, h = 1556 ) :
	
		# Nuke is flipped in the vertical, and would use a bound of ((0,2),(0,2)) for a 2x2 image.
		return IECore.Box2i( IECore.V2i( x , h - t ), IECore.V2i( r - 1, h - y - 1 ) )
	
	def testCrop(self):
	
		inputFile = "test/IECore/data/exrFiles/colorBarsWithDataWindow.exr"
					
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
			{ 
				"cropBox": self.__fromNukeCrop( 1125, 195, 1551, 681 ),
				"checkFile": "test/IECore/data/expectedResults/imageCropDataWindow2Matched.exr",
				"matchDataWindow" : True,
				"resetOrigin" : False				
			},
			{ 
				"cropBox": self.__fromNukeCrop( 1125, 195, 1551, 681 ),
				"checkFile": "test/IECore/data/expectedResults/imageCropDataWindow2.exr",
				"matchDataWindow" : False,
				"resetOrigin" : False				
			},
			{ 
				"cropBox": IECore.Box2i( IECore.V2i( -10, -10 ), IECore.V2i( 3000, 3000 ) ),
				"checkFile": inputFile,
				"matchDataWindow" : False,
				"resetOrigin" : False				
			},
		]
		
		self.__testCrop( inputFile , tests )

	def __testCrop( self, inputfile, tests ):
		r = IECore.Reader.create(inputfile)
		img = r.read()
		self.assertEqual(type(img), IECore.ImagePrimitive)		

		cropOp = IECore.ImageCropOp()
		errors = []

		testIdx = 0
		for testCase in tests:
			testIdx = testIdx + 1
			cropOp['copyInput'] = True
			cropOp['cropBox'] = testCase['cropBox']
			cropOp['matchDataWindow'] = testCase.get( 'matchDataWindow', True )
			resetOrigin = testCase.get( 'resetOrigin', True )
			cropOp['resetOrigin'] = resetOrigin
			cropOp['extendDataWindow'] = False
			cropOp['object'] = img
	
			croppedImg = cropOp()

			if not croppedImg.arePrimitiveVariablesValid():
				raise Exception, "Invalid cropped image in test case: " + str(testCase) + ". Image info (displayWindow, dataWindow,bufferSize): " + str( croppedImg.displayWindow) + ", " + str( croppedImg.dataWindow ) + ", " + str( len(croppedImg["R"].data) )

                
			#Uncomment to generate missing expected result files
			#if not os.path.exists( testCase['checkFile'] ) :
			#	IECore.Writer.create( croppedImg, testCase['checkFile'] ).write()

			expectedImg = IECore.Reader.create( testCase['checkFile'] ).read()
			
			if croppedImg == expectedImg :
				
				continue
			
			errors.append( "Crop test case failed:" + str(testCase) + ". Cropped image: " + str(croppedImg.displayWindow) + " " + str(croppedImg.dataWindow) + " Loaded image: " + str(expectedImg.displayWindow) + " " + str(expectedImg.dataWindow) )

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
		
	def testCropEnlarge( self ) :
	
		image = IECore.Reader.create( "test/IECore/data/exrFiles/checker2.exr" ).read()
		
		op = IECore.ImageCropOp()
		result = op(
			object = image,
			cropBox = IECore.Box2i(
				IECore.V2i( 28, 92 ),
				IECore.V2i( 580, 455 )
			),
			resetOrigin = False,
			matchDataWindow = True,
		)
		
		self.assert_( result.arePrimitiveVariablesValid() )

		expectedResult = IECore.Reader.create( "test/IECore/data/expectedResults/imageCropEnlarge.exr" ).read()
		
		self.assertEqual( result, expectedResult )
				

	def setUp( self ) :
		
		self.tearDown()
							
	def tearDown( self ) :
	
		pass	

if __name__ == "__main__":
	unittest.main()   
	
