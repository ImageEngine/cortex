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


class RemoveSmoothSkinningInfluencesOpTest( unittest.TestCase ) :
	
	def createSSD( self, weights ) :
		
		names = StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
		poses = M44fVectorData( [M44f(1),M44f(2),M44f(3)] )
		offsets = IntVectorData( [0, 2, 5, 6, 8] )
		counts = IntVectorData( [2, 3, 1, 2, 3] )
		indices = IntVectorData( [0, 1, 0, 1, 2, 1, 1, 2, 0, 1, 2] )
		
		ssd = SmoothSkinningData( names, poses, offsets, counts, indices, weights )
		
		return ssd
	
	def original( self ) :
		
		weights = FloatVectorData( [0.7, 0.7, 0.2, 0.6, 0.0, 0.1, 1.2, 0.8, 0.4, 0.6, 0.4] )

		return self.createSSD( weights )
	
	def weightLimited( self ) :
		
		weights = FloatVectorData( [0.7, 0.7, 0.0, 0.6, 0.0, 0.0, 1.2, 0.8, 0.0, 0.6, 0.0] )

		return self.createSSD( weights )
	
	def maxInfluenced( self ) :
		
		weights = FloatVectorData( [0.7, 0.7, 0.2, 0.6, 0.0, 0.1, 1.2, 0.8, 0.0, 0.6, 0.4] )

		return self.createSSD( weights )
	
	def indexed( self ) :
		
		weights = FloatVectorData( [0.0, 0.7, 0.0, 0.6, 0.0, 0.1, 1.2, 0.0, 0.0, 0.6, 0.0] )

		return self.createSSD( weights )
	
	def compressedAfterIndexed( self ) :
		
		names = StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
		poses = M44fVectorData( [M44f(1),M44f(2),M44f(3)] )
		offsets = IntVectorData( [0, 1, 2, 2, 3] )
		counts = IntVectorData( [1, 1, 0, 1, 2] )
		indices = IntVectorData( [0, 0, 2, 0, 2] )
		weights = FloatVectorData( [0.7, 0.2, 0.8, 0.4, 0.4] )

		return SmoothSkinningData( names, poses, offsets, counts, indices, weights )
	
	def testTypes( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp types"""
		
		ssd = self.original()

		op = RemoveSmoothSkinningInfluencesOp()
		self.assertEqual( type(op), RemoveSmoothSkinningInfluencesOp )
		self.assertEqual( op.typeId(), TypeId.RemoveSmoothSkinningInfluencesOp )
		op.parameters()['input'].setValue( IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

	def testWeightLimitMode( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp in weight limit mode"""
		
		ssd = self.original()
		weightLimited = self.weightLimited()
		
		op = RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mode'].setValue( RemoveSmoothSkinningInfluencesOp.Mode.WeightLimit )
		op.parameters()['minWeight'].setValue( 0.401 )
		op.parameters()['compressResult'].setTypedValue( False )
		result = op.operate()
		
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )
		
		self.assertEqual( result.influenceNames(), weightLimited.influenceNames() )
		self.assertEqual( result.influencePose(), weightLimited.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), weightLimited.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), weightLimited.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), weightLimited.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), weightLimited.pointInfluenceWeights() )
		self.assertEqual( result, weightLimited )

	def testMaxInfluencesMode( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp in max influences mode"""
		
		ssd = self.original()
		maxInfluenced = self.maxInfluenced()
		
		op = RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mode'].setValue( RemoveSmoothSkinningInfluencesOp.Mode.MaxInfluences )
		op.parameters()['maxInfluences'].setValue( 2 )
		op.parameters()['compressResult'].setTypedValue( False )
		result = op.operate()
		
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )
		
		self.assertEqual( result.influenceNames(), maxInfluenced.influenceNames() )
		self.assertEqual( result.influencePose(), maxInfluenced.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), maxInfluenced.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), maxInfluenced.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), maxInfluenced.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), maxInfluenced.pointInfluenceWeights() )
		self.assertEqual( result, maxInfluenced )
	
	def testIndexedMode( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp in indexed mode"""
		
		ssd = self.original()
		indexed = self.indexed()
		
		op = RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mode'].setValue( RemoveSmoothSkinningInfluencesOp.Mode.Indexed )
		op.parameters()['influenceIndices'].setFrameListValue( FrameList.parse( "0-2x2" ) )
		op.parameters()['compressResult'].setTypedValue( False )
		result = op.operate()
		
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )
		
		self.assertEqual( result.influenceNames(), indexed.influenceNames() )
		self.assertEqual( result.influencePose(), indexed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), indexed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), indexed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), indexed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), indexed.pointInfluenceWeights() )
		self.assertEqual( result, indexed )
		
	def testCompressionParameter( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp in indexed mode with compression on"""
		
		ssd = self.original()
		compressedAfterIndexed = self.compressedAfterIndexed()
		
		op = RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mode'].setValue( RemoveSmoothSkinningInfluencesOp.Mode.Indexed )
		op.parameters()['influenceIndices'].setFrameListValue( FrameList.parse( "1" ) )
		op.parameters()['compressResult'].setTypedValue( True )
		result = op.operate()
		
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )
		
		self.assertEqual( result.influenceNames(), compressedAfterIndexed.influenceNames() )
		self.assertEqual( result.influencePose(), compressedAfterIndexed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), compressedAfterIndexed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), compressedAfterIndexed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), compressedAfterIndexed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), compressedAfterIndexed.pointInfluenceWeights() )
		self.assertEqual( result, compressedAfterIndexed )

if __name__ == "__main__":
	unittest.main()
