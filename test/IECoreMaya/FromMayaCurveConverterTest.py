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
import imath

import IECore
import IECoreScene
import IECoreMaya

class FromMayaCurveConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		circle = maya.cmds.circle( ch = False )[0]
		circle = str( maya.cmds.listRelatives( circle, shapes=True )[0] )

		converter = IECoreMaya.FromMayaShapeConverter.create( circle, IECoreScene.CurvesPrimitive.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )

		converter = IECoreMaya.FromMayaShapeConverter.create( circle, IECoreScene.Primitive.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )

		converter = IECoreMaya.FromMayaShapeConverter.create( circle )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( circle, IECoreScene.CurvesPrimitive.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( circle, IECoreScene.Primitive.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( circle )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )

	def testCubicCircle( self ) :

		circle = maya.cmds.circle( ch = False )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECoreScene.CurvesPrimitive.staticTypeId() )

		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )

		curve = converter.convert()

		self.assertTrue( curve.isInstanceOf( IECoreScene.CurvesPrimitive.staticTypeId() ) )

		# check topology
		self.assertEqual( curve.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( curve.verticesPerCurve(), IECore.IntVectorData( [ 8 ] ) )
		self.assertEqual( curve.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( curve.periodic(), True )

		# check primvars
		self.assertEqual( list(curve.keys()), [ "P" ] )
		self.assertEqual( curve["P"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( curve["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		p = curve["P"].data
		self.assertEqual( len( p ), 8 )
		for pp in p :
			self.assertEqual( pp.z, 0 )
			self.assertAlmostEqual( pp.length(), 1.1, 1 )

	def testLinearCircle( self ) :

		circle = maya.cmds.circle( ch = False, degree=1 )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECoreScene.CurvesPrimitive.staticTypeId() )

		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )

		curve = converter.convert()
		self.assertTrue( curve.isInstanceOf( IECoreScene.CurvesPrimitive.staticTypeId() ) )

		# check topology
		# bizarrely maya doesn't make linear circles as periodic - it just repeats the first point
		# at the end instead.
		self.assertEqual( curve.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 9 )
		self.assertEqual( curve.verticesPerCurve(), IECore.IntVectorData( [ 9 ] ) )
		self.assertEqual( curve.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( curve.periodic(), False )

		# check primvars
		self.assertEqual( list(curve.keys()), [ "P" ] )
		self.assertEqual( curve["P"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( curve["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		p = curve["P"].data
		self.assertEqual( len( p ), 9 )
		for pp in p :
			self.assertEqual( pp.z, 0 )
			self.assertAlmostEqual( pp.length(), 1, 5 )

	def testCubicArc( self ) :

		arc = maya.cmds.circle( ch = False, sweep=180 )[0]
		arc = maya.cmds.listRelatives( arc, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( str( arc ), IECoreScene.CurvesPrimitive.staticTypeId() )

		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaCurveConverter.staticTypeId() ) )

		curve = converter.convert()
		IECore.Writer.create( curve, "/tmp/curve.cob" ).write()
		self.assertTrue( curve.isInstanceOf( IECoreScene.CurvesPrimitive.staticTypeId() ) )

		# check topology
		self.assertEqual( curve.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 15 )
		self.assertEqual( curve.verticesPerCurve(), IECore.IntVectorData( [ 15 ] ) )
		self.assertEqual( curve.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( curve.periodic(), False )

		# check primvars
		self.assertEqual( list(curve.keys()), [ "P" ] )
		self.assertEqual( curve["P"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( curve["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		p = curve["P"].data
		self.assertEqual( len( p ), 15 )
		for pp in p :
			self.assertEqual( pp.z, 0 )
			self.assertTrue( pp.length() > 0.999 and pp.length() < 1.11)

		self.assertEqual( p[0], p[1] )
		self.assertEqual( p[1], p[2] )
		self.assertEqual( p[-1], p[-2] )
		self.assertEqual( p[-2], p[-3] )

	def testSpaces( self ) :

		arc = maya.cmds.circle( ch = False, sweep=180 )[0]
		maya.cmds.move( 1, 2, 3, arc )
		arc = maya.cmds.listRelatives( arc, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( str( arc ), IECoreScene.CurvesPrimitive.staticTypeId() )

		self.assertEqual( converter["space"].getNumericValue(), IECoreMaya.FromMayaCurveConverter.Space.Object )
		c = converter.convert()
		self.assertTrue(
			IECore.BoxAlgo.contains(
				imath.Box3f( imath.V3f( -1.1, -1.01, -0.01 ), imath.V3f( 0.01, 1.01, 0.01 ) ),
				c.bound()
			)
		)

		converter["space"].setNumericValue( IECoreMaya.FromMayaCurveConverter.Space.World )
		c = converter.convert()
		self.assertTrue(
			IECore.BoxAlgo.contains(
				imath.Box3f( imath.V3f( -0.1, 0.99, 2.99 ), imath.V3f( 1.01, 3.01, 3.01 ) ),
				c.bound()
			)
		)

	def testCubicCircleAsLinear( self ) :

		circle = maya.cmds.circle( ch = False )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECoreScene.CurvesPrimitive.staticTypeId() )
		converter["linearBasis"].setTypedValue( True )

		curve = converter.convert()
		self.assertTrue( curve.isInstanceOf( IECoreScene.CurvesPrimitive.staticTypeId() ) )

		# check topology
		self.assertEqual( curve.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( curve.verticesPerCurve(), IECore.IntVectorData( [ 8 ] ) )
		self.assertEqual( curve.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( curve.periodic(), True )

		# check primvars
		self.assertEqual( list(curve.keys()), [ "P" ] )
		self.assertEqual( curve["P"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( curve["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		p = curve["P"].data
		self.assertEqual( len( p ), 8 )
		for pp in p :
			self.assertEqual( pp.z, 0 )
			self.assertTrue( pp.length() > 0.999 and pp.length() < 1.11)

	def testBlindData( self ) :

		circle = maya.cmds.circle( ch = False )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]

		maya.cmds.addAttr( circle, dataType="string", longName="ieString" )
		maya.cmds.setAttr( circle + ".ieString", "banana", type="string" )

		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECoreScene.CurvesPrimitive.staticTypeId() )
		converter['blindDataAttrPrefix'] = IECore.StringData("ie")
		curve = converter.convert()

		self.assertEqual( len( list(curve.blindData().keys()) ), 2 )
		self.assertEqual( curve.blindData()["name"], IECore.StringData( "nurbsCircleShape1" ) )
		self.assertEqual( curve.blindData()["ieString"], IECore.StringData( "banana" ) )

	def testPrimVars( self ) :

		circle = maya.cmds.circle( ch = False )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]

		maya.cmds.addAttr( circle, attributeType="float", longName="iePrimVarDouble", defaultValue=1 )

		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECoreScene.CurvesPrimitive.staticTypeId() )
		curve = converter.convert()

		self.assertEqual( len( list(curve.keys()) ), 2 )
		self.assertEqual( curve["Double"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( curve["Double"].data, IECore.FloatData( 1 ) )

	def testConvertFromPlug( self ) :

		circle = maya.cmds.circle( ch = False )[0]
		maya.cmds.move( 1, 2, 3, circle )
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]

		converter = IECoreMaya.FromMayaPlugConverter.create( circle + ".worldSpace" )

		converter["space"].setNumericValue( IECoreMaya.FromMayaShapeConverter.Space.World )
		curve = converter.convert()
		self.assertEqual( curve["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertTrue(
			IECore.BoxAlgo.contains(
				imath.Box3f( imath.V3f( -1.11 ) + imath.V3f( 1, 2, 3 ), imath.V3f( 1.11 ) + imath.V3f( 1, 2, 3 ) ),
				curve.bound()
			)
		)

if __name__ == "__main__":
	IECoreMaya.TestProgram()
