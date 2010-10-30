##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

		
		points = self.points()
		torus = self.torusA()
		merge = self.geo().createNode( "merge" )
		merge.setInput( 0, self.points() )
		merge.setInput( 1, self.twoTorii() )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniGroupConverter ) ) )

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
		self.failUnless( result.children()[0].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[0].blindData()["name"].value, "torusAGroup" )
		self.assertEqual( result.children()[0]["P"].data.size(), 100 )
		self.failUnless( result.children()[1].isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( result.children()[1].blindData()["name"].value, "torusBGroup" )
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

if __name__ == "__main__":
    unittest.main()
