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
import imath
import IECore
import IECoreScene

class PointsAlgoTest( unittest.TestCase ) :

	def points( self ) :

		testObject = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 10 ) ] ) )

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0 ] ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 10 ) ) )

		# indexed
		testObject["f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 ] ) )
		testObject["g"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0.5 ] ), IECore.IntVectorData( [ 0 ] ) )
		testObject["h"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 ] ) )
		testObject["i"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 ] ) )

		testObject["j"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "test" ) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testPointsConstantToVertex( self ) :
		points = self.points()
		p = points["a"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsConstantToUniform( self ) :
		points = self.points()
		p = points["a"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )

		p = points["j"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.StringVectorData( [ "test" ] ) )

	def testPointsConstantToVarying( self ) :
		points = self.points()
		p = points["a"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsConstantToFaceVarying( self ) :
		points = self.points()
		p = points["a"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsVertexToConstant( self ) :
		points = self.points()
		p = points["b"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsVertexToUniform( self ) :
		points = self.points()
		p = points["b"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsVertexToUniformZeroPointCount( self ) :
		def makeEmptyPoints() :
			testObject = IECoreScene.PointsPrimitive( IECore.V3fVectorData() )
			testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData() )

			self.assertTrue( testObject.arePrimitiveVariablesValid() )
			return testObject

		points = makeEmptyPoints()

		p = points["b"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable( points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [] ) )

	def testPointsVertexToVarying( self ) :
		points = self.points()
		p = points["b"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsVertexToFaceVarying( self ) :
		points = self.points()
		p = points["b"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsUniformToConstant( self ) :
		points = self.points()
		p = points["c"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( 0 ) )

	def testPointsUniformToVertex( self ) :
		points = self.points()
		p = points["c"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsUniformToVarying( self ) :
		points = self.points()
		p = points["c"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsUniformToFaceVarying( self ) :
		points = self.points()
		p = points["c"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsVaryingToConstant( self ) :
		points = self.points()
		p = points["d"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsVaryingToVertex( self ) :
		points = self.points()
		p = points["d"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsVaryingToUniform( self ) :
		points = self.points()
		p = points["d"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsVaryingToFaceVarying( self ) :
		points = self.points()
		p = points["d"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsFaceVaryingToConstant( self ) :
		points = self.points()
		p = points["e"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsFaceVaryingToVertex( self ) :
		points = self.points()
		p = points["e"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsFaceVaryingToUniform( self ) :
		points = self.points()
		p = points["e"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsFaceVaryingToVarying( self ) :
		points = self.points()
		p = points["e"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsIndexedVertexToUniform( self ) :
		points = self.points()
		p = points["f"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.9 ] ) )
		self.assertEqual( p.indices, None )

	def testPointsIndexedUniformToVertex( self ) :
		points = self.points()
		p = points["g"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0 ] * 10 ) )

	def testPointsIndexedUniformToVarying( self ) :
		points = self.points()
		p = points["g"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0 ] * 10 ) )

	def testPointsIndexedUniformToFaceVarying( self ) :
		points = self.points()
		p = points["g"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0 ] * 10 ) )

	def testPointsIndexedVaryingToUniform( self ) :
		points = self.points()
		p = points["h"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.9 ] ) )
		self.assertEqual( p.indices, None )

	def testPointsIndexedFaceVaryingToUniform( self ) :
		points = self.points()
		p = points["i"]
		IECoreScene.PointsAlgo.resamplePrimitiveVariable(points, p, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.9 ] ) )
		self.assertEqual( p.indices, None )

class DeletePointsTest( unittest.TestCase ) :
	def points( self ) :

		testObject = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 10 ) ] ) )

		testObject["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0 ] ) )
		testObject["d"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["e"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 10 ) ) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testRaisesExceptionIfPrimitiveVariableTypeIsInvalid(self):
		points  = self.points()
		points["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 10 ) ] ) )

		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertRaises( RuntimeError, lambda : IECoreScene.PointsAlgo.deletePoints(points, points["delete"]) )

	def testRaisesExceptionIfPrimitiveVariableInterpolationIncorrect(self):
		points  = self.points()
		points["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0 ]  ) )

		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertRaises( RuntimeError, lambda : IECoreScene.PointsAlgo.deletePoints(points, points["delete"]) )

	def testCanDeleteAllPoints(self):
		points  = self.points()
		points["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.BoolVectorData( [True] * 10 ) )

		self.assertTrue( points.arePrimitiveVariablesValid() )

		points = IECoreScene.PointsAlgo.deletePoints(points, points["delete"])

		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertEqual( points.numPoints, 0 )
		self.assertEqual( points["delete"].data, IECore.BoolVectorData() )

	def testDeletesNoPointsIfInverted(self):
		points  = self.points()
		points["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.BoolVectorData( [True] * 10 ) )

		self.assertTrue( points.arePrimitiveVariablesValid() )
		
		invertedPoints = IECoreScene.PointsAlgo.deletePoints(points, points["delete"], invert=True)

		self.assertTrue( invertedPoints.arePrimitiveVariablesValid() )

		self.assertEqual( invertedPoints.numPoints, 10 )
		self.assertEqual( invertedPoints["delete"].data, IECore.BoolVectorData( [True] * 10 ) )

		self.assertEqual( invertedPoints["a"].data, IECore.FloatData( 0.5 ) )
		self.assertEqual( invertedPoints["b"].data, IECore.FloatVectorData( range( 0, 10 ) ) )
		self.assertEqual( invertedPoints["c"].data, IECore.FloatVectorData( [ 0 ] ) )
		self.assertEqual( invertedPoints["d"].data, IECore.FloatVectorData( range( 0, 10 ) ) )
		self.assertEqual( invertedPoints["e"].data, IECore.FloatVectorData( range( 0, 10 ) ) )


	def testPrimitiveVariablesCorrectlyFiltered(self):
		points  = self.points()
		points["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [0.0, 1.0] * 5) )

		self.assertTrue( points.arePrimitiveVariablesValid() )
		points = IECoreScene.PointsAlgo.deletePoints(points, points["delete"])
		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertEqual( points.numPoints, 5 )
		self.assertEqual( points["delete"].data, IECore.FloatVectorData( [0.0] * 5 ) )
		self.assertEqual( points["delete"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( points["a"].data, IECore.FloatData( 0.5 ) )
		self.assertEqual( points["a"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( points["b"].data, IECore.FloatVectorData( range( 0, 10, 2 ) ) )
		self.assertEqual( points["b"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( points["c"].data, IECore.FloatVectorData( [0] ) )
		self.assertEqual( points["c"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( points["d"].data, IECore.FloatVectorData( range( 0, 10, 2 ) ) )
		self.assertEqual( points["d"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( points["e"].data, IECore.FloatVectorData( range( 0, 10, 2 ) ) )
		self.assertEqual( points["e"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)

	def testPrimitiveVariablesCorrectlyFilteredIfDeleteInverterd(self):
		points  = self.points()
		points["delete"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [0.0, 1.0] * 5) )

		self.assertTrue( points.arePrimitiveVariablesValid() )
		points = IECoreScene.PointsAlgo.deletePoints(points, points["delete"], invert=True)
		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertEqual( points.numPoints, 5 )
		self.assertEqual( points["delete"].data, IECore.FloatVectorData( [1.0] * 5 ) )
		self.assertEqual( points["delete"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( points["a"].data, IECore.FloatData( 0.5 ) )
		self.assertEqual( points["a"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( points["b"].data, IECore.FloatVectorData( range( 1, 11, 2 ) ) )
		self.assertEqual( points["b"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( points["c"].data, IECore.FloatVectorData( [0] ) )
		self.assertEqual( points["c"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( points["d"].data, IECore.FloatVectorData( range( 1, 11, 2 ) ) )
		self.assertEqual( points["d"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( points["e"].data, IECore.FloatVectorData( range( 1, 11, 2 ) ) )
		self.assertEqual( points["e"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying)


class MergePointsTest( unittest.TestCase ) :

	def testCanMergeTwoPointsPrimitivesWithNoPrimvars( self ) :
		pointsA = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 3 )] ) )
		pointsB = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 5, 9 )] ) )

		mergedPoints = IECoreScene.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( mergedPoints.numPoints, 3 + 4 )

		self.assertEqual( mergedPoints["P"].data[0], imath.V3f( 0 ) )
		self.assertEqual( mergedPoints["P"].data[1], imath.V3f( 1 ) )
		self.assertEqual( mergedPoints["P"].data[2], imath.V3f( 2 ) )

		self.assertEqual( mergedPoints["P"].data[3], imath.V3f( 5 ) )
		self.assertEqual( mergedPoints["P"].data[4], imath.V3f( 6 ) )
		self.assertEqual( mergedPoints["P"].data[5], imath.V3f( 7 ) )
		self.assertEqual( mergedPoints["P"].data[6], imath.V3f( 8 ) )

	def testCanMergeTwoPointsPrimitivesWithConstantPrimvars( self ) :
		pointsA = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 2 )] ) )
		pointsB = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 2 )] ) )

		pointsA["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.1 ) )
		pointsB["bar"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.2 ) )

		mergedPoints = IECoreScene.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( mergedPoints["foo"], IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.1 ) ) )
		self.assertEqual( mergedPoints["bar"], IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.2 ) ) )

	def testFirstConstantPrimvarIsTaken( self ) :
		pointsA = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 2 )] ) )
		pointsB = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 2 )] ) )

		pointsA["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		pointsB["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.6 ) )

		mergedPoints = IECoreScene.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( mergedPoints["foo"], IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) ) )

	def testRaisesExceptionIfSamePrimvarHasDifferentInterpolation( self ) :
		pointsA = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 2 )] ) )
		pointsB = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 2 )] ) )

		# constant then vertex)
		pointsA["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		pointsB["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [1, 2] ) )

		self.assertRaises( RuntimeError, lambda : IECoreScene.PointsAlgo.mergePoints( [pointsA, pointsB] ) )

		# swap the order (vertex then constant)
		pointsA["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [1, 2] ) )
		pointsB["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )

		self.assertRaises( RuntimeError, lambda : IECoreScene.PointsAlgo.mergePoints( [pointsA, pointsB] ) )

	def testMissingPrimvarIsExpanedToDefaultValue( self ) :
		pointsA = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )
		pointsB = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		pointsA["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [0, 1, 2, 3] ) )

		mergedPoints = IECoreScene.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( len( mergedPoints["foo"].data ), 8 )

		self.assertEqual( mergedPoints["foo"].data[0], 0 )
		self.assertEqual( mergedPoints["foo"].data[1], 1 )
		self.assertEqual( mergedPoints["foo"].data[2], 2 )
		self.assertEqual( mergedPoints["foo"].data[3], 3 )

		self.assertEqual( mergedPoints["foo"].data[4], 0 )
		self.assertEqual( mergedPoints["foo"].data[5], 0 )
		self.assertEqual( mergedPoints["foo"].data[6], 0 )
		self.assertEqual( mergedPoints["foo"].data[7], 0 )

	def testConvertsTypesIfPossible( self ) :
		pointsA = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )
		pointsB = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		pointsA["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [4, 5, 6, 7] ) )
		pointsB["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [0, 1, 2, 3] ) )

		mergedPoints = IECoreScene.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( mergedPoints["foo"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		self.assertIsInstance( mergedPoints["foo"].data, IECore.IntVectorData )
		self.assertEqual( len( mergedPoints["foo"].data ), 8 )

		self.assertEqual( mergedPoints["foo"].data[0], 4 )
		self.assertEqual( mergedPoints["foo"].data[1], 5 )
		self.assertEqual( mergedPoints["foo"].data[2], 6 )
		self.assertEqual( mergedPoints["foo"].data[3], 7 )

		self.assertEqual( mergedPoints["foo"].data[4], 0 )
		self.assertEqual( mergedPoints["foo"].data[5], 1 )
		self.assertEqual( mergedPoints["foo"].data[6], 2 )
		self.assertEqual( mergedPoints["foo"].data[7], 3 )

	def testRaisesExceptionIfTypesAreIncompatible( self ) :
		pointsA = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )
		pointsB = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		pointsA["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [4, 5, 6, 7] ) )
		pointsB["foo"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.StringVectorData( ["a", "b", "c", "d"] ) )

		self.assertRaises( RuntimeError, lambda : IECoreScene.PointsAlgo.mergePoints( [pointsA, pointsB] ) )


class SegmentPointsTest( unittest.TestCase ) :

	def testCanSegmentUsingIntegerPrimvar( self ) :
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		points["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [0, 0, 1, 1] ) )

		segmentValues = IECore.IntVectorData( [0, 1] )
		segments = IECoreScene.PointsAlgo.segment( points, points["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData([imath.V3f( 0 ), imath.V3f( 1 )] ) )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData([imath.V3f( 2 ), imath.V3f( 3 )] ) )

	def testCanSegmentUsingStringPrimvar( self ) :
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		points["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.StringVectorData( ["a", "b", "a", "b"] ) )

		segmentValues = IECore.StringVectorData( ["a", "b"] )
		segments = IECoreScene.PointsAlgo.segment( points, points["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData([imath.V3f( 0 ), imath.V3f( 2 )] ) )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData([imath.V3f( 1 ), imath.V3f( 3 )] ) )


	def testSegmentsFullyIfNoSegmentValuesGiven( self ) :
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		points["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.StringVectorData( ["a", "b", "a", "b"] ) )

		segments = IECoreScene.PointsAlgo.segment( points, points["s"] )

		self.assertEqual( len( segments ), 2 )

		if segments[0]["s"].data[0] == 'a':
			s0 = segments[0]
			s1 = segments[1]
		else:
			s0 = segments[1]
			s1 = segments[0]

		self.assertEqual( s0["P"].data, IECore.V3fVectorData([imath.V3f( 0 ), imath.V3f( 2 )] ) )
		self.assertEqual( s1["P"].data, IECore.V3fVectorData([imath.V3f( 1 ), imath.V3f( 3 )] ) )

	def testRaisesExceptionIfSegmentKeysNotSameTypeAsPrimvar( self ):
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		points["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.StringVectorData( ["a", "b", "a", "b"] ) )

		segmentValues = IECore.IntVectorData( [1, 2] )

		def t():
			segments = IECoreScene.PointsAlgo.segment( points, points["s"], segmentValues )

		self.assertRaises(RuntimeError, t)

	def testEmptyPrimitiveIfNotMatching( self ):
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		points["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.StringVectorData( ["a", "b", "a", "b"] ) )

		segmentValues = IECore.StringVectorData( ["e", "f"] )
		segments = IECoreScene.PointsAlgo.segment( points, points["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData() )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData() )

	def testSegmentSubset( self ):
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [imath.V3f( x ) for x in range( 0, 4 )] ) )

		points["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.StringVectorData( ["a", "b", "a", "b"] ) )

		segmentValues = IECore.StringVectorData( ["a"] )
		segments = IECoreScene.PointsAlgo.segment( points, points["s"], segmentValues )

		self.assertEqual( len( segments ), 1 )
		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [imath.V3f( 0 ), imath.V3f( 2 )] ) )
		self.assertEqual( segments[0]["s"].data, IECore.StringVectorData( ["a", "a" ] ) )


if __name__ == "__main__":
	unittest.main()
