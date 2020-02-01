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

import IECoreScene

import IECore

import imath
import unittest

class CurvesAlgoTest( unittest.TestCase ) :

	def nearlyEqual(self, a, b, places = 5):
		self.assertEqual(len(a), len(b))
		for ea, eb in zip(a, b):
			self.assertAlmostEqual(ea, eb, places)

	# region bSpline

	# bSpline
	# -------
	def curvesBSpline( self ) :

		testObject = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 8, 8 ] ),
			IECore.CubicBasisf.bSpline(),
			False,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0.75, 0 ),
					imath.V3f( 0.25, 1, 0 ),
					imath.V3f( 1, 1, 0 ),
					imath.V3f( 1, 1, 0 ),
					imath.V3f( 1, 1, 0 ),
					imath.V3f( 0, 0, 1 ),
					imath.V3f( 0, 0, 1 ),
					imath.V3f( 0, 0, 1 ),
					imath.V3f( 0, 0.75, 1 ),
					imath.V3f( 0.25, 1, 1 ),
					imath.V3f( 1, 1, 1 ),
					imath.V3f( 1, 1, 1 ),
					imath.V3f( 1, 1, 1 )
				]
			)
		)

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 16 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 2 ) ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 12 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 12 ) ) )

		# indexed
		testObject["f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 ] ) )
		testObject["g"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1 ] ) )
		testObject["h"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ) )
		testObject["i"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ) )

		# with geometric interpretation
		testObject["uniform_UV_V2f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.V2fVectorData( [imath.V2f(0,1), imath.V2f(2,3)], IECore.GeometricData.Interpretation.UV ) )
		testObject["vertex_Point_V3f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [imath.V3f(i, i, i) for i in range(0, 16)], IECore.GeometricData.Interpretation.Point ) )
		testObject["varying_Color_V3f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.V3fVectorData( [imath.V3f(i, i, i) for i in range(0, 12)], IECore.GeometricData.Interpretation.Color ) )
		testObject["facevarying_Normal_V3f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.V3fVectorData( [imath.V3f(i, i, i) for i in range(0, 12)], IECore.GeometricData.Interpretation.Normal) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testBSplineCurvesConstantToVertex( self ) :

		curves = self.curvesBSpline()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 16 ) )

	def testBSplineCurvesConstantToUniform( self ) :

		curves = self.curvesBSpline()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 2 ) )

	def testBSplineCurvesConstantToVarying( self ) :

		curves = self.curvesBSpline()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 12 ) )

	def testBSplineCurvesConstantToFaceVarying( self ) :

		curves = self.curvesBSpline()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 12 ) )

	def testBSplineCurvesVertexToConstant( self ) :

		curves = self.curvesBSpline()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,16))/16. ) )

	def testBSplineCurvesVertexToUniform( self ) :
		curves = self.curvesBSpline()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,8))/8., sum(range(8,16))/8. ] ) )

	def testBSplineCurvesVertexToVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		expected = IECore.FloatVectorData( range( 1, 7 ) + range( 9, 15 ) )
		for i in range( 0, p.data.size() ) :
			self.assertAlmostEqual( p.data[i], expected[i], 5 )

	def testBSplineCurvesVertexToFaceVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		expected = IECore.FloatVectorData( range( 1, 7 ) + range( 9, 15 ) )
		for i in range( 0, p.data.size() ) :
			self.assertAlmostEqual( p.data[i], expected[i], 5 )

	def testBSplineCurvesUniformToConstant( self ) :
		curves = self.curvesBSpline()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,2))/2. ) )

	def testBSplineCurvesUniformToVertex( self ) :
		curves = self.curvesBSpline()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 8 ) + ( [ 1 ] * 8 ) ) )

		p = curves["uniform_UV_V2f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		actual = p.data
		expected = IECore.V2fVectorData( ( [ imath.V2f(0, 1) ] * 8 ) + ( [ imath.V2f(2, 3) ] * 8 ), IECore.GeometricData.Interpretation.UV )

		self.assertEqual( actual, expected )

	def testBSplineCurvesUniformToVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

		p = curves["uniform_UV_V2f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable( curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		actual = p.data
		expected = IECore.V2fVectorData( ( [ imath.V2f(0, 1) ] * 6 ) + ( [ imath.V2f(2, 3) ] * 6 ), IECore.GeometricData.Interpretation.UV )

		self.assertEqual( actual, expected )

	def testBSplineCurvesUniformToFaceVarying( self ) :

		curves = self.curvesBSpline()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

		p = curves["uniform_UV_V2f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable( curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		actual = p.data
		expected = IECore.V2fVectorData( ( [ imath.V2f(0, 1) ] * 6 ) + ( [ imath.V2f(2, 3) ] * 6 ), IECore.GeometricData.Interpretation.UV )

		self.assertEqual( actual, expected )

	def testBSplineCurvesVaryingToConstant( self ) :
		curves = self.curvesBSpline()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,12))/12. ) )

	def testBSplineCurvesVaryingToVertex( self ) :
		curves = self.curvesBSpline()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 0.625, 1.25, 1.875, 2.5, 3.125, 3.75, 4.375, 6, 6.625, 7.25, 7.875, 8.5, 9.125, 9.75, 10.375 ] ) )

		p = curves["varying_Color_V3f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable( curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		actual = p.data
		expected = IECore.V3fVectorData( [ imath.V3f(i, i, i) for i in [ 0, 0.625, 1.25, 1.875, 2.5, 3.125, 3.75, 4.375, 6, 6.625, 7.25, 7.875, 8.5, 9.125, 9.75, 10.375 ] ], IECore.GeometricData.Interpretation.Color )

		self.assertEqual( actual, expected )

	def testBSplineCurvesVaryingToUniform( self ) :
		curves = self.curvesBSpline()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,6))/6., sum(range(6,12))/6. ] ) )

		p = curves["varying_Color_V3f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable( curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		actual = p.data
		expected = IECore.V3fVectorData( [ imath.V3f(i, i, i) for i in [ sum(range(0,6))/6., sum(range(6,12))/6. ] ], IECore.GeometricData.Interpretation.Color )

		self.assertEqual( actual, expected )

	def testBSplineCurvesVaryingToFaceVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 12 ) ) )

		p = curves["varying_Color_V3f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable( curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		actual = p.data
		expected = IECore.V3fVectorData( [ imath.V3f(i, i, i) for i in IECore.FloatVectorData( range( 0, 12 ) ) ], IECore.GeometricData.Interpretation.Color )

		self.assertEqual( actual, expected )

	def testBSplineCurvesFaceVaryingToConstant( self ) :
		curves = self.curvesBSpline()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,12))/12. ) )

	def testBSplineCurvesFaceVaryingToVertex( self ) :
		curves = self.curvesBSpline()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 0.625, 1.25, 1.875, 2.5, 3.125, 3.75, 4.375, 6, 6.625, 7.25, 7.875, 8.5, 9.125, 9.75, 10.375 ] ) )

		p = curves["facevarying_Normal_V3f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable( curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		actual = p.data
		expected = IECore.V3fVectorData( [ imath.V3f(i, i, i) for i in  [ 0, 0.625, 1.25, 1.875, 2.5, 3.125, 3.75, 4.375, 6, 6.625, 7.25, 7.875, 8.5, 9.125, 9.75, 10.375 ] ], IECore.GeometricData.Interpretation.Normal )

		self.assertEqual( actual, expected )

	def testBSplineCurvesFaceVaryingToUniform( self ) :
		curves = self.curvesBSpline()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,6))/6., sum(range(6,12))/6. ] ) )

	def testBSplineCurvesFaceVaryingToVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 12 ) ) )

		p = curves["facevarying_Normal_V3f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable( curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		actual = p.data
		expected = IECore.V3fVectorData( [ imath.V3f(i, i, i) for i in  IECore.FloatVectorData( range( 0, 12 ) ) ], IECore.GeometricData.Interpretation.Normal )

		self.assertEqual( actual, expected )

	def testBSplineCurvesIndexedVertexToUniform( self ) :
		curves = self.curvesBSpline()
		p = curves["f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.875, 1 ] ) )
		self.assertEqual( p.indices, None )

	def testBSplineCurvesIndexedVertexToVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["f"]

		self.assertRaises( RuntimeError, IECoreScene.CurvesAlgo.resamplePrimitiveVariable, curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

	def testBSplineCurvesIndexedVertexToFaceVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["f"]

		self.assertRaises( RuntimeError, IECoreScene.CurvesAlgo.resamplePrimitiveVariable, curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

	def testBSplineCurvesIndexedUniformToVertex( self ) :
		curves = self.curvesBSpline()
		p = curves["g"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 3 ) ) )
		self.assertEqual( p.indices, IECore.IntVectorData( ( [ 0 ] * 8 ) + ( [ 1 ] * 8 ) ) )

	def testBSplineCurvesIndexedUniformToVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["g"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 3 ) ) )
		self.assertEqual( p.indices, IECore.IntVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

	def testBSplineCurvesIndexedUniformToFaceVarying( self ) :

		curves = self.curvesBSpline()
		p = curves["g"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 3 ) ) )
		self.assertEqual( p.indices, IECore.IntVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

	def testBSplineCurvesIndexedVaryingToVertex( self ) :
		curves = self.curvesBSpline()
		p = curves["h"]

		self.assertRaises( RuntimeError, IECoreScene.CurvesAlgo.resamplePrimitiveVariable, curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

	def testBSplineCurvesIndexedVaryingToUniform( self ) :
		curves = self.curvesBSpline()
		p = curves["h"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 1, 1 ] ) )
		self.assertEqual( p.indices, None )

	def testBSplineCurvesIndexedVaryingToFaceVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["h"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 3 ) ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ) )

	def testBSplineCurvesIndexedFaceVaryingToVertex( self ) :
		curves = self.curvesBSpline()
		p = curves["i"]

		self.assertRaises( RuntimeError, IECoreScene.CurvesAlgo.resamplePrimitiveVariable, curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

	def testBSplineCurvesIndexedFaceVaryingToUniform( self ) :
		curves = self.curvesBSpline()
		p = curves["i"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 1, 1 ] ) )
		self.assertEqual( p.indices, None )

	def testBSplineCurvesIndexedFaceVaryingToVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["i"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 3 ) ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2 ] ) )
	# endregion

	# region catmullrom

	# catmullrom
	# ----------
	def curvesCatmullRom( self ) :

		testObject = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 8, 8 ] ),
			IECore.CubicBasisf.catmullRom(),
			False,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0.75, 0 ),
					imath.V3f( 0.25, 1, 0 ),
					imath.V3f( 1, 1, 0 ),
					imath.V3f( 1, 1, 0 ),
					imath.V3f( 1, 1, 0 ),
					imath.V3f( 0, 0, 1 ),
					imath.V3f( 0, 0, 1 ),
					imath.V3f( 0, 0, 1 ),
					imath.V3f( 0, 0.75, 1 ),
					imath.V3f( 0.25, 1, 1 ),
					imath.V3f( 1, 1, 1 ),
					imath.V3f( 1, 1, 1 ),
					imath.V3f( 1, 1, 1 )
				]
			)
		)

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 16 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 2 ) ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 12 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 12 ) ) )
		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testCatmullRomCurvesConstantToVertex( self ) :

		curves = self.curvesCatmullRom()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 16 ) )

	def testCatmullRomCurvesConstantToUniform( self ) :

		curves = self.curvesCatmullRom()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 2 ) )

	def testCatmullRomCurvesConstantToVarying( self ) :

		curves = self.curvesCatmullRom()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 12 ) )

	def testCatmullRomCurvesConstantToFaceVarying( self ) :

		curves = self.curvesCatmullRom()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 12 ) )

	def testCatmullRomCurvesVertexToConstant( self ) :

		curves = self.curvesCatmullRom()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,16))/16. ) )

	def testCatmullRomCurvesVertexToUniform( self ) :
		curves = self.curvesCatmullRom()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,8))/8., sum(range(8,16))/8. ] ) )

	def testCatmullRomCurvesVertexToVarying( self ) :
		curves = self.curvesCatmullRom()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		expected = IECore.FloatVectorData( range( 1, 7 ) + range( 9, 15 ) )
		for i in range( 0, p.data.size() ) :
			self.assertAlmostEqual( p.data[i], expected[i], 5 )

	def testCatmullRomCurvesVertexToFaceVarying( self ) :
		curves = self.curvesCatmullRom()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		expected = IECore.FloatVectorData( range( 1, 7 ) + range( 9, 15 ) )
		for i in range( 0, p.data.size() ) :
			self.assertAlmostEqual( p.data[i], expected[i], 5 )

	def testCatmullRomCurvesUniformToConstant( self ) :
		curves = self.curvesCatmullRom()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,2))/2. ) )

	def testCatmullRomCurvesUniformToVertex( self ) :
		curves = self.curvesCatmullRom()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 8 ) + ( [ 1 ] * 8 ) ) )

	def testCatmullRomCurvesUniformToVarying( self ) :
		curves = self.curvesCatmullRom()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

	def testCatmullRomCurvesUniformToFaceVarying( self ) :

		curves = self.curvesCatmullRom()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

	def testCatmullRomCurvesVaryingToConstant( self ) :
		curves = self.curvesCatmullRom()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,12))/12. ) )

	def testCatmullRomCurvesVaryingToVertex( self ) :
		curves = self.curvesCatmullRom()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 0.625, 1.25, 1.875, 2.5, 3.125, 3.75, 4.375, 6, 6.625, 7.25, 7.875, 8.5, 9.125, 9.75, 10.375 ] ) )

	def testCatmullRomCurvesVaryingToUniform( self ) :
		curves = self.curvesCatmullRom()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,6))/6., sum(range(6,12))/6. ] ) )

	def testCatmullRomCurvesVaryingToFaceVarying( self ) :
		curves = self.curvesCatmullRom()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 12 ) ) )

	def testCatmullRomCurvesFaceVaryingToConstant( self ) :
		curves = self.curvesCatmullRom()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,12))/12. ) )

	def testCatmullRomCurvesFaceVaryingToVertex( self ) :
		curves = self.curvesCatmullRom()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 0.625, 1.25, 1.875, 2.5, 3.125, 3.75, 4.375, 6, 6.625, 7.25, 7.875, 8.5, 9.125, 9.75, 10.375 ] ) )

	def testCatmullRomCurvesFaceVaryingToUniform( self ) :
		curves = self.curvesCatmullRom()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,6))/6., sum(range(6,12))/6. ] ) )

	def testCatmullRomCurvesFaceVaryingToVarying( self ) :
		curves = self.curvesCatmullRom()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 12 ) ) )
	# endregion

	# region linear

	# linear
	# ------
	def curvesLinear( self ) :

		testObject = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 2, 2 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 1, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 1, 0, 0 )
				]
			)
		)

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 4 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 2 ) ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 4 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 4 ) ) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testLinearCurvesConstantToVertex( self ) :
		curves = self.curvesLinear()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 4 ) )

	def testLinearCurvesConstantToUniform( self ) :

		curves = self.curvesLinear()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 2 ) )

	def testLinearCurvesConstantToVarying( self ) :

		curves = self.curvesLinear()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 4 ) )

	def testLinearCurvesConstantToFaceVarying( self ) :

		curves = self.curvesLinear()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 4 ) )

	def testLinearCurvesVertexToConstant( self ) :

		curves = self.curvesLinear()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,4))/4. ) )

	def testLinearCurvesVertexToUniform( self ) :
		curves = self.curvesLinear()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,2))/2., sum(range(2,4))/2. ] ) )

	def testLinearCurvesVertexToVarying( self ) :
		curves = self.curvesLinear()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		expected = IECore.FloatVectorData( range( 0, 4 ) )
		for i in range( 0, p.data.size() ) :
			self.assertAlmostEqual( p.data[i], expected[i], 5 )

	def testLinearCurvesVertexToFaceVarying( self ) :
		curves = self.curvesLinear()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		expected = IECore.FloatVectorData( range( 0, 4 )  )
		for i in range( 0, p.data.size() ) :
			self.assertAlmostEqual( p.data[i], expected[i], 5 )

	def testLinearCurvesUniformToConstant( self ) :
		curves = self.curvesLinear()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,2))/2. ) )

	def testLinearCurvesUniformToVertex( self ) :
		curves = self.curvesLinear()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 2 ) + ( [ 1 ] * 2 ) ) )

	def testLinearCurvesUniformToVarying( self ) :
		curves = self.curvesLinear()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 2 ) + ( [ 1 ] * 2 ) ) )

	def testLinearCurvesUniformToFaceVarying( self ) :
		curves = self.curvesLinear()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 2 ) + ( [ 1 ] * 2 ) ) )

	def testLinearCurvesVaryingToConstant( self ) :
		curves = self.curvesLinear()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,4))/4. ) )

	def testLinearCurvesVaryingToVertex( self ) :
		curves = self.curvesLinear()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 0.5, 2, 2.5 ] ) )

	def testLinearCurvesVaryingToUniform( self ) :
		curves = self.curvesLinear()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,2))/2., sum(range(2,4))/2. ] ) )

	def testLinearCurvesVaryingToFaceVarying( self ) :
		curves = self.curvesLinear()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 4 ) ) )

	def testLinearCurvesFaceVaryingToConstant( self ) :
		curves = self.curvesLinear()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,4))/4. ) )

	def testLinearCurvesFaceVaryingToVertex( self ) :
		curves = self.curvesLinear()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0, 0.5, 2, 2.5 ] ) )

	def testLinearCurvesFaceVaryingToUniform( self ) :
		curves = self.curvesLinear()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,2))/2., sum(range(2,4))/2. ] ) )

	def testLinearCurvesFaceVaryingToVarying( self ) :
		curves = self.curvesLinear()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 4 ) ) )

	def testCanSegmentUsingIntegerPrimvar( self ) :
		curves = self.curvesLinear()

		curves["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [0, 1] ) )

		segmentValues = IECore.IntVectorData( [0, 1] )
		segments = IECoreScene.CurvesAlgo.segment( curves, curves["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0].numCurves(), 1 )
		self.assertEqual( segments[1].numCurves(), 1 )

		p0 = imath.V3f( 0, 0, 0 )
		p1 = imath.V3f( 0, 1, 0 )
		p2 = imath.V3f( 0, 0, 0 )
		p3 = imath.V3f( 1, 0, 0 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [p0, p1], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData( [p2, p3], IECore.GeometricData.Interpretation.Point ) )

	def testCanSegmentUsingStringPrimvar( self ) :
		curves = self.curvesLinear()

		curves["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ["a", "b"] ) )

		segmentValues = IECore.StringVectorData( ["a", "b"] )
		segments = IECoreScene.CurvesAlgo.segment( curves, curves["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0].numCurves(), 1 )
		self.assertEqual( segments[1].numCurves(), 1 )

		p0 = imath.V3f( 0, 0, 0 )
		p1 = imath.V3f( 0, 1, 0 )
		p2 = imath.V3f( 0, 0, 0 )
		p3 = imath.V3f( 1, 0, 0 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [p0, p1], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData( [p2, p3], IECore.GeometricData.Interpretation.Point ) )

	def testRaisesExceptionIfSegmentKeysNotSameTypeAsPrimvar( self ) :
		curves = self.curvesLinear()

		curves["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ["a", "b"] ) )

		segmentValues = IECore.IntVectorData( [1, 2] )

		def t() :
			IECoreScene.CurvesAlgo.segment( curves, curves["s"], segmentValues )

		self.assertRaises( RuntimeError, t )

	def testEmptyPrimitiveIfNotMatching( self ) :
		curves = self.curvesLinear()

		curves["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ["a", "b"] ) )

		segmentValues = IECore.StringVectorData( ["e", "f"] )
		segments = IECoreScene.CurvesAlgo.segment( curves, curves["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Point ) )

	def testSegmentSubset( self ) :
		curves = self.curvesLinear()

		curves["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [0, 1] ) )

		segmentValues = IECore.IntVectorData( [0] )
		segments = IECoreScene.CurvesAlgo.segment( curves, curves["s"], segmentValues )

		self.assertEqual( len( segments ), 1 )

		self.assertEqual( segments[0].numCurves(), 1 )

		p0 = imath.V3f( 0, 0, 0 )
		p1 = imath.V3f( 0, 1, 0 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [p0, p1], IECore.GeometricData.Interpretation.Point ) )

	def testSegmentsFullyIfNoSegmentValuesGiven( self ):

		curves = self.curvesLinear()

		curves["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ["0", "1"] ) )

		segments = IECoreScene.CurvesAlgo.segment( curves, curves["s"] )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0].numCurves(), 1 )
		self.assertEqual( segments[1].numCurves(), 1 )

		p0 = imath.V3f( 0, 0, 0 )
		p1 = imath.V3f( 0, 1, 0 )
		p2 = imath.V3f( 0, 0, 0 )
		p3 = imath.V3f( 1, 0, 0 )

		if segments[0]["s"].data[0] == "0":
			s0 = segments[0]
			s1 = segments[1]
		else:
			s0 = segments[1]
			s1 = segments[0]

		self.assertEqual( s0["P"].data, IECore.V3fVectorData( [p0, p1], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( s1["P"].data, IECore.V3fVectorData( [p2, p3], IECore.GeometricData.Interpretation.Point ) )

	def testSegmentUsingIndexedPrimitiveVariable( self ) :
		curves = self.curvesLinear()
		curves["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ["a", "b"] ), IECore.IntVectorData( [ 1, 0] ) )

		segmentValues = IECore.StringVectorData( ["a", "b"] )
		segments = IECoreScene.CurvesAlgo.segment( curves, curves["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0].numCurves(), 1 )
		self.assertEqual( segments[1].numCurves(), 1 )

		p0 = imath.V3f( 0, 0, 0 )
		p1 = imath.V3f( 0, 1, 0 )
		p2 = imath.V3f( 0, 0, 0 )
		p3 = imath.V3f( 1, 0, 0 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [p2, p3], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData( [p0, p1], IECore.GeometricData.Interpretation.Point ) )

	# endregion

	# region bezier

	# bezier
	# ------
	def curvesBezier( self ) :

		testObject = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 7, 7 ] ),
			IECore.CubicBasisf.bezier(),
			False,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 1, 0 ),
					imath.V3f( 1, 1, 0 ),
					imath.V3f( 1, 0, 0 ),
					imath.V3f( 1, -1, 0 ),
					imath.V3f( 2, -1, 0 ),
					imath.V3f( 2, 0, 0 ),

					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0, 1 ),
					imath.V3f( 1, 0, 1 ),
					imath.V3f( 1, 0, 0 ),
					imath.V3f( 1, 0, -1 ),
					imath.V3f( 2, 0, -1 ),
					imath.V3f( 2, 0, 0 )
				]
			)
		)

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 14 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 2 ) ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 6 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 6 ) ) )
		testObject["f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "test" ) )
		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testBezierCurvesConstantToVertex( self ) :
		curves = self.curvesBezier()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 14 ) )

	def testBezierCurvesConstantToUniform( self ) :
		curves = self.curvesBezier()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 2 ) )

		p = curves["f"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.StringVectorData( [ "test" ] * 2 ) )

	def testBezierCurvesConstantToVarying( self ) :
		curves = self.curvesBezier()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 6 ) )

	def testBezierCurvesConstantToFaceVarying( self ) :
		curves = self.curvesBezier()
		p = curves["a"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 6 ) )

	def testBezierCurvesVertexToConstant( self ) :
		curves = self.curvesBezier()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,14))/14. ) )

	def testBezierCurvesVertexToUniform( self ) :
		curves = self.curvesBezier()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,7))/7., sum(range(7,14))/7. ] ) )

	def testBezierCurvesVertexToVarying( self ) :
		curves = self.curvesBezier()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [0, 3, 6] + [7, 10, 13] ) )

	def testBezierCurvesVertexToFaceVarying( self ) :
		curves = self.curvesBezier()
		p = curves["b"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [0, 3, 6] + [7, 10, 13] ) )

	def testBezierCurvesUniformToConstant( self ) :
		curves = self.curvesBezier()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,2))/2. ) )

	def testBezierCurvesUniformToVertex( self ) :
		curves = self.curvesBezier()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 7 ) + ( [ 1 ] * 7 ) ) )

	def testBezierCurvesUniformToVarying( self ) :
		curves = self.curvesBezier()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 3 ) + ( [ 1 ] * 3 ) ) )

	def testBezierCurvesUniformToFaceVarying( self ) :
		curves = self.curvesBezier()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 3 ) + ( [ 1 ] * 3 ) ) )

	def testBezierCurvesVaryingToConstant( self ) :
		curves = self.curvesBezier()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,6))/6. ) )

	def testBezierCurvesVaryingToVertex( self ) :
		curves = self.curvesBezier()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.nearlyEqual( p.data, IECore.FloatVectorData( [i * (2.0 / 7.0) for i in range(0, 7)]  + [3.0 + i * (2.0 / 7.0) for i in range(0, 7)] ) )

	def testBezierCurvesVaryingToUniform( self ) :
		curves = self.curvesBezier()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,3))/3., sum(range(3,6))/3. ] ) )

	def testBezierCurvesVaryingToFaceVarying( self ) :
		curves = self.curvesBezier()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 6 ) ) )

	def testBezierCurvesFaceVaryingToConstant( self ) :
		curves = self.curvesBezier()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,6))/6. ) )

	def testBezierCurvesFaceVaryingToVertex( self ) :
		curves = self.curvesBezier()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.nearlyEqual( p.data, IECore.FloatVectorData( [i * (2.0 / 7.0) for i in range(0, 7)]  + [3.0 + i * (2.0 / 7.0) for i in range(0, 7)] ) )

	def testBezierCurvesFaceVaryingToUniform( self ) :
		curves = self.curvesBezier()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,3))/3., sum(range(3,6))/3. ] ) )

	def testBezierCurvesFaceVaryingToVarying( self ) :
		curves = self.curvesBezier()
		p = curves["e"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 6 ) ) )

	# endregion

