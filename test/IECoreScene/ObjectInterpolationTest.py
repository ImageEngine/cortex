##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
#  Copyright (c) 2012, John Haddon. All rights reserved.
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
import imath

import IECore
import IECoreScene

class ObjectInterpolationTest( unittest.TestCase ) :

	def testPrimitiveInterpolation( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = IECoreScene.TransformOp()( input=m1, matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 2 ) ) ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3, IECoreScene.TransformOp()( input=m1, matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1.5 ) ) ) ) )

	def testPrimitiveInterpolationMaintainsUninterpolableValuesFromFirstPrimitive( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m1["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "hi" ) )
		m2 = m1.copy()

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3["c"], IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "hi" ) ) )

		m2["c"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "bye" ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3["c"], IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "hi" ) ) )

	def testPrimitiveInterpolationMaintainsValuesMissingFromSecondPrimitive( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

		m1["v"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3, 4 ] ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3["v"], IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3, 4 ] ) ) )

	def testPrimitiveInterpolationWithBlindData( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = m1.copy()

		m1.blindData()["a"] = IECore.FloatData( 10 )
		m2.blindData()["a"] = IECore.FloatData( 20 )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3.blindData()["a"], IECore.FloatData( 15 ) )

	def testPrimitiveInterpolationWithBlindDataMaintainsValuesMissingFromSecondPrimitive( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = m1.copy()

		m1.blindData()["a"] = IECore.FloatData( 10 )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3.blindData()["a"], IECore.FloatData( 10 ) )

	def testPrimVarsWithDifferingDataArraysAreSkipped( self ):

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = m1.copy()

		m1["v"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 1, 2] ), IECore.IntVectorData( [ 0, 1, 2, 3 ] ) )
		m2["v"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2 ] ), IECore.IntVectorData( [ 0, 1, 0, 1 ] ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )

		self.assertTrue( "v" in m3 )

		self.assertEqual( m3["v"], m1["v"])

	def testPrimVarsWithDifferentIndicesAreSkipped( self ):

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = m1.copy()

		m1["v"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3, 4] ), IECore.IntVectorData( [ 0, 1, 2, 3 ] ) )
		m2["v"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 4, 3, 2, 1] ), IECore.IntVectorData( [ 3, 2, 1, 0 ] ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )

		self.assertTrue( "v" in m3 )
		self.assertEqual( m3["v"], m1["v"])

	def testPrimVarInterpolationChangeSkipped( self ):

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = m1.copy()

		m1["v"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3, 4] ), IECore.IntVectorData( [ 0, 1, 2, 3 ] ) )
		m2["v"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 4, 3, 2, 1] ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )

		self.assertTrue( "v" in m3 )
		self.assertEqual( m3["v"], m1["v"])

if __name__ == "__main__":
    unittest.main()
