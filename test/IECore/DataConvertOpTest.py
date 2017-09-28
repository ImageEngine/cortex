##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

class DataConvertOpTest( unittest.TestCase ) :

	def testScaling( self ) :

		o = IECore.DataConvertOp()(

			data = IECore.FloatVectorData( [ 0, 0.5, 1 ] ),
			targetType = IECore.UCharVectorData.staticTypeId()

		)

		self.assertEqual(

			o,
			IECore.UCharVectorData( [ 0, 128, 255 ] )

		)

	def testDimensionUpConversion( self ) :

		o = IECore.DataConvertOp()(

			data = IECore.FloatVectorData( [ 0, 0.5, 1, 0.1, 2, 10 ] ),
			targetType = IECore.V3fVectorData.staticTypeId()

		)

		self.assertEqual(

			o,
			IECore.V3fVectorData( [ IECore.V3f( 0, 0.5, 1 ), IECore.V3f( 0.1, 2, 10 ) ] )

		)

	def testDimensionDownConversion( self ) :

		o = IECore.DataConvertOp()(

			data = IECore.V3iVectorData( [ IECore.V3i( 1, 2, 3 ), IECore.V3i( 4, 5, 6 ) ] ),
			targetType = IECore.IntVectorData.staticTypeId()

		)

		self.assertEqual(

			o,
			IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] )

		)

	def testWrongSizeForDimensions( self ) :

		self.assertRaises(

			RuntimeError,
			IECore.DataConvertOp(),
			data = IECore.FloatVectorData( [ 1, 2 ] ),
			targetType = IECore.V3fVectorData.staticTypeId()

		)

if __name__ == "__main__":
    unittest.main()
