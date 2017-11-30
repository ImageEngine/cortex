##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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


class TransferSmoothSkinningWeightsOpTest( unittest.TestCase ) :

	def createSSD( self, weights, indices, offsets, counts ) :

		names = IECore.StringVectorData( [ 'jointA', 'jointB', 'jointC' ] )
		poses = IECore.M44fVectorData( [IECore.M44f(1),IECore.M44f(2),IECore.M44f(3)] )

		ssd = IECoreScene.SmoothSkinningData( names, poses, offsets, counts, indices, weights )

		return ssd

	def original( self ) :

		weights = IECore.FloatVectorData( [0.1, 0.1, 0.1, 0.2, 0.2, 0.3] )
		indices = IECore.IntVectorData( [0, 1, 0, 1, 2, 1] )
		offsets = IECore.IntVectorData( [0, 2, 5] )
		counts = IECore.IntVectorData( [2, 3, 1] )

		return self.createSSD( weights, indices, offsets, counts )

	def transferredA( self ) :

		weights = IECore.FloatVectorData( [0.2, 0.3, 0.2, 0.3] )
		indices = IECore.IntVectorData( [1, 1, 2, 1] )
		offsets = IECore.IntVectorData( [0, 1, 3] )
		counts = IECore.IntVectorData( [1, 2, 1] )

		return self.createSSD( weights, indices, offsets, counts )

	def transferredAC( self ) :

		weights = IECore.FloatVectorData( [0.2, 0.5, 0.3] )
		indices = IECore.IntVectorData( [1, 1, 1] )
		offsets = IECore.IntVectorData( [0, 1, 2] )
		counts = IECore.IntVectorData( [1, 1, 1] )

		return self.createSSD( weights, indices, offsets, counts )

	def testTypes( self ) :
		""" Test TransferSmoothSkinningWeightsOp types"""

		ssd = self.original()

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		self.assertEqual( type(op), IECoreScene.TransferSmoothSkinningWeightsOp )
		self.assertEqual( op.typeId(), IECore.TypeId.TransferSmoothSkinningWeightsOp )
		op.parameters()['input'].setValue( IECore.IntData(1) )
		self.assertRaises( RuntimeError, op.operate )

	def testSameNames( self ) :
		""" Test TransferSmoothSkinningWeightsOp with same names"""

		ssd = self.original()

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['sourceInfluenceNames'].setValue( IECore.StringVectorData(["jointA"]) )
		op.parameters()['targetInfluenceName'].setValue( IECore.StringData("jointA") )
		self.assertRaises( RuntimeError, op.operate )

	def testTransferredSingle( self ) :
		""" Test TransferSmoothSkinningWeightsOp with one transferred influence"""

		ssd = self.original()
		transferred = self.transferredA()

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['sourceInfluenceNames'].setValue( IECore.StringVectorData(["jointA"]) )
		op.parameters()['targetInfluenceName'].setValue( IECore.StringData("jointB") )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		self.assertEqual( result.influenceNames(), transferred.influenceNames() )
		self.assertEqual( result.influencePose(), transferred.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), transferred.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), transferred.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), transferred.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), transferred.pointInfluenceWeights() )
		self.assertEqual( result, transferred )

	def testTransferredMultiple( self ) :
		""" Test TransferSmoothSkinningWeightsOp with multiple transferred influences"""

		ssd = self.original()
		transferred = self.transferredAC()

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['sourceInfluenceNames'].setValue( IECore.StringVectorData(["jointA", "jointC"]) )
		op.parameters()['targetInfluenceName'].setValue( IECore.StringData("jointB") )
		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertNotEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertNotEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		self.assertEqual( result.influenceNames(), transferred.influenceNames() )
		self.assertEqual( result.influencePose(), transferred.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), transferred.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), transferred.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), transferred.pointInfluenceIndices() )
		self.assertEqual( result.pointInfluenceWeights(), transferred.pointInfluenceWeights() )
		self.assertEqual( result, transferred )

	def testWrongNames( self ) :
		""" Test TransferSmoothSkinningWeightsOp with wrong names"""

		ssd = self.original()

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['sourceInfluenceNames'].setValue( IECore.StringVectorData( [ 'jointA', 'badName', 'jointC' ] ) )
		op.parameters()['targetInfluenceName'].setValue( IECore.StringVectorData( [ 'jointB' ] ) )
		self.assertRaises( RuntimeError, op.operate )

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['sourceInfluenceNames'].setValue( IECore.StringVectorData( [ 'jointA', 'jointB' ] ) )
		op.parameters()['targetInfluenceName'].setValue( IECore.StringVectorData( [ 'badName' ] ) )
		self.assertRaises( RuntimeError, op.operate )

	def testNoNames( self ) :
		""" Test ReorderSmoothSkinningInfluencesOp with no new names"""

		ssd = self.original()

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['sourceInfluenceNames'].setValue( IECore.StringVectorData([]) )
		op.parameters()['targetInfluenceName'].setValue( IECore.StringData("jointB") )
		self.assertRaises( RuntimeError, op.operate )

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['sourceInfluenceNames'].setValue( IECore.StringVectorData(["jointA", ""]) )
		op.parameters()['targetInfluenceName'].setValue( IECore.StringData("jointB") )
		self.assertRaises( RuntimeError, op.operate )

		op = IECoreScene.TransferSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['sourceInfluenceNames'].setValue( IECore.StringVectorData(['jointA', 'jointB']) )
		op.parameters()['targetInfluenceName'].setValue( IECore.StringData("") )
		self.assertRaises( RuntimeError, op.operate )

if __name__ == "__main__":
	unittest.main()
