##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
import imath

import IECore
import IECoreScene

class TestPointsPrimitive( unittest.TestCase ) :

	def testPrimitiveVariable( self ) :

		v = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 1 ) )
		self.assertEqual( v.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( v.data, IECore.FloatData( 1 ) )

		v.interpolation = IECoreScene.PrimitiveVariable.Interpolation.Vertex
		self.assertEqual( v.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		v.data = IECore.IntVectorData( [ 1, 2, 3, 4 ] )
		self.assertEqual( v.data, IECore.IntVectorData( [ 1, 2, 3, 4 ] ) )

	def testPrimitive( self ) :

		"""This test mainly tests the Primitive aspects of the PointPrimitive"""

		p = IECoreScene.PointsPrimitive( 10 )

		self.assertEqual( p.numPoints, 10 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 10 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 10 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 10 )

		self.assertEqual( p, p )
		self.assertEqual( p, p.copy() )

		# try adding a primvar
		self.assertEqual( len( p ), 0 )
		self.assert_( not "P" in p )
		p["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData() )
		self.assertEqual( p, p )
		self.assertEqual( p, p.copy() )
		self.assertEqual( len( p ), 1 )
		self.assert_( "P" in p )
		self.assertEqual( p["P"].data, IECore.V3fVectorData() )

		# and removing it
		self.assertEqual( p["P"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		del p["P"]
		self.assertEqual( len( p ), 0 )
		self.assert_( not "P" in p )

		# and adding it and another
		p["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData() )
		self.assert_( not "N" in p )
		p["N"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData() )
		self.assert_( "N" in p )
		self.assertEqual( len( p ), 2 )
		self.assertEqual( p["N"].data, IECore.V3fVectorData() )
		self.assertEqual( p["N"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		# and overwriting one with the other
		p["N"] = p["P"]
		self.assert_( p["N"].data.isSame( p["P"].data ) )

	def testConstructors( self ) :

		p = IECoreScene.PointsPrimitive( 20 )
		self.assertEqual( p.numPoints, 20 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 20 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 20 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 20 )
		self.assertEqual( len( p ), 0 )

		p = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 1 ) ] ) )
		self.assertEqual( p.numPoints, 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 1 )
		self.assertEqual( len( p ), 1 )
		self.assert_( "P" in p )
		self.assertEqual( p["P"].data, IECore.V3fVectorData( [ imath.V3f( 1 ) ], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( p["P"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		p = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 1 ) ] ), IECore.FloatVectorData( [ 1 ] ) )
		self.assertEqual( p.numPoints, 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 1 )
		self.assertEqual( p.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 1 )
		self.assertEqual( len( p ), 2 )
		self.assert_( "P" in p )
		self.assert_( "r" in p )
		self.assertEqual( p["P"].data, IECore.V3fVectorData( [ imath.V3f( 1 ) ], IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( p["P"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p["r"].data, IECore.FloatVectorData( [ 1 ] ) )
		self.assertEqual( p["r"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

	def testNumPointsAccess( self ) :

		p = IECoreScene.PointsPrimitive( 20 )
		self.assertEqual( p.numPoints, 20 )
		p.numPoints = 40
		self.assertEqual( p.numPoints, 40 )

	def testHash( self ) :

		p = IECoreScene.PointsPrimitive( 1 )
		p2 = IECoreScene.PointsPrimitive( 2 )

		self.assertNotEqual( p.hash(), p2.hash() )
		self.assertNotEqual( p.topologyHash(), p2.topologyHash() )

		p3 = p2.copy()
		self.assertEqual( p3.hash(), p2.hash() )
		self.assertEqual( p3.topologyHash(), p2.topologyHash() )
		p3["primVar"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.IntData( 10 ) )
		self.assertNotEqual( p3.hash(), p2.hash() )
		self.assertEqual( p3.topologyHash(), p2.topologyHash() )

	def testBound( self ) :

		p = IECoreScene.PointsPrimitive( 2 )
		self.assertEqual( p.bound(), imath.Box3f() )

		p["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ imath.V3f( 1, 2, 3 ), imath.V3f( 12, 13, 14 ) ] )
		)

		# when no width is specified, it defaults to 1
		self.assertEqual(
			p.bound(),
			imath.Box3f(
				imath.V3f( 1, 2, 3 ) - imath.V3f( 1 ) / 2.0,
				imath.V3f( 12, 13, 14 ) + imath.V3f( 1 ) / 2.0
			)
		)

		# constantwidth overrides the default
		p["constantwidth"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, 2.0 )

		self.assertEqual(
			p.bound(),
			imath.Box3f(
				imath.V3f( 1, 2, 3 ) - imath.V3f( 2 ) / 2.0,
				imath.V3f( 12, 13, 14 ) + imath.V3f( 2 ) / 2.0
			)
		)

		# vertex width works too, and multiplies with constantwidth

		p["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 2, 4 ] ) )

		self.assertEqual(
			p.bound(),
			imath.Box3f(
				imath.V3f( 1, 2, 3 ) - imath.V3f( 2 * 2 ) / 2.0,
				imath.V3f( 12, 13, 14 ) + imath.V3f( 2 * 4 ) / 2.0
			)
		)

		# aspect ratio should have no effect whatsoever if type is not "patch"

		p["patchaspectratio"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, 2.0 )
		del p["width"]
		del p["constantwidth"]

		self.assertEqual(
			p.bound(),
			imath.Box3f(
				imath.V3f( 1, 2, 3 ) - imath.V3f( 1 ) / 2.0,
				imath.V3f( 12, 13, 14 ) + imath.V3f( 1 ) / 2.0
			)
		)

		# but it should take effect when type is "patch"

		p["type"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, "patch" )

		diagonal = math.sqrt( 1 ** 2 + 0.5 ** 2 )

		self.assertEqual(
			p.bound(),
			imath.Box3f(
				imath.V3f( 1, 2, 3 ) - imath.V3f( diagonal ) / 2.0,
				imath.V3f( 12, 13, 14 ) + imath.V3f( diagonal ) / 2.0
			)
		)

		# and "constantwidth" should still be taken into account for patches

		p["constantwidth"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, 2.0 )

		self.assertEqual(
			p.bound(),
			imath.Box3f(
				imath.V3f( 1, 2, 3 ) - imath.V3f( 2 * diagonal ) / 2.0,
				imath.V3f( 12, 13, 14 ) + imath.V3f( 2 * diagonal ) / 2.0
			)
		)

		# as should "width"

		p["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 2, 4 ] ) )

		self.assertEqual(
			p.bound(),
			imath.Box3f(
				imath.V3f( 1, 2, 3 ) - imath.V3f( 2 * 2 * diagonal ) / 2.0,
				imath.V3f( 12, 13, 14 ) + imath.V3f( 2 * 4 * diagonal ) / 2.0
			)
		)

if __name__ == "__main__":
    unittest.main()
