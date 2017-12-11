##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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
import imath
import IECore
import IECoreScene


class ContrastSmoothSkinningWeightsOpTest( unittest.TestCase ) :

	def mesh( self ) :

		vertsPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [
			0, 1, 3, 2, 2, 3, 5, 4, 4, 5, 7, 6, 6, 7, 9, 8,
			8, 9, 11, 10, 10, 11, 13, 12, 12, 13, 15, 14, 14, 15, 1, 0,
			1, 15, 13, 3, 3, 13, 11, 5, 5, 11, 9, 7, 14, 0, 2, 12,
			12, 2, 4, 10, 10, 4, 6, 8
		] )

		return IECoreScene.MeshPrimitive( vertsPerFace, vertexIds )

	def createSSD( self, offsets, counts, indices, weights ) :

		names = IECore.StringVectorData( [ "|joint1", "|joint1|joint2", "|joint1|joint2|joint3" ] )
		poses = IECore.M44fVectorData( [
			imath.M44f( 1, -0, 0, -0, -0, 1, -0, 0, 0, -0, 1, -0, -0, 2, -0, 1 ),
			imath.M44f( 1, -0, 0, -0, -0, 1, -0, 0, 0, -0, 1, -0, -0, 0, -0, 1 ),
			imath.M44f( 1, -0, 0, -0, -0, 1, -0, 0, 0, -0, 1, -0, -0, -2, -0, 1 )
		] )

		return IECoreScene.SmoothSkinningData( names, poses, offsets, counts, indices, weights )

	def original( self ) :

		offsets = IECore.IntVectorData( [ 0, 1, 2, 4, 6, 7, 8, 10, 12, 14, 16, 17, 18, 20, 22, 23 ] )
		counts = IECore.IntVectorData( [ 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1 ] )
		indices = IECore.IntVectorData( [ 0, 0, 0, 1, 0, 1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 0, 1, 0, 1, 0, 0 ] )

		weights = IECore.FloatVectorData( [
			1, 1, 0.8, 0.2, 0.8, 0.2, 1, 1, 0.5, 0.5, 0.5, 0.5,
			0.5, 0.5, 0.5, 0.5, 1, 1, 0.8, 0.2, 0.8, 0.2, 1, 1
		] )

		return self.createSSD( offsets, counts, indices, weights )

	def __testContrast( self, center ) :

		ssd = self.original()
		dop = IECoreScene.DecompressSmoothSkinningDataOp()
		dop.parameters()['input'].setValue( ssd )
		decompressedOrig = dop.operate()
		origWeights = decompressedOrig.pointInfluenceWeights()

		# ratio 0.2 iterations 1
		op = IECoreScene.ContrastSmoothSkinningWeightsOp()
		op.parameters()['copyInput'].setValue( True )
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['contrastCenter'].setValue( center )
		op.parameters()['contrastRatio'].setValue( 0.2 )
		op.parameters()['iterations'].setValue( 1 )
		op.parameters()['applyLocks'].setValue( False )
		result = op.operate()
		dop.parameters()['input'].setValue( result )
		ratio02iteration1 = dop.operate()
		ratio02iteration1Weights = ratio02iteration1.pointInfluenceWeights()

		# ratio 0.5 iterations 1
		op.parameters()['contrastRatio'].setValue( 0.5 )
		op.parameters()['iterations'].setValue( 1 )
		result = op.operate()
		dop.parameters()['input'].setValue( result )
		ratio05iteration1 = dop.operate()
		ratio05iteration1Weights = ratio05iteration1.pointInfluenceWeights()

		# ratio 0.8 iterations 1
		op.parameters()['contrastRatio'].setValue( 0.8 )
		op.parameters()['iterations'].setValue( 1 )
		result = op.operate()
		dop.parameters()['input'].setValue( result )
		ratio08iteration1 = dop.operate()
		ratio08iteration1Weights = ratio08iteration1.pointInfluenceWeights()

		# ratio 0.2 iterations 2
		op.parameters()['contrastRatio'].setValue( 0.2 )
		op.parameters()['iterations'].setValue( 2 )
		op.parameters()['applyLocks'].setValue( False )
		result = op.operate()
		dop.parameters()['input'].setValue( result )
		ratio02iteration2 = dop.operate()
		ratio02iteration2Weights = ratio02iteration2.pointInfluenceWeights()

		# ratio 0.5 iterations 2
		op.parameters()['contrastRatio'].setValue( 0.5 )
		op.parameters()['iterations'].setValue( 2 )
		result = op.operate()
		dop.parameters()['input'].setValue( result )
		ratio05iteration2 = dop.operate()
		ratio05iteration2Weights = ratio05iteration2.pointInfluenceWeights()

		# ratio 0.8 iterations 2
		op.parameters()['contrastRatio'].setValue( 0.8 )
		op.parameters()['iterations'].setValue( 2 )
		result = op.operate()
		dop.parameters()['input'].setValue( result )
		ratio08iteration2 = dop.operate()
		ratio08iteration2Weights = ratio08iteration2.pointInfluenceWeights()

		for i in range( 0, origWeights.size() ) :
			if origWeights[i] > center :

				if origWeights[i] < 1 :
					self.assert_( ratio02iteration1Weights[i] > origWeights[i] )
					self.assert_( ratio05iteration1Weights[i] > ratio02iteration1Weights[i] )
					self.assert_( ratio08iteration1Weights[i] > ratio05iteration1Weights[i] )

					self.assert_( ratio02iteration2Weights[i] > origWeights[i] )
					self.assert_( ratio05iteration2Weights[i] > ratio02iteration2Weights[i] )
					self.assert_( ratio08iteration2Weights[i] > ratio05iteration2Weights[i] )

					self.assert_( ratio02iteration2Weights[i] > ratio02iteration1Weights[i] )
					self.assert_( ratio05iteration2Weights[i] > ratio05iteration1Weights[i] )
					self.assert_( ratio08iteration2Weights[i] > ratio08iteration1Weights[i] )

			elif origWeights[i] < center :

				if origWeights[i] > 0 :
					self.assert_( ratio02iteration1Weights[i] < origWeights[i] )
					self.assert_( ratio05iteration1Weights[i] < ratio02iteration1Weights[i] )
					self.assert_( ratio08iteration1Weights[i] < ratio05iteration1Weights[i] )

					self.assert_( ratio02iteration2Weights[i] < origWeights[i] )
					self.assert_( ratio05iteration2Weights[i] < ratio02iteration2Weights[i] )
					self.assert_( ratio08iteration2Weights[i] < ratio05iteration2Weights[i] )

					self.assert_( ratio02iteration2Weights[i] < ratio02iteration1Weights[i] )
					self.assert_( ratio05iteration2Weights[i] < ratio05iteration1Weights[i] )
					self.assert_( ratio08iteration2Weights[i] < ratio08iteration1Weights[i] )

			else :

				self.assertEqual( ratio02iteration1Weights[i], origWeights[i] )
				self.assertEqual( ratio05iteration1Weights[i], ratio02iteration1Weights[i] )
				self.assertEqual( ratio08iteration1Weights[i], ratio05iteration1Weights[i] )

				self.assertEqual( ratio02iteration2Weights[i], origWeights[i] )
				self.assertEqual( ratio05iteration2Weights[i], ratio02iteration2Weights[i] )
				self.assertEqual( ratio08iteration2Weights[i], ratio05iteration2Weights[i] )

				self.assertEqual( ratio02iteration2Weights[i], ratio02iteration1Weights[i] )
				self.assertEqual( ratio05iteration2Weights[i], ratio05iteration1Weights[i] )
				self.assertEqual( ratio08iteration2Weights[i], ratio08iteration1Weights[i] )

	def testContrast( self ):
		""" Test ContrastSmoothSkinningWeightsOp changing ratio and iterations"""
		self.__testContrast( center = 0.4 )
		self.__testContrast( center = 0.5 )
		self.__testContrast( center = 0.6 )

	def testLocks( self ) :
		""" Test ContrastSmoothSkinningWeightsOp locking mechanism"""

		ssd = self.original()
		op = IECoreScene.ContrastSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['contrastRatio'].setValue( 1.0 )
		op.parameters()['iterations'].setValue( 3 )
		op.parameters()['applyLocks'].setValue( True )
		op.parameters()['influenceLocks'].setValue( IECore.BoolVectorData( [ True, False, False ] ) )

		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		# make sure locked weights did not change and the rest has changed.
		dop = IECoreScene.DecompressSmoothSkinningDataOp()
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
			elif not origWeights[i] in [ 0, 0.5, 1 ]:
				self.assertNotEqual( resultWeights[i], origWeights[i] )

	def testVertexSelection( self ) :
		""" Test ContrastSmoothSkinningWeightsOp using selected vertices"""

		ssd = self.original()
		op = IECoreScene.ContrastSmoothSkinningWeightsOp()
		op.parameters()['input'].setValue( ssd )
		op.parameters()['mesh'].setValue( self.mesh() )
		op.parameters()['contrastRatio'].setValue( 0.5 )
		op.parameters()['iterations'].setValue( 2 )
		op.parameters()['applyLocks'].setValue( False )
		op.parameters()['vertexIndices'].setFrameListValue( IECore.FrameList.parse( "2-4,10-12" ) )

		result = op.operate()
		self.assertEqual( result.influenceNames(), ssd.influenceNames() )
		self.assertEqual( result.influencePose(), ssd.influencePose() )
		self.assertEqual( result.pointIndexOffsets(), ssd.pointIndexOffsets() )
		self.assertEqual( result.pointInfluenceCounts(), ssd.pointInfluenceCounts() )
		self.assertEqual( result.pointInfluenceIndices(), ssd.pointInfluenceIndices() )
		self.assertNotEqual( result.pointInfluenceWeights(), ssd.pointInfluenceWeights() )
		self.assertNotEqual( result, ssd )

		# make sure only selected vertices changed and the rest is unchanged.
		dop = IECoreScene.DecompressSmoothSkinningDataOp()
		dop.parameters()['input'].setValue( result )
		decompressedResult = dop.operate()
		dop.parameters()['input'].setValue( ssd )
		decompressedOrig = dop.operate()
		resultOffsets = decompressedResult.pointIndexOffsets()
		resultCounts = decompressedResult.pointInfluenceCounts()
		resultIndices = decompressedResult.pointInfluenceIndices()
		resultWeights = decompressedResult.pointInfluenceWeights()
		origWeights = decompressedOrig.pointInfluenceWeights()
		frames = op.parameters()['vertexIndices'].getFrameListValue().asList()
		nonSelectedVerts = [ x for x in range( 0, resultOffsets.size() ) if x not in frames ]
		for i in range( 0, resultOffsets.size() ) :

			if i in frames :
				for j in range( 0, resultCounts[i] ) :
					current = resultOffsets[i] + j
					if origWeights[current] > 0 and origWeights[current] < 1:
						self.assertNotEqual( resultWeights[current], origWeights[current] )
			else :
				for j in range( 0, resultCounts[i] ) :
					current = resultOffsets[i] + j
					self.assertEqual( resultWeights[current], origWeights[current] )

	def testErrorStates( self ) :
		""" Test ContrastSmoothSkinningWeightsOp with various error states"""

		ssd = self.original()
		op = IECoreScene.ContrastSmoothSkinningWeightsOp()
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
