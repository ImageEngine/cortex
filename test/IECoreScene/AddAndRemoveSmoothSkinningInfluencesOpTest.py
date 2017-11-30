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


class AddAndRemoveSmoothSkinningInfluencesOpTest( unittest.TestCase ) :

	def createSSD( self, names, poses, indices ) :

		offsets = IECore.IntVectorData( [0, 2, 5, 6, 8] )
		counts = IECore.IntVectorData( [2, 3, 1, 2, 3] )
		weights = IECore.FloatVectorData( [0.7, 0.7, 0.2, 0.6, 0.0, 0.1, 1.2, 0.8, 0.4, 0.6, 0.4] )

		ssd = IECoreScene.SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def original( self ) :

		names = IECore.StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
		poses = IECore.M44fVectorData( [IECore.M44f(1),IECore.M44f(2),IECore.M44f(3)] )
		indices = IECore.IntVectorData( [0, 1, 0, 1, 2, 1, 1, 2, 0, 1, 2] )

		return self.createSSD( names, poses, indices )

	def added( self ) :

		names = IECore.StringVectorData( [ 'newA', 'jointA', 'newC', 'newB', 'jointB', 'jointC', 'newD' ] )
		poses = IECore.M44fVectorData( [ IECore.M44f(4), IECore.M44f(1), IECore.M44f(6), IECore.M44f(5), IECore.M44f(2), IECore.M44f(3), IECore.M44f(7) ] )
		indices = IECore.IntVectorData( [1, 4, 1, 4, 5, 4, 4, 5, 1, 4, 5] )

		return self.createSSD( names, poses, indices )

	def removed( self ) :

		names = IECore.StringVectorData( [ 'jointA', 'newC', 'newB', 'jointC' ] )
		poses = IECore.M44fVectorData( [ IECore.M44f(1), IECore.M44f(6), IECore.M44f(5), IECore.M44f(3) ] )
		offsets = IECore.IntVectorData( [0, 1, 3, 3, 4] )
		counts = IECore.IntVectorData( [1, 2, 0, 1, 2] )
		indices = IECore.IntVectorData( [0, 0, 3, 3, 0, 3] )
		weights = IECore.FloatVectorData( [0.7, 0.2, 0.0, 0.8, 0.4, 0.4] )

		ssd = IECoreScene.SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def testTypes( self ) :
		""" Test AddSmoothSkinningInfluencesOp and RemoveSmoothSkinningInfluencesOp types"""

		ssd = self.original()

		op = IECoreScene.AddSmoothSkinningInfluencesOp()
		self.assertEqual( type(op), IECoreScene.AddSmoothSkinningInfluencesOp )
		self.assertEqual( op.typeId(), IECoreScene.TypeId.AddSmoothSkinningInfluencesOp )
		op.parameters()['input'].setValue( IECore.IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

		op = IECoreScene.RemoveSmoothSkinningInfluencesOp()
		self.assertEqual( type(op), IECoreScene.RemoveSmoothSkinningInfluencesOp )
		self.assertEqual( op.typeId(), IECoreScene.TypeId.RemoveSmoothSkinningInfluencesOp )
		op.parameters()['input'].setValue( IECore.IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

	def testAddingNothing( self ) :
		""" Test AddSmoothSkinningInfluencesOp with no new influences"""

		ssd = self.original()

		op = IECoreScene.AddSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()

		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertEqual( result, ssd )

	def testAdding( self ) :
		""" Test AddSmoothSkinningInfluencesOp"""

		ssd = self.original()

		op = IECoreScene.AddSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['influenceNames'].setValue( IECore.StringVectorData( [ "newA", "newB", "newC", "newD" ] ) )
		op.parameters()['influencePose'].setValue( IECore.M44fVectorData( [ IECore.M44f(4), IECore.M44f(5), IECore.M44f(6), IECore.M44f(7) ] ) )
		op.parameters()['indices'].setValue( IECore.IntVectorData( [ 0, 2, 2, 6 ] ) )
		result = op.operate()

		self.assertNotEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertNotEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		added = self.added()
		self.assertEqual( result.influenceNames(), added.influenceNames() )
		self.assertEqual( result.influencePose(), added.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), added.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), added.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), added.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), added.pointInfluenceWeights() )
		self.assertEqual( result, added )

	def testRemovingNothing( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp with no new influences"""

		ssd = self.original()

		op = IECoreScene.RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		result = op.operate()

		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertEqual( result, ssd )

	def testRemovingNamedMode( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp in named mode"""

		ssd = self.added()

		op = IECoreScene.RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mode'].setValue( IECoreScene.RemoveSmoothSkinningInfluencesOp.Mode.Named )
		op.parameters()['influenceNames'].setValue( IECore.StringVectorData( [ "newA", "jointB", "newD" ] ) )
		result = op.operate()

		self.assertNotEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertNotEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		removed = self.removed()
		self.assertEqual( result.influenceNames(), removed.influenceNames() )
		self.assertEqual( result.influencePose(), removed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), removed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), removed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), removed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), removed.pointInfluenceWeights() )
		self.assertEqual( result, removed )

	def testRemovingIndexedMode( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp in index mode"""

		ssd = self.added()

		op = IECoreScene.RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mode'].setValue( IECoreScene.RemoveSmoothSkinningInfluencesOp.Mode.Indexed )
		op.parameters()['indices'].setValue( IECore.IntVectorData( [ 0, 4, 6 ] ) )
		result = op.operate()

		self.assertNotEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertNotEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		removed = self.removed()
		self.assertEqual( result.influenceNames(), removed.influenceNames() )
		self.assertEqual( result.influencePose(), removed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), removed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), removed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), removed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), removed.pointInfluenceWeights() )
		self.assertEqual( result, removed )

	def testRemovingWeightlessMode( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp in weightless mode"""

		ssd = self.added()

		op = IECoreScene.RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mode'].setValue( IECoreScene.RemoveSmoothSkinningInfluencesOp.Mode.Weightless )
		result = op.operate()

		self.assertNotEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertNotEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		removed = self.original()
		self.assertEqual( result.influenceNames(), removed.influenceNames() )
		self.assertEqual( result.influencePose(), removed.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), removed.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), removed.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), removed.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), removed.pointInfluenceWeights() )
		self.assertEqual( result, removed )

	def testAddOpErrorStates( self ) :
		""" Test AddSmoothSkinningInfluencesOp with various error states"""

		ssd = self.original()

		op = IECoreScene.AddSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['influenceNames'].setValue( IECore.StringVectorData( [ "newA", "newB", "newC" ] ) )
		op.parameters()['influencePose'].setValue( IECore.M44fVectorData( [ IECore.M44f(1), IECore.M44f(2) ] ) )
		op.parameters()['indices'].setValue( IECore.IntVectorData( [ 1, 3 ] ) )

		# wrong number of pose matrices
		self.assertRaises( RuntimeError, op.operate )

		# wrong number of indices
		op.parameters()['influencePose'].setValue( IECore.M44fVectorData( [ IECore.M44f(1), IECore.M44f(2), IECore.M44f(3) ] ) )
		self.assertRaises( RuntimeError, op.operate )

		# index validity
		op.parameters()['indices'].setValue( IECore.IntVectorData( [ 1, 3, 6 ] ) )
		self.assertRaises( RuntimeError, op.operate )

		# existing influenceName
		op.parameters()['indices'].setValue( IECore.IntVectorData( [ 1, 2, 3 ] ) )
		op.parameters()['influenceNames'].setValue( IECore.StringVectorData( [ "jointA", "newB", "newC" ] ) )
		self.assertRaises( RuntimeError, op.operate )

	def testRemoveOpErrorStates( self ) :
		""" Test RemoveSmoothSkinningInfluencesOp with various error states"""

		ssd = self.original()

		op = IECoreScene.RemoveSmoothSkinningInfluencesOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['influenceNames'].setValue( IECore.StringVectorData( [ "newA", "newB", "newC" ] ) )

		# index validity
		op.parameters()['mode'].setValue( IECoreScene.RemoveSmoothSkinningInfluencesOp.Mode.Indexed )
		op.parameters()['indices'].setValue( IECore.IntVectorData( [ 1, 3 ] ) )
		self.assertRaises( RuntimeError, op.operate )

		# name validity
		op.parameters()['mode'].setValue( IECoreScene.RemoveSmoothSkinningInfluencesOp.Mode.Named )
		op.parameters()['influenceNames'].setValue( IECore.StringVectorData( [ "jointFAKE", "newB", "newC" ] ) )
		self.assertRaises( RuntimeError, op.operate )

if __name__ == "__main__":
	unittest.main()
