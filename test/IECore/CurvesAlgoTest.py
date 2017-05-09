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

import unittest
import IECore

class CurvesAlgoTest( unittest.TestCase ) :

	## \todo: test curves with other basis

	def curves( self ) :

		testObject = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 8, 8 ] ),
			IECore.CubicBasisf.bSpline(),
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 0.75, 0 ),
					IECore.V3f( 0.25, 1, 0 ),
					IECore.V3f( 1, 1, 0 ),
					IECore.V3f( 1, 1, 0 ),
					IECore.V3f( 1, 1, 0 ),
					IECore.V3f( 0, 0, 1 ),
					IECore.V3f( 0, 0, 1 ),
					IECore.V3f( 0, 0, 1 ),
					IECore.V3f( 0, 0.75, 1 ),
					IECore.V3f( 0.25, 1, 1 ),
					IECore.V3f( 1, 1, 1 ),
					IECore.V3f( 1, 1, 1 ),
					IECore.V3f( 1, 1, 1 )
				]
			)
		)

		testObject["a"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 16 ) ) )
		testObject["c"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 2 ) ) )
		testObject["d"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 12 ) ) )
		testObject["e"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 12 ) ) )
		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testCurvesConstantToVertex( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["a"], IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ 0.5 ] * 16 ) )

	def testCurvesConstantToUniform( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["a"], IECore.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ 0.5 ] * 2 ) )

	def testCurvesConstantToVarying( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["a"], IECore.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ 0.5 ] * 12 ) )

	def testCurvesConstantToFaceVarying( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["a"], IECore.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ 0.5 ] * 12 ) )

	def testCurvesVertexToConstant( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["b"], IECore.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( resampled.data, IECore.FloatData( sum(range(0,16))/16. ) )

	def testCurvesVertexToUniform( self ) :
		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["b"], IECore.PrimitiveVariable.Interpolation.Uniform)
		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ sum(range(0,8))/8., sum(range(8,16))/8. ] ) )

	def testCurvesVertexToVarying( self ) :
		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["b"], IECore.PrimitiveVariable.Interpolation.Varying)
		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		expected = IECore.FloatVectorData( range( 1, 7 ) + range( 9, 15 ) )
		for i in range( 0, resampled.data.size() ) :
			self.assertAlmostEqual( resampled.data[i], expected[i], 5 )

	def testCurvesVertexToFaceVarying( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["b"], IECore.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		expected = IECore.FloatVectorData( range( 1, 7 ) + range( 9, 15 ) )
		for i in range( 0, resampled.data.size() ) :
			self.assertAlmostEqual( resampled.data[i], expected[i], 5 )

	def testCurvesUniformToConstant( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["c"], IECore.PrimitiveVariable.Interpolation.Constant)


		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( resampled.data, IECore.FloatData( sum(range(0,2))/2. ) )

	def testCurvesUniformToVertex( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["c"], IECore.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resampled.data, IECore.FloatVectorData( ( [ 0 ] * 8 ) + ( [ 1 ] * 8 ) ) )


	def testCurvesUniformToVarying( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["c"], IECore.PrimitiveVariable.Interpolation.Varying)


		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( resampled.data, IECore.FloatVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

	def testCurvesUniformToFaceVarying( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["c"], IECore.PrimitiveVariable.Interpolation.FaceVarying)


		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( resampled.data, IECore.FloatVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

	def testCurvesVaryingToConstant( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["d"], IECore.PrimitiveVariable.Interpolation.Constant)


		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( resampled.data, IECore.FloatData( sum(range(0,12))/12. ) )

	def testCurvesVaryingToVertex( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["d"], IECore.PrimitiveVariable.Interpolation.Vertex)


		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ 0, 0.625, 1.25, 1.875, 2.5, 3.125, 3.75, 4.375, 6, 6.625, 7.25, 7.875, 8.5, 9.125, 9.75, 10.375 ] ) )

	def testCurvesVaryingToUniform( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["d"], IECore.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ sum(range(0,6))/6., sum(range(6,12))/6. ] ) )

	def testCurvesVaryingToFaceVarying( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["d"], IECore.PrimitiveVariable.Interpolation.FaceVarying)


		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( resampled.data, IECore.FloatVectorData( range( 0, 12 ) ) )

	def testCurvesFaceVaryingToConstant( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["e"], IECore.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( resampled.data, IECore.FloatData( sum(range(0,12))/12. ) )

	def testCurvesFaceVaryingToVertex( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["e"], IECore.PrimitiveVariable.Interpolation.Vertex)


		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ 0, 0.625, 1.25, 1.875, 2.5, 3.125, 3.75, 4.375, 6, 6.625, 7.25, 7.875, 8.5, 9.125, 9.75, 10.375 ] ) )

	def testCurvesFaceVaryingToUniform( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["e"], IECore.PrimitiveVariable.Interpolation.Uniform)


		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( resampled.data, IECore.FloatVectorData( [ sum(range(0,6))/6., sum(range(6,12))/6. ] ) )

	def testCurvesFaceVaryingToVarying( self ) :

		curves = self.curves()
		resampled = IECore.CurvesAlgo.resamplePrimitiveVariable(curves, curves["e"], IECore.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( resampled.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( resampled.data, IECore.FloatVectorData( range( 0, 12 ) ) )

if __name__ == "__main__":
	unittest.main()
