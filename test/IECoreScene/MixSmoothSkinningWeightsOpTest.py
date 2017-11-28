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
import IECoreScene


class MixSmoothSkinningWeightsOpTest( unittest.TestCase ) :

	def createSSD( self, weights ) :

		names = IECore.StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
		poses = IECore.M44fVectorData( [IECore.M44f(1),IECore.M44f(2),IECore.M44f(3)] )
		offsets = IECore.IntVectorData( [0, 2, 5, 6] )
		counts = IECore.IntVectorData( [2, 3, 1, 2] )
		indices = IECore.IntVectorData( [0, 1, 0, 1, 2, 1, 1, 2] )

		ssd = IECoreScene.SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def original( self ) :

		weights = IECore.FloatVectorData( [0.7, 0.7, 0.2, 0.6, 0.2, 0.1, 1.2, 0.8] )

		return self.createSSD( weights )

	def toMix( self ) :

		weights = IECore.FloatVectorData( [0.5, 0.5, 0.25, 0.75, 0.2, 1.0, 0.6, 0.4] )

		return self.createSSD( weights )

	def mixed50_50_50( self ) :

		weights = IECore.FloatVectorData( [0.6, 0.6, 0.225, 0.675, 0.2, 0.55, 0.9, 0.6] )

		return self.createSSD( weights )

	def mixed75_75_75( self ) :

		weights = IECore.FloatVectorData( [0.65, 0.65, 0.2125, 0.6375, 0.2, 0.325, 1.05, 0.7] )

		return self.createSSD( weights )

	def mixed40_60_80( self ) :

		weights = IECore.FloatVectorData( [0.58, 0.62, 0.23, 0.66, 0.2, 0.46, 0.96, 0.72] )

		return self.createSSD( weights )

	def mixed0_50_100( self ) :

		weights = IECore.FloatVectorData( [0.5, 0.6, 0.25, 0.675, 0.2, 0.55, 0.9, 0.8] )

		return self.createSSD( weights )

	def testTypes( self ) :
		""" Test MixSmoothSkinningWeightsOp types"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		self.assertEqual( type(op), IECoreScene.MixSmoothSkinningWeightsOp )
		self.assertEqual( op.typeId(), IECoreScene.TypeId.MixSmoothSkinningWeightsOp )
		op.parameters()['input'].setValue( IECore.IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

	def testSelfMixing( self ) :
		""" Test MixSmoothSkinningWeightsOp by mixing with itself"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['skinningDataToMix'].setValue( ssd )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.5, 0.5, 0.5 ] ) )
		result = op.operate()

		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertEqual( result, ssd )

	def testMix50_50_50( self ) :
		""" Test MixSmoothSkinningWeightsOp with a 50-50 split between all weights"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['skinningDataToMix'].setValue( self.toMix() )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.5, 0.5, 0.5 ] ) )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		mixed = self.mixed50_50_50()

		self.assertEqual( result.influenceNames(), mixed.influenceNames() )
		self.assertEqual( result.influencePose(), mixed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), mixed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), mixed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), mixed.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		mixedWeights = mixed.pointInfluenceWeights()
		for i in range( 0, result.pointInfluenceWeights().size() ) :
			self.assertAlmostEqual( resultWeights[i], mixedWeights[i], 6 )

	def testMix75_75_75( self ) :
		""" Test MixSmoothSkinningWeightsOp with a 75-25 split between all weights"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['skinningDataToMix'].setValue( self.toMix() )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.75, 0.75, 0.75 ] ) )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		mixed = self.mixed75_75_75()

		self.assertEqual( result.influenceNames(), mixed.influenceNames() )
		self.assertEqual( result.influencePose(), mixed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), mixed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), mixed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), mixed.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		mixedWeights = mixed.pointInfluenceWeights()
		for i in range( 0, result.pointInfluenceWeights().size() ) :
			self.assertAlmostEqual( resultWeights[i], mixedWeights[i], 6 )

	def testMix40_60_80( self ) :
		""" Test MixSmoothSkinningWeightsOp with a mixed split between all weights"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['skinningDataToMix'].setValue( self.toMix() )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.4, 0.6, 0.8 ] ) )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		mixed = self.mixed40_60_80()

		self.assertEqual( result.influenceNames(), mixed.influenceNames() )
		self.assertEqual( result.influencePose(), mixed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), mixed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), mixed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), mixed.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		mixedWeights = mixed.pointInfluenceWeights()
		for i in range( 0, result.pointInfluenceWeights().size() ) :
			self.assertAlmostEqual( resultWeights[i], mixedWeights[i], 6 )

	def testLockedInput( self ) :
		""" Test MixSmoothSkinningWeightsOp with locked input weights"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['skinningDataToMix'].setValue( self.toMix() )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 1, 1, 1 ] ) )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		mixedWeights = ssd.pointInfluenceWeights()
		for i in range( 0, result.pointInfluenceWeights().size() ) :
			self.assertAlmostEqual( resultWeights[i], mixedWeights[i], 6 )

	def testLockedMixingData( self ) :
		""" Test MixSmoothSkinningWeightsOp with locked mixing weights"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['skinningDataToMix'].setValue( self.toMix() )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0, 0, 0 ] ) )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		mixed = self.toMix()

		self.assertEqual( result.influenceNames(), mixed.influenceNames() )
		self.assertEqual( result.influencePose(), mixed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), mixed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), mixed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), mixed.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		mixedWeights = mixed.pointInfluenceWeights()
		for i in range( 0, result.pointInfluenceWeights().size() ) :
			self.assertAlmostEqual( resultWeights[i], mixedWeights[i], 6 )

	def testMix0_50_100( self ) :
		""" Test MixSmoothSkinningWeightsOp with some mixed and some locked weights"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['skinningDataToMix'].setValue( self.toMix() )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0, 0.5, 1 ] ) )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		mixed = self.mixed0_50_100()

		self.assertEqual( result.influenceNames(), mixed.influenceNames() )
		self.assertEqual( result.influencePose(), mixed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), mixed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), mixed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), mixed.pointInfluenceIndices() )
		resultWeights = result.pointInfluenceWeights()
		mixedWeights = mixed.pointInfluenceWeights()
		for i in range( 0, result.pointInfluenceWeights().size() ) :
			self.assertAlmostEqual( resultWeights[i], mixedWeights[i], 6 )

	def testErrorStates( self ) :
		""" Test MixSmoothSkinningWeightsOp with the various error states"""

		ssd = self.original()

		op = IECoreScene.MixSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )

		# no data to mix
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.5, 0.5, 0.5 ] ) )
		self.assertRaises( RuntimeError, op.operate )

		# wrong number of mixing weights
		op.parameters()['skinningDataToMix'].setValue( self.toMix() )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.5, 0.5 ] ) )
		self.assertRaises( RuntimeError, op.operate )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.5, 0.5, 0.5, 0.5 ] ) )
		self.assertRaises( RuntimeError, op.operate )

		# wrong number of influences
		bad = IECoreScene.SmoothSkinningData( IECore.StringVectorData( [ 'jointA', 'jointB' ] ), ssd.influencePose(), ssd.pointIndexOffsets(), ssd.pointInfluenceCounts(), ssd.pointInfluenceIndices(), ssd.pointInfluenceWeights() )
		op.parameters()['skinningDataToMix'].setValue( bad )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.5, 0.5, 0.5 ] ) )
		self.assertRaises( RuntimeError, op.operate )
		bad = IECoreScene.SmoothSkinningData( IECore.StringVectorData( [ 'jointA', 'jointB', 'jointC', 'jointD' ] ), ssd.influencePose(), ssd.pointIndexOffsets(), ssd.pointInfluenceCounts(), ssd.pointInfluenceIndices(), ssd.pointInfluenceWeights() )
		op.parameters()['skinningDataToMix'].setValue( bad )
		self.assertRaises( RuntimeError, op.operate )

		# wrong number of points
		bad = IECoreScene.SmoothSkinningData( ssd.influenceNames(), ssd.influencePose(), IECore.IntVectorData( [0, 2, 5, 6, 8] ), ssd.pointInfluenceCounts(), ssd.pointInfluenceIndices(), ssd.pointInfluenceWeights() )
		op.parameters()['skinningDataToMix'].setValue( bad )
		op.parameters()['mixingWeights'].setValue( IECore.FloatVectorData( [ 0.5, 0.5, 0.5 ] ) )
		self.assertRaises( RuntimeError, op.operate )

if __name__ == "__main__":
	unittest.main()
