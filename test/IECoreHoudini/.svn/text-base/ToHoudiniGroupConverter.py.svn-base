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
import os, shutil
import random

class TestToHoudiniGroupConverter( IECoreHoudini.TestCase ) :
	
	__testOTL = "test/IECoreHoudini/data/otls/testHDAs.otl"
	__testOTLBackups = os.path.join( os.path.dirname( __testOTL ), "backup" )
	__testOTLCopy = os.path.join( __testOTLBackups, os.path.basename( __testOTL ) )
	
	def points( self ) :
		pData = IECore.V3fVectorData( [
			IECore.V3f( 0, 1, 2 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ),
			IECore.V3f( 4 ), IECore.V3f( 5 ), IECore.V3f( 6 ), IECore.V3f( 7 ),
			IECore.V3f( 8 ), IECore.V3f( 9 ), IECore.V3f( 10 ), IECore.V3f( 11 ),
		] )
		
		points = IECore.PointsPrimitive( pData )
		
		floatData = IECore.FloatData( 1.5 )
		v2fData = IECore.V2fData( IECore.V2f( 1.5, 2.5 ) )
		v3fData = IECore.V3fData( IECore.V3f( 1.5, 2.5, 3.5 ) )
		color3fData = IECore.Color3fData( IECore.Color3f( 1.5, 2.5, 3.5 ) )
		intData = IECore.IntData( 1 )
		v2iData = IECore.V2iData( IECore.V2i( 1, 2 ) )
		v3iData = IECore.V3iData( IECore.V3i( 1, 2, 3 ) )
		stringData = IECore.StringData( "this is a string" )
		
		intRange = range( 1, 13 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		v2fVectorData = IECore.V2fVectorData( [ IECore.V2f( x, x+0.5 ) for x in intRange ] )
		v3fVectorData = IECore.V3fVectorData( [ IECore.V3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		intVectorData = IECore.IntVectorData( intRange )
		v2iVectorData = IECore.V2iVectorData( [ IECore.V2i( x, -x ) for x in intRange ] )
		v3iVectorData = IECore.V3iVectorData( [ IECore.V3i( x, -x, x*2 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %d!" % x for x in intRange ] )
		
		detailInterpolation = IECore.PrimitiveVariable.Interpolation.Constant
		pointInterpolation = IECore.PrimitiveVariable.Interpolation.Vertex
		
		# add all valid detail attrib types
		points["floatDetail"] = IECore.PrimitiveVariable( detailInterpolation, floatData )
		points["v2fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2fData )
		points["v3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3fData )
		points["color3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, color3fData )
		points["intDetail"] = IECore.PrimitiveVariable( detailInterpolation, intData )
		points["v2iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2iData )
		points["v3iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3iData )
		points["stringDetail"] = IECore.PrimitiveVariable( detailInterpolation, stringData )
		
		# add all valid point attrib types
		points["floatPoint"] = IECore.PrimitiveVariable( pointInterpolation, floatVectorData )
		points["v2fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2fVectorData )
		points["v3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3fVectorData )
		points["color3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, color3fVectorData )
		points["intPoint"] = IECore.PrimitiveVariable( pointInterpolation, intVectorData )
		points["v2iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2iVectorData )
		points["v3iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3iVectorData )
		points["stringPoint"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData )
		points["stringPointIndices"] = IECore.PrimitiveVariable( pointInterpolation, IECore.IntVectorData( range( 0, 12 ) ) )
		
		points.blindData()['name'] = "pointsGroup"
		
		return points

	def mesh( self ) :
		vertsPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [ 1, 5, 4, 0, 2, 6, 5, 1, 3, 7, 6, 2, 0, 4, 7, 3, 2, 1, 0, 3, 5, 6, 7, 4 ] )
		mesh = IECore.MeshPrimitive( vertsPerFace, vertexIds )
		
		floatData = IECore.FloatData( 1.5 )
		v2fData = IECore.V2fData( IECore.V2f( 1.5, 2.5 ) )
		v3fData = IECore.V3fData( IECore.V3f( 1.5, 2.5, 3.5 ) )
		color3fData = IECore.Color3fData( IECore.Color3f( 1.5, 2.5, 3.5 ) )
		intData = IECore.IntData( 1 )
		v2iData = IECore.V2iData( IECore.V2i( 1, 2 ) )
		v3iData = IECore.V3iData( IECore.V3i( 1, 2, 3 ) )
		stringData = IECore.StringData( "this is a string" )
		
		intRange = range( 1, 25 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		v2fVectorData = IECore.V2fVectorData( [ IECore.V2f( x, x+0.5 ) for x in intRange ] )
		v3fVectorData = IECore.V3fVectorData( [ IECore.V3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		intVectorData = IECore.IntVectorData( intRange )
		v2iVectorData = IECore.V2iVectorData( [ IECore.V2i( x, -x ) for x in intRange ] )
		v3iVectorData = IECore.V3iVectorData( [ IECore.V3i( x, -x, x*2 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %d!" % x for x in intRange ] )
		
		detailInterpolation = IECore.PrimitiveVariable.Interpolation.Constant
		pointInterpolation = IECore.PrimitiveVariable.Interpolation.Vertex
		primitiveInterpolation = IECore.PrimitiveVariable.Interpolation.Uniform
		vertexInterpolation = IECore.PrimitiveVariable.Interpolation.FaceVarying
		
		# add all valid detail attrib types
		mesh["floatDetail"] = IECore.PrimitiveVariable( detailInterpolation, floatData )
		mesh["v2fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2fData )
		mesh["v3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3fData )
		mesh["color3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, color3fData )
		mesh["intDetail"] = IECore.PrimitiveVariable( detailInterpolation, intData )
		mesh["v2iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2iData )
		mesh["v3iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3iData )
		mesh["stringDetail"] = IECore.PrimitiveVariable( detailInterpolation, stringData )
		
		# add all valid point attrib types
		pData = IECore.V3fVectorData( [
			IECore.V3f( 0, 1, 2 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ),
			IECore.V3f( 4 ), IECore.V3f( 5 ), IECore.V3f( 6 ), IECore.V3f( 7 ),
		] )
		mesh["P"] = IECore.PrimitiveVariable( pointInterpolation, pData )
		mesh["floatPoint"] = IECore.PrimitiveVariable( pointInterpolation, floatVectorData[:8] )
		mesh["v2fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2fVectorData[:8] )
		mesh["v3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3fVectorData[:8] )
		mesh["color3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, color3fVectorData[:8] )
		mesh["intPoint"] = IECore.PrimitiveVariable( pointInterpolation, intVectorData[:8] )
		mesh["v2iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2iVectorData[:8] )
		mesh["v3iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3iVectorData[:8] )
		mesh["stringPoint"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:8] )
		mesh["stringPointIndices"] = IECore.PrimitiveVariable( pointInterpolation, IECore.IntVectorData( range( 0, 8 ) ) )
		
		# add all valid primitive attrib types
		mesh["floatPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, floatVectorData[:6] )
		mesh["v2fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v2fVectorData[:6] )
		mesh["v3fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v3fVectorData[:6] )
		mesh["color3fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, color3fVectorData[:6] )
		mesh["intPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, intVectorData[:6] )
		mesh["v2iPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v2iVectorData[:6] )
		mesh["v3iPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v3iVectorData[:6] )
		mesh["stringPrim"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:6] )
		mesh["stringPrimIndices"] = IECore.PrimitiveVariable( primitiveInterpolation, IECore.IntVectorData( range( 0, 6 ) ) )
		
		# add all valid vertex attrib types
		mesh["floatVert"] = IECore.PrimitiveVariable( vertexInterpolation, floatVectorData )
		mesh["v2fVert"] = IECore.PrimitiveVariable( vertexInterpolation, v2fVectorData )
		mesh["v3fVert"] = IECore.PrimitiveVariable( vertexInterpolation, v3fVectorData )
		mesh["color3fVert"] = IECore.PrimitiveVariable( vertexInterpolation, color3fVectorData )
		mesh["intVert"] = IECore.PrimitiveVariable( vertexInterpolation, intVectorData )
		mesh["v2iVert"] = IECore.PrimitiveVariable( vertexInterpolation, v2iVectorData )
		mesh["v3iVert"] = IECore.PrimitiveVariable( vertexInterpolation, v3iVectorData )
		mesh["stringVert"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData )
		mesh["stringVertIndices"] = IECore.PrimitiveVariable( vertexInterpolation, IECore.IntVectorData( range( 0, 24 ) ) )
		
		mesh.blindData()['name'] = "meshGroupA"
		
		return mesh
	
	def meshGroup( self ) :
		group = IECore.Group()
		group.addChild( self.mesh() )
		return group
	
	def twoMeshes( self ) :
		group = self.meshGroup()
		mesh = self.mesh()
		mesh.blindData()['name'] = "meshGroupB"
		group.addChild( mesh )
		return group
	
	def pointTwoBox( self ) :
		group = IECore.Group()
		group.addChild( self.points() )
		mesh = self.mesh()
		del mesh.blindData()['name']
		group.addChild( mesh )
		mesh = self.mesh()
		del mesh.blindData()['name']
		group.addChild( mesh )
		group.blindData()['name'] = "curveBoxGroup"
		return group
	
	def buildScene( self ) :
		group = IECore.Group()
		group.addChild( self.points() )
		group.addChild( self.twoMeshes() )
		group.addChild( self.pointTwoBox() )
		return group

	def geo( self ) :
		geo = hou.node( "/obj/geo1" )
		if not geo :
			obj = hou.node( "/obj" )
			geo = obj.createNode( "geo", run_init_scripts=False )

		return geo

	def emptySop( self ) :
		return self.geo().createNode( "null" )

	def testCreateConverter( self )  :
		converter = IECoreHoudini.ToHoudiniGroupConverter( self.meshGroup() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniGroupConverter ) ) )

	def testFactory( self ) :
		scene = self.buildScene()
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( scene )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniGroupConverter ) ) )
		
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.points() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPointsConverter ) ) )

		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.mesh() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPolygonsConverter ) ) )
		
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.twoMeshes() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniGroupConverter ) ) )
		
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.pointTwoBox() )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniGroupConverter ) ) )
		
		self.failUnless( IECore.TypeId.Group in IECoreHoudini.ToHoudiniGeometryConverter.supportedTypes() )

	def testConvertScene( self ) :
		null = self.emptySop()
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( self.buildScene() ).convert( null ) )
		primGroups = null.geometry().primGroups()
		primGroupMap = {}
		for x in primGroups :
			primGroupMap[x.name()] = x
		
		self.assertEqual( len(primGroups), 3 )
		self.failUnless( "meshGroupA" in primGroupMap )
		self.assertEqual( len(primGroupMap["meshGroupA"].prims()), 6 )
		self.failUnless( "meshGroupB" in primGroupMap )
		self.assertEqual( len(primGroupMap["meshGroupB"].prims()), 6 )
		self.failUnless( "curveBoxGroup" in primGroupMap )
		self.assertEqual( len(primGroupMap["curveBoxGroup"].prims()), 12 )
		
		result = IECoreHoudini.FromHoudiniGroupConverter( null ).convert()
		children = result.children()
		for i in range ( 0, len(children) ) :
			name = children[i].blindData()['name'].value
			self.failUnless( name in primGroupMap )
			self.assertEqual( children[i].variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), len(primGroupMap[name].prims()) )
		
	def testAppending( self ) :
		null = self.emptySop()
		scene = self.buildScene()
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( scene ).convert( null ) )
		primGroups = null.geometry().primGroups()
		primGroupMap = {}
		for x in primGroups :
			primGroupMap[x.name()] = x
		
		self.assertEqual( len(primGroups), 3 )
		self.failUnless( "meshGroupA" in primGroupMap )
		self.assertEqual( len(primGroupMap["meshGroupA"].prims()), 6 )
		self.failUnless( "meshGroupB" in primGroupMap )
		self.assertEqual( len(primGroupMap["meshGroupB"].prims()), 6 )
		self.failUnless( "curveBoxGroup" in primGroupMap )
		self.assertEqual( len(primGroupMap["curveBoxGroup"].prims()), 12 )
		
		result = IECoreHoudini.FromHoudiniGroupConverter( null ).convert()
		children = result.children()
		for i in range ( 0, len(children) ) :
			name = children[i].blindData()['name'].value
			self.failUnless( name in primGroupMap )
			self.assertEqual( children[i].variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), len(primGroupMap[name].prims()) )
			
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( scene ).convert( null, append=True ) )
		primGroups = null.geometry().primGroups()
		
		self.assertEqual( len(primGroups), 3 )
		self.failUnless( "meshGroupA" in primGroupMap )
		self.assertEqual( len(primGroupMap["meshGroupA"].prims()), 12 )
		self.failUnless( "meshGroupB" in primGroupMap )
		self.assertEqual( len(primGroupMap["meshGroupB"].prims()), 12 )
		self.failUnless( "curveBoxGroup" in primGroupMap )
		self.assertEqual( len(primGroupMap["curveBoxGroup"].prims()), 24 )
		
		result = IECoreHoudini.FromHoudiniGroupConverter( null ).convert()
		children = result.children()
		for i in range ( 0, len(children) ) :
			name = children[i].blindData()['name'].value
			self.failUnless( name in primGroupMap )
			self.assertEqual( children[i].variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), len(primGroupMap[name].prims()) )
		
	def testConvertGroupedPoints( self ) :
		null = self.emptySop()
		group = IECore.Group()
		group.addChild( self.points() )
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( group ).convert( null ) )
		primGroups = null.geometry().primGroups()

		self.assertEqual( len(primGroups), 0 )
	
	def testConvertGroupedMesh( self ) :
		null = self.emptySop()
		group = IECore.Group()
		group.addChild( self.mesh() )
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( group ).convert( null ) )
		primGroups = null.geometry().primGroups()
		
		self.assertEqual( len(primGroups), 1 )
		self.assertEqual( primGroups[0].name(), "meshGroupA" )
		self.assertEqual( len(primGroups[0].prims()), 6 )
	
	def testConvertGroupedMultiMeshes( self ) :
		null = self.emptySop()
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( self.twoMeshes() ).convert( null ) )
		primGroups = null.geometry().primGroups()
		
		self.assertEqual( len(primGroups), 2 )
		self.assertEqual( primGroups[0].name(), "meshGroupA" )
		self.assertEqual( len(primGroups[0].prims()), 6 )
		self.assertEqual( primGroups[1].name(), "meshGroupB" )
		self.assertEqual( len(primGroups[1].prims()), 6 )
	
	def testConvertGroupedPointsAndPolygons( self ) :
		null = self.emptySop()
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( self.pointTwoBox() ).convert( null ) )
		primGroups = null.geometry().primGroups()
		primGroupMap = {}
		for x in primGroups :
			primGroupMap[x.name()] = x
		self.assertEqual( len(primGroups), 1 )
		self.failUnless( "curveBoxGroup"in primGroupMap )
		self.assertEqual( len(primGroupMap["curveBoxGroup"].prims()), 12 )
		
	def testAdjustedStringVectorIndices( self ) :
		null = self.emptySop()
		group = self.twoMeshes()
		group.children()[0]["commonString"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringVectorData( [ "first" ] ) )
		indices = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [ 0 ] * 6 ) )
		group.children()[0]["commonStringIndices"] = indices
		group.children()[1]["commonString"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringVectorData( [ "second" ] ) )
		group.children()[1]["commonStringIndices"] = indices
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( group ).convert( null ) )
		geometry = null.geometry()
		primGroups = geometry.primGroups()
		
		expected = [ "first", "second" ]
		self.assertEqual( len(primGroups), 2 )
		for i in range( 0, len(primGroups) ) :
			self.assertEqual( len(primGroups[i].prims()), 6 )
			for prim in primGroups[i].prims() :
				self.assertEqual( prim.stringListAttribValue( "commonString" ), tuple( [ expected[i] ] ) )
				self.assertEqual( prim.attribValue( "commonString" ), expected[i] )
	
	def testTransforms( self ) :
		
		def add( parent, child, vec ) :
			child.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( vec ) ) )
			parent.addChild( child )
			if random.random() > 0.75 :
				child.addChild( self.mesh() )
		
		group = IECore.Group()
		group.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 5, 0, 0 ) ) ) )
		for i in range( 0, 50 ) :
			add( group, self.meshGroup(), IECore.V3f( random.random(), random.random(), random.random() ) * 3 )
		
		null = self.emptySop()
		self.failUnless( IECoreHoudini.ToHoudiniGroupConverter( group ).convert( null ) )
		
		houdiniBound = null.geometry().boundingBox()
		bound = IECore.Box3f( IECore.V3f( list(houdiniBound.minvec()) ), IECore.V3f( list(houdiniBound.maxvec()) ) )
		self.assertEqual( group.bound(), bound )

	def testCannotConvertIntoReadOnlyHOMGeo( self ) :
		
		group = self.buildScene()
		sop = self.emptySop()
		
		converter = IECoreHoudini.ToHoudiniGroupConverter( group )
		self.failUnless( not converter.convertToGeo( sop.geometry() ) )
		
		self.assertEqual( sop.geometry().points(), tuple() )
		self.assertEqual( sop.geometry().prims(), tuple() )
	
	def testConvertIntoWritableHOMGeo( self ) :
		
		if not os.path.isdir( TestToHoudiniGroupConverter.__testOTLBackups ) :
			os.mkdir( TestToHoudiniGroupConverter.__testOTLBackups )
		
		shutil.copyfile( TestToHoudiniGroupConverter.__testOTL, TestToHoudiniGroupConverter.__testOTLCopy )
		hou.hda.installFile( TestToHoudiniGroupConverter.__testOTLCopy )
		
		sop = hou.node( "/obj" ).createNode( "geo", run_init_scripts=False ).createNode( "testPythonSop" )
		geo = sop.geometry()
		self.assertEqual( sop.geometry().points(), tuple() )
		self.assertEqual( sop.geometry().prims(), tuple() )
		
		sop.type().definition().sections()["PythonCook"].setContents( """
import IECore
import IECoreHoudini
group = IECore.Group()
mesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) ) )
mesh.blindData()["name"] = IECore.StringData( "mesh" )
points = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( x ) for x in range( 0, 20 ) ] ) )
points.blindData()["name"] = IECore.StringData( "points" )
curves = IECore.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.linear(), False, IECore.V3fVectorData( [ IECore.V3f( x ) for x in range( 0, 4 ) ] ) )
curves.blindData()["name"] = IECore.StringData( "curves" )
group.addChild( mesh )
group.addChild( points )
group.addChild( curves )
IECoreHoudini.ToHoudiniGroupConverter( group ).convertToGeo( hou.pwd().geometry() )"""
		)
		
		self.assertEqual( len(sop.geometry().points()), 32 )
		self.assertEqual( len(sop.geometry().prims()), 7 )
		
		sop.createInputNode( 0, "torus" )
		
		self.assertEqual( len(sop.geometry().points()), 32 )
		self.assertEqual( len(sop.geometry().prims()), 7 )
		
		code = sop.type().definition().sections()["PythonCook"].contents()
		sop.type().definition().sections()["PythonCook"].setContents( code.replace( "convertToGeo( hou.pwd().geometry() )", "convertToGeo( hou.pwd().geometry(), append=True )" ) )
		
		self.assertEqual( len(sop.geometry().points()), 132 )
		self.assertEqual( len(sop.geometry().prims()), 107 )
		
		sop.destroy()
	
	def tearDown( self ) :
		
		if TestToHoudiniGroupConverter.__testOTLCopy in "".join( hou.hda.loadedFiles() ) :
			hou.hda.uninstallFile( TestToHoudiniGroupConverter.__testOTLCopy )
		
		if os.path.isdir( TestToHoudiniGroupConverter.__testOTLBackups ) :
			shutil.rmtree( TestToHoudiniGroupConverter.__testOTLBackups )

if __name__ == "__main__":
    unittest.main()
