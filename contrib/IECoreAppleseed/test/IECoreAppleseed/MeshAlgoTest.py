##########################################################################
#
#  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

import os
import unittest
import imath

import IECore
import IECoreScene
import IECoreAppleseed

class MeshAlgoTest( unittest.TestCase ):

	def testFaceVaryingNonIndexedUVs( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -6 ), imath.V2f( 6 ) ) )
		m["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, m["uv"].expandedData() )
		uvData = m["uv"].data

		obj = IECoreAppleseed.ObjectAlgo.convert( m )
		self.assertEqual( obj.get_tex_coords_count(), 6 )

		quadTo2TrisIndices = [0, 1, 2, 0, 2, 3]
		for i in range( 0, obj.get_tex_coords_count() ):
			uv = obj.get_tex_coords( i )
			j = quadTo2TrisIndices[i]
			self.assertEqual( uv, uvData[j] )

	def testVertexUVs( self ) :

		# 2---1
		# |  /
		# | /
		# 0

		mesh = IECoreScene.MeshPrimitive(
			IECore.IntVectorData( [ 3 ] ),
			IECore.IntVectorData( [ 0, 1, 2 ] )
		)
		mesh["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [
				imath.V3f( 0, 0, 0 ),
				imath.V3f( 1, 1, 0 ),
				imath.V3f( 0, 1, 0 ),
			] )
		)

		mesh["uv"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V2fVectorData( [
				imath.V2f( 0, 0 ),
				imath.V2f( 1, 1 ),
				imath.V2f( 0, 1 )
			] ),
		)

		obj = IECoreAppleseed.ObjectAlgo.convert( mesh )
		self.assertEqual( obj.get_tex_coords_count(), 3 )

		for i in range( 0, obj.get_tex_coords_count() ):
			self.assertEqual( mesh["uv"].data[i], obj.get_tex_coords( i ) )

		tri = obj.get_triangle( 0 )
		self.assertEqual( [ tri.m_a0, tri.m_a1, tri.m_a2 ], [ 0, 1, 2 ] )

	def testFaceVaryingIndexedUVs( self ) :

		# 3----2
		# |  / |
		# | /  |
		# 0----1

		mesh = IECoreScene.MeshPrimitive(
			IECore.IntVectorData( [ 3, 3 ] ),
			IECore.IntVectorData( [ 0, 1, 2, 0, 2, 3 ] )
		)
		mesh["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [
				imath.V3f( 0, 0, 0 ),
				imath.V3f( 1, 0, 0 ),
				imath.V3f( 1, 1, 0 ),
				imath.V3f( 0, 1, 0 )
			] )
		)

		mesh["uv"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.FaceVarying,
			IECore.V2fVectorData( [
				imath.V2f( 0, 0 ),
				imath.V2f( 1, 0 ),
				imath.V2f( 1, 1 ),
				imath.V2f( 0, 1 )
			] ),
			mesh.vertexIds
		)

		obj = IECoreAppleseed.ObjectAlgo.convert( mesh )
		self.assertEqual( obj.get_tex_coords_count(), 4 )

		for i in range( 0, obj.get_tex_coords_count() ):
			self.assertEqual( mesh["uv"].data[i], obj.get_tex_coords( i ) )

		tri = obj.get_triangle( 0 )
		self.assertEqual( [ tri.m_a0, tri.m_a1, tri.m_a2 ], [ 0, 1, 2 ] )

		tri = obj.get_triangle( 1 )
		self.assertEqual( [ tri.m_a0, tri.m_a1, tri.m_a2 ], [ 0, 2, 3 ] )

	def testDeformationMotionBlurPow2NumSamples( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -2 ), imath.V2f( 2 ) ) )

		me = IECoreAppleseed.ObjectAlgo.convert( [ m1, m2 ], [ 0.25, 0.75 ], 0.25, 0.75 )
		self.failUnless( me.get_motion_segment_count() == 1 )

	def testDeformationMotionBlurNonPow2NumSamples( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -2 ), imath.V2f( 2 ) ) )
		m3 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -3 ), imath.V2f( 3 ) ) )

		me = IECoreAppleseed.ObjectAlgo.convert( [ m1, m2, m3 ], [ 0.25, 0.5, 0.75 ], 0.25, 0.75 )
		self.failUnless( me.get_motion_segment_count() == 3 )

if __name__ == "__main__":
	unittest.main()
