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
from IECore import *


class NormalizeSmoothSkinningWeightsOpTest( unittest.TestCase ) :

	def createSSD( self, weights ) :

		names = StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
		poses = M44fVectorData( [M44f(1),M44f(2),M44f(3)] )
		offsets = IntVectorData( [0, 2, 5, 6] )
		counts = IntVectorData( [2, 3, 1, 2] )
		indices = IntVectorData( [0, 1, 0, 1, 2, 1, 1, 2] )

		ssd = SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def original( self ) :

		weights = FloatVectorData( [0.7, 0.7, 0.2, 0.6, 0.0, 0.1, 1.2, 0.8] )

		return self.createSSD( weights )

	def normalized( self ) :

		weights = FloatVectorData( [0.5, 0.5, 0.25, 0.75, 0.0, 1.0, 0.6, 0.4] )

		return self.createSSD( weights )

	def normalizedWithLocks( self ) :

		weights = FloatVectorData( [0.7, 0.3, 0.2, 0.8, 0.0, 1.0, 0.6, 0.4] )

		return self.createSSD( weights )

	def testTypes( self ) :
		""" Test NormalizeSmoothSkinningWeightsOp types"""

		ssd = self.original()

		op = NormalizeSmoothSkinningWeightsOp()
		self.assertEqual( type(op), NormalizeSmoothSkinningWeightsOp )
		self.assertEqual( op.typeId(), TypeId.NormalizeSmoothSkinningWeightsOp )
		op.parameters()['input'].setValue( IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

	def testNormalizingNormalized( self ) :
		""" Test NormalizeSmoothSkinningWeightsOp with weights which are already normalized"""

		ssd = self.normalized()

		op = NormalizeSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()

		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertEqual( result, ssd )

	def testNormalizingUnnormalized( self ) :
		""" Test NormalizeSmoothSkinningWeightsOp with unnormalized weights"""

		ssd = self.original()

		op = NormalizeSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		normalized = self.normalized()

		self.assertEqual( result.influenceNames(), normalized.influenceNames() )
		self.assertEqual( result.influencePose(), normalized.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), normalized.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), normalized.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), normalized.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), normalized.pointInfluenceWeights() )
		self.assertEqual( result, normalized )

	def testLocks( self ) :
		""" Test NormalizeSmoothSkinningWeightsOp locking mechanism"""

		ssd = self.original()

		op = NormalizeSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['applyLocks'].setValue( True )
		op.parameters()['influenceLocks'].setValue( BoolVectorData( [ True, False, False ] ) )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		normalized = self.normalizedWithLocks()

		self.assertEqual( result.influenceNames(), normalized.influenceNames() )
		self.assertEqual( result.influencePose(), normalized.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), normalized.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), normalized.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), normalized.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), normalized.pointInfluenceWeights() )
		self.assertEqual( result, normalized )

		# make sure locked weights did not change
		dop = DecompressSmoothSkinningDataOp()
		dop.parameters()['input'].setValue( result )
		decompressedResult = dop.operate()
		dop.parameters()['input'].setValue( ssd )
		decompressedOrig = dop.operate()
		resultIndices = decompressedResult.pointInfluenceIndices()
		resultWeights = decompressedResult.pointInfluenceWeights()
		origWeights = decompressedOrig.pointInfluenceWeights()
		for i in range( 0, resultWeights.size() ) :
			if resultIndices[i] == 0 :
				self.assertEqual( resultWeights[i], origWeights[i] )

if __name__ == "__main__":
	unittest.main()
