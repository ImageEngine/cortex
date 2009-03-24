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

import math
import unittest
from IECore import *

class ImageCompositeOpTest( unittest.TestCase ) :

	def __test( self, operation, expectedResultFileName ) :
	
		op = ImageCompositeOp()
		
		imageA = Reader.create( "test/IECore/data/exrFiles/checker1Premult.exr" ).read()
		imageB = Reader.create( "test/IECore/data/exrFiles/checker2Premult.exr" ).read()
		
		result = op(
			input = imageB,
			imageA = imageA,
			operation = operation
		)
		
		expectedResult = Reader.create( expectedResultFileName ).read()
		diffOp = ImageDiffOp()
 
		diff = diffOp(
                        imageA = result,
                        imageB = expectedResult
                )

 		self.failIf( diff.value )
		
	def testConstruction( self ) :
	
		op = ImageCompositeOp()
		self.assertEqual( op.parameters()["operation"].getValue().value, ImageCompositeOp.Operation.Over )
		self.assertEqual( op.parameters()["alphaChannelName"].getValue().value, "A" )		
		self.assertEqual( op.parameters()["channels"].getValue(), StringVectorData( [ "R", "G", "B" ] ) )
		
	def testChannelSubset( self ):
	
		op = ImageCompositeOp()
		
		imageA = Reader.create( "test/IECore/data/exrFiles/checker1Premult.exr" ).read()
		imageB = Reader.create( "test/IECore/data/exrFiles/checker2Premult.exr" ).read()
		
		result = op(
			input = imageB,
			imageA = imageA,
			channels = StringVectorData( [ "R", "G" ] ),
			operation = ImageCompositeOp.Operation.Over
		)	
		
		self.assert_( result.arePrimitiveVariablesValid() )

	def testOver( self ) :
	
		self.__test( ImageCompositeOp.Operation.Over, "test/IECore/data/expectedResults/imageCompositeOpOver.exr" )
		
	def testMax( self ) :
	
		self.__test( ImageCompositeOp.Operation.Max, "test/IECore/data/expectedResults/imageCompositeOpMax.exr" )
		
	def testMin( self ) :
	
		self.__test( ImageCompositeOp.Operation.Min, "test/IECore/data/expectedResults/imageCompositeOpMin.exr" )
		
	def testMultiply( self ) :		
	
		self.__test( ImageCompositeOp.Operation.Multiply, "test/IECore/data/expectedResults/imageCompositeOpMultiply.exr" )			
		

if __name__ == "__main__":
    unittest.main()   
