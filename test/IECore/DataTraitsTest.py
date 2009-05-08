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

import math
import unittest
from IECore import *
import random
import os

class DataTraitsTest( unittest.TestCase ) :

	__simpleData = [ BoolData(), CharData(), StringData() ]

	__simpleNumericData = [ UCharData(), IntData(), UIntData(), FloatData(), DoubleData(),
				HalfData(), ShortData(), UShortData(), Int64Data(), UInt64Data() ]

	##\ todo: it seems that vectors, colors, and boxes shouldn't qualify as matrixData
	__matrixData = [ V2fData(), V2dData(), V2iData(), V3fData(), V3dData(), V3iData(),
			 Color3fData(), Color3dData(), Color4fData(), Color4dData(), Box2iData(), Box3iData(), Box2fData(),
			 Box2dData(), Box3fData(), Box3dData(), M33fData(), M33dData(), M44fData(), M44dData() ]

	__sequenceData = [ BoolVectorData(), CharVectorData(), UCharVectorData(), StringVectorData(),
			   IntVectorData(), UIntVectorData(), HalfVectorData(), FloatVectorData(), DoubleVectorData(),
			   ShortVectorData(), UShortVectorData(), Int64VectorData(), UInt64VectorData(), V2fVectorData(),
			   V2dVectorData(), V2iVectorData(), V3fVectorData(), V3dVectorData(), V3iVectorData(),
			   QuatfVectorData(), QuatdVectorData(), Box2iVectorData(), Box2fVectorData(), Box2dVectorData(),
			   Box3iVectorData(), Box3fVectorData(), Box3dVectorData(), M33fVectorData(), M33dVectorData(),
			   M44fVectorData(), M44dVectorData(), Color3fVectorData(), Color3dVectorData(), Color4fVectorData(),
			   Color4dVectorData() ]

	##\ todo: it seems that transformation matrices should qualify as matrixData
	__complexData = [ TransformationMatrixfData(), TransformationMatrixdData(), QuatfData(), QuatdData(),
			  SplineffData(), SplineddData(), SplinefColor3fData(), SplinefColor4fData(),
			  CubeColorLookupfData(), CubeColorLookupdData() ]

	__compoundData = [ CompoundData() ]

	def testIsSimpleDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__simpleData )
		trueData.extend( DataTraitsTest.__simpleNumericData )
		for data in trueData :
			self.assertTrue( DataTraits.isSimpleDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__matrixData )
		falseData.extend( DataTraitsTest.__sequenceData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__compoundData )
		for data in falseData :
			self.assertFalse( DataTraits.isSimpleDataType( data ) )

	def testIsSimpleNumericDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__simpleNumericData )
		for data in trueData :
			self.assertTrue( DataTraits.isSimpleNumericDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__simpleData )
		falseData.extend( DataTraitsTest.__matrixData )
		falseData.extend( DataTraitsTest.__sequenceData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__compoundData )
		for data in falseData :
			self.assertFalse( DataTraits.isSimpleNumericDataType( data ) )

	def testIsMatrixDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__matrixData )
		for data in trueData :
			self.assertTrue( DataTraits.isMatrixDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__simpleData )
		falseData.extend( DataTraitsTest.__simpleNumericData )
		falseData.extend( DataTraitsTest.__sequenceData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__compoundData )
		for data in falseData :
			self.assertFalse( DataTraits.isMatrixDataType( data ) )

	def testIsMappingDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__compoundData )
		for data in trueData :
			self.assertTrue( DataTraits.isMappingDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__simpleData )
		falseData.extend( DataTraitsTest.__simpleNumericData )
		falseData.extend( DataTraitsTest.__sequenceData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__matrixData )
		for data in falseData :
			self.assertFalse( DataTraits.isMappingDataType( data ) )

	def testIsSequenceDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__sequenceData )
		for data in trueData :
			self.assertTrue( DataTraits.isSequenceDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__simpleData )
		falseData.extend( DataTraitsTest.__simpleNumericData )
		falseData.extend( DataTraitsTest.__matrixData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__compoundData )
		for data in falseData :
			self.assertFalse( DataTraits.isSequenceDataType( data ) )

if __name__ == "__main__":
    unittest.main()

