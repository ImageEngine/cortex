##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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
import IECoreScene
import imath


class MeshAlgoSegmentTest( unittest.TestCase ) :

	def testCanSegmentUsingIntegerPrimvar( self ) :
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 2 ) )

		mesh["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [0, 0, 1, 1] ) )

		segmentValues = IECore.IntVectorData( [0, 1] )
		segments = IECoreScene.MeshAlgo.segment( mesh, mesh["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0].numFaces(), 2 )
		self.assertEqual( segments[1].numFaces(), 2 )

		p00 = imath.V3f( 0, 0, 0 )
		p10 = imath.V3f( 1, 0, 0 )
		p20 = imath.V3f( 2, 0, 0 )

		p01 = imath.V3f( 0, 1, 0 )
		p11 = imath.V3f( 1, 1, 0 )
		p21 = imath.V3f( 2, 1, 0 )

		p02 = imath.V3f( 0, 2, 0 )
		p12 = imath.V3f( 1, 2, 0 )
		p22 = imath.V3f( 2, 2, 0 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [p00, p10, p20, p01, p11, p21] ) )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData( [p01, p11, p21, p02, p12, p22] ) )

	def testCanSegmentUsingStringPrimvar( self ) :
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 2 ) )

		# checkerboard pattern to segment
		mesh["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ['a', 'b', 'b', 'a'] ) )

		segmentValues = IECore.StringVectorData( ['a', 'b'] )
		segments = IECoreScene.MeshAlgo.segment( mesh, mesh["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0].numFaces(), 2 )
		self.assertEqual( segments[1].numFaces(), 2 )

		p00 = imath.V3f( 0, 0, 0 )
		p10 = imath.V3f( 1, 0, 0 )
		p20 = imath.V3f( 2, 0, 0 )

		p01 = imath.V3f( 0, 1, 0 )
		p11 = imath.V3f( 1, 1, 0 )
		p21 = imath.V3f( 2, 1, 0 )

		p02 = imath.V3f( 0, 2, 0 )
		p12 = imath.V3f( 1, 2, 0 )
		p22 = imath.V3f( 2, 2, 0 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [p00, p10, p01, p11, p21, p12, p22] ) )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData( [p10, p20, p01, p11, p21, p02, p12] ) )

	def testSegmentsFullyIfNoSegmentValuesGiven( self ) :
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 2 ) )

		# checkerboard pattern to segment
		mesh["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ['a', 'b', 'b', 'a'] ) )

		segments = IECoreScene.MeshAlgo.segment( mesh, mesh["s"] )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0].numFaces(), 2 )
		self.assertEqual( segments[1].numFaces(), 2 )

		p00 = imath.V3f( 0, 0, 0 )
		p10 = imath.V3f( 1, 0, 0 )
		p20 = imath.V3f( 2, 0, 0 )

		p01 = imath.V3f( 0, 1, 0 )
		p11 = imath.V3f( 1, 1, 0 )
		p21 = imath.V3f( 2, 1, 0 )

		p02 = imath.V3f( 0, 2, 0 )
		p12 = imath.V3f( 1, 2, 0 )
		p22 = imath.V3f( 2, 2, 0 )

		if segments[0]["s"].data[0] == 'a':
			s0 = segments[0]
			s1 = segments[1]
		else:
			s0 = segments[1]
			s1 = segments[0]

		self.assertEqual( s0["P"].data, IECore.V3fVectorData( [p00, p10, p01, p11, p21, p12, p22] ) )
		self.assertEqual( s1["P"].data, IECore.V3fVectorData( [p10, p20, p01, p11, p21, p02, p12] ) )


	def testRaisesExceptionIfSegmentKeysNotSameTypeAsPrimvar( self ) :
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 2 ) )

		mesh["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ["a", "b", "a", "b"] ) )

		segmentValues = IECore.IntVectorData( [1, 2] )

		def t() :
			IECoreScene.MeshAlgo.segment( mesh, mesh["s"], segmentValues )

		self.assertRaises( RuntimeError, t )

	def testEmptyPrimitiveIfNotMatching( self ) :
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 2 ) )

		mesh["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ["a", "b", "a", "b"] ) )

		segmentValues = IECore.StringVectorData( ["e", "f"] )
		segments = IECoreScene.MeshAlgo.segment( mesh, mesh["s"], segmentValues )

		self.assertEqual( len( segments ), 2 )

		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData() )
		self.assertEqual( segments[1]["P"].data, IECore.V3fVectorData() )

	def testSegmentSubset( self ) :
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 2 ) ), imath.V2i( 2 ) )

		mesh["s"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringVectorData( ["a", "a", "a", "b"] ) )

		segmentValues = IECore.StringVectorData( ["b"] )
		segments = IECoreScene.MeshAlgo.segment( mesh, mesh["s"], segmentValues )

		p00 = imath.V3f( 0, 0, 0 )
		p10 = imath.V3f( 1, 0, 0 )
		p20 = imath.V3f( 2, 0, 0 )

		p01 = imath.V3f( 0, 1, 0 )
		p11 = imath.V3f( 1, 1, 0 )
		p21 = imath.V3f( 2, 1, 0 )

		p02 = imath.V3f( 0, 2, 0 )
		p12 = imath.V3f( 1, 2, 0 )
		p22 = imath.V3f( 2, 2, 0 )

		self.assertEqual( len( segments ), 1 )
		self.assertEqual( segments[0]["P"].data, IECore.V3fVectorData( [p11, p21, p12, p22] ) )
		self.assertEqual( segments[0]["s"].data, IECore.StringVectorData( ["b"] ) )
