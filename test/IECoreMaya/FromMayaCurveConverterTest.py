##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreMaya
import unittest
import MayaUnitTest
import maya.cmds

class FromMayaCurveConverterTest( unittest.TestCase ) :

	def testCubicCircle( self ) :
		
		circle = maya.cmds.circle( ch = False )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]
		
		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECore.CurvesPrimitive.staticTypeId() )
		
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )
		
		curve = converter.convert()
		
		self.assert_( curve.isInstanceOf( IECore.CurvesPrimitive.staticTypeId() ) )
				
		# check topology
		self.assertEqual( curve.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( curve.verticesPerCurve(), IECore.IntVectorData( [ 8 ] ) )
		self.assertEqual( curve.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( curve.periodic(), True )
		
		# check primvars
		self.assertEqual( curve.keys(), [ "P" ] )
		self.assertEqual( curve["P"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		
		p = curve["P"].data
		self.assertEqual( len( p ), 8 )
		for pp in p :
			self.assertEqual( pp.z, 0 )
			self.assertAlmostEqual( pp.length(), 1.1, 1 )
						
	def testLinearCircle( self ) :
	
		circle = maya.cmds.circle( ch = False, degree=1 )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]
		
		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECore.CurvesPrimitive.staticTypeId() )
		
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCurveConverter ) ) )
		
		curve = converter.convert()
		self.assert_( curve.isInstanceOf( IECore.CurvesPrimitive.staticTypeId() ) )
		
		# check topology
		# bizarrely maya doesn't make linear circles as periodic - it just repeats the first point
		# at the end instead.
		self.assertEqual( curve.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 9 )
		self.assertEqual( curve.verticesPerCurve(), IECore.IntVectorData( [ 9 ] ) )
		self.assertEqual( curve.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( curve.periodic(), False )
		
		# check primvars
		self.assertEqual( curve.keys(), [ "P" ] )
		self.assertEqual( curve["P"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		
		p = curve["P"].data
		self.assertEqual( len( p ), 9 )
		for pp in p :
			self.assertEqual( pp.z, 0 )
			self.assertAlmostEqual( pp.length(), 1, 5 )

	def testCubicArc( self ) :
	
		arc = maya.cmds.circle( ch = False, sweep=180 )[0]
		arc = maya.cmds.listRelatives( arc, shapes=True )[0]
		
		converter = IECoreMaya.FromMayaShapeConverter.create( str( arc ), IECore.CurvesPrimitive.staticTypeId() )
		
		self.assert_( converter.isInstanceOf( IECoreMaya.FromMayaCurveConverter.staticTypeId() ) )
		
		curve = converter.convert()
		IECore.Writer.create( curve, "/tmp/curve.cob" ).write()
		self.assert_( curve.isInstanceOf( IECore.CurvesPrimitive.staticTypeId() ) )
		
		# check topology
		self.assertEqual( curve.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 15 )
		self.assertEqual( curve.verticesPerCurve(), IECore.IntVectorData( [ 15 ] ) )
		self.assertEqual( curve.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( curve.periodic(), False )
		
		# check primvars
		self.assertEqual( curve.keys(), [ "P" ] )
		self.assertEqual( curve["P"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		
		p = curve["P"].data
		self.assertEqual( len( p ), 15 )
		for pp in p :
			self.assertEqual( pp.z, 0 )
			self.assert_( pp.length() > 0.999 and pp.length() < 1.11)
	
		self.assertEqual( p[0], p[1] )
		self.assertEqual( p[1], p[2] )
		self.assertEqual( p[-1], p[-2] )
		self.assertEqual( p[-2], p[-3] )
	
	def testSpaces( self ) :
	
		arc = maya.cmds.circle( ch = False, sweep=180 )[0]
		maya.cmds.move( 1, 2, 3, arc )
		arc = maya.cmds.listRelatives( arc, shapes=True )[0]
		
		converter = IECoreMaya.FromMayaShapeConverter.create( str( arc ), IECore.CurvesPrimitive.staticTypeId() )
		
		self.assertEqual( converter.space.getNumericValue(), IECoreMaya.FromMayaCurveConverter.Space.Object )
		c = converter.convert()
		self.assert_( IECore.Box3f( IECore.V3f( -1.1, -1.01, -0.01 ), IECore.V3f( 0.01, 1.01, 0.01 ) ).contains( c.bound() ) ) 		
		
		converter.space.setNumericValue( IECoreMaya.FromMayaCurveConverter.Space.World )
		c = converter.convert()
		self.assert_( IECore.Box3f( IECore.V3f( -0.1, 0.99, 2.99 ), IECore.V3f( 1.01, 3.01, 3.01 ) ).contains( c.bound() ) ) 		

	def testCubicCircleAsLinear( self ) :
	
		circle = maya.cmds.circle( ch = False )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]
		
		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECore.CurvesPrimitive.staticTypeId() )
		converter.linearBasis.setTypedValue( True )
		
		curve = converter.convert()
		self.assert_( curve.isInstanceOf( IECore.CurvesPrimitive.staticTypeId() ) )
		
		# check topology
		self.assertEqual( curve.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( curve.verticesPerCurve(), IECore.IntVectorData( [ 8 ] ) )
		self.assertEqual( curve.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( curve.periodic(), True )
		
		# check primvars
		self.assertEqual( curve.keys(), [ "P" ] )
		self.assertEqual( curve["P"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		
		p = curve["P"].data
		self.assertEqual( len( p ), 8 )
		for pp in p :
			self.assertEqual( pp.z, 0 )
			self.assert_( pp.length() > 0.999 and pp.length() < 1.11)
	
	def testBlindData( self ) :
	
		circle = maya.cmds.circle( ch = False )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]
		
		maya.cmds.addAttr( circle, dataType="string", longName="ieString" )
		maya.cmds.setAttr( circle + ".ieString", "banana", type="string" )
		
		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECore.CurvesPrimitive.staticTypeId() )
		curve = converter.convert()
		
		self.assertEqual( len( curve.blindData().keys() ), 2 )
		self.assertEqual( curve.blindData()["name"], IECore.StringData( "nurbsCircleShape1" ) )
		self.assertEqual( curve.blindData()["ieString"], IECore.StringData( "banana" ) )
						
	def testPrimVars( self ) :
		
		circle = maya.cmds.circle( ch = False )[0]
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]
		
		maya.cmds.addAttr( circle, attributeType="float", longName="delightDouble", defaultValue=1 )
		
		converter = IECoreMaya.FromMayaShapeConverter.create( str( circle ), IECore.CurvesPrimitive.staticTypeId() )
		curve = converter.convert()
		
		self.assertEqual( len( curve.keys() ), 2 )
		self.assertEqual( curve["Double"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( curve["Double"].data, IECore.FloatData( 1 ) )
		
	def testConvertFromPlug( self ) :
		
		raise NotImplementedError
							
if __name__ == "__main__":
	MayaUnitTest.TestProgram()
