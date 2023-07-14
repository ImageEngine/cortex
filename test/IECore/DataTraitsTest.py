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
import IECore
import datetime
import imath


class DataTraitsTest( unittest.TestCase ) :

	__simpleData = [ IECore.BoolData(), IECore.CharData(), IECore.StringData() ]

	__simpleNumericData = [ IECore.UCharData(), IECore.IntData(), IECore.UIntData(), IECore.FloatData(), IECore.DoubleData(),
				IECore.HalfData(), IECore.ShortData(), IECore.UShortData(), IECore.Int64Data(), IECore.UInt64Data() ]

	##\ todo: it seems that vectors, colors, and boxes shouldn't qualify as matrixData
	__matrixData = [ IECore.V2fData(), IECore.V2dData(), IECore.V2iData(), IECore.V3fData(), IECore.V3dData(), IECore.V3iData(),
			 IECore.Color3fData(), IECore.Color4fData(), IECore.Box2iData(), IECore.Box3iData(), IECore.Box2fData(),
			 IECore.Box2dData(), IECore.Box3fData(), IECore.Box3dData(), IECore.M33fData(), IECore.M33dData(), IECore.M44fData(), IECore.M44dData() ]

	__sequenceData = [ IECore.BoolVectorData(), IECore.CharVectorData(), IECore.UCharVectorData(), IECore.StringVectorData(),
			   IECore.IntVectorData(), IECore.UIntVectorData(), IECore.HalfVectorData(), IECore.FloatVectorData(), IECore.DoubleVectorData(),
			   IECore.ShortVectorData(), IECore.UShortVectorData(), IECore.Int64VectorData(), IECore.UInt64VectorData(), IECore.V2fVectorData(),
			   IECore.V2dVectorData(), IECore.V2iVectorData(), IECore.V3fVectorData(), IECore.V3dVectorData(), IECore.V3iVectorData(),
			   IECore.QuatfVectorData(), IECore.QuatdVectorData(), IECore.Box2iVectorData(), IECore.Box2fVectorData(), IECore.Box2dVectorData(),
			   IECore.Box3iVectorData(), IECore.Box3fVectorData(), IECore.Box3dVectorData(), IECore.M33fVectorData(), IECore.M33dVectorData(),
			   IECore.M44fVectorData(), IECore.M44dVectorData(), IECore.Color3fVectorData(), IECore.Color4fVectorData() ]

	##\ todo: it seems that transformation matrices should qualify as matrixData
	__complexData = [ IECore.TransformationMatrixfData(), IECore.TransformationMatrixdData(), IECore.QuatfData(), IECore.QuatdData(),
			  IECore.SplineffData(), IECore.SplineddData(), IECore.SplinefColor3fData(), IECore.SplinefColor4fData(),
	]

	__compoundData = [ IECore.CompoundData() ]

	def testIsSimpleDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__simpleData )
		trueData.extend( DataTraitsTest.__simpleNumericData )
		for data in trueData :
			self.assertTrue( IECore.DataTraits.isSimpleDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__matrixData )
		falseData.extend( DataTraitsTest.__sequenceData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__compoundData )
		for data in falseData :
			self.assertFalse( IECore.DataTraits.isSimpleDataType( data ) )

	def testIsSimpleNumericDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__simpleNumericData )
		for data in trueData :
			self.assertTrue( IECore.DataTraits.isSimpleNumericDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__simpleData )
		falseData.extend( DataTraitsTest.__matrixData )
		falseData.extend( DataTraitsTest.__sequenceData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__compoundData )
		for data in falseData :
			self.assertFalse( IECore.DataTraits.isSimpleNumericDataType( data ) )

	def testIsMappingDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__compoundData )
		for data in trueData :
			self.assertTrue( IECore.DataTraits.isMappingDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__simpleData )
		falseData.extend( DataTraitsTest.__simpleNumericData )
		falseData.extend( DataTraitsTest.__sequenceData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__matrixData )
		for data in falseData :
			self.assertFalse( IECore.DataTraits.isMappingDataType( data ) )

	def testIsSequenceDataType( self ) :

		trueData = []
		trueData.extend( DataTraitsTest.__sequenceData )
		for data in trueData :
			self.assertTrue( IECore.DataTraits.isSequenceDataType( data ) )

		falseData = []
		falseData.extend( DataTraitsTest.__simpleData )
		falseData.extend( DataTraitsTest.__simpleNumericData )
		falseData.extend( DataTraitsTest.__matrixData )
		falseData.extend( DataTraitsTest.__complexData )
		falseData.extend( DataTraitsTest.__compoundData )
		for data in falseData :
			self.assertFalse( IECore.DataTraits.isSequenceDataType( data ) )

	def testDataFromElement( self ) :

		dataMap = (
			( True, IECore.BoolData, IECore.BoolVectorData ),
			( 10, IECore.IntData, IECore.IntVectorData ),
			( "abc", IECore.StringData, IECore.StringVectorData ),
			( IECore.InternedString( "abc" ), IECore.InternedStringData, IECore.InternedStringVectorData ),
			( 1.1, IECore.DoubleData, IECore.DoubleVectorData ),
			( imath.V2f( 1.0, 2.0 ), IECore.V2fData, IECore.V2fVectorData ),
			( imath.V2d( 1.0, 2.0 ), IECore.V2dData, IECore.V2dVectorData ),
			( imath.V2i( 1, 2 ), IECore.V2iData, IECore.V2iVectorData ),
			( imath.V3i( 1, 2, 3 ), IECore.V3iData, IECore.V3iVectorData ),
			( imath.V3f( 1.0, 2.0, 3.0 ), IECore.V3fData, IECore.V3fVectorData ),
			( imath.V3d( 1.0, 2.0, 3.0 ), IECore.V3dData, IECore.V3dVectorData ),
			( imath.Quatf(), IECore.QuatfData, IECore.QuatfVectorData ),
			( imath.Quatd(), IECore.QuatdData, IECore.QuatdVectorData ),
			( imath.Color3f(), IECore.Color3fData, IECore.Color3fVectorData ),
			( imath.Color4f(), IECore.Color4fData, IECore.Color4fVectorData ),
			( imath.Box2i(), IECore.Box2iData, IECore.Box2iVectorData ),
			( imath.Box3i(), IECore.Box3iData, IECore.Box3iVectorData ),
			( imath.Box2f(), IECore.Box2fData, IECore.Box2fVectorData ),
			( imath.Box2d(), IECore.Box2dData, IECore.Box2dVectorData ),
			( imath.Box3f(), IECore.Box3fData, IECore.Box3fVectorData ),
			( imath.Box3d(), IECore.Box3dData, IECore.Box3dVectorData ),
			( imath.M33f(), IECore.M33fData, IECore.M33fVectorData ),
			( imath.M33d(), IECore.M33dData, IECore.M33dVectorData ),
			( imath.M44f(), IECore.M44fData, IECore.M44fVectorData ),
			( imath.M44d(), IECore.M44dData, IECore.M44dVectorData ),
			( { "age" : 10 }, IECore.CompoundData, None ),
			( IECore.TransformationMatrixf(), IECore.TransformationMatrixfData, None ),
			( IECore.TransformationMatrixd(), IECore.TransformationMatrixdData, None ),
			( IECore.LineSegment3f( imath.V3f( 0 ), imath.V3f( 1 ) ), IECore.LineSegment3fData, None ),
			( IECore.LineSegment3d( imath.V3d( 0 ), imath.V3d( 1 ) ), IECore.LineSegment3dData, None ),
			( IECore.Splineff(), IECore.SplineffData, None ),
			( IECore.Splinedd(), IECore.SplineddData, None ),
			( IECore.SplinefColor3f(), IECore.SplinefColor3fData, None ),
			( IECore.SplinefColor4f(), IECore.SplinefColor4fData, None ),
			( datetime.datetime( 2023, 7, 14 ), IECore.DateTimeData, None ),
			( IECore.TimeCode(), IECore.TimeCodeData, None ),
			( IECore.PathMatcher(), IECore.PathMatcherData, None ),
		)

		for element, dataType, vectorType in dataMap :
			# test single element conversion
			self.assertEqual( IECore.dataFromElement( element ), dataType( element ) )
			if vectorType is None :
				continue

			# test list of elements conversion to a vector data
			self.assertEqual(
				IECore.dataFromElement( [ element ] ),
				vectorType( [ element ] ),
			)

	def testDataTypeFromElementType( self ) :
		self.assertEqual( IECore.dataTypeFromElementType( int ) , IECore.IntData )
		self.assertRaises( TypeError, IECore.dataTypeFromElementType, list )


if __name__ == "__main__":
	unittest.main()

