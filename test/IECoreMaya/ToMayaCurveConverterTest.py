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

import maya.cmds
import maya.OpenMaya as OpenMaya
import imath

import IECore
import IECoreScene
import IECoreMaya

class ToMayaCurveConverterTest( IECoreMaya.TestCase ) :

	def testConstructor( self ) :

		i = IECore.IntVectorData( [ 8 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 0 ), imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ), imath.V3f( 3 ), imath.V3f( 3 ) ] )
		coreCurves = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), False, p )

		converter = IECoreMaya.ToMayaCurveConverter( coreCurves )
		transform = maya.cmds.createNode( "transform" )
		converter.convert( transform )
		self.assertEqual( maya.cmds.nodeType( maya.cmds.listRelatives( transform, shapes=True )[0] ), "nurbsCurve" )

	def testConversion( self ) :

		# open, cubic curve:
		i = IECore.IntVectorData( [ 8 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 0 ), imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ), imath.V3f( 3 ), imath.V3f( 3 )   ] )
		coreCurves = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), False, p )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreCurves )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaCurveConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaObjectConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECore.FromCoreConverter.staticTypeId() ) )

		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )

		mayaCurve = maya.cmds.listRelatives( transform, shapes=True )[0]

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".boundingBoxMin" ), [( 0, 0, 0 )] )
		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".boundingBoxMax" ), [( 3, 3, 3 )] )

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".degree" ), 3 )

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".cv[*]" ), [(0,0,0),(1,1,1),(2,2,2),(3,3,3)] )

	def testPeriodic( self ) :

		i = IECore.IntVectorData( [ 4 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 1, 0, 0 ) ] )
		coreCurves = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), True, p )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreCurves )
		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )

		mayaCurve = maya.cmds.listRelatives( transform, shapes=True )[0]

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".boundingBoxMin" ), [( 0, 0, 0 )] )
		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".boundingBoxMax" ), [( 1, 1, 0 )] )

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".degree" ), 3 )

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".cv[*]" ), [(0,0,0),(0,0,0),(0,0,0),(0,1,0),(1,1,0),(1,0,0)] )

	def testOpenLinear( self ) :

		# open, cubic curve:
		i = IECore.IntVectorData( [ 4 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ) ] )
		coreCurves = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.linear(), False, p )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreCurves )

		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )

		mayaCurve = maya.cmds.listRelatives( transform, shapes=True )[0]

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".boundingBoxMin" ), [( 0, 0, 0 )] )
		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".boundingBoxMax" ), [( 3, 3, 3 )] )

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".degree" ), 1 )

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".cv[*]" ), [(0,0,0),(1,1,1),(2,2,2),(3,3,3)] )

	def testPeriodicLinear( self ) :

		i = IECore.IntVectorData( [ 4 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0, 0, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 1, 1, 0 ), imath.V3f( 1, 0, 0 ) ] )
		coreCurves = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.linear(), True, p )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreCurves )
		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )

		mayaCurve = maya.cmds.listRelatives( transform, shapes=True )[0]

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".boundingBoxMin" ), [( 0, 0, 0 )] )
		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".boundingBoxMax" ), [( 1, 1, 0 )] )

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".degree" ), 1 )

		self.assertEqual( maya.cmds.getAttr( mayaCurve + ".cv[*]" ), [(0,0,0),(0,1,0),(1,1,0),(1,0,0)] )

	def testCurveIndex( self ):

		i = IECore.IntVectorData( [ 8, 9, 10, 11] )
		cvs = [ imath.V3f( 0 ), imath.V3f( 0 ), imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ), imath.V3f( 3 ), imath.V3f( 3 ) ]
		cvs.extend([ imath.V3f( 4 ), imath.V3f( 4 ), imath.V3f( 4 ), imath.V3f( 5 ), imath.V3f( 6 ), imath.V3f( 7 ), imath.V3f( 8 ), imath.V3f( 8 ), imath.V3f( 8 ) ])
		cvs.extend([ imath.V3f( 9 ), imath.V3f( 9 ), imath.V3f( 9 ), imath.V3f( 10 ), imath.V3f( 11 ), imath.V3f( 12 ), imath.V3f( 13 ), imath.V3f( 14 ), imath.V3f( 14 ), imath.V3f( 14 ) ])
		cvs.extend([ imath.V3f( 15 ), imath.V3f( 15 ), imath.V3f( 15 ), imath.V3f( 16 ), imath.V3f( 17 ), imath.V3f( 18 ), imath.V3f( 19 ), imath.V3f( 20 ), imath.V3f( 21 ), imath.V3f( 21 ), imath.V3f( 21 ) ])
		p = IECore.V3fVectorData( cvs )
		coreCurves = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), False, p )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreCurves )
		transform1 = maya.cmds.createNode( "transform" )
		transform2 = maya.cmds.createNode( "transform" )
		transform3 = maya.cmds.createNode( "transform" )
		transform4 = maya.cmds.createNode( "transform" )

		# should default to converting curve zero:
		self.assertEqual( converter["index"].getNumericValue(), 0 )

		# convert the curves separately:
		self.assertTrue( converter.convert( transform1 ) )
		converter["index"].setNumericValue( 1 )
		self.assertTrue( converter.convert( transform2 ) )
		converter["index"].setNumericValue( 2 )
		self.assertTrue( converter.convert( transform3 ) )
		converter["index"].setNumericValue( 3 )
		self.assertTrue( converter.convert( transform4 ) )

		mayaCurve1 = maya.cmds.listRelatives( transform1, shapes=True )[0]
		mayaCurve2 = maya.cmds.listRelatives( transform2, shapes=True )[0]
		mayaCurve3 = maya.cmds.listRelatives( transform3, shapes=True )[0]
		mayaCurve4 = maya.cmds.listRelatives( transform4, shapes=True )[0]

		self.assertEqual( maya.cmds.getAttr( mayaCurve1 + ".cv[*]" ), [ (n,n,n) for n in range( 0,4 ) ] )
		self.assertEqual( maya.cmds.getAttr( mayaCurve2 + ".cv[*]" ), [ (n,n,n) for n in range( 4,9 ) ] )
		self.assertEqual( maya.cmds.getAttr( mayaCurve3 + ".cv[*]" ), [ (n,n,n) for n in range( 9,15 ) ] )
		self.assertEqual( maya.cmds.getAttr( mayaCurve4 + ".cv[*]" ), [ (n,n,n) for n in range( 15,22 ) ] )

		# should error gracefully if we specify a rubbish curve index:
		converter["index"].setNumericValue( -1 )
		self.assertFalse( converter.convert( transform1 ) )
		converter["index"].setNumericValue( 4 )
		self.assertFalse( converter.convert( transform1 ) )

	def testWrongCubicCurve( self ):

		i = IECore.IntVectorData( [ 4 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ) ] )
		coreCurves = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), False, p )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreCurves )
		transform = maya.cmds.createNode( "transform" )
		self.assertFalse( converter.convert( transform ) )

	def testToMayaAndBack( self ):

		# open, cubic curve:
		i = IECore.IntVectorData( [ 8 ] )
		p = IECore.V3fVectorData( [ imath.V3f( 0 ), imath.V3f( 0 ), imath.V3f( 0 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ), imath.V3f( 3 ), imath.V3f( 3 )   ] )
		coreCurves = IECoreScene.CurvesPrimitive( i, IECore.CubicBasisf.bSpline(), False, p )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreCurves )
		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )

		mayaCurve = maya.cmds.listRelatives( transform, shapes=True )[0]
		converter = IECoreMaya.FromMayaShapeConverter.create( mayaCurve, IECoreScene.CurvesPrimitive.staticTypeId() )
		curve = converter.convert()

		self.assertEqual( curve['P'], coreCurves['P'] )
		self.assertEqual( curve.numCurves(), coreCurves.numCurves() )
		self.assertEqual( curve.basis(), coreCurves.basis() )
		self.assertEqual( curve.verticesPerCurve(), coreCurves.verticesPerCurve() )



if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
