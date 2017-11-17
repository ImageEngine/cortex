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
import IECoreScene

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

	def testBSplineCurvesUniformToVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

	def testBSplineCurvesUniformToFaceVarying( self ) :

		curves = self.curvesBSpline()
		p = curves["c"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( ( [ 0 ] * 6 ) + ( [ 1 ] * 6 ) ) )

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

	def testBSplineCurvesVaryingToUniform( self ) :
		curves = self.curvesBSpline()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,6))/6., sum(range(6,12))/6. ] ) )

	def testBSplineCurvesVaryingToFaceVarying( self ) :
		curves = self.curvesBSpline()
		p = curves["d"]
		IECoreScene.CurvesAlgo.resamplePrimitiveVariable(curves, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 12 ) ) )

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
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 1, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 1, 0, 0 )
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
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 1, 0 ),
					IECore.V3f( 1, 1, 0 ),
					IECore.V3f( 1, 0, 0 ),
					IECore.V3f( 1, -1, 0 ),
					IECore.V3f( 2, -1, 0 ),
					IECore.V3f( 2, 0, 0 ),

					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 0, 1 ),
					IECore.V3f( 1, 0, 1 ),
					IECore.V3f( 1, 0, 0 ),
					IECore.V3f( 1, 0, -1 ),
					IECore.V3f( 2, 0, -1 ),
					IECore.V3f( 2, 0, 0 )
				]
			)
		)

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 14 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( range( 0, 2 ) ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 6 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 6 ) ) )
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
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 1, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 1, 0, 0 )
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
		self.assertEqual(actualCurves["P"].data, IECore.V3fVectorData([]) )

	def testOneArrayDeletesNoCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [1, 1] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 2)
		self.assertEqual(actualCurves["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 0, 1, 0 ), IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ) ] ) )

	def testCanUseBoolArrayToDeleteAllCurves(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.BoolVectorData( [True, True] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 0)
		self.assertEqual(actualCurves["P"].data, IECore.V3fVectorData([]) )

	def testBoolArrayTrueDeletesNoCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.BoolVectorData( [True, True] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 2)
		self.assertEqual(actualCurves["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 0, 1, 0 ), IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ) ] ) )

	def testCanUseFloatArrayToDeleteAllCurves(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [0.1, 1.0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 0)
		self.assertEqual(actualCurves["P"].data, IECore.V3fVectorData([]) )

	def testFloatArrayNonZeroDeletesNoCurvesIfInverted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [0.1, 1.0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar, invert=True)

		self.assertEqual(actualCurves.numCurves(), 2)
		self.assertEqual(actualCurves["P"].data, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 0, 1, 0 ), IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ) ] ) )

	def testPrimvarsAreDeleted(self):
		deletePrimVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [1, 0] ) ) )
		curves = self.createLinearCurves()
		actualCurves = IECoreScene.CurvesAlgo.deleteCurves(curves, deletePrimVar)

		self.assertEqual(actualCurves.numCurves(), 1)
		self.assertEqual( actualCurves["P"].data, IECore.V3fVectorData( [IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 )] ) )
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
		self.assertEqual( actualCurves["P"].data, IECore.V3fVectorData( [IECore.V3f( 0, 0, 0 ), IECore.V3f( 0, 1, 0 )] ) )

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

if __name__ == "__main__":
	unittest.main()
