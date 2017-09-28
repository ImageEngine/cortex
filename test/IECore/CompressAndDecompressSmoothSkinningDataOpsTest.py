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


class CompressAndDecompressSmoothSkinningDataOpsTest( unittest.TestCase ) :

	def compressed( self ) :

		names = StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
        	poses = M44fVectorData( [M44f(),M44f(),M44f()] )
        	offsets = IntVectorData( [0, 2, 4] )
        	counts = IntVectorData( [2, 2, 1] )
        	indices = IntVectorData( [0, 1, 0, 1, 1] )
        	weights = FloatVectorData( [0.5, 0.5, 0.2, 0.8, 1.0] )

		ssd = SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def noncompressed( self ) :

		names = StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
        	poses = M44fVectorData( [M44f(),M44f(),M44f()] )
        	offsets = IntVectorData( [0, 2, 5] )
        	counts = IntVectorData( [2, 3, 1] )
        	indices = IntVectorData( [0, 1, 0, 1, 2, 1] )
        	weights = FloatVectorData( [0.5, 0.5, 0.2, 0.8, 0.0, 1.0] )

		ssd = SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def decompressed( self ) :

		names = StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
        	poses = M44fVectorData( [M44f(),M44f(),M44f()] )
        	offsets = IntVectorData( [0, 3, 6] )
        	counts = IntVectorData( [3, 3, 3] )
        	indices = IntVectorData( [0, 1, 2, 0, 1, 2, 0, 1, 2] )
        	weights = FloatVectorData( [0.5, 0.5, 0.0, 0.2, 0.8, 0.0, 0.0, 1.0, 0.0] )

		ssd = SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def testTypes( self ) :
		""" Test CompressSmoothSkinningDataOp and DecompressSmoothSkinningDataOp types"""

		ssd = self.compressed()

		op = CompressSmoothSkinningDataOp()
		self.assertEqual( type(op), CompressSmoothSkinningDataOp )
		self.assertEqual( op.typeId(), TypeId.CompressSmoothSkinningDataOp )
		op.parameters()['input'].setValue( IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

		op = DecompressSmoothSkinningDataOp()
		self.assertEqual( type(op), DecompressSmoothSkinningDataOp )
		self.assertEqual( op.typeId(), TypeId.DecompressSmoothSkinningDataOp )
		op.parameters()['input'].setValue( IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

	def testCompressingCompressed( self ) :
		""" Test CompressSmoothSkinningDataOp with compressed SmoothSkinningData"""

		ssd = self.compressed()

		op = CompressSmoothSkinningDataOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()

		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertEqual( result, ssd )

	def testCompressingNoncompressed( self ) :
		""" Test CompressSmoothSkinningDataOp with non-compressed SmoothSkinningData"""

		ssd = self.noncompressed()

		op = CompressSmoothSkinningDataOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()
		self.assertNotEqual( result, ssd )

		compressed = self.compressed()
		self.assertEqual( result.pointIndexOffsets(), compressed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), compressed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), compressed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), compressed.pointInfluenceWeights() )
		self.assertEqual( result, compressed )


	def testCompressingDecompressed( self ) :
		""" Test CompressSmoothSkinningDataOp with decompressed SmoothSkinningData"""

		ssd = self.decompressed()

		op = CompressSmoothSkinningDataOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()
		self.assertNotEqual( result, ssd )

		compressed = self.compressed()
		self.assertEqual( result.pointIndexOffsets(), compressed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), compressed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), compressed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), compressed.pointInfluenceWeights() )
		self.assertEqual( result, compressed )

	def testCompressingWithThreshold( self ) :
		""" Test CompressSmoothSkinningDataOp threshold parameter"""

		ssd = self.noncompressed()

		op = CompressSmoothSkinningDataOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['threshold'].setNumericValue( 0.5 )
		result = op.operate()
		self.assertNotEqual( result, ssd )

		self.assertNotEqual( result, self.compressed() )

		for val in result.pointInfluenceWeights() :
			self.assert_( val > 0.5 )

		self.assertEqual( result.pointIndexOffsets(), IntVectorData( [0, 0, 1] ) )
		self.assertEqual( result.pointInfluenceCounts(), IntVectorData( [0, 1, 1] ) )
		self.assertEqual( result.pointInfluenceIndices(), IntVectorData( [1, 1] ) )
		self.assertEqual( result.pointInfluenceWeights(), FloatVectorData( [0.8, 1.0] ) )

		decompressed = self.decompressed()
		op.parameters()['input'].setValue( decompressed )
		result2 = op.operate()
		self.assertNotEqual( result2, decompressed )
		self.assertEqual( result2, result )

	def testDecompressingCompressed( self ) :
		""" Test DeompressSmoothSkinningDataOp with compressed SmoothSkinningData"""

		ssd = self.compressed()

		op = DecompressSmoothSkinningDataOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()
		self.assertNotEqual( result, ssd )

		decompressed = self.decompressed()
		self.assertEqual( result.pointIndexOffsets(), decompressed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), decompressed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), decompressed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), decompressed.pointInfluenceWeights() )
		self.assertEqual( result, decompressed )

	def testDecompressingNoncompressed( self ) :
		""" Test DeompressSmoothSkinningDataOp with compressed SmoothSkinningData"""

		ssd = self.noncompressed()

		op = DecompressSmoothSkinningDataOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()
		self.assertNotEqual( result, ssd )

		decompressed = self.decompressed()
		self.assertEqual( result.pointIndexOffsets(), decompressed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), decompressed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), decompressed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), decompressed.pointInfluenceWeights() )
		self.assertEqual( result, decompressed )

	def testDecompressingDecompressed( self ) :
		""" Test DeompressSmoothSkinningDataOp """

		ssd = self.decompressed()

		op = DecompressSmoothSkinningDataOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()

		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertEqual( result, ssd )

if __name__ == "__main__":
    unittest.main()
