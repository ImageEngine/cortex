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

class PointsAlgoTest( unittest.TestCase ) :

	def points( self ) :

		testObject = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( x ) for x in range( 0, 10 ) ] ) )

		testObject["a"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["c"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0 ] ) )
		testObject["d"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["e"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 10 ) ) )

		# indexed
		testObject["f"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 ] ) )
		testObject["g"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0.5 ] ), IECore.IntVectorData( [ 0 ] ) )
		testObject["h"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 ] ) )
		testObject["i"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 3 ) ), IECore.IntVectorData( [ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 ] ) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testPointsConstantToVertex( self ) :
		points = self.points()
		p = points["a"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsConstantToUniform( self ) :
		points = self.points()
		p = points["a"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )

	def testPointsConstantToVarying( self ) :
		points = self.points()
		p = points["a"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsConstantToFaceVarying( self ) :
		points = self.points()
		p = points["a"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsVertexToConstant( self ) :
		points = self.points()
		p = points["b"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsVertexToUniform( self ) :
		points = self.points()
		p = points["b"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsVertexToVarying( self ) :
		points = self.points()
		p = points["b"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsVertexToFaceVarying( self ) :
		points = self.points()
		p = points["b"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsUniformToConstant( self ) :
		points = self.points()
		p = points["c"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( 0 ) )

	def testPointsUniformToVertex( self ) :
		points = self.points()
		p = points["c"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsUniformToVarying( self ) :
		points = self.points()
		p = points["c"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsUniformToFaceVarying( self ) :
		points = self.points()
		p = points["c"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsVaryingToConstant( self ) :
		points = self.points()
		p = points["d"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsVaryingToVertex( self ) :
		points = self.points()
		p = points["d"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsVaryingToUniform( self ) :
		points = self.points()
		p = points["d"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsVaryingToFaceVarying( self ) :
		points = self.points()
		p = points["d"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsFaceVaryingToConstant( self ) :
		points = self.points()
		p = points["e"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsFaceVaryingToVertex( self ) :
		points = self.points()
		p = points["e"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsFaceVaryingToUniform( self ) :
		points = self.points()
		p = points["e"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsFaceVaryingToVarying( self ) :
		points = self.points()
		p = points["e"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsIndexedVertexToUniform( self ) :
		points = self.points()
		p = points["f"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.9 ] ) )
		self.assertEqual( p.indices, None )

	def testPointsIndexedUniformToVertex( self ) :
		points = self.points()
		p = points["g"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0 ] * 10 ) )

	def testPointsIndexedUniformToVarying( self ) :
		points = self.points()
		p = points["g"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0 ] * 10 ) )

	def testPointsIndexedUniformToFaceVarying( self ) :
		points = self.points()
		p = points["g"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0 ] * 10 ) )

	def testPointsIndexedVaryingToUniform( self ) :
		points = self.points()
		p = points["h"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.9 ] ) )
		self.assertEqual( p.indices, None )

	def testPointsIndexedFaceVaryingToUniform( self ) :
		points = self.points()
		p = points["i"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.9 ] ) )
		self.assertEqual( p.indices, None )

class DeletePointsTest( unittest.TestCase ) :
	def points( self ) :

		testObject = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( x ) for x in range( 0, 10 ) ] ) )

		testObject["a"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["c"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0 ] ) )
		testObject["d"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["e"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 10 ) ) )

		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testRaisesExceptionIfPrimitiveVariableTypeIsInvalid(self):
		points  = self.points()
		points["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( x ) for x in range( 0, 10 ) ] ) )

		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertRaises( RuntimeError, lambda : IECore.PointsAlgo.deletePoints(points, points["delete"]) )

	def testRaisesExceptionIfPrimitiveVariableInterpolationIncorrect(self):
		points  = self.points()
		points["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0 ]  ) )

		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertRaises( RuntimeError, lambda : IECore.PointsAlgo.deletePoints(points, points["delete"]) )

	def testCanDeleteAllPoints(self):
		points  = self.points()
		points["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.BoolVectorData( [True] * 10 ) )

		self.assertTrue( points.arePrimitiveVariablesValid() )

		points = IECore.PointsAlgo.deletePoints(points, points["delete"])

		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertEqual( points.numPoints, 0 )
		self.assertEqual( points["delete"].data, IECore.BoolVectorData() )

	def testPrimitiveVariablesCorrectlyFiltered(self):
		points  = self.points()
		points["delete"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [0.0, 1.0] * 5) )

		self.assertTrue( points.arePrimitiveVariablesValid() )
		points = IECore.PointsAlgo.deletePoints(points, points["delete"])
		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertEqual( points.numPoints, 5 )
		self.assertEqual( points["delete"].data, IECore.FloatVectorData( [0.0] * 5 ) )
		self.assertEqual( points["delete"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( points["a"].data, IECore.FloatData( 0.5 ) )
		self.assertEqual( points["a"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant)

		self.assertEqual( points["b"].data, IECore.FloatVectorData( range( 0, 10, 2 ) ) )
		self.assertEqual( points["b"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex)

		self.assertEqual( points["c"].data, IECore.FloatVectorData( [0] ) )
		self.assertEqual( points["c"].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform)

		self.assertEqual( points["d"].data, IECore.FloatVectorData( range( 0, 10, 2 ) ) )
		self.assertEqual( points["d"].interpolation, IECore.PrimitiveVariable.Interpolation.Varying)

		self.assertEqual( points["e"].data, IECore.FloatVectorData( range( 0, 10, 2 ) ) )
		self.assertEqual( points["e"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying)


class MergePointsTest( unittest.TestCase ) :

	def testCanMergeTwoPointsPrimitivesWithNoPrimvars( self ) :
		pointsA = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 0, 3 )] ) )
		pointsB = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 5, 9 )] ) )

		mergedPoints = IECore.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( mergedPoints.numPoints, 3 + 4 )

		self.assertEqual( mergedPoints["P"].data[0], IECore.V3f( 0 ) )
		self.assertEqual( mergedPoints["P"].data[1], IECore.V3f( 1 ) )
		self.assertEqual( mergedPoints["P"].data[2], IECore.V3f( 2 ) )

		self.assertEqual( mergedPoints["P"].data[3], IECore.V3f( 5 ) )
		self.assertEqual( mergedPoints["P"].data[4], IECore.V3f( 6 ) )
		self.assertEqual( mergedPoints["P"].data[5], IECore.V3f( 7 ) )
		self.assertEqual( mergedPoints["P"].data[6], IECore.V3f( 8 ) )

	def testCanMergeTwoPointsPrimitivesWithConstantPrimvars( self ) :
		pointsA = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 2 )] ) )
		pointsB = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 2 )] ) )

		pointsA["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.1 ) )
		pointsB["bar"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.2 ) )

		mergedPoints = IECore.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( mergedPoints["foo"], IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.1 ) ) )
		self.assertEqual( mergedPoints["bar"], IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.2 ) ) )

	def testFirstConstantPrimvarIsTaken( self ) :
		pointsA = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 2 )] ) )
		pointsB = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 2 )] ) )

		pointsA["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		pointsB["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.6 ) )

		mergedPoints = IECore.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( mergedPoints["foo"], IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) ) )

	def testRaisesExceptionIfSamePrimvarHasDifferentInterpolation( self ) :
		pointsA = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 2 )] ) )
		pointsB = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 2 )] ) )

		# constant then vertex)
		pointsA["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		pointsB["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [1, 2] ) )

		self.assertRaises( RuntimeError, lambda : IECore.PointsAlgo.mergePoints( [pointsA, pointsB] ) )

		# swap the order (vertex then constant)
		pointsA["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [1, 2] ) )
		pointsB["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )

		self.assertRaises( RuntimeError, lambda : IECore.PointsAlgo.mergePoints( [pointsA, pointsB] ) )

	def testMissingPrimvarIsExpanedToDefaultValue( self ) :
		pointsA = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 0, 4 )] ) )
		pointsB = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 0, 4 )] ) )

		pointsA["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [0, 1, 2, 3] ) )

		mergedPoints = IECore.PointsAlgo.mergePoints( [pointsA, pointsB] )

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
		pointsA = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 0, 4 )] ) )
		pointsB = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 0, 4 )] ) )

		pointsA["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [4, 5, 6, 7] ) )
		pointsB["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [0, 1, 2, 3] ) )

		mergedPoints = IECore.PointsAlgo.mergePoints( [pointsA, pointsB] )

		self.assertEqual( mergedPoints["foo"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )

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
		pointsA = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 0, 4 )] ) )
		pointsB = IECore.PointsPrimitive( IECore.V3fVectorData( [IECore.V3f( x ) for x in range( 0, 4 )] ) )

		pointsA["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [4, 5, 6, 7] ) )
		pointsB["foo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.StringVectorData( ["a", "b", "c", "d"] ) )

		self.assertRaises( RuntimeError, lambda : IECore.PointsAlgo.mergePoints( [pointsA, pointsB] ) )


if __name__ == "__main__":
	unittest.main()
