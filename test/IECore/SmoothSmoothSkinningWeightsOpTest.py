##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
import random
import IECore


class SmoothSmoothSkinningWeightsOpTest( unittest.TestCase ) :

	def mesh( self ) :

		vertsPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [
			0, 1, 3, 2, 2, 3, 5, 4, 4, 5, 7, 6, 6, 7, 9, 8,
			8, 9, 11, 10, 10, 11, 13, 12, 12, 13, 15, 14, 14, 15, 1, 0,
			1, 15, 13, 3, 3, 13, 11, 5, 5, 11, 9, 7, 14, 0, 2, 12,
			12, 2, 4, 10, 10, 4, 6, 8
		] )

		return IECore.MeshPrimitive( vertsPerFace, vertexIds )

	def createSSD( self, offsets, counts, indices, weights ) :

		names = IECore.StringVectorData( [ "|joint1", "|joint1|joint2", "|joint1|joint2|joint3" ] )
		poses = IECore.M44fVectorData( [
			IECore.M44f( 1, -0, 0, -0, -0, 1, -0, 0, 0, -0, 1, -0, -0, 2, -0, 1 ),
			IECore.M44f( 1, -0, 0, -0, -0, 1, -0, 0, 0, -0, 1, -0, -0, 0, -0, 1 ),
			IECore.M44f( 1, -0, 0, -0, -0, 1, -0, 0, 0, -0, 1, -0, -0, -2, -0, 1 )
		] )

		return IECore.SmoothSkinningData( names, poses, offsets, counts, indices, weights )

	def original( self ) :

		offsets = IECore.IntVectorData( [ 0, 1, 2, 4, 6, 7, 8, 10, 12, 14, 16, 17, 18, 20, 22, 23 ] )
		counts = IECore.IntVectorData( [ 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1 ] )
		indices = IECore.IntVectorData( [ 0, 0, 0, 1, 0, 1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 0, 1, 0, 1, 0, 0 ] )

		weights = IECore.FloatVectorData( [
			1, 1, 0.8, 0.2, 0.8, 0.2, 1, 1, 0.5, 0.5, 0.5, 0.5,
			0.5, 0.5, 0.5, 0.5, 1, 1, 0.8, 0.2, 0.8, 0.2, 1, 1
		] )

		return self.createSSD( offsets, counts, indices, weights )

	def smooth1_50( self ) :

		offsets = IECore.IntVectorData( [ 0, 2, 4, 6, 8, 11, 14, 16, 18, 20, 22, 25, 28, 30, 32, 34 ] )
		counts = IECore.IntVectorData( [ 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2 ] )
		indices = IECore.IntVectorData( [
			0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 0, 1, 2, 1, 2, 1, 2,
			1, 2, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 0, 1, 0, 1, 0, 1
		] )

		weights = IECore.FloatVectorData( [
			0.966667, 0.0333333, 0.966667, 0.0333333, 0.725, 0.275, 0.725, 0.275,
			0.1, 0.8375, 0.0625, 0.1, 0.8375, 0.0625, 0.583333, 0.416667,
			0.583333, 0.416667, 0.583333, 0.416667, 0.583333, 0.416667, 0.1, 0.8375,
			0.0625, 0.1, 0.8375, 0.0625, 0.725, 0.275, 0.725, 0.275,
			0.966667, 0.0333333, 0.966667, 0.0333333
		] )

		return self.createSSD( offsets, counts, indices, weights )

	def smooth1_100( self ) :

		offsets = IECore.IntVectorData( [ 0, 2, 4, 6, 8, 11, 14, 16, 18, 20, 22, 25, 28, 30, 32, 34 ] )
		counts = IECore.IntVectorData( [ 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2 ] )
		indices = IECore.IntVectorData( [
			0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 0, 1, 2, 1, 2, 1, 2,
			1, 2, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 0, 1, 0, 1, 0, 1
		] )

		weights = IECore.FloatVectorData( [
			0.933333, 0.0666667, 0.933333, 0.0666667, 0.65, 0.35, 0.65, 0.35,
			0.2, 0.675, 0.125, 0.2, 0.675, 0.125, 0.666667, 0.333333,
			0.666667, 0.333333, 0.666667, 0.333333, 0.666667, 0.333333, 0.2, 0.675,
			0.125, 0.2, 0.675, 0.125, 0.65, 0.35, 0.65, 0.35,
			0.933333, 0.0666667, 0.933333, 0.0666667
		] )

		return self.createSSD( offsets, counts, indices, weights )

	def smooth3_30( self ) :

		offsets = IECore.IntVectorData( [ 0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45 ] )
		counts = IECore.IntVectorData( [ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 ] )
		indices = IECore.IntVectorData( [
			0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
			0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2
		] )

		weights = IECore.FloatVectorData( [
			0.933725, 0.0659938, 0.00028125, 0.933725, 0.0659938, 0.00028125, 0.691672, 0.301016,
			0.0073125, 0.691672, 0.301016, 0.0073125, 0.145912, 0.767439, 0.0866484, 0.145912,
			0.767439, 0.0866484, 0.0161625, 0.6094, 0.374438, 0.0161625, 0.6094, 0.374438,
			0.0161625, 0.6094, 0.374438, 0.0161625, 0.6094, 0.374438, 0.145912, 0.767439,
			0.0866484, 0.145912, 0.767439, 0.0866484, 0.691672, 0.301016, 0.0073125, 0.691672,
			0.301016, 0.0073125, 0.933725, 0.0659938, 0.00028125, 0.933725, 0.0659938, 0.00028125
		] )

		return self.createSSD( offsets, counts, indices, weights )

	def smoothSelectVerts( self ) :

		offsets = IECore.IntVectorData( [ 0, 1, 2, 4, 6, 9, 10, 12, 14, 16, 18, 21, 24, 26, 28, 29 ] )
		counts = IECore.IntVectorData( [ 1, 1, 2, 2, 3, 1, 2, 2, 2, 2, 3, 3, 2, 2, 1, 1 ] )
		indices = IECore.IntVectorData( [
			0, 0, 0, 1, 0, 1, 0, 1, 2, 1, 1, 2, 1, 2, 1, 2,
			1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 0, 1, 0, 0
		] )

		weights = IECore.FloatVectorData( [
			1, 1, 0.725, 0.275, 0.725, 0.275, 0.1, 0.8375, 0.0625,
			1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.1,
			0.8375, 0.0625, 0.1, 0.8375, 0.0625, 0.725, 0.275, 0.8, 0.2, 1, 1
		] )

		return self.createSSD( offsets, counts, indices, weights )

	def smoothWithLocks( self ) :

		offsets = IECore.IntVectorData( [ 0, 1, 2, 5, 8, 10, 12, 14, 16, 18, 20, 22, 24, 27, 30, 31 ] )
		counts = IECore.IntVectorData( [ 1, 1, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 1, 1 ] )
		indices = IECore.IntVectorData( [
			0, 0, 0, 1, 2, 0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
			1, 2, 1, 2, 1, 2, 1, 2, 0, 1, 2, 0, 1, 2, 0, 0
		] )

		weights = IECore.FloatVectorData( [
			1, 1, 0.8, 0.193898, 0.00610161, 0.8, 0.193898, 0.00610161,
			0.902086, 0.0979137, 0.902086, 0.0979137, 0.624712, 0.375288, 0.624712, 0.375288,
			0.624712, 0.375288, 0.624712, 0.375288, 0.902086, 0.0979137, 0.902086, 0.0979137,
			0.8, 0.193898, 0.00610161, 0.8, 0.193898, 0.00610161, 1, 1
		] )

		return self.createSSD( offsets, counts, indices, weights )

	def testTypes( self ) :
		""" Test SmoothSmoothSkinningWeightsOp types"""

		ssd = self.original()

		op = IECore.SmoothSmoothSkinningWeightsOp()
		self.assertEqual( type(op), IECore.SmoothSmoothSkinningWeightsOp )
		self.assertEqual( op.typeId(), IECore.TypeId.SmoothSmoothSkinningWeightsOp )
		op.parameters()['input'].setValue( IECore.IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

	def testSmooth1_0( self ) :
		""" Test SmoothSmoothSkinningWeightsOp with 1 iteration and 0.0 smooth-ratio"""

		ssd = self.original()
		op = IECore.SmoothSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['smoothingRatio'].setValue( 0.0 )
		op.parameters()['iterations'].setValue( 1 )
		op.parameters()['applyLocks'].setValue( False )

		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertEqual( result, ssd )

	def testSmooth1_100( self ) :
		""" Test SmoothSmoothSkinningWeightsOp with 1 iteration and 1.0 smooth-ratio"""

		ssd = self.original()
		op = IECore.SmoothSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['smoothingRatio'].setValue( 1.0 )
		op.parameters()['iterations'].setValue( 1 )
		op.parameters()['applyLocks'].setValue( False )

		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		smooth = self.smooth1_100()
		self.assertEqual( result.influenceNames(), smooth.influenceNames() )
		self.assertEqual( result.influencePose(), smooth.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), smooth.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), smooth.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), smooth.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		smoothWeights = smooth.pointInfluenceWeights()
		for i in range( 0, resultWeights.size() ) :
			self.assertAlmostEqual( resultWeights[i], smoothWeights[i], 6 )

	def testSmooth1_50( self ) :
		""" Test SmoothSmoothSkinningWeightsOp with 1 iteration and 0.5 smooth-ratio"""

		ssd = self.original()
		op = IECore.SmoothSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['smoothingRatio'].setValue( 0.5 )
		op.parameters()['iterations'].setValue( 1 )
		op.parameters()['applyLocks'].setValue( False )

		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		smooth = self.smooth1_50()
		self.assertEqual( result.influenceNames(), smooth.influenceNames() )
		self.assertEqual( result.influencePose(), smooth.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), smooth.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), smooth.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), smooth.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		smoothWeights = smooth.pointInfluenceWeights()
		for i in range( 0, resultWeights.size() ) :
			self.assertAlmostEqual( resultWeights[i], smoothWeights[i], 6 )

	def testSmooth3_30( self ) :
		""" Test SmoothSmoothSkinningWeightsOp with 3 iterations and 0.3 smooth-ratio"""

		ssd = self.original()
		op = IECore.SmoothSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['smoothingRatio'].setValue( 0.3 )
		op.parameters()['iterations'].setValue( 3 )
		op.parameters()['applyLocks'].setValue( False )

		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		smooth = self.smooth3_30()
		self.assertEqual( result.influenceNames(), smooth.influenceNames() )
		self.assertEqual( result.influencePose(), smooth.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), smooth.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), smooth.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), smooth.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		smoothWeights = smooth.pointInfluenceWeights()
		for i in range( 0, resultWeights.size() ) :
			self.assertAlmostEqual( resultWeights[i], smoothWeights[i], 6 )

	def testLocks( self ) :
		""" Test SmoothSmoothSkinningWeightsOp locking mechanism"""

		ssd = self.original()
		op = IECore.SmoothSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['smoothingRatio'].setValue( 0.3 )
		op.parameters()['iterations'].setValue( 3 )
		op.parameters()['applyLocks'].setValue( True )
		op.parameters()['influenceLocks'].setValue( IECore.BoolVectorData( [ True, False, False ] ) )

		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		smooth = self.smoothWithLocks()
		self.assertEqual( result.influenceNames(), smooth.influenceNames() )
		self.assertEqual( result.influencePose(), smooth.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), smooth.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), smooth.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), smooth.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		smoothWeights = smooth.pointInfluenceWeights()
		for i in range( 0, resultWeights.size() ) :
			self.assertAlmostEqual( resultWeights[i], smoothWeights[i], 6 )

		# make sure locked weights did not change
		dop = IECore.DecompressSmoothSkinningDataOp()
		dop.parameters()['input'].setValue( result )
		decompressedResult = dop.operate()
		dop.parameters()['input'].setValue( ssd )
		decompressedOrig = dop.operate()
		resultIndices = decompressedResult.pointInfluenceIndices()
		resultWeights = decompressedResult.pointInfluenceWeights()
		origWeights = decompressedOrig.pointInfluenceWeights()
		for i in range( 0, resultWeights.size() ) :
			if resultIndices[i] == 0 :
				self.assertAlmostEqual( resultWeights[i], origWeights[i], 6 )

		# make sure the result is normalized
		nop = IECore.NormalizeSmoothSkinningWeightsOp()
		nop.parameters()['input'].setValue( result )
		normalized = nop.operate()
		self.assertEqual( result.influenceNames(), normalized.influenceNames() )
		self.assertEqual( result.influencePose(), normalized.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), normalized.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), normalized.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), normalized.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		normalizedWeights = normalized.pointInfluenceWeights()
		for i in range( 0, resultWeights.size() ) :
			self.assertAlmostEqual( resultWeights[i], normalizedWeights[i], 6 )

	def testVertexSelection( self ) :
		""" Test SmoothSmoothSkinningWeightsOp using selected vertices"""

		ssd = self.original()
		op = IECore.SmoothSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['smoothingRatio'].setValue( 0.5 )
		op.parameters()['iterations'].setValue( 1 )
		op.parameters()['applyLocks'].setValue( False )
		op.parameters()['vertexIndices'].setFrameListValue( IECore.FrameList.parse( "2-4,10-12" ) )

		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		smooth = self.smoothSelectVerts()
		self.assertEqual( result.influenceNames(), smooth.influenceNames() )
		self.assertEqual( result.influencePose(), smooth.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), smooth.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), smooth.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), smooth.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		smoothWeights = smooth.pointInfluenceWeights()
		for i in range( 0, resultWeights.size() ) :
			self.assertAlmostEqual( resultWeights[i], smoothWeights[i], 6 )

		# make sure only selected vertices changed
		dop = IECore.DecompressSmoothSkinningDataOp()
		dop.parameters()['input'].setValue( result )
		decompressedResult = dop.operate()
		dop.parameters()['input'].setValue( ssd )
		decompressedOrig = dop.operate()
		resultOffsets = decompressedResult.pointIndexOffsets()
		resultCounts = decompressedResult.pointInfluenceCounts()
		resultIndices = decompressedResult.pointInfluenceIndices()
		resultWeights = decompressedResult.pointInfluenceWeights()
		origWeights = decompressedOrig.pointInfluenceWeights()
		nonSelectedVerts = [ x for x in range( 0, resultOffsets.size() ) if x not in op.parameters()['vertexIndices'].getFrameListValue().asList() ]
		for i in nonSelectedVerts :
			for j in range( 0, resultCounts[i] ) :
				current = resultOffsets[i] + j
				self.assertAlmostEqual( resultWeights[current], origWeights[current], 6 )

	def testErrorStates( self ) :
		""" Test SmoothSmoothSkinningWeightsOp with various error states"""

		ssd = self.original()
		op = IECore.SmoothSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )

		# bad mesh
		op.parameters()['mesh'].setValue( IECore.IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

		# wrong number of verts
		op.parameters()['mesh'].setValue( op.parameters()['mesh'].defaultValue )
		self.assertRaises( RuntimeError, op.operate )

		# wrong number of locks
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['applyLocks'].setValue( True )
		op.parameters()['influenceLocks'].setValue( IECore.BoolVectorData( [ True, False, True, False ] ) )
		self.assertRaises( RuntimeError, op.operate )

		# invalid vertex ids
		op.parameters()['applyLocks'].setValue( False )
		op.parameters()['vertexIndices'].setFrameListValue( IECore.FrameList.parse( "10-18" ) )
		self.assertRaises( RuntimeError, op.operate )

if __name__ == "__main__":
	unittest.main()
