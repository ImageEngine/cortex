##########################################################################
#
#  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

import hou
import IECore
import IECoreHoudini
import unittest
import os

class TestFromHoudiniGroupConverter( IECoreHoudini.TestCase ) :

	def geo( self ) :
		geo = hou.node( "/obj/geo1" )
		if not geo :
			obj = hou.node( "/obj" )
			geo = obj.createNode( "geo", run_init_scripts=False )

		return geo

	def points( self ) :
		points = self.geo().createNode( "popnet" )
		points.createNode( "location" )
		color = points.createOutputNode( "color" )
		color.parm( "colortype" ).set( 2 )
		group = color.createOutputNode( "group" )
		group.parm( "crname" ).set( "pointsGroup" )
		hou.setFrame( 50 )
		return group

	def torusA( self ) :
		torus = self.geo().createNode( "torus" )
		color = torus.createOutputNode( "color" )
		color.parm( "colortype" ).set( 2 )
		group = color.createOutputNode( "group" )
		group.parm( "crname" ).set( "torusAGroup" )
		return group
	
	def torusB( self ) :
		torus = self.geo().createNode( "torus" )
		xform = torus.createOutputNode( "xform" )
		xform.parmTuple( "t" ).set( ( 5, 0, 0 ) )
		group = xform.createOutputNode( "group" )
		group.parm( "crname" ).set( "torusBGroup" )
		return group
	
	def twoTorii( self ) :
		merge = self.geo().createNode( "merge" )
		merge.setInput( 0, self.torusA() )
		merge.setInput( 1, self.torusB() )
		return merge
	
	def curve( self ) :
		curve = self.geo().createNode( "curve" )
		curve.parm( "type" ).set( 1 ) # NURBS
		curve.parm( "coords" ).set( '0.160607,0,6.34083 -4.27121,0,5.13891 2.88941,0,5.29624 -4.70504,0,3.14992 2.281,0,3.42478 -3.78185,0,1.35239' )
		color = curve.createOutputNode( "color" )
		color.parm( "colortype" ).set( 2 )
		return color
		
	def box( self ) :
		return self.geo().createNode( "box" )
	
	def curveBox( self ) :
		merge = self.geo().createNode( "merge" )
		merge.setInput( 0, self.curve() )
		merge.setInput( 1, self.box() )
		group = merge.createOutputNode( "group" )
		group.parm( "crname" ).set( "curveBoxGroup" )
		return group
	
	def buildScene( self ) :
		merge = self.geo().createNode( "merge" )
		merge.setInput( 0, self.points() )
		merge.setInput( 1, self.twoTorii() )
		merge.setInput( 2, self.curveBox() )
		return merge

	def testCreateConverter( self )  :
		converter = IECoreHoudini.FromHoudiniGroupConverter( self.box() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )

	def testFactory( self ) :
		scene = self.buildScene()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( scene )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( self.box(), IECore.TypeId.Group )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
				
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( scene, IECore.TypeId.Parameter )
		self.assertEqual( converter, None )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( self.points() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( self.curve() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( self.torusA() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		
		merge = self.twoTorii()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
		
		merge.inputs()[0].bypass( True )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )

		merge.inputs()[1].bypass( True )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		
		group = self.curveBox()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( group )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
		
		group.bypass( True )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( group )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
		
		self.failUnless( IECore.TypeId.Group in IECoreHoudini.FromHoudiniGeometryConverter.supportedTypes() )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.createDummy( IECore.TypeId.Group )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.createDummy( [ IECore.TypeId.Group ] )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
		
		points = self.points()
		torus = self.torusA()
		merge = self.geo().createNode( "merge" )
		merge.setInput( 0, self.points() )
		merge.setInput( 1, self.twoTorii() )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
	
	def testConvertFromHOMGeo( self ) :
		geo = self.curveBox().geometry()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.createFromGeo( geo )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )
		
		result = converter.convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		
		converter2 = IECoreHoudini.FromHoudiniGeometryConverter.createFromGeo( geo, IECore.TypeId.Group )
		self.failUnless( converter2.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )

	def testConvertScene( self ) :
		result = IECoreHoudini.FromHoudiniGroupConverter( self.buildScene() ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 4 )
		self.assertEqual( result.children()[0].blindData()["name"].value, "pointsGroup" )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		self.assertEqual( result.children()[1].blindData()["name"].value, "torusAGroup" )
		self.failUnless( result.children()[1].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[2].blindData()["name"].value, "torusBGroup" )
		self.failUnless( result.children()[2].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[3].blindData()["name"].value, "curveBoxGroup" )
		self.failUnless( result.children()[3].isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()[3].children()), 2 )
		self.assertEqual( result.children()[3].children()[0].blindData(), IECore.CompoundData() )
		self.failUnless( result.children()[3].children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[3].children()[1].blindData(), IECore.CompoundData() )
		self.failUnless( result.children()[3].children()[1].isInstanceOf( IECore.TypeId.CurvesPrimitive ) )
	
	def testConvertSceneFromAttributeValue( self, keepGroups=False ) :
		attribName = "notAGroup"
		merge = self.buildScene()
		attribs = []
		values = []
		groups = [ x for x in merge.parent().children() if x.type().name() == "group" ]
		for group in groups :
			group.bypass( not keepGroups )
			attrib = group.createOutputNode( "attribcreate" )
			attrib.parm( "name1" ).set( attribName )
			attrib.parm( "class1" ).set( 1 ) # Prim
			attrib.parm( "type1" ).set( 3 ) # String
			attrib.parm( "string1" ).set( group.parm( "crname" ).eval() )
			connection = group.outputConnections()[0]
			connection.outputNode().setInput( connection.inputIndex(), attrib )
			attribs.append( attrib )
			values.append( attrib.parm( "string1" ).eval() )
		
		if not keepGroups :
			self.assertEqual( merge.geometry().primGroups(), tuple() )
		
		converter = IECoreHoudini.FromHoudiniGroupConverter( merge )
		converter["groupingMode"].setTypedValue( IECoreHoudini.FromHoudiniGroupConverter.GroupingMode.AttributeValue )
		converter["groupingAttribute"].setTypedValue( attribName )
		result = converter.convert()
		
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 5 )
		self.assertEqual( set([ x.typeId() for x in result.children() ]), set([IECore._IECore.TypeId.CurvesPrimitive, IECore._IECore.TypeId.MeshPrimitive, IECore._IECore.TypeId.PointsPrimitive]) )
		
		for child in result.children() :
			
			self.assertEqual( child.blindData(), IECore.CompoundData() )
			self.failUnless( child.arePrimitiveVariablesValid() )
			self.failUnless( attribName in child )
			self.failUnless( attribName+"Indices" in child )
			self.assertEqual( child[attribName].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
			self.assertEqual( child[attribName+"Indices"].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
			# all primitives point to the same grouping attribute value
			self.assertEqual( child[attribName].data.size(), 1 )
			correctAttrib = [ x for x in attribs if child[attribName].data[0] == x.parm( "string1" ).eval() ]
			self.assertEqual( len(correctAttrib), 1 )
			self.assertEqual( child[attribName+"Indices"].data, IECore.IntVectorData( [ 0 ] * child.variableSize( child[attribName+"Indices"].interpolation ) ) )
			if child[attribName].data[0] == "pointsGroup" :
				self.failUnless( child.isInstanceOf( IECore.TypeId.PointsPrimitive ) )
			elif child[attribName].data[0] == "torusAGroup" :
				self.failUnless( child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
			elif child[attribName].data[0] == "torusBGroup" :
				self.failUnless( child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
			elif child[attribName].data[0] == "curveBoxGroup" :
				self.failUnless( child.isInstanceOf( IECore.TypeId.CurvesPrimitive ) or child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
			else :
				raise RuntimeError, "An unexpected attribute value exists"
		
		null = merge.parent().createNode( "null" )
		self.failUnless( IECoreHoudini.ToHoudiniGeometryConverter.create( result ).convert( null ) )
		for prim in null.geometry().prims() :
			self.failUnless( prim.stringAttribValue( attribName ) in values )
	
	def testConvertSceneFromAttributeValueWithGroups( self ) :
		
		self.testConvertSceneFromAttributeValue( keepGroups = True )
	
	def testAttributeValueExistsButSomeValuesAreEmpty( self ) :
		
		attribName = "notAGroup"
		merge = self.buildScene()
		attribs = []
		values = []
		groups = [ x for x in merge.parent().children() if x.type().name() == "group" ]
		for group in groups :
			group.bypass( True )
			attrib = group.createOutputNode( "attribcreate" )
			attrib.parm( "name1" ).set( attribName )
			attrib.parm( "class1" ).set( 1 ) # Prim
			attrib.parm( "type1" ).set( 3 ) # String
			attrib.parm( "string1" ).set( group.parm( "crname" ).eval() )
			connection = group.outputConnections()[0]
			connection.outputNode().setInput( connection.inputIndex(), attrib )
			attribs.append( attrib )
			values.append( attrib.parm( "string1" ).eval() )
			if group.parm( "crname" ).eval() == "torusBGroup" :
				attrib.bypass( True )
				values[-1] = ""
		
		converter = IECoreHoudini.FromHoudiniGroupConverter( merge )
		converter["groupingMode"].setTypedValue( IECoreHoudini.FromHoudiniGroupConverter.GroupingMode.AttributeValue )
		converter["groupingAttribute"].setTypedValue( attribName )
		result = converter.convert()
		
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 5 )
		self.assertEqual( set([ x.typeId() for x in result.children() ]), set([IECore._IECore.TypeId.CurvesPrimitive, IECore._IECore.TypeId.MeshPrimitive, IECore._IECore.TypeId.PointsPrimitive]) )
		
		for child in result.children() :
			
			self.assertEqual( child.blindData(), IECore.CompoundData() )
			self.failUnless( child.arePrimitiveVariablesValid() )
			self.failUnless( attribName in child )
			self.failUnless( attribName+"Indices" in child )
			self.assertEqual( child[attribName].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
			self.assertEqual( child[attribName+"Indices"].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
			# all primitives point to the same grouping attribute value
			self.assertEqual( child[attribName].data.size(), 1 )
			correctAttrib = [ x for x in attribs if child[attribName].data[0] == x.parm( "string1" ).eval() ]
			self.failUnless( len(correctAttrib) in [ 0, 1 ] )
			self.assertEqual( child[attribName+"Indices"].data, IECore.IntVectorData( [ 0 ] * child.variableSize( child[attribName+"Indices"].interpolation ) ) )
			if child[attribName].data[0] == "pointsGroup" :
				self.failUnless( child.isInstanceOf( IECore.TypeId.PointsPrimitive ) )
			elif child[attribName].data[0] == "torusAGroup" :
				self.failUnless( child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
			elif child[attribName].data[0] == "" :
				self.failUnless( child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
			elif child[attribName].data[0] == "curveBoxGroup" :
				self.failUnless( child.isInstanceOf( IECore.TypeId.CurvesPrimitive ) or child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
			else :
				raise RuntimeError, "An unexpected attribute value exists"
		
		null = merge.parent().createNode( "null" )
		self.failUnless( IECoreHoudini.ToHoudiniGeometryConverter.create( result ).convert( null ) )
		for prim in null.geometry().prims() :
			self.failUnless( prim.stringAttribValue( attribName ) in values )
	
	def testConvertGroupedPoints( self ) :
		result = IECoreHoudini.FromHoudiniGroupConverter( self.points() ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		self.assertEqual( result.children()[0].blindData()["name"].value, "pointsGroup" )
	
	def testConvertUngroupedPoints( self ) :
		group = self.points()
		group.bypass( True )
		result = IECoreHoudini.FromHoudiniGroupConverter( group ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		self.assertEqual( result.children()[0].blindData(), IECore.CompoundData() )
	
	def testConvertGroupedCurves( self ) :
		curve = self.curve()
		group = curve.createOutputNode( "group" )
		group.parm( "crname" ).set( "curvesGroup" )
		result = IECoreHoudini.FromHoudiniGroupConverter( group ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.CurvesPrimitive ) )
		self.assertEqual( result.children()[0].blindData()["name"].value, "curvesGroup" )
	
	def testConvertUngroupedCurves( self ) :
		result = IECoreHoudini.FromHoudiniGroupConverter( self.curve() ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.CurvesPrimitive ) )
		self.assertEqual( result.children()[0].blindData(), IECore.CompoundData() )
	
	def testConvertGroupedPolygons( self ) :
		result = IECoreHoudini.FromHoudiniGroupConverter( self.torusA() ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[0].blindData()["name"].value, "torusAGroup" )
	
	def testConvertUngroupedPolygons( self ) :
		group = self.torusA()
		group.bypass( True )
		result = IECoreHoudini.FromHoudiniGroupConverter( group ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[0].blindData(), IECore.CompoundData() )
	
	def testConvertGroupedMultiPolygons( self ) :
		result = IECoreHoudini.FromHoudiniGroupConverter( self.twoTorii() ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 2 )
		childNames = [ x.blindData()["name"].value for x in result.children() ]
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.failUnless( "torusAGroup" in childNames )
		self.assertEqual( result.children()[0]["P"].data.size(), 100 )
		self.failUnless( result.children()[1].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.failUnless( "torusBGroup" in childNames )
		self.assertEqual( result.children()[1]["P"].data.size(), 100 )
	
	def testConvertUngroupedMultiPolygons( self ) :
		merge = self.twoTorii()
		merge.inputs()[0].bypass( True )
		merge.inputs()[1].bypass( True )
		result = IECoreHoudini.FromHoudiniGroupConverter( merge ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[0].blindData(), IECore.CompoundData() )
		self.assertEqual( result.children()[0]["P"].data.size(), 200 )
	
	def testConvertPartiallyGroupedMultiPolygons( self ) :
		merge = self.twoTorii()
		merge.inputs()[0].bypass( True )
		result = IECoreHoudini.FromHoudiniGroupConverter( merge ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 2 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[0].blindData()["name"].value, "torusBGroup" )
		self.assertEqual( result.children()[0]["P"].data.size(), 100 )
		self.failUnless( result.children()[1].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[1].blindData(), IECore.CompoundData() )
		self.assertEqual( result.children()[1]["P"].data.size(), 100 )
	
	def testConvertGroupedCurvesAndPolygons( self ) :
		result = IECoreHoudini.FromHoudiniGroupConverter( self.curveBox() ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( result.children()[0].blindData()["name"].value, "curveBoxGroup" )
		self.assertEqual( len(result.children()[0].children()), 2 )
		self.assertEqual( result.children()[0].children()[0].blindData(), IECore.CompoundData() )
		self.failUnless( result.children()[0].children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[0].children()[1].blindData(), IECore.CompoundData() )
		self.failUnless( result.children()[0].children()[1].isInstanceOf( IECore.TypeId.CurvesPrimitive ) )
	
	def testConvertUngroupedCurvesAndPolygons( self ) :
		group = self.curveBox()
		group.bypass( True )
		result = IECoreHoudini.FromHoudiniGroupConverter( group ).convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 1 )
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( result.children()[0].blindData(), IECore.CompoundData() )
		self.assertEqual( len(result.children()[0].children()), 2 )
		self.assertEqual( result.children()[0].children()[0].blindData(), IECore.CompoundData() )
		self.failUnless( result.children()[0].children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[0].children()[1].blindData(), IECore.CompoundData() )
		self.failUnless( result.children()[0].children()[1].isInstanceOf( IECore.TypeId.CurvesPrimitive ) )
	
	def testFakeAttributeValue( self ) :
		converter = IECoreHoudini.FromHoudiniGroupConverter( self.buildScene() )
		converter["groupingMode"].setTypedValue( IECoreHoudini.FromHoudiniGroupConverter.GroupingMode.AttributeValue )
		converter["groupingAttribute"].setTypedValue( "notAnAttr" )
		result = converter.convert()
		self.assertEqual( result, None )
	
	def testNonPrimitiveAttributeValue( self ) :
		merge = self.buildScene()
		attrib = merge.createOutputNode( "attribcreate" )
		attrib.parm( "name1" ).set( "pointStr" )
		attrib.parm( "class1" ).set( 2 ) # Point
		attrib.parm( "type1" ).set( 3 ) # String
		converter = IECoreHoudini.FromHoudiniGroupConverter( attrib )
		converter["groupingMode"].setTypedValue( IECoreHoudini.FromHoudiniGroupConverter.GroupingMode.AttributeValue )
		converter["groupingAttribute"].setTypedValue( "pointStr" )
		result = converter.convert()
		self.assertEqual( result, None )
	
	def testNonStringAttributeValue( self ) :
		converter = IECoreHoudini.FromHoudiniGroupConverter( self.buildScene() )
		converter["groupingMode"].setTypedValue( IECoreHoudini.FromHoudiniGroupConverter.GroupingMode.AttributeValue )
		converter["groupingAttribute"].setTypedValue( "born" )
		result = converter.convert()
		self.assertEqual( result, None )
	
	def testObjectWasDeleted( self ) :
		torii = self.twoTorii()
		converter = IECoreHoudini.FromHoudiniGroupConverter( torii )
		g1 = converter.convert()
		torii.destroy()
		g2 = converter.convert()
		self.assertEqual( g2, g1 )
		self.assertRaises( RuntimeError, IECore.curry( IECoreHoudini.FromHoudiniGroupConverter, torii ) )
	
	def testObjectWasDeletedFactory( self ) :
		torii = self.twoTorii()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( torii )
		g1 = converter.convert()
		torii.destroy()
		g2 = converter.convert()
		self.assertEqual( g2, g1 )
		self.assertRaises( RuntimeError, IECore.curry( IECoreHoudini.FromHoudiniGeometryConverter.create, torii ) )
		
	def testAdjustedStringVectorIndices( self ) :
		box = self.box()
		geo = box.parent()
		group = box.createOutputNode( "group" )
		attr = group.createOutputNode( "attribcreate", exact_type_name=True )
		attr.parm( "class" ).set( 1 ) # prim
		attr.parm( "type" ).set( 3 ) # string
		attr.parm( "string" ).set( "box1" )
		box2 = geo.createNode( "box" )
		group2 = box2.createOutputNode( "group" )
		attr2 = group2.createOutputNode( "attribcreate", exact_type_name=True )
		attr2.parm( "class" ).set( 1 ) # prim
		attr2.parm( "type" ).set( 3 ) # string
		attr2.parm( "string" ).set( "box2" )
		merge = attr.createOutputNode( "merge" )
		merge.setInput( 1, attr2 )
		g = IECoreHoudini.FromHoudiniGroupConverter( merge ).convert()
		for c in g.children() :
			null = geo.createNode( "null" )
			IECoreHoudini.ToHoudiniPolygonsConverter( c ).convert( null )
			m = IECoreHoudini.FromHoudiniPolygonsConverter( null ).convert()
			self.assertEqual( m.vertexIds, c.vertexIds )
			self.assertEqual( m.verticesPerFace, c.verticesPerFace )
			self.assertEqual( m.keys(), c.keys() )
			for key in m.keys() :
				self.assertEqual( m[key].interpolation, c[key].interpolation )
				self.assertEqual( m[key].data, c[key].data )
				self.assertEqual( m[key], c[key] )

if __name__ == "__main__":
    unittest.main()