class CurvesAlgoDeleteCurvesTest ( unittest.TestCase ):

	def createLinearCurves(self):

		testObject = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 2, 2 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 1, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 1, 0, 0 )
				]
			)
		)

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 4 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 2 ) ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 4 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 4 ) ) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testAllZeroArrayDoesNotDeleteCurves(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [0, 0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 2)

	def testAllZeroArrayDeletesAllCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [0, 0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 0)

	def testBoolFalseArrayDoesNotDeleteCurves(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.BoolVectorData( [False, False] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 2)

	def testBoolFalseArrayDeletesCurvesAllCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.BoolVectorData( [False, False] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 0)

	def testFloatZeroArrayDoesNotDeleteCurves(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [0.0, 0.0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 2)

	def testFloatZeroArrayDeletesCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [0.0, 0.0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 0)

	def testBasisAndPeriodicityIsCopied(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [0, 0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.periodic(), False)
		self.assertEqual(actualCurves.basis(), IECore.CubicBasisf.linear())

	def testOneArrayDeletesAllCurves(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [1, 1] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 0)
		self.assertEqual( actualCurves["P"].data, IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Point ) )

	def testOneArrayDeletesNoCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [1, 1] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 2)
		self.assertEqual( actualCurves["P"].data,
			IECore.V3fVectorData( [imath.V3f( 0, 0, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 )],
				IECore.GeometricData.Interpretation.Point ) )

	def testCanUseBoolArrayToDeleteAllCurves(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.BoolVectorData( [True, True] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 0)
		self.assertEqual( actualCurves["P"].data, IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Point ) )

	def testBoolArrayTrueDeletesNoCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.BoolVectorData( [True, True] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 2)
		self.assertEqual( actualCurves["P"].data,
			IECore.V3fVectorData( [imath.V3f( 0, 0, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 )],
				IECore.GeometricData.Interpretation.Point ) )

	def testCanUseFloatArrayToDeleteAllCurves(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [0.1, 1.0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 0)
		self.assertEqual( actualCurves["P"].data, IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Point ) )

	def testFloatArrayNonZeroDeletesNoCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [0.1, 1.0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 2)
		self.assertEqual( actualCurves["P"].data,
			IECore.V3fVectorData( [imath.V3f( 0, 0, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 )],
				IECore.GeometricData.Interpretation.Point ) )

	def testPrimvarsAreDeleted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [1, 0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 1)
		self.assertEqual( actualCurves["P"].data,
			IECore.V3fVectorData( [imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 )], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( actualCurves["a"].data, IECore.FloatData( 0.5 ) )
		self.assertEqual( actualCurves["a"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( actualCurves["b"].data, IECore.FloatVectorData([2, 3])  )
		self.assertEqual( actualCurves["b"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( actualCurves["c"].data, IECore.FloatVectorData([1])  )
		self.assertEqual( actualCurves["c"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( actualCurves["d"].data, IECore.FloatVectorData([2, 3])  )
		self.assertEqual( actualCurves["d"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( actualCurves["e"].data, IECore.FloatVectorData([2, 3])  )
		self.assertEqual( actualCurves["e"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

	def testPrimvarsAreDeletedIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [1, 0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 1)
		self.assertEqual( actualCurves["P"].data,
			IECore.V3fVectorData( [imath.V3f( 0, 0, 0 ), imath.V3f( 0, 1, 0 )], IECore.GeometricData.Interpretation.Point ) )

		self.assertEqual( actualCurves["a"].data, IECore.FloatData( 0.5 ) )
		self.assertEqual( actualCurves["a"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( actualCurves["b"].data, IECore.FloatVectorData([0, 1])  )
		self.assertEqual( actualCurves["b"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( actualCurves["c"].data, IECore.FloatVectorData([0])  )
		self.assertEqual( actualCurves["c"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( actualCurves["d"].data, IECore.FloatVectorData([0, 1])  )
		self.assertEqual( actualCurves["d"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( actualCurves["e"].data, IECore.FloatVectorData([0, 1])  )
		self.assertEqual( actualCurves["e"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

	def createArrayOfCurves(self):
		'''
		3x3 array in XZ plane with the curve along Y 1 segment long and length 1 unit.
		:return: CurvesPrimitive
		'''

		testObject = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 2, 2, 2, 2, 2 ,2, 2, 2 ,2  ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 1, 0 ),

					imath.V3f( 1, 0, 0 ),
					imath.V3f( 1, 1, 0 ),

					imath.V3f( 2, 0, 0 ),
					imath.V3f( 2, 1, 0 ),

					imath.V3f( 0, 0, 1 ),
					imath.V3f( 0, 1, 1 ),

					imath.V3f( 1, 0, 1 ),
					imath.V3f( 1, 1, 1 ),

					imath.V3f( 2, 0, 1 ),
					imath.V3f( 2, 1, 1 ),

					imath.V3f( 0, 0, 2 ),
					imath.V3f( 0, 1, 2 ),

					imath.V3f( 1, 0, 2 ),
					imath.V3f( 1, 1, 2 ),

					imath.V3f( 2, 0, 2 ),
					imath.V3f( 2, 1, 2 ),
				]
			)
		)

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 18 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 9 ) ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 18 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 18 ) ) )

		testObject["bIndexed"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [3, 2, 1] ),
			IECore.IntVectorData( [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2] ) )
		testObject["cIndexed"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [3, 2, 1] ),
			IECore.IntVectorData( [0, 1, 2, 0, 1, 2, 0, 1, 2] ) )
		testObject["dIndexed"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.IntVectorData( [3, 2, 1] ),
			IECore.IntVectorData( [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2] ) )
		testObject["eIndexed"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.IntVectorData( [3, 2, 1, 0] ),
			IECore.IntVectorData( [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2] ) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testCanDeleteCurvesContainingIndexedPrimVar( self ) :
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform,
			IECore.IntVectorData( [ 0, 0, 0, 1, 1, 1, 0, 0, 0 ] ) ) )

		curves = self.createArrayOfCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves( curves, deletePrimVar )

		self.assertEqual(actualCurves.numCurves(), 6)
		self.assertEqual( actualCurves["P"].data, IECore.V3fVectorData( [
			imath.V3f( 0, 0, 0 ),
			imath.V3f( 0, 1, 0 ),

			imath.V3f( 1, 0, 0 ),
			imath.V3f( 1, 1, 0 ),

			imath.V3f( 2, 0, 0 ),
			imath.V3f( 2, 1, 0 ),

			# imath.V3f( 0, 0, 1 ),
			# imath.V3f( 0, 1, 1 ),
			#
			# imath.V3f( 1, 0, 1 ),
			# imath.V3f( 1, 1, 1 ),
			#
			# imath.V3f( 2, 0, 1 ),
			# imath.V3f( 2, 1, 1 ),

			imath.V3f( 0, 0, 2 ),
			imath.V3f( 0, 1, 2 ),

			imath.V3f( 1, 0, 2 ),
			imath.V3f( 1, 1, 2 ),

			imath.V3f( 2, 0, 2 ),
			imath.V3f( 2, 1, 2 ),
		], IECore.GeometricData.Interpretation.Point ) )

		self.assertEqual( actualCurves["a"].data, IECore.FloatData( 0.5 ) )
		self.assertIsNone( actualCurves["a"].indices )
		self.assertEqual( actualCurves["a"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( actualCurves["b"].data, IECore.FloatVectorData([
			0, 1, 2, 3, 4, 5,
			# 6, 7, 8, 9, 10, 11
			12, 13, 14, 15, 16, 17
		]
		) )
		self.assertIsNone( actualCurves["b"].indices )
		self.assertEqual( actualCurves["b"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( actualCurves["c"].data, IECore.FloatVectorData(
			[
				0, 1, 2,
				#3, 4, 5,
				6, 7, 8
			]
		)  )
		self.assertIsNone( actualCurves["c"].indices )
		self.assertEqual( actualCurves["c"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( actualCurves["d"].data, IECore.FloatVectorData([
			0, 1, 2, 3, 4, 5,
			# 6, 7, 8, 9, 10, 11
			12, 13, 14, 15, 16, 17
		] )  )
		self.assertIsNone( actualCurves["d"].indices )
		self.assertEqual( actualCurves["d"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying)



		self.assertEqual( actualCurves["e"].data, IECore.FloatVectorData([
			0, 1, 2, 3, 4, 5,
			# 6, 7, 8, 9, 10, 11
			12, 13, 14, 15, 16, 17
		])  )
		self.assertIsNone( actualCurves["e"].indices )
		self.assertEqual( actualCurves["e"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( actualCurves["bIndexed"].data, IECore.IntVectorData( [3, 1] ) )
		self.assertEqual( actualCurves["bIndexed"].indices, IECore.IntVectorData( [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1] ) )
		self.assertEqual( actualCurves["bIndexed"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( actualCurves["cIndexed"].data, IECore.IntVectorData( [3, 2, 1] ) )
		self.assertEqual( actualCurves["cIndexed"].indices, IECore.IntVectorData( [0, 1, 2, 0, 1, 2] ) )
		self.assertEqual( actualCurves["cIndexed"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( actualCurves["dIndexed"].data, IECore.IntVectorData( [3, 1] ) )
		self.assertEqual( actualCurves["dIndexed"].indices, IECore.IntVectorData( [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1] ) )
		self.assertEqual( actualCurves["dIndexed"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( actualCurves["eIndexed"].data, IECore.IntVectorData( [3, 1] ) )
		self.assertEqual( actualCurves["eIndexed"].indices, IECore.IntVectorData( [0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1] ) )
		self.assertEqual( actualCurves["eIndexed"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

	def testCanDeleteCurvesIndexedPrimvarForDeleteFlag( self ):

		indexedDeletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform,
			IECore.IntVectorData( [1, 0] ), IECore.IntVectorData( [0, 0, 0, 1, 1, 1, 0, 0, 0] ) ) )

		# expanding the indexed data: [1,1,1, 0,0,0, 1,1,1]
		# so 6 curves should be deleted, leaving the middle three in the output curves

		curves = self.createArrayOfCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves( curves, indexedDeletePrimVar )

		self.assertEqual(actualCurves.numCurves(), 3)

		self.assertEqual( actualCurves["bIndexed"].data, IECore.IntVectorData( [2] ) )
		self.assertEqual( actualCurves["bIndexed"].indices, IECore.IntVectorData( [0, 0, 0, 0, 0, 0] ) )
		self.assertEqual( actualCurves["bIndexed"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( actualCurves["cIndexed"].data, IECore.IntVectorData( [3, 2, 1] ) )
		self.assertEqual( actualCurves["cIndexed"].indices, IECore.IntVectorData( [0, 1, 2] ) )
		self.assertEqual( actualCurves["cIndexed"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( actualCurves["dIndexed"].data, IECore.IntVectorData( [2] ) )
		self.assertEqual( actualCurves["dIndexed"].indices, IECore.IntVectorData( [0, 0, 0, 0, 0, 0] ) )
		self.assertEqual( actualCurves["dIndexed"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( actualCurves["eIndexed"].data, IECore.IntVectorData( [2] ) )
		self.assertEqual( actualCurves["eIndexed"].indices, IECore.IntVectorData( [0, 0, 0, 0, 0, 0] ) )
		self.assertEqual( actualCurves["eIndexed"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

	def curvesBad( self ) :

		testObject = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 2, 2 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 1, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 1, 0, 0 )
				]
			)
		)

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 5 ) ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 3 ) ) )

		return testObject

	def testDeleteInvalidPrimVars( self ):

		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform,
			IECore.IntVectorData( [1, 0] ) )

		curves = self.curvesBad()
		self.assertRaises( RuntimeError, IECoreScene.CurvesAlgo.deleteCurves, curves, deletePrimVar )


class CurvesAlgoUpdateEndpointMultiplicityTest( unittest.TestCase ):

	def createLinearCurves(self):

		testObject = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 2, 2 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 1, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 1, 0, 0 )
				]
			)
		)

		# indexed primvar (ensure we only replicate the indices)
		testObject["bPrimVar"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [666, 3] ), IECore.IntVectorData([1,0,1,0] ) )

		# Uniform interpolated primitive variable to verify we don't do anything in this case
		testObject["cPrimVar"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData([101,99]) )

		# Varying interpolated primitive variables
		testObject["dPrimVar"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( [3, 666, 3, 666] ) )
		testObject["ePrimVar"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( [666, 3] ), IECore.IntVectorData([1,0,1,0] ) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testCanConvertLinearToBSplineAndBack( self ) :

		linearCurves = self.createLinearCurves()

		actualBSplineCurves = IECoreScene.CurvesAlgo.updateEndpointMultiplicity( linearCurves, IECore.CubicBasisf.bSpline() )

		self.assertEqual( actualBSplineCurves.basis(), IECore.CubicBasisf.bSpline())
		self.assertEqual( actualBSplineCurves.verticesPerCurve(), IECore.IntVectorData( [6, 6] ) )

		self.assertEqual(
			actualBSplineCurves["P"].data,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0, 0 ),

					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 1, 0 ),

					imath.V3f( 0, 1, 0 ),
					imath.V3f( 0, 1, 0 ),

					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 0, 0 ),

					imath.V3f( 0, 0, 0 ),
					imath.V3f( 1, 0, 0 ),

					imath.V3f( 1, 0, 0 ),
					imath.V3f( 1, 0, 0 )
				],
				IECore.GeometricData.Interpretation.Point # Note this tests the geometric interpretation has been copied correctly.
			)
		)

		bPrimVarData = actualBSplineCurves["bPrimVar"].data
		bPrimVarIndices = actualBSplineCurves["bPrimVar"].indices

		self.assertEqual( bPrimVarData, IECore.FloatVectorData( [666, 3] ) )
		self.assertEqual( bPrimVarIndices, IECore.IntVectorData( [1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0] ) )

		self.assertEqual( actualBSplineCurves["cPrimVar"],
			IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [101, 99] ) ) )

		self.assertEqual( actualBSplineCurves["dPrimVar"].data, IECore.FloatVectorData( [3, 3, 666, 666, 3, 3, 666, 666] ) )

		self.assertEqual( actualBSplineCurves["ePrimVar"].data, IECore.FloatVectorData( [666, 3] ) )
		self.assertEqual( actualBSplineCurves["ePrimVar"].indices, IECore.IntVectorData( [1, 1, 0, 0, 1, 1, 0, 0] ) )

		self.assertTrue( actualBSplineCurves.arePrimitiveVariablesValid() )

		backToLinear = IECoreScene.CurvesAlgo.updateEndpointMultiplicity( actualBSplineCurves, IECore.CubicBasisf.linear() )

		self.assertEqual( backToLinear.basis(), IECore.CubicBasisf.linear())
		self.assertEqual( backToLinear.verticesPerCurve(), IECore.IntVectorData( [2, 2] ) )

		self.assertEqual( backToLinear["P"].data,
			IECore.V3fVectorData(
				[
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 0, 1, 0 ),
					imath.V3f( 0, 0, 0 ),
					imath.V3f( 1, 0, 0 )
				],
				IECore.GeometricData.Interpretation.Point # Note this tests the geometric interpretation has been copied correctly.
			))

		self.assertEqual( backToLinear["bPrimVar"].data, IECore.FloatVectorData( [666, 3] ) )
		self.assertEqual( backToLinear["bPrimVar"].indices, IECore.IntVectorData( [1,0,1,0] ) )

		self.assertEqual( backToLinear["cPrimVar"],
			IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [101, 99] ) ) )

		self.assertEqual( backToLinear["dPrimVar"].data, IECore.FloatVectorData( [3, 666, 3, 666] ) )

		self.assertEqual( backToLinear["ePrimVar"].data, IECore.FloatVectorData( [666, 3] ) )
		self.assertEqual( backToLinear["ePrimVar"].indices, IECore.IntVectorData( [1,0,1,0] ) )

		self.assertTrue( backToLinear.arePrimitiveVariablesValid() )

	def testSameBasisLeavesCurvesUnmodified( self ) :

		linearCurves = self.createLinearCurves()

		newLinearCurves = IECoreScene.CurvesAlgo.updateEndpointMultiplicity( linearCurves, IECore.CubicBasisf.linear() )
		self.assertEqual( newLinearCurves, linearCurves )

		# this function isn't under test but just a convenient way to generate the bspline test data
		bSplineCurves = IECoreScene.CurvesAlgo.updateEndpointMultiplicity( linearCurves, IECore.CubicBasisf.bSpline() )

		newBSplineCurves = IECoreScene.CurvesAlgo.updateEndpointMultiplicity( bSplineCurves, IECore.CubicBasisf.bSpline() )
		self.assertEqual( newBSplineCurves, bSplineCurves )


	


if __name__ == "__main__":
	unittest.main()
