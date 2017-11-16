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
import IECore

class TestObjectInterpolation( unittest.TestCase ) :

	def testSimpleLinearInterpolation( self ) :

		self.assertEqual( IECore.linearObjectInterpolation( IECore.IntData(1), IECore.IntData(2), 0.5 ), None )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.FloatData(1), IECore.FloatData(2), 0.5 ), IECore.FloatData(1.5) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.DoubleData(1), IECore.DoubleData(2), 0.5 ), IECore.DoubleData(1.5) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.V2fData( IECore.V2f(1) ), IECore.V2fData( IECore.V2f(2) ), 0.5 ), IECore.V2fData( IECore.V2f(1.5) ) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.V3fData( IECore.V3f(1) ), IECore.V3fData( IECore.V3f(2) ), 0.5 ), IECore.V3fData( IECore.V3f(1.5) ) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.V2dData( IECore.V2d(1) ), IECore.V2dData( IECore.V2d(2) ), 0.5 ), IECore.V2dData( IECore.V2d(1.5) ) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.V3dData( IECore.V3d(1) ), IECore.V3dData( IECore.V3d(2) ), 0.5 ), IECore.V3dData( IECore.V3d(1.5) ) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.Box3fData( IECore.Box3f( IECore.V3f(1), IECore.V3f(1) ) ), IECore.Box3fData( IECore.Box3f( IECore.V3f(2), IECore.V3f(2) ) ), 0.5 ), IECore.Box3fData( IECore.Box3f( IECore.V3f(1.5), IECore.V3f(1.5) ) ) )

	def testVectorLinearInterpolation( self ):

		self.assertEqual( IECore.linearObjectInterpolation( IECore.IntVectorData( [1] ), IECore.IntVectorData( [2] ), 0.5 ), None )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.FloatVectorData( [1]), IECore.FloatVectorData( [2] ), 0.5 ), IECore.FloatVectorData([1.5]) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.DoubleVectorData( [1]), IECore.DoubleVectorData( [2] ), 0.5 ), IECore.DoubleVectorData([1.5]) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.V2fVectorData( [IECore.V2f(1)] ), IECore.V2fVectorData( [IECore.V2f(2)] ), 0.5 ), IECore.V2fVectorData( [IECore.V2f(1.5)] ) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.V3fVectorData( [IECore.V3f(1)] ), IECore.V3fVectorData( [IECore.V3f(2)] ), 0.5 ), IECore.V3fVectorData( [IECore.V3f(1.5)] ) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.V2dVectorData( [IECore.V2d(1)] ), IECore.V2dVectorData( [IECore.V2d(2)] ), 0.5 ), IECore.V2dVectorData( [IECore.V2d(1.5)] ) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.V3dVectorData( [IECore.V3d(1)] ), IECore.V3dVectorData( [IECore.V3d(2)] ), 0.5 ), IECore.V3dVectorData( [IECore.V3d(1.5)] ) )

	def testMatrixInterpolation( self ):
		m1 = IECore.M44d()
		m1.translate( IECore.V3d(1,0,0) )
		m2 = IECore.M44d()
		m2.translate( IECore.V3d(0,1,0) )
		m2.scale( IECore.V3d(9,9,9) )
		mx = IECore.M44d()
		mx.translate( IECore.V3d(0.5,0.5,0) )
		mx.scale( IECore.V3d(5,5,5) )
		self.assertEqual( IECore.linearObjectInterpolation( IECore.M44dData(m1), IECore.M44dData(m2), 0.5 ), IECore.M44dData(mx) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.M44dData(m1), IECore.M44dData(m1), IECore.M44dData(m2), IECore.M44dData(m2), 0.5 ), IECore.M44dData(mx) )

	def __buildTree( self, compoundType, seed ):

		def buildCompound( compoundType, seed ):
			c = compoundType()
			intSeed = int( seed )
			c[ "str" ] = IECore.StringData( str(intSeed) )
			c[ "int" ] = IECore.IntData( intSeed )
			c[ "float" ] = IECore.FloatData( seed )
			c[ "double" ] = IECore.DoubleData( seed )
			c[ "box" ] = IECore.Box3fData( IECore.Box3f( IECore.V3f( seed ), IECore.V3f( seed ) ) )
			c[ "color" ] = IECore.Color3fData( IECore.Color3f( seed, seed, seed ) )
			c[ "v3d" ] = IECore.V3dData( IECore.V3d( seed, seed, seed ) )
			return c

		c = buildCompound( compoundType, seed )
		c[ 'first' ] = buildCompound( compoundType, seed )
		c[ 'first' ][ 'second' ] = buildCompound( compoundType, seed )
		return c

	def testCompoundDataInterpolation( self ):

		data0 = self.__buildTree( IECore.CompoundData, 0 )
		data1 = self.__buildTree( IECore.CompoundData, 1 )
		data2 = self.__buildTree( IECore.CompoundData, 2 )
		data3 = self.__buildTree( IECore.CompoundData, 3 )
		self.assertEqual( IECore.linearObjectInterpolation( data1, data2, 0.5 ), self.__buildTree( IECore.CompoundData, 1.5 ) )
		self.assertEqual( IECore.cubicObjectInterpolation( data0, data1, data2, data3, 0.5 ), self.__buildTree( IECore.CompoundData, 1.5 ) )

	def testCompoundObjectInterpolation( self ):

		data0 = self.__buildTree( IECore.CompoundObject, 0 )
		data1 = self.__buildTree( IECore.CompoundObject, 1 )
		data2 = self.__buildTree( IECore.CompoundObject, 2 )
		data3 = self.__buildTree( IECore.CompoundObject, 3 )
		self.assertEqual( IECore.linearObjectInterpolation( data1, data2, 0.5 ), self.__buildTree( IECore.CompoundObject, 1.5 ) )
		self.assertEqual( IECore.cubicObjectInterpolation( data0, data1, data2, data3, 0.5 ), self.__buildTree( IECore.CompoundObject, 1.5 ) )

	def testSimpleCubicInterpolation( self ) :

		self.assertEqual( IECore.cubicObjectInterpolation( IECore.IntData(1), IECore.IntData(2), IECore.IntData(3), IECore.IntData(4), 0.5 ), None )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.FloatData(1), IECore.FloatData(2), IECore.FloatData(3), IECore.FloatData(4), 0.5 ), IECore.FloatData(2.5) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.DoubleData(1), IECore.DoubleData(2), IECore.DoubleData(3), IECore.DoubleData(4), 0.5 ), IECore.DoubleData(2.5) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.V2fData( IECore.V2f(1) ), IECore.V2fData( IECore.V2f(2) ), IECore.V2fData( IECore.V2f(3) ), IECore.V2fData( IECore.V2f(4) ), 0.5 ), IECore.V2fData( IECore.V2f(2.5) ) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.V3fData( IECore.V3f(1) ), IECore.V3fData( IECore.V3f(2) ), IECore.V3fData( IECore.V3f(3) ), IECore.V3fData( IECore.V3f(4) ), 0.5 ), IECore.V3fData( IECore.V3f(2.5) ) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.V2dData( IECore.V2d(1) ), IECore.V2dData( IECore.V2d(2) ), IECore.V2dData( IECore.V2d(3) ), IECore.V2dData( IECore.V2d(4) ), 0.5 ), IECore.V2dData( IECore.V2d(2.5) ) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.V3dData( IECore.V3d(1) ), IECore.V3dData( IECore.V3d(2) ), IECore.V3dData( IECore.V3d(3) ), IECore.V3dData( IECore.V3d(4) ), 0.5 ), IECore.V3dData( IECore.V3d(2.5) ) )

	def testVectorCubicInterpolation( self ):

		self.assertEqual( IECore.cubicObjectInterpolation( IECore.IntVectorData( [1] ), IECore.IntVectorData( [2] ), IECore.IntVectorData( [1] ), IECore.IntVectorData( [2] ), 0.5 ), None )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.FloatVectorData( [1]), IECore.FloatVectorData( [2] ), IECore.FloatVectorData( [3]), IECore.FloatVectorData( [4] ), 0.5 ), IECore.FloatVectorData([2.5]) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.DoubleVectorData( [1]), IECore.DoubleVectorData( [2] ), IECore.DoubleVectorData( [3]), IECore.DoubleVectorData( [4] ), 0.5 ), IECore.DoubleVectorData([2.5]) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.V2fVectorData( [IECore.V2f(1)] ), IECore.V2fVectorData( [IECore.V2f(2)] ), IECore.V2fVectorData( [IECore.V2f(3)] ), IECore.V2fVectorData( [IECore.V2f(4)] ), 0.5 ), IECore.V2fVectorData( [IECore.V2f(2.5)] ) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.V3fVectorData( [IECore.V3f(1)] ), IECore.V3fVectorData( [IECore.V3f(2)] ), IECore.V3fVectorData( [IECore.V3f(3)] ), IECore.V3fVectorData( [IECore.V3f(4)] ), 0.5 ), IECore.V3fVectorData( [IECore.V3f(2.5)] ) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.V2dVectorData( [IECore.V2d(1)] ), IECore.V2dVectorData( [IECore.V2d(2)] ), IECore.V2dVectorData( [IECore.V2d(3)] ), IECore.V2dVectorData( [IECore.V2d(4)] ), 0.5 ), IECore.V2dVectorData( [IECore.V2d(2.5)] ) )
		self.assertEqual( IECore.cubicObjectInterpolation( IECore.V3dVectorData( [IECore.V3d(1)] ), IECore.V3dVectorData( [IECore.V3d(2)] ), IECore.V3dVectorData( [IECore.V3d(3)] ), IECore.V3dVectorData( [IECore.V3d(4)] ), 0.5 ), IECore.V3dVectorData( [IECore.V3d(2.5)] ) )

	def testNonSupportedInterpolation( self ):

		obj1 = IECore.Camera()
		obj2 = IECore.Camera()
		obj3 = IECore.Camera()
		obj4 = IECore.Camera()
		self.assertEqual( IECore.linearObjectInterpolation( obj1, obj2, 0.5), None )
		self.assertEqual( IECore.cubicObjectInterpolation( obj1, obj2, obj3, obj4, 0.5), None )

	def testPrimitiveInterpolation( self ) :

		m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		m2 = IECore.TransformOp()( input=m1, matrix = IECore.M44fData( IECore.M44f.createScaled( IECore.V3f( 2 ) ) ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3, IECore.TransformOp()( input=m1, matrix = IECore.M44fData( IECore.M44f.createScaled( IECore.V3f( 1.5 ) ) ) ) )

	def testPrimitiveInterpolationMaintainsUninterpolableValuesFromFirstPrimitive( self ) :

		m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		m1["c"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "hi" ) )
		m2 = m1.copy()

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3["c"], IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "hi" ) ) )

		m2["c"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "bye" ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3["c"], IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "hi" ) ) )

	def testPrimitiveInterpolationMaintainsValuesMissingFromSecondPrimitive( self ) :

		m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		m2 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )

		m1["v"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3, 4 ] ) )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3["v"], IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3, 4 ] ) ) )

	def testPrimitiveInterpolationWithBlindData( self ) :

		m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		m2 = m1.copy()

		m1.blindData()["a"] = IECore.FloatData( 10 )
		m2.blindData()["a"] = IECore.FloatData( 20 )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3.blindData()["a"], IECore.FloatData( 15 ) )

	def testPrimitiveInterpolationWithBlindDataMaintainsValuesMissingFromSecondPrimitive( self ) :

		m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		m2 = m1.copy()

		m1.blindData()["a"] = IECore.FloatData( 10 )

		m3 = IECore.linearObjectInterpolation( m1, m2, 0.5 )
		self.assertEqual( m3.blindData()["a"], IECore.FloatData( 10 ) )

if __name__ == "__main__":
    unittest.main()
