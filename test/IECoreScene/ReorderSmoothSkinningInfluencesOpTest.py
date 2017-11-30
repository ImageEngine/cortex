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


class ReorderSmoothSkinningInfluencesOpTest( unittest.TestCase ) :

	def createSSD( self, names, poses, indices ) :

		offsets = IECore.IntVectorData( [0, 2, 5] )
		counts = IECore.IntVectorData( [2, 3, 1] )
		weights = IECore.FloatVectorData( [0.5, 0.5, 0.2, 0.8, 0.0, 1.0] )

		ssd = IECoreScene.SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def original( self ) :

		names = IECore.StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
		poses = IECore.M44fVectorData( [IECore.M44f(1),IECore.M44f(2),IECore.M44f(3)] )
		indices = IECore.IntVectorData( [0, 1, 0, 1, 2, 1] )

		return self.createSSD( names, poses, indices )

	def reordered( self ) :

		names = IECore.StringVectorData( [ 'jointB', 'jointC', 'jointA' ] )
		poses = IECore.M44fVectorData( [IECore.M44f(2),IECore.M44f(3),IECore.M44f(1)] )
		indices = IECore.IntVectorData( [2, 0, 2, 0, 1, 0] )

		return self.createSSD( names, poses, indices )

	def testTypes( self ) :
		""" Test ReorderSmoothSkinningInfluencesOp types"""

		ssd = self.original()

		op = IECoreScene.ReorderSmoothSkinningInfluencesOp()
		self.assertEqual( type(op), IECoreScene.ReorderSmoothSkinningInfluencesOp )
		self.assertEqual( op.typeId(), IECoreScene.TypeId.ReorderSmoothSkinningInfluencesOp )
		op.parameters()['input'].setValue( IECore.IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

	def testSameNames( self ) :
		""" Test ReorderSmoothSkinningInfluencesOp with same names"""

		ssd = self.original()

		op = IECoreScene.ReorderSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['reorderedInfluenceNames'].setValue( ssd.influenceNames() )
		result = op.operate()

		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertEqual( result, ssd )

	def testReordering( self ) :
		""" Test ReorderSmoothSkinningInfluencesOp with reordered names"""

		ssd = self.original()
		reordered = self.reordered()

		op = IECoreScene.ReorderSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['reorderedInfluenceNames'].setValue( reordered.influenceNames() )
		result = op.operate()
		self.assertNotEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertNotEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		self.assertEqual( result.influenceNames(), reordered.influenceNames() )
		self.assertEqual( result.influencePose(), reordered.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), reordered.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), reordered.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), reordered.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), reordered.pointInfluenceWeights() )
		self.assertEqual( result, reordered )

	def testWrongNames( self ) :
		""" Test ReorderSmoothSkinningInfluencesOp with wrong names"""

		ssd = self.original()

		op = IECoreScene.ReorderSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['reorderedInfluenceNames'].setValue( IECore.StringVectorData( [ 'jointA', 'badName', 'jointC' ] ) )
		self.assertRaises( RuntimeError, op.operate )

	def testExtraNames( self ) :
		""" Test ReorderSmoothSkinningInfluencesOp with extra names"""

		ssd = self.original()

		op = IECoreScene.ReorderSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['reorderedInfluenceNames'].setValue( IECore.StringVectorData( [ 'jointA', 'jointB', 'jointC', 'jointD' ] ) )
		self.assertRaises( RuntimeError, op.operate )

	def testNoNames( self ) :
		""" Test ReorderSmoothSkinningInfluencesOp with no new names"""

		ssd = self.original()

		op = IECoreScene.ReorderSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['reorderedInfluenceNames'].setValue( IECore.StringVectorData([]) )
		self.assertRaises( RuntimeError, op.operate )

if __name__ == "__main__":
	unittest.main()
