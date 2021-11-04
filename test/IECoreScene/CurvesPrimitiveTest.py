##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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
import os.path
import math
import unittest
import imath
import IECore
import IECoreScene

class CurvesPrimitiveTest( unittest.TestCase ) :

	def testConstructors( self ) :

		c = IECoreScene.CurvesPrimitive()
		self.assertEqual( c.verticesPerCurve(), IECore.IntVectorData() )
		self.assertEqual( c.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( c.periodic(), False )
		self.assertEqual( c.keys(), [] )
		self.assertEqual( c.numCurves(), 0 )

		c = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 2 ] ) )
		self.assertEqual( c.verticesPerCurve(), IECore.IntVectorData( [ 2 ] ) )
		self.assertEqual( c.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( c.periodic(), False )
		self.assertEqual( c.keys(), [] )
		self.assertEqual( c.numCurves(), 1 )

		c = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.bSpline() )
		self.assertEqual( c.verticesPerCurve(), IECore.IntVectorData( [ 4 ] ) )
		self.assertEqual( c.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( c.periodic(), False )
		self.assertEqual( c.keys(), [] )
		self.assertEqual( c.numCurves(), 1 )

		c = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.bSpline(), True )
		self.assertEqual( c.verticesPerCurve(), IECore.IntVectorData( [ 4 ] ) )
		self.assertEqual( c.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( c.periodic(), True )
		self.assertEqual( c.keys(), [] )
		self.assertEqual( c.numCurves(), 1 )

		i = IECore.IntVectorData( [ 4 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ) ] )
		c = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), True, p )
		self.assertEqual( c.verticesPerCurve(), IECore.IntVectorData( [ 4 ] ) )
		self.assertEqual( c.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( c.periodic(), True )
		self.assertEqual( c.keys(), [ "P" ] )
		self.assertNotEqual( c["P"].data, p )
		self.assertEqual( c["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		pp = p.copy()
		pp.setInterpretation( IECore.GeometricData.Interpretation.Point )
		self.assertEqual( c["P"].data, pp )
		self.assertFalse( c["P"].data.isSame( p ) )
		self.assertFalse( c["P"].data.isSame( pp ) )
		self.assertFalse( c.verticesPerCurve().isSame( i ) )

	def testConstructorValidation( self ) :

		self.assertRaises( Exception, IECoreScene.CurvesPrimitive, IECore.IntVectorData( [ 1 ] ) )
		self.assertRaises( Exception, IECoreScene.CurvesPrimitive, IECore.IntVectorData( [ 3 ] ), IECore.CubicBasisf.bSpline() )
		self.assertRaises( Exception, IECoreScene.CurvesPrimitive, IECore.IntVectorData( [ 5 ] ), IECore.CubicBasisf.bezier() )

	def testConstructorKeywords( self ) :

		c = IECoreScene.CurvesPrimitive(
			verticesPerCurve = IECore.IntVectorData( [ 3 ] ),
			periodic = True,
			p = IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 3 ) ] )
		)

		self.assertEqual( c.verticesPerCurve(), IECore.IntVectorData( [ 3 ] ) )
		self.assertEqual( c.periodic(), True )
		self.assertEqual( c.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( c["P"].data, IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 3 ) ], IECore.GeometricData.Interpretation.Point ) )

	def testCopy( self ) :

		i = IECore.IntVectorData( [ 4 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ) ] )
		c = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), True, p )

		cc = c.copy()
		self.assertEqual( c, cc )

	def testIO( self ) :

		i = IECore.IntVectorData( [ 4 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ) ] )
		c = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), True, p )

		IECore.Writer.create( c, os.path.join( "test", "IECore", "data", "curves.cob" ) ).write()

		cc = IECore.Reader.create( os.path.join( "test", "IECore", "data", "curves.cob" ) ).read()

		self.assertEqual( cc, c )

		c = IECore.Reader.create( os.path.join( "test", "IECore", "data", "cobFiles", "torusCurves.cob" ) ).read()

	def testVariableSize( self ) :

		c = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.bSpline(), True )

		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 4 )

		# asking for the constant size of a single curve makes no sense
		self.assertRaises( Exception, c.variableSize, IECoreScene.PrimitiveVariable.Interpolation.Constant, 0 )
		# as does asking for the size of a nonexistent curve
		self.assertRaises( Exception, c.variableSize, IECoreScene.PrimitiveVariable.Interpolation.Vertex, 1 )

		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform, 0 ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex, 0 ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying, 0 ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, 0 ), 4 )
		self.assertEqual( c.numSegments( 0 ), 4 )

		c = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.bSpline(), False )

		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 2 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 2 )

		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform, 0 ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex, 0 ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying, 0 ), 2 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, 0 ), 2 )
		self.assertEqual( c.numSegments( 0 ), 1 )

	def testSetTopology( self ) :

		c = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.bSpline(), True )

		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 4 )

		newVertsPerCurve = IECore.IntVectorData( [ 4, 4 ] )
		c.setTopology( newVertsPerCurve, IECore.CubicBasisf.bezier(), False )

		self.assertEqual( c.verticesPerCurve(), newVertsPerCurve )
		self.assertEqual( c.basis(), IECore.CubicBasisf.bezier() )
		self.assertEqual( c.periodic(), False )
		self.assertEqual( c.numCurves(), 2 )

		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 2 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( c.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 4 )

		newVertsPerCurve.append( 10 )
		self.assertEqual( c.verticesPerCurve(), IECore.IntVectorData( [ 4, 4 ] ) )

	def testHash( self ) :

		c = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.bSpline(), True )
		h = c.hash()
		t = c.topologyHash()

		c2 = c.copy()
		self.assertEqual( c2.hash(), h )
		self.assertEqual( c2.topologyHash(), t )

		c.setTopology( IECore.IntVectorData( [ 5 ] ), IECore.CubicBasisf.bSpline(), True )
		self.assertNotEqual( c.hash(), h )
		self.assertNotEqual( c.topologyHash(), h )
		h = c.hash()
		t = c.topologyHash()

		c.setTopology( IECore.IntVectorData( [ 5 ] ), IECore.CubicBasisf.catmullRom(), True )
		self.assertNotEqual( c.hash(), h )
		self.assertNotEqual( c.topologyHash(), h )
		h = c.hash()
		t = c.topologyHash()

		c.setTopology( IECore.IntVectorData( [ 5 ] ), IECore.CubicBasisf.catmullRom(), False )
		self.assertNotEqual( c.hash(), h )
		self.assertNotEqual( c.topologyHash(), h )
		h = c.hash()
		t = c.topologyHash()

		c["primVar"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.IntData( 10 ) )
		self.assertNotEqual( c.hash(), h )
		self.assertEqual( c.topologyHash(), t )

	def tearDown( self ) :

		if os.path.isfile( os.path.join( "test", "IECore", "data", "curves.cob" ) ) :
			os.remove( os.path.join( "test", "IECore", "data", "curves.cob" ) )

if __name__ == "__main__":
    unittest.main()

