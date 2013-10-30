##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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
import hou
import IECore
import IECoreHoudini
import unittest

class TestSceneCache( IECoreHoudini.TestCase ) :
	
	__testFile = "test/test.scc"
	__testOutFile = "test/testOut.scc"
	__testLinkedOutFile = "test/testOut.lscc"
	__testHip = "test/test.hip"
	__testBgeo = "test/test.bgeo"
	__testBgeoGz = "test/test.bgeo.gz"
	__testGeo = "test/test.geo"
	
	def sop( self, parent=None ) :
		if not parent :
			parent = hou.node( "/obj" ).createNode( "geo", run_init_scripts=False )
		sop = parent.createNode( "ieSceneCacheSource" )
		sop.parm( "file" ).set( TestSceneCache.__testFile )
		return sop
	
	def xform( self, parent=None ) :
		if not parent :
			parent = hou.node( "/obj" )
		xform = parent.createNode( "ieSceneCacheTransform" )
		xform.parm( "file" ).set( TestSceneCache.__testFile )
		return xform
	
	def geometry( self, parent=None ) :
		if not parent :
			parent = hou.node( "/obj" )
		geometry = parent.createNode( "ieSceneCacheGeometry" )
		geometry.parm( "file" ).set( TestSceneCache.__testFile )
		return geometry
	
	def rop( self, rootObject ) :
		
		rop = hou.node( "/out" ).createNode( "ieSceneCacheWriter" )
		rop.parm( "file" ).set( TestSceneCache.__testOutFile )
		rop.parm( "rootObject" ).set( rootObject.path() )
		return rop
	
	def writeSCC( self, rotation=IECore.V3d( 0, 0, 0 ), time=0 ) :
		
		scene = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Write )
		
		sc = scene.createChild( str( 1 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 6 ) )
		sc.writeObject( mesh, time )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		sc.writeTransform( IECore.M44dData( matrix ), time )
		
		sc = sc.createChild( str( 2 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 1, 0 ) ] * 6 ) )
		sc.writeObject( mesh, time )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		sc.writeTransform( IECore.M44dData( matrix ), time )
		
		sc = sc.createChild( str( 3 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 1 ) ] * 6 ) )
		sc.writeObject( mesh, time )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 3, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		sc.writeTransform( IECore.M44dData( matrix ), time )
		
		return scene
	
	def testBadFile( self ) :
		
		def testNode( node ) :
			
			node.parm( "file" ).set( "" )
			self.assertRaises( hou.OperationFailed, IECore.curry( node.cook, True ) )
			self.failUnless( node.errors() )		
			node.parm( "file" ).set( "/tmp/fake" )
			self.assertRaises( hou.OperationFailed, IECore.curry( node.cook, True ) )
			self.failUnless( node.errors() )
			node.parm( "file" ).set( TestSceneCache.__testFile )
			self.assertRaises( hou.OperationFailed, IECore.curry( node.cook, True ) )
			self.failUnless( node.errors() )
			self.writeSCC()
			node.cook( force=True )
			self.failUnless( not node.errors() )
		
		testNode( self.sop() )
		os.remove( TestSceneCache.__testFile )
		testNode( self.xform() )
		os.remove( TestSceneCache.__testFile )
		testNode( self.geometry() )
	
	def testBadPath( self ) :
		
		def testNode( node ) :
			
			node.cook( force=True )
			self.failUnless( not node.errors() )
			node.parm( "root" ).set( "/1/fake" )
			self.assertRaises( hou.OperationFailed, IECore.curry( node.cook, True ) )
			self.failUnless( node.errors() )
			node.parm( "root" ).set( "/1/2" )
			node.cook( force=True )
			self.failUnless( not node.errors() )
			
			if isinstance( node, hou.ObjNode ) :
				node.parm( "expand" ).pressButton()
				self.failUnless( node.children() )
				node.parm( "root" ).set( "/1/fake" )
				node.parm( "collapse" ).pressButton()
				node.parm( "expand" ).pressButton()
				self.assertEqual( node.children(), tuple() )
		
		self.writeSCC()
		testNode( self.sop() )
		testNode( self.xform() )
		testNode( self.geometry() )
	
	def testObjSubPaths( self ) :
		
		self.writeSCC()
		
		def testSimple( node ) :
			
			node.parm( "reload" ).pressButton()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			node.parm( "root" ).set( "/1" )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 1, 0, 0 ) )
			node.parm( "root" ).set( "/1/2" )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 3, 0, 0 ) )
			node.parm( "root" ).set( "/1/2/3" )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 6, 0, 0 ) )

		xform = self.xform()
		geo = self.geometry()
		testSimple( xform )
		testSimple( geo )
		
		self.writeSCC( rotation = IECore.V3d( 0, 0, IECore.degreesToRadians( -30 ) ) )
		
		def testRotated( node ) :
			
			node.parm( "reload" ).pressButton()
			node.parm( "root" ).set( "/" )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			self.assertEqual( node.parmTransform().extractRotates(), hou.Vector3( 0, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, 0 ) ) )
			node.parm( "root" ).set( "/1" )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 1, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -30 ) ) )
			node.parm( "root" ).set( "/1/2" )
			self.failUnless( node.parmTransform().extractTranslates().isAlmostEqual( hou.Vector3( 2.73205, -1, 0 ) ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -60 ) ) )
			node.parm( "root" ).set( "/1/2/3" )
			self.failUnless( node.parmTransform().extractTranslates().isAlmostEqual( hou.Vector3( 4.23205, -3.59808, 0 ) ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -90 ) ) )
		
		testRotated( xform )
		testRotated( geo )
	
	def testObjSpaceModes( self ) :
		
		self.writeSCC()
		
		def testSimple( node ) :
			
			node.parm( "reload" ).pressButton()
			node.parm( "root" ).set( "/1/2" )
			self.assertEqual( node.parm( "space" ).eval(), IECoreHoudini.SceneCacheNode.Space.World )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 3, 0, 0 ) )
			node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Path )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Local )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 2, 0, 0 ) )
			node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Object )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
		
		xform = self.xform()
		geo = self.geometry()
		testSimple( xform )
		testSimple( geo )
		
		self.writeSCC( rotation = IECore.V3d( 0, 0, IECore.degreesToRadians( -30 ) ) )
		
		def testRotated( node ) :
			
			node.parm( "reload" ).pressButton()
			node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.World )
			self.failUnless( node.parmTransform().extractTranslates().isAlmostEqual( hou.Vector3( 2.73205, -1, 0 ) ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -60 ) ) )
			node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Path )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, 0 ) ) )
			node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Local )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 2, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -30 ) ) )
			node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Object )
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, 0 ) ) )
		
		testRotated( xform )
		testRotated( geo )
	
	def testSopSubPaths( self ) :
		
		self.writeSCC()
		node = self.sop()
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "root" ).set( "/1" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "root" ).set( "/1/2" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 12 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "root" ).set( "/1/2/3" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
	
	def testSopSpaceModes( self ) :
		
		self.writeSCC()
		node = self.sop()
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		self.assertEqual( node.parm( "space" ).eval(), IECoreHoudini.SceneCacheNode.Space.World )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 6.5, 0.5, 0.5 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Path )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 6.5, 0.5, 0.5 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Local )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 2.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Object )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
		
		node.parm( "root" ).set( "/1" )
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.World )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 6.5, 0.5, 0.5 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Path )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 5, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 2.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 5.5, 0.5, 0.5 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Local )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 2.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Object )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/2', '/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
	
	def testSopShapeFilter( self ) :
		
		self.writeSCC()
		node = self.sop()
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 3 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 6.5, 0.5, 0.5 ) )
		
		node.parm( "shapeFilter" ).set( "* ^2" )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 12 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1','/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 2 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 6.5, 0.5, 0.5 ) )
		
		node.parm( "shapeFilter" ).set( "3" )
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 1 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 6.5, 0.5, 0.5 ) )
		
		node.parm( "shapeFilter" ).set( "" )
		self.assertEqual( len(node.geometry().prims()), 0 )
		self.assertEqual( len(node.geometry().primGroups()), 0 )
	
	def testSopAttributeFilter( self ) :
		
		self.writeSCC()
		node = self.sop()
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["Cd", "ieMeshInterpolation", "name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "P" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["ieMeshInterpolation", "name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "* ^Cs" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["ieMeshInterpolation", "name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "Cs" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["Cd", "ieMeshInterpolation", "name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["ieMeshInterpolation", "name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
	
	def testExpandGeo( self ) :
		
		self.writeSCC()
		geo = self.geometry()
		self.assertEqual( geo.children(), tuple() )
		geo.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		geo.parm( "expand" ).pressButton()
		self.assertEqual( len(geo.children()), 1 )
		node = geo.children()[0]
		self.assertEqual( node.name(), "root" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 6, 0, 0 ) )
		self.assertEqual( geo.parmTuple( "outT" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( geo.parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( geo.parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		
		geo.parm( "root" ).set( "/1/2" )
		geo.parm( "collapse" ).pressButton()
		geo.parm( "expand" ).pressButton()
		self.assertEqual( len(geo.children()), 1 )
		node = geo.children()[0]
		self.assertEqual( node.name(), "2" )
		node.parm( "root" ).set( "/1/2" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 12 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 6, 0, 0 ) )
		self.assertEqual( geo.parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( geo.parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( geo.parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
	
	def cookAll( self, node ) :
		node.cook( force=True )
		for child in node.children() :
			self.cookAll( child )
	
	def testExpandSubNetwork( self ) :
		
		self.writeSCC()
		xform = self.xform()
		self.assertEqual( xform.children(), tuple() )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 1 )
		self.failUnless( isinstance( hou.node( xform.path()+"/1" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/1/geo" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/1/2" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/1/2/geo" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/1/2/3" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/1/2/3/geo" ), hou.ObjNode ) )
		geo = hou.node( xform.path()+"/1/2/geo" )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outT" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/1" ).parmTuple( "outT" ).eval(), ( 1, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/1/geo" ).parmTuple( "outT" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/geo" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/geo" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2" ).parmTuple( "outT" ).eval(), ( 2, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/geo" ).parmTuple( "outT" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/geo" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/geo" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/3" ).parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/3" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/3" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/3/geo" ).parmTuple( "outT" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/3/geo" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1/2/3/geo" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		
		xform.parm( "root" ).set( "/1/2" )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 2 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/3" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/3/geo" ), hou.ObjNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/3/geo" ).parmTuple( "outT" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/3/geo" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/3/geo" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		
		xform.parm( "root" ).set( "/1" )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 2 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( hou.node( xform.path()+"/geo" ), hou.ObjNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 1, 0, 0 ) )
		
		next = hou.node( xform.path()+"/2" )
		self.failUnless( isinstance( next, hou.ObjNode ) )
		self.assertEqual( len(next.children()), 0 )
		next.parm( "expand" ).pressButton()
		self.assertEqual( len(next.children()), 2 )
		self.failUnless( isinstance( hou.node( xform.path()+"/2/3" ), hou.ObjNode ) )
		geo = hou.node( xform.path()+"/2/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		
		next = hou.node( xform.path()+"/2/3" )
		self.failUnless( isinstance( next, hou.ObjNode ) )
		self.assertEqual( len(next.children()), 0 )
		next.parm( "expand" ).pressButton()
		self.assertEqual( len(next.children()), 1 )
		self.failUnless( isinstance( hou.node( xform.path()+"/2/3/geo" ), hou.ObjNode ) )
	
	def testExpandParenting( self ) :
		
		self.writeSCC()
		xform = self.xform()
		self.assertEqual( xform.children(), tuple() )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 3 )
		self.failUnless( isinstance( hou.node( xform.path()+"/1" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/1" ).children()[0], hou.SopNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/2" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/2" ).children()[0], hou.SopNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/3" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/3" ).children()[0], hou.SopNode ) )
		self.assertEqual( len(hou.node( xform.path()+"/1" ).inputs()), 0 )
		self.assertEqual( len(hou.node( xform.path()+"/2" ).inputs()), 1 )
		self.assertEqual( len(hou.node( xform.path()+"/3" ).inputs()), 1 )
		self.assertEqual( len(hou.node( xform.path()+"/1" ).outputs()), 1 )
		self.assertEqual( len(hou.node( xform.path()+"/2" ).outputs()), 1 )
		self.assertEqual( len(hou.node( xform.path()+"/3" ).outputs()), 0 )
		self.assertEqual( hou.node( xform.path()+"/1" ).outputs()[0], hou.node( xform.path()+"/2" ) )
		self.assertEqual( hou.node( xform.path()+"/2" ).outputs()[0], hou.node( xform.path()+"/3" ) )
		self.cookAll( xform )
		geo = hou.node( xform.path()+"/2" )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outT" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/1" ).parmTuple( "outT" ).eval(), ( 1, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/1" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/2" ).parmTuple( "outT" ).eval(), ( 2, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/2" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/2" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		
		xform.parm( "root" ).set( "/1/2" )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 2 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.failUnless( isinstance( geo.children()[0], hou.SopNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/3" ), hou.ObjNode ) )
		self.failUnless( isinstance( hou.node( xform.path()+"/3" ).children()[0], hou.SopNode ) )
		self.assertEqual( len(geo.inputs()), 0 )
		self.assertEqual( len(hou.node( xform.path()+"/3" ).inputs()), 1 )
		self.assertEqual( len(geo.outputs()), 1 )
		self.assertEqual( len(hou.node( xform.path()+"/3" ).outputs()), 0 )
		self.assertEqual( geo.outputs()[0], hou.node( xform.path()+"/3" ) )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( xform.parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( hou.node( xform.path()+"/3" ).parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		
		xform.parm( "root" ).set( "/1" )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 2 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.failUnless( isinstance( geo.children()[0], hou.SopNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 1, 0, 0 ) )
		
		next = hou.node( xform.path()+"/2" )
		self.failUnless( isinstance( next, hou.ObjNode ) )
		self.assertEqual( len(geo.inputs()), 0 )
		self.assertEqual( len(next.inputs()), 1 )
		self.assertEqual( len(geo.outputs()), 1 )
		self.assertEqual( len(next.outputs()), 0 )
		self.assertEqual( geo.outputs()[0], next )
		self.assertEqual( len(next.children()), 1 )
		self.failUnless( isinstance( next.children()[0], hou.SopNode ) )
	
	def testExpandFlatGeometry( self ) :
		
		self.writeSCC()
		xform = self.xform()
		self.assertEqual( xform.children(), tuple() )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.FlatGeometry )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 1 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.failUnless( isinstance( geo.children()[0], hou.SopNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		self.assertEqual( node.name(), "root" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 6, 0, 0 ) )
		
		xform.parm( "root" ).set( "/1/2" )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 1 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.failUnless( isinstance( geo.children()[0], hou.SopNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		self.assertEqual( node.name(), "2" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 12 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/', '/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 6, 0, 0 ) )
		
		xform.parm( "root" ).set( "/1" )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 1 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.failUnless( isinstance( geo.children()[0], hou.SopNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		self.assertEqual( node.name(), "1" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 1, 0, 0 ) )
	
	def writeTaggedSCC( self ) :
		
		scene = self.writeSCC()
		sc1 = scene.child( str( 1 ) )
		sc2 = sc1.child( str( 2 ) )
		sc3 = sc2.child( str( 3 ) )
		sc1.writeTags( [ "a" ] )
		sc2.writeTags( [ "b" ] )
		sc3.writeTags( [ "c" ] )
		
		return scene

	def writeDualTaggedSCC( self ) :

		scene = self.writeTaggedSCC()
		sc1 = scene.child( str(1 ) )
		sc4 = sc1.createChild( str(4) )
		sc4.writeTags( [ "d" ] )
		box = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		sc5 = sc4.createChild( str(5) )
		sc5.writeObject( box, 0 )
	
	def testParmTrickleDown( self ) :
		
		def checkParms( node, geoType, tagFilter, attribFilter, shapeFilter ) :
			
			self.assertEqual( node.parm( "geometryType" ).eval(), geoType )
			self.assertEqual( node.parm( "attributeFilter" ).eval(), attribFilter )
			self.assertEqual( node.parm( "shapeFilter" ).eval(), shapeFilter )
			if isinstance( node, hou.ObjNode ) :
				self.assertEqual( node.parm( "expanded" ).eval(), True )
				
				if node.isObjectDisplayed() :
					self.assertEqual( node.parm( "tagFilter" ).eval(), tagFilter )
				else :
					self.assertEqual( node.parm( "tagFilter" ).eval(), "*" )
			
			for child in node.children() :
				checkParms( child, geoType, tagFilter, attribFilter, shapeFilter )
		
		self.writeDualTaggedSCC()
		xform = self.xform()
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		xform.parm( "attributeFilter" ).set( "*" )
		xform.parm( "shapeFilter" ).set( "*" )
		xform.parm( "expand" ).pressButton()
		checkParms( xform, IECoreHoudini.SceneCacheNode.GeometryType.Cortex, "*", "*", "*" )
		
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		xform.parm( "attributeFilter" ).set( "P ^N" )
		xform.parm( "shapeFilter" ).set( "2 ^3" )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		checkParms( xform, IECoreHoudini.SceneCacheNode.GeometryType.Houdini, "*", "P ^N", "2 ^3" )
		
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		xform.parm( "attributeFilter" ).set( "*" )
		xform.parm( "shapeFilter" ).set( "*" )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		checkParms( xform, IECoreHoudini.SceneCacheNode.GeometryType.Cortex, "*", "*", "*" )
		
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		xform.parm( "attributeFilter" ).set( "P ^N" )
		xform.parm( "shapeFilter" ).set( "2 ^3" )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		checkParms( xform, IECoreHoudini.SceneCacheNode.GeometryType.Houdini, "*", "P ^N", "2 ^3" )
		
		# now check just pushing the parms
		
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		xform.parm( "attributeFilter" ).set( "P ^N" )
		xform.parm( "shapeFilter" ).set( "2 ^3" )
		xform.parm( "push" ).pressButton()
		checkParms( xform, IECoreHoudini.SceneCacheNode.GeometryType.Cortex, "*", "P ^N", "2 ^3" )
		self.assertTrue( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4/5" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4/5/geo" ).isObjectDisplayed() )
		
		xform.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		xform.parm( "attributeFilter" ).set( "P N" )
		xform.parm( "shapeFilter" ).set( "2 3" )
		xform.parm( "tagFilter" ).set( "d" )
		xform.parm( "push" ).pressButton()
		checkParms( xform, IECoreHoudini.SceneCacheNode.GeometryType.Houdini, "d", "P N", "2 3" )
		self.assertTrue( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/geo" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/1/2" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/1/2/geo" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4/5" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4/5/geo" ).isObjectDisplayed() )
	
	def testTagFilter( self ) :
		
		self.writeDualTaggedSCC()
		
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		self.assertEqual( sorted( xform.parm( "tagFilter" ).menuItems() ), [ "*", "ObjectType:MeshPrimitive", "a", "b", "c", "d" ] )
		xform.parm( "tagFilter" ).set( "b" )
		xform.parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3/geo" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/1/4" ).isObjectDisplayed() )
		self.assertEqual( hou.node( xform.path()+"/1/4/5" ), None )
		
		xform.parm( "collapse" ).pressButton()
		xform.parm( "tagFilter" ).set( "d" )
		xform.parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/geo" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/1/2" ).isObjectDisplayed() )
		self.assertEqual( hou.node( xform.path()+"/1/2/geo" ), None )
		self.assertEqual( hou.node( xform.path()+"/1/2/3" ), None )
		self.assertTrue( hou.node( xform.path()+"/1/4" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4/5" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4/5/geo" ).isObjectDisplayed() )
		
		xform.parm( "collapse" ).pressButton()
		xform.parm( "tagFilter" ).set( "b d" )
		xform.parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4/5" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/4/5/geo" ).isObjectDisplayed() )
		
		xform.parm( "collapse" ).pressButton()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "tagFilter" ).set( "b" )
		xform.parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertEqual( hou.node( xform.path()+"/1/geo" ), None )
		hou.node( xform.path()+"/1" ).parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2" ).isObjectDisplayed() )
		self.assertEqual( hou.node( xform.path()+"/1/2/geo" ), None )
		hou.node( xform.path()+"/1/2" ).parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1/2/geo" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3" ).isObjectDisplayed() )
		self.assertEqual( hou.node( xform.path()+"/1/2//3geo" ), None )
		hou.node( xform.path()+"/1/2/3" ).parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1/2/3/geo" ).isObjectDisplayed() )
		self.assertEqual( hou.node( xform.path()+"/1/4/5" ), None )
		self.assertEqual( hou.node( xform.path()+"/1/4/5/geo" ), None )
		# is displayed because we forced it by expanding 4 explicitly
		hou.node( xform.path()+"/1/4" ).parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1/4/5" ).isObjectDisplayed() )
		self.assertEqual( hou.node( xform.path()+"/1/4/5/geo" ), None )
		hou.node( xform.path()+"/1/4/5" ).parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1/4/5/geo" ).isObjectDisplayed() )
		
		xform.parm( "collapse" ).pressButton()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "tagFilter" ).set( "b" )
		xform.parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/2" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/3" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/4" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/5" ).isObjectDisplayed() )
	
	def testSopTagFilter( self ) :
		
		self.writeDualTaggedSCC()
		
		sop = self.sop()
		sop.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		sop.parm( "tagFilter" ).set( "b" )
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		sop.parm( "tagFilter" ).set( "a" )
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 24 )
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3', '/1/4/5' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		self.assertEqual( prims[18].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		
		sop.parm( "tagFilter" ).set( "d" )
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 12 )
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/4/5' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )

		sop.parm( "tagFilter" ).set( "b d" )
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 24 )
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3', '/1/4/5' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		self.assertEqual( prims[18].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
	
	def testEmptyTags( self ) :
		
		scene = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Write )
		
		sc = scene.createChild( str( 1 ) )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0 )
		
		sc = sc.createChild( str( 2 ) )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0 )
		
		sc = sc.createChild( str( 3 ) )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 3, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0 )
		
		del scene, sc
		
		xform = self.xform()
		xform.parm( "expand" ).pressButton()
		self.assertEqual( len(xform.children()), 1 )
		self.assertEqual( len(hou.node( xform.path()+"/1" ).children()), 1 )
		self.assertEqual( len(hou.node( xform.path()+"/1/2" ).children()), 1 )
		self.assertEqual( len(hou.node( xform.path()+"/1/2/3" ).children()), 0 )
		
		scene = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( scene.readTags( IECore.SceneInterface.EveryTag ), [] )
		
		self.assertTrue( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2" ).isObjectDisplayed() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3" ).isObjectDisplayed() )
		
		xform.parm( "tagFilter" ).set( "" )
		xform.parm( "push" ).pressButton()
		self.assertFalse( hou.node( xform.path()+"/1" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/1/2" ).isObjectDisplayed() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3" ).isObjectDisplayed() )
	
	def testLiveTags( self ) :
		
		self.writeTaggedSCC()
		
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "expand" ).pressButton()
		scene = IECoreHoudini.HoudiniScene( xform.path() )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.EveryTag ) ]), [ "ObjectType:MeshPrimitive", "a", "b", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.LocalTag ) ]), [] )
		for tag in scene.readTags(IECore.SceneInterface.EveryTag) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.EveryTag ) )
		scene = IECoreHoudini.HoudiniScene( xform.path()+"/1/2" )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.AncestorTag ) ]), [ "ObjectType:MeshPrimitive", "a" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.DescendantTag ) ]), [ "ObjectType:MeshPrimitive", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.LocalTag ) ]), [ "ObjectType:MeshPrimitive", "b" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.EveryTag ) ]), [ "ObjectType:MeshPrimitive", "a", "b", "c" ] )
		for tag in scene.readTags( IECore.SceneInterface.EveryTag ) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.EveryTag ) )
		
		# test tags exist even when children aren't expanded 
		xform.parm( "collapse" ).pressButton()
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "expand" ).pressButton()
		scene = IECoreHoudini.HoudiniScene( xform.path() )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.EveryTag ) ]), [ "ObjectType:MeshPrimitive", "a", "b", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.LocalTag ) ]), [] )
		for tag in scene.readTags( IECore.SceneInterface.EveryTag ) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.EveryTag ) )
		hou.node( xform.path()+"/1" ).parm( "expand" ).pressButton()
		scene = IECoreHoudini.HoudiniScene( xform.path()+"/1/2" )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.AncestorTag ) ]), [ "ObjectType:MeshPrimitive", "a" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.DescendantTag ) ]), [ "ObjectType:MeshPrimitive", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.LocalTag ) ]), [ "ObjectType:MeshPrimitive", "b" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.EveryTag ) ]), [ "ObjectType:MeshPrimitive", "a", "b", "c" ] )
		for tag in scene.readTags( IECore.SceneInterface.EveryTag ) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.EveryTag ) )
		
		# test tags for parented expansion
		xform.parm( "collapse" ).pressButton()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "expand" ).pressButton()
		scene = IECoreHoudini.HoudiniScene( xform.path() )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.EveryTag ) ]), [ "ObjectType:MeshPrimitive", "a", "b", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.LocalTag ) ]), [] )
		for tag in scene.readTags( IECore.SceneInterface.EveryTag ) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.EveryTag ) )
		scene = IECoreHoudini.HoudiniScene( xform.path()+"/2" )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.AncestorTag ) ]), [ "ObjectType:MeshPrimitive", "a" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.DescendantTag ) ]), [ "ObjectType:MeshPrimitive", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.LocalTag ) ]), [ "ObjectType:MeshPrimitive", "b" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.EveryTag ) ]), [ "ObjectType:MeshPrimitive", "a", "b", "c" ] )
		for tag in scene.readTags( IECore.SceneInterface.EveryTag ) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.EveryTag ) )
		
		# test user tags
		b = hou.node( xform.path()+"/2" )
		b.parm( "ieTags" ).set( "testing user:tags" )
		group = b.renderNode().createOutputNode( "group" )
		group.parm( "crname" ).set( "ieTag_green" )
		group2 = group.createOutputNode( "group" )
		group2.parm( "crname" ).set( "notATag" )
		group2.setRenderFlag( True )
		scene = IECoreHoudini.HoudiniScene( xform.path() )
		# they don't currently affect parents, just the immediate node
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.EveryTag ) ]), [ "ObjectType:MeshPrimitive", "a", "b", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.LocalTag ) ]), [] )
		for tag in scene.readTags( IECore.SceneInterface.EveryTag ) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "testing", IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "user:tags", IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "green", IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "notATag", IECore.SceneInterface.EveryTag ) )
		scene = IECoreHoudini.HoudiniScene( xform.path()+"/2" )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.AncestorTag ) ]), [ "ObjectType:MeshPrimitive", "a" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.DescendantTag ) ]), [ "ObjectType:MeshPrimitive", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.LocalTag ) ]), [ "ObjectType:MeshPrimitive", "b", "green", "testing", "user:tags" ] )
		for tag in scene.readTags(IECore.SceneInterface.EveryTag) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.EveryTag ) )
		self.assertFalse( scene.hasTag( "notATag", IECore.SceneInterface.EveryTag ) )
	
	def writeAttributeSCC( self ) :
		
		scene = self.writeSCC()
		sc1 = scene.child( str( 1 ) )
		sc2 = sc1.child( str( 2 ) )
		sc3 = sc2.child( str( 3 ) )
		sc1.writeAttribute( "label", IECore.StringData( "a" ), 0 )
		sc1.writeAttribute( "color", IECore.Color3dData( IECore.Color3d( 0.5 ) ), 0 )
		sc2.writeAttribute( "label", IECore.StringData( "b" ), 0 )
		sc2.writeAttribute( "material", IECore.StringData( "rubber" ), 0 )
		sc3.writeAttribute( "label", IECore.StringData( "c" ), 0 )
		sc3.writeAttribute( "animColor", IECore.Color3dData( IECore.Color3d( 0 ) ), 0 )
		sc3.writeAttribute( "animColor", IECore.Color3dData( IECore.Color3d( 0.5 ) ), 0.5 )
		sc3.writeAttribute( "animColor", IECore.Color3dData( IECore.Color3d( 1 ) ), 1 )
		
		return scene
	
	def testLiveAttributes( self ) :
		
		self.writeAttributeSCC()
		
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		
		# its a link before it is expanded
		scene = IECoreHoudini.HoudiniScene( xform.path() )
		self.assertEqual( scene.attributeNames(), [ IECore.LinkedScene.linkAttribute ] )
		self.assertTrue( scene.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertEqual(
			scene.readAttribute( IECore.LinkedScene.linkAttribute, 0 ),
			IECore.CompoundData( {
				"time" : IECore.DoubleData( 0 ),
				"fileName" : IECore.StringData( "test/test.scc" ),
				"root" : IECore.InternedStringVectorData( [] )
			} )
		)
		
		# the link disapears once expanded
		xform.parm( "expand" ).pressButton()
		self.assertEqual( scene.attributeNames(), [] )
		self.assertFalse( scene.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertEqual( scene.readAttribute( IECore.LinkedScene.linkAttribute, 0 ), None )
		
		# nodes expose their attributes
		a = scene.child( "1" )
		self.assertEqual( sorted(a.attributeNames()), [ "color", "label", "sceneInterface:animatedObjectPrimVars" ] )
		for attr in a.attributeNames() :
			self.assertTrue( a.hasAttribute( attr ) )
		self.assertFalse( a.hasAttribute( "material" ) )
		self.assertEqual( a.readAttribute( "label", 0 ), IECore.StringData( "a" ) )
		self.assertEqual( a.readAttribute( "color", 0 ), IECore.Color3dData( IECore.Color3d( 0.5 ) ) )
		
		b = a.child( "2" )
		self.assertEqual( sorted(b.attributeNames()), [ "label", "material", "sceneInterface:animatedObjectPrimVars" ] )
		for attr in b.attributeNames() :
			self.assertTrue( b.hasAttribute( attr ) )
		self.assertFalse( b.hasAttribute( "color" ) )
		self.assertEqual( b.readAttribute( "label", 0 ), IECore.StringData( "b" ) )
		self.assertEqual( b.readAttribute( "material", 0 ), IECore.StringData( "rubber" ) )
		
		c = b.child( "3" )
		self.assertEqual( sorted(c.attributeNames()), [ "animColor", "label", "sceneInterface:animatedObjectPrimVars" ] )
		for attr in c.attributeNames() :
			self.assertTrue( c.hasAttribute( attr ) )
		self.assertFalse( c.hasAttribute( "color" ) )
		self.assertFalse( c.hasAttribute( "material" ) )
		self.assertEqual( c.readAttribute( "label", 0 ), IECore.StringData( "c" ) )
		self.assertEqual( c.readAttribute( "animColor", 0 ), IECore.Color3dData( IECore.Color3d( 0 ) ) )
		self.assertEqual( c.readAttribute( "animColor", 0.5 ), IECore.Color3dData( IECore.Color3d( 0.5 ) ) )
		self.assertEqual( c.readAttribute( "animColor", 1 ), IECore.Color3dData( IECore.Color3d( 1 ) ) )
	
	def testReloadButton( self ) :
		
		def testNode( node ) :
			
			self.assertEqual( node.cookCount(), 0 )
			node.cook()
			self.assertEqual( node.cookCount(), 1 )
			node.cook()
			self.assertEqual( node.cookCount(), 1 )
			node.parm( "reload" ).pressButton()
			node.cook()
			self.assertEqual( node.cookCount(), 2 )
			node.cook()
			self.assertEqual( node.cookCount(), 2 )
			node.parm( "reload" ).pressButton()
			node.cook()
			self.assertEqual( node.cookCount(), 3 )
		
		self.writeSCC()
		testNode( self.sop() )
		testNode( self.xform() )
		testNode( self.geometry() )
	
	def writeAnimSCC( self ) :
		
		scene = self.writeSCC()
		sc1 = scene.child( str( 1 ) )
		sc2 = sc1.child( str( 2 ) )
		sc3 = sc2.child( str( 3 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		
		for time in [ 0.5, 1, 1.5, 2, 5, 10 ] :
			
			matrix = IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) )
			sc1.writeTransform( IECore.M44dData( matrix ), time )
			
			mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( time, 1, 0 ) ] * 6 ) )
			sc2.writeObject( mesh, time )
			matrix = IECore.M44d.createTranslated( IECore.V3d( 2, time, 0 ) )
			sc2.writeTransform( IECore.M44dData( matrix ), time )
			
			matrix = IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) )
			sc3.writeTransform( IECore.M44dData( matrix ), time )
		
		return scene
	
	def testAnimatedScene( self ) :
		
		self.writeAnimSCC()
		times = range( 0, 10 )
		halves = [ x + 0.5 for x in times ]
		quarters = [ x + 0.25 for x in times ]
		times.extend( [ x + 0.75 for x in times ] )
		times.extend( halves )
		times.extend( quarters )
		times.sort()
		
		spf = 1.0 / hou.fps()
		
		sop = self.sop()
		for time in times :
			hou.setTime( time - spf )
			sop.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
			prims = sop.geometry().prims()
			self.assertEqual( len(prims), 18 )
			nameAttr = sop.geometry().findPrimAttrib( "name" )
			self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
			for name in nameAttr.strings() :
				self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
			self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, time, 0 ) )
			self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 2*time, 0 ) )
			self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 3*time, 0 ) )
			self.assertEqual( prims[6].attribValue( "Cd" ), ( time, 1, 0 ) )
			
			sop.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
			prims = sop.geometry().prims()
			self.assertEqual( len(prims), 3 )
			nameAttr = sop.geometry().findPrimAttrib( "name" )
			self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
			for name in nameAttr.strings() :
				self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
			self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, time + 0.5, 0.5 ) )
			self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 3.5, 2*time + 0.5, 0.5 ) )
			self.assertEqual( prims[2].vertices()[0].point().position(), hou.Vector3( 6.5, 3*time + 0.5, 0.5 ) )
		
		xform = self.xform()
		xform.parm( "expand" ).pressButton()
		a = xform.children()[0]
		b = [ x for x in a.children() if x.name() != "geo" ][0]
		c = [ x for x in b.children() if x.name() != "geo" ][0]
		for time in times :
			self.assertEqual( IECore.M44d( list(xform.worldTransformAtTime( time - spf ).asTuple()) ), IECore.M44d() )
			self.assertEqual( IECore.M44d( list(a.worldTransformAtTime( time - spf ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(b.worldTransformAtTime( time - spf ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 2, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(c.worldTransformAtTime( time - spf ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) ) )
		
		for time in times :
			hou.setTime( time - spf )
			self.assertEqual( IECore.M44d( list(xform.parmTransform().asTuple()) ), IECore.M44d() )
			self.assertEqual( IECore.M44d( list(a.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(b.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 2, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(c.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) ) )
	
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		a = xform.children()[0]
		b = xform.children()[1]
		c = xform.children()[2]
		for time in times :
			self.assertEqual( IECore.M44d( list(xform.worldTransformAtTime( time - spf ).asTuple()) ), IECore.M44d() )
			self.assertEqual( IECore.M44d( list(a.worldTransformAtTime( time - spf ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(b.worldTransformAtTime( time - spf ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 3, 2*time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(c.worldTransformAtTime( time -spf ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 6, 3*time, 0 ) ) )
	
		for time in times :
			hou.setTime( time - spf )
			self.assertEqual( IECore.M44d( list(xform.parmTransform().asTuple()) ), IECore.M44d() )
			self.assertEqual( IECore.M44d( list(a.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(b.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 2, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(c.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) ) )
	
	def testOBJOutputParms( self ) :
		
		self.writeAnimSCC()
		xform = self.xform()
		xform.parm( "expand" ).pressButton()
		c = hou.node( xform.path()+"/1/2/3" )
		
		spf = 1.0 / hou.fps()
		hou.setTime( 0 - spf )
		self.assertEqual( c.cookCount(), 0 )
		self.assertEqual( c.parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( c.parmTuple( "outR" ).eval(), ( 0, 0, 0 ) )
		self.assertEqual( c.parmTuple( "outS" ).eval(), ( 1, 1, 1 ) )
		# evaluating the output parms forces a cook
		self.assertEqual( c.cookCount(), 0 )
		
		# referencing the output parms forces a cook
		null = hou.node( "/obj" ).createNode( "null" )
		self.assertEqual( null.parmTuple( "t" ).eval(), ( 0, 0, 0 ) )
		null.parm( "tx" ).setExpression( 'ch( "%s/outTx" )' % c.path() )
		null.parm( "ty" ).setExpression( 'ch( "%s/outTy" )' % c.path() )
		null.parm( "tz" ).setExpression( 'ch( "%s/outTz" )' % c.path() )
		self.assertEqual( null.parmTuple( "t" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( c.parmTuple( "outT" ).eval(), ( 3, 0, 0 ) )
		self.assertEqual( c.cookCount(), 0 )
		
		# changing the time updates the output parms
		hou.setTime( 2.5 - spf )
		self.assertEqual( null.parmTuple( "t" ).eval(), ( 3, 2.5, 0 ) )
		self.assertEqual( c.parmTuple( "outT" ).eval(), ( 3, 2.5, 0 ) )
		self.assertEqual( c.cookCount(), 0 )
		
		# referencing the node via origin returns the same results
		null2 = hou.node( "/obj" ).createNode( "null" )
		self.assertEqual( null2.parmTuple( "t" ).eval(), ( 0, 0, 0 ) )
		null2.parm( "tx" ).setExpression( 'origin( "", "%s", "TX" )' % c.path() )
		null2.parm( "ty" ).setExpression( 'origin( "", "%s", "TY" )' % c.path() )
		null2.parm( "tz" ).setExpression( 'origin( "", "%s", "TZ" )' % c.path() )
		self.assertEqual( null2.parmTuple( "t" ).eval(), ( 3, 2.5, 0 ) )
		self.assertEqual( c.parmTuple( "outT" ).eval(), ( 3, 2.5, 0 ) )
		self.assertEqual( c.cookCount(), 1 )
		
		# evaluating at different times also works
		self.assertEqual( c.parmTuple( "outT" ).evalAtTime( 2.75 - spf ), ( 3, 2.75, 0 ) )
		self.assertEqual( null.parmTuple( "t" ).evalAtTime( 5.25 - spf ), ( 3, 5.25, 0 ) )
		self.assertEqual( c.cookCount(), 1 )
	
	def compareScene( self, a, b, time = 0, bakedObjects = [], parentTransform = None ) :
		
		self.assertEqual( a.name(), b.name() )
		self.assertEqual( a.path(), b.path() )
		if b.name() in bakedObjects :
			aTransform = a.readTransformAsMatrix( time )
			parentTransform = aTransform if parentTransform is None else aTransform * parentTransform
			self.assertEqual( b.readTransformAsMatrix( time ), IECore.M44d() )
		else :
			self.assertEqual( set(a.readTags(IECore.SceneInterface.LocalTag)), set(b.readTags(IECore.SceneInterface.LocalTag)) )
			self.assertEqual( set(a.readTags(IECore.SceneInterface.DescendantTag)), set(b.readTags(IECore.SceneInterface.DescendantTag)) )
			self.assertEqual( set(a.readTags(IECore.SceneInterface.AncestorTag)), set(b.readTags(IECore.SceneInterface.AncestorTag)) )
			self.assertEqual( set(a.readTags(IECore.SceneInterface.EveryTag)), set(b.readTags(IECore.SceneInterface.EveryTag)) )
			self.assertTrue( a.readTransformAsMatrix( time ).equalWithAbsError( b.readTransformAsMatrix( time ), 1e-6 ) )
			ab = a.readBound( time )
			bb = b.readBound( time )
			self.assertTrue( ab.min.equalWithAbsError( bb.min, 1e-6 ) )
			self.assertTrue( ab.max.equalWithAbsError( bb.max, 1e-6 ) )
		
		aAttrs = a.attributeNames()
		bAttrs = b.attributeNames()
		# need to remove the animatedObjectPrimVars attribute since it doesn't exist in some circumstances
		if "sceneInterface:animatedObjectPrimVars" in aAttrs :
			aAttrs.remove( "sceneInterface:animatedObjectPrimVars" )
		if "sceneInterface:animatedObjectPrimVars" in bAttrs :
			bAttrs.remove( "sceneInterface:animatedObjectPrimVars" )
		self.assertEqual( aAttrs, bAttrs )
		for attr in aAttrs :
			self.assertTrue( a.hasAttribute( attr ) )
			self.assertTrue( b.hasAttribute( attr ) )
			self.assertEqual( a.readAttribute( attr, time ), b.readAttribute( attr, time ) )
		
		self.assertEqual( a.hasObject(), b.hasObject() )
		if a.hasObject() :
			ma = a.readObject( time )
			mb = b.readObject( time )
			# need to adjust P for baked objects
			if b.name() in bakedObjects :
				IECore.TransformOp()( input=ma, copyInput=False, matrix=IECore.M44dData( parentTransform ) )
			self.assertEqual( ma, mb )
		
		self.assertEqual( sorted( a.childNames() ), sorted( b.childNames() ) )
		for child in a.childNames() :
			self.compareScene( a.child( child ), b.child( child ), time = time, bakedObjects = bakedObjects, parentTransform = parentTransform )
	
	def testRopHierarchy( self ) :
		
		# test a parented xform
		self.writeTaggedSCC()
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "expand" ).pressButton()
		rop = self.rop( xform )
		self.assertFalse( os.path.exists( TestSceneCache.__testOutFile ) )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		self.assertTrue( os.path.exists( TestSceneCache.__testOutFile ) )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output )
		
		# test a subnet xform
		os.remove( TestSceneCache.__testOutFile )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "expand" ).pressButton()
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output )
		
		# test a mixed subnet/parented xform
		os.remove( TestSceneCache.__testOutFile )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		a = xform.children()[0]
		a.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		a.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		a.parm( "expand" ).pressButton()
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output )
	
	def testRopFlattened( self ) :
		
		# test a flat xform
		self.writeSCC()
		xform = self.xform()
		rop = self.rop( xform )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.FlatGeometry )
		xform.parm( "expand" ).pressButton()
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output, bakedObjects = [ "1", "2", "3" ] )
		
		# test a mixed subnet/flat xform
		os.remove( TestSceneCache.__testOutFile )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		a = xform.children()[0]
		a.parm( "expand" ).pressButton()
		b = a.children()[1]
		b.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.FlatGeometry )
		b.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		b.parm( "expand" ).pressButton()
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output, bakedObjects = [ "3" ] )
		
		# test a OBJ Geo
		os.remove( TestSceneCache.__testOutFile )
		geo = self.geometry()
		geo.parm( "expand" ).pressButton()
		rop.parm( "rootObject" ).set( geo.path() )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output, bakedObjects = [ "1", "2", "3" ] )
	
	def testRopFlattenedWithGaps( self ) :
		
		scene = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Write )
		
		sc = scene.createChild( str( 1 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 6 ) )
		sc.writeObject( mesh, 0 )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0 )
		
		sc = sc.createChild( str( 2 ) )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0 )
		
		sc = sc.createChild( str( 3 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 1 ) ] * 6 ) )
		sc.writeObject( mesh, 0 )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 3, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0 )
		
		del scene, sc
		
		geo = self.geometry()
		geo.parm( "expand" ).pressButton()
		self.assertEqual( len(geo.children()), 1 )
		node = geo.children()[0]
		self.assertEqual( node.name(), "root" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 2 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 6.5, 0.5, 0.5 ) )
		
		rop = self.rop( geo )
		rop.parm( "rootObject" ).set( geo.path() )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output, bakedObjects = [ "1", "2", "3" ] )
	
	def testRopTopLevelGeo( self ) :
		
		self.writeSCC()
		geo = self.geometry()
		geo.parm( "expand" ).pressButton()
		attr = geo.children()[0].createOutputNode( "attribute" )
		attr.parm( "primdel" ).set( "name" )
		attr.setDisplayFlag( True )
		attr.setRenderFlag( True )
		rop = self.rop( geo )
		rop.parm( "trange" ).set( 1 )
		rop.parmTuple( "f" ).set( ( 1, 10, 1 ) )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( output.name(), "/" )
		self.assertEqual( output.readTransformAsMatrix( 0 ), IECore.M44d() )
		self.assertFalse( output.hasObject() )
		self.assertEqual( output.childNames(), [ "ieSceneCacheGeometry1" ] )
		root = output.child( "ieSceneCacheGeometry1" )
		self.assertEqual( root.name(), "ieSceneCacheGeometry1" )
		self.assertEqual( root.readTransformAsMatrix( 0 ), IECore.M44d() )
		self.assertTrue( root.hasObject() )
		obj = root.readObject( 0 )
		self.assertTrue( obj.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(obj.children()), 3 )
		for child in obj.children() :
			self.assertTrue( child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertEqual( root.childNames(), [] )
	
	def testRopLinked( self ) :
		
		self.writeTaggedSCC()
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "expand" ).pressButton()
		a = xform.children()[0]
		a.parm( "expand" ).pressButton()
		# leaving b and below as a link
		
		rop = self.rop( xform )
		rop.parm( "file" ).set( TestSceneCache.__testLinkedOutFile )
		self.assertFalse( os.path.exists( TestSceneCache.__testLinkedOutFile ) )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		self.assertTrue( os.path.exists( TestSceneCache.__testLinkedOutFile ) )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		linked = IECore.LinkedScene( TestSceneCache.__testLinkedOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, linked )
		
		# make sure there really is a link
		unlinked = IECore.SceneCache( TestSceneCache.__testLinkedOutFile, IECore.IndexedIO.OpenMode.Read )
		a = unlinked.child( "1" )
		self.assertFalse( a.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertFalse( a.hasAttribute( IECore.LinkedScene.fileNameLinkAttribute ) )
		self.assertFalse( a.hasAttribute( IECore.LinkedScene.rootLinkAttribute ) )
		self.assertFalse( a.hasAttribute( IECore.LinkedScene.timeLinkAttribute ) )
		b = a.child( "2" )
		self.assertEqual( b.childNames(), [] )
		self.assertFalse( b.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertTrue( b.hasAttribute( IECore.LinkedScene.fileNameLinkAttribute ) )
		self.assertTrue( b.hasAttribute( IECore.LinkedScene.rootLinkAttribute ) )
		self.assertEqual( b.readAttribute( IECore.LinkedScene.fileNameLinkAttribute, 0 ), IECore.StringData( TestSceneCache.__testFile ) )
		self.assertEqual( b.readAttribute( IECore.LinkedScene.rootLinkAttribute, 0 ), IECore.InternedStringVectorData( [ "1", "2" ] ) )
		self.assertEqual( b.readAttribute( IECore.LinkedScene.timeLinkAttribute, 0 ), IECore.DoubleData( 1.0 / hou.fps() ) )
		
		# make sure we can force link expansion
		xform.parm( "collapse" ).pressButton()
		xform.parm( "file" ).set( TestSceneCache.__testLinkedOutFile )
		self.assertEqual( xform.children(), tuple() )
		self.assertEqual( xform.parm( "expanded" ).eval(), False )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "expand" ).pressButton()
		rop.parm( "file" ).set( TestSceneCache.__testOutFile )
		self.assertFalse( os.path.exists( TestSceneCache.__testOutFile ) )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		self.assertTrue( os.path.exists( TestSceneCache.__testOutFile ) )
		expanded = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, expanded )
		self.compareScene( expanded, linked )
		
		# make sure we can read back the whole structure in Houdini
		xform.parm( "collapse" ).pressButton()
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "expand" ).pressButton()
		live = IECoreHoudini.HoudiniScene( xform.path(), rootPath = [ xform.name() ] )
		self.compareScene( orig, live )
		
		# make sure it doesn't crash if the linked scene doesn't exist anymore
		xform.parm( "collapse" ).pressButton()
		os.remove( TestSceneCache.__testFile )
		IECore.SharedSceneInterfaces.clear()
		xform.parm( "reload" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertEqual( xform.parm( "root" ).menuItems(), ( "/", "/1", "/1/2" ) )
	
	def testRopForceObjects( self ) :
		
		s = self.writeAttributeSCC()
		d = s.child( "1" ).createChild( "4" )
		e = d.createChild( "5" )
		box = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		d.writeObject( box, 0 )
		e.writeObject( box, 0 )
		
		del s, d, e
		
		def testLinks( bakedObjects = None ) :
			
			if os.path.exists( TestSceneCache.__testLinkedOutFile ) :
				os.remove( TestSceneCache.__testLinkedOutFile )
			
			rop.parm( "execute" ).pressButton()
			self.assertEqual( rop.errors(), "" )
			self.assertTrue( os.path.exists( TestSceneCache.__testLinkedOutFile ) )
			linked = IECore.LinkedScene( TestSceneCache.__testLinkedOutFile, IECore.IndexedIO.OpenMode.Read )
			if bakedObjects :
				live = IECoreHoudini.HoudiniScene( xform.path(), rootPath = [ xform.name() ] )
				self.compareScene( linked, live, bakedObjects = bakedObjects )
			else :
				orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
				self.compareScene( orig, linked )
			
			# make sure the links are where we expect
			unlinked = IECore.SceneCache( TestSceneCache.__testLinkedOutFile, IECore.IndexedIO.OpenMode.Read )
			a = unlinked.child( "1" )
			self.assertFalse( a.hasAttribute( IECore.LinkedScene.linkAttribute ) )
			self.assertFalse( a.hasAttribute( IECore.LinkedScene.fileNameLinkAttribute ) )
			self.assertFalse( a.hasAttribute( IECore.LinkedScene.rootLinkAttribute ) )
			self.assertFalse( a.hasAttribute( IECore.LinkedScene.timeLinkAttribute ) )
			b = a.child( "2" )
			self.assertEqual( b.childNames(), [] )
			self.assertFalse( b.hasAttribute( IECore.LinkedScene.linkAttribute ) )
			self.assertTrue( b.hasAttribute( IECore.LinkedScene.fileNameLinkAttribute ) )
			self.assertTrue( b.hasAttribute( IECore.LinkedScene.rootLinkAttribute ) )
			self.assertEqual( b.readAttribute( IECore.LinkedScene.fileNameLinkAttribute, 0 ), IECore.StringData( TestSceneCache.__testFile ) )
			self.assertEqual( b.readAttribute( IECore.LinkedScene.rootLinkAttribute, 0 ), IECore.InternedStringVectorData( [ "1", "2" ] ) )
			self.assertEqual( b.readAttribute( IECore.LinkedScene.timeLinkAttribute, 0 ), IECore.DoubleData( 0 ) )
			d = a.child( "4" )
			self.assertFalse( d.hasAttribute( IECore.LinkedScene.linkAttribute ) )
			self.assertFalse( d.hasAttribute( IECore.LinkedScene.fileNameLinkAttribute ) )
			self.assertFalse( d.hasAttribute( IECore.LinkedScene.rootLinkAttribute ) )
			self.assertFalse( d.hasAttribute( IECore.LinkedScene.timeLinkAttribute ) )
		
		# force b and below as links even though they are expanded
		hou.setTime( -1.0 / hou.fps() )
		xform = self.xform()
		xform.parm( "expand" ).pressButton()
		rop = self.rop( xform )
		rop.parm( "file" ).set( TestSceneCache.__testLinkedOutFile )
		rop.parm( "forceObjects" ).set( "*4*" )
		testLinks()
		
		# make sure parents expand if their child is forced
		rop.parm( "forceObjects" ).set( "*5*" )
		testLinks()
		
		# make sure normal geo gets expanded regardless
		geo = xform.createNode( "geo", "real" )
		geo.createNode( "box" )
		testLinks( bakedObjects = [ "real" ] )
		unlinked = IECore.SceneCache( TestSceneCache.__testLinkedOutFile, IECore.IndexedIO.OpenMode.Read )
		real = unlinked.child( "real" )
		self.assertFalse( real.hasAttribute( IECore.LinkedScene.linkAttribute ) )
		self.assertFalse( real.hasAttribute( IECore.LinkedScene.fileNameLinkAttribute ) )
		self.assertFalse( real.hasAttribute( IECore.LinkedScene.rootLinkAttribute ) )
		self.assertFalse( real.hasAttribute( IECore.LinkedScene.timeLinkAttribute ) )
		self.assertTrue( real.hasObject() )
		geo.destroy()
		
		# make sure natural links (unexpanded branches) still work
		hou.node( xform.path() + "/1/2" ).parm( "collapse" ).pressButton()
		testLinks()
		
		# make sure normal SceneCaches aren't broken by forceObjects
		rop.parm( "file" ).set( TestSceneCache.__testOutFile )
		self.assertFalse( os.path.exists( TestSceneCache.__testOutFile ) )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		self.assertTrue( os.path.exists( TestSceneCache.__testOutFile ) )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		result = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, result )
	
	def testRopErrors( self ) :
		
		xform = self.xform()
		rop = self.rop( xform )
		rop.parm( "file" ).set( "/tmp/bad.file" )
		rop.parm( "execute" ).pressButton()
		self.assertNotEqual( rop.errors(), "" )
		self.assertFalse( os.path.exists( TestSceneCache.__testOutFile ) )
		
		rop.parm( "file" ).set( TestSceneCache.__testOutFile )
		rop.parm( "rootObject" ).set( "/obj/fake/object" )
		self.assertNotEqual( rop.errors(), "" )
		self.assertFalse( os.path.exists( TestSceneCache.__testOutFile ) )
		
		rop.parm( "rootObject" ).set( xform.path() )
		xform.destroy()
		self.assertNotEqual( rop.errors(), "" )
		self.assertFalse( os.path.exists( TestSceneCache.__testOutFile ) )
		
		sop = self.sop()
		rop.parm( "rootObject" ).set( sop.path() )
		rop.parm( "execute" ).pressButton()
		self.assertNotEqual( rop.errors(), "" )
		self.assertFalse( os.path.exists( TestSceneCache.__testOutFile ) )
		
		newXform = self.xform()
		rop.parm( "rootObject" ).set( newXform.path() )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		self.assertTrue( os.path.exists( TestSceneCache.__testOutFile ) )
	
	def testAnimatedRop( self ) :
		
		self.writeAnimSCC()
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "expand" ).pressButton()
		rop = self.rop( xform )
		rop.parm( "trange" ).set( 1 )
		rop.parmTuple( "f" ).set( ( 0, 10 * hou.fps(), 1 ) )
		rop.parm( "execute" ).pressButton()
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		
		times = range( 0, 10 )
		halves = [ x + 0.5 for x in times ]
		quarters = [ x + 0.25 for x in times ]
		times.extend( [ x + 0.75 for x in times ] )
		times.extend( halves )
		times.extend( quarters )
		times.sort()
		
		for time in times :
			self.compareScene( orig, output, time )
	
	def testRopCookCounts( self ) :
		
		self.writeAnimSCC()
		xform = self.xform()
		xform.parm( "expand" ).pressButton()
		a = hou.node( xform.path()+"/1/geo/1" ) # static
		b = hou.node( xform.path()+"/1/2/geo/2" ) # animated
		c = hou.node( xform.path()+"/1/2/3/geo/3" ) # static
		# make sure nothing has been cooked
		self.assertEqual( a.cookCount(), 0 )
		self.assertEqual( b.cookCount(), 0 )
		self.assertEqual( c.cookCount(), 0 )
		
		# cook current frame
		rop = self.rop( xform )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		self.assertEqual( a.cookCount(), 1 )
		self.assertEqual( b.cookCount(), 1 )
		self.assertEqual( c.cookCount(), 1 )
		
		# cook single frame that is not the current frame
		self.assertNotEqual( hou.frame(), 10 )
		rop.parm( "trange" ).set( 1 )
		rop.parm( "f1" ).set( 10 )
		rop.parm( "f2" ).set( 10 )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		self.assertEqual( a.cookCount(), 1 )
		self.assertEqual( b.cookCount(), 2 )
		self.assertEqual( c.cookCount(), 1 )
		
		# cook a range
		rop.parm( "f1" ).set( 1 )
		rop.parm( "f2" ).set( 10 )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		self.assertEqual( a.cookCount(), 1 )
		self.assertEqual( b.cookCount(), 12 )
		self.assertEqual( c.cookCount(), 1 )
		
		# with flat geo
		sop = self.sop()
		self.assertEqual( sop.cookCount(), 0 )
		rop.parm( "rootObject" ).set( sop.parent().path() )
		rop.parm( "trange" ).set( 1 )
		rop.parm( "f1" ).set( 10 )
		rop.parm( "f2" ).set( 10 )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( sop.cookCount(), 1 )
	
	def testLiveScene( self ) :
		
		self.writeTaggedSCC()
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "expand" ).pressButton()
		a = xform.children()[0]
		a.parm( "expand" ).pressButton()
		b = a.children()[1]
		b.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.FlatGeometry )
		b.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		b.parm( "expand" ).pressButton()
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		live = IECoreHoudini.HoudiniScene( xform.path(), rootPath = [ xform.name() ] )
		self.compareScene( orig, live, bakedObjects = [ "3" ] )
	
	def testTopologyChanges( self ) :
		
		plane = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		box = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1 ) ) )
		box["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ IECore.Color3f( 1, 0, 0 ) ] * box.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) ) )
		box2 = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( 2 ), IECore.V3f( 3 ) ) )
		box2["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ IECore.Color3f( 0, 1, 0 ) ] * box.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) ) )
		
		s = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Write )
		a = s.createChild( "a" )
		b = a.createChild( "b" )
		c = a.createChild( "c" )
		d = a.createChild( "d" )
		
		# animated topology
		b.writeObject( box, 0 )
		b.writeObject( plane, 1 )
		
		# static
		c.writeObject( box, 0 )
		c.writeObject( box, 1 )
		
		# animated P and Cd
		d.writeObject( box, 0 )
		d.writeObject( box2, 1 )
		del s, a, b, c, d
		
		spf = 1.0 / hou.fps()
		hou.setTime( 0 - spf )
		node = self.sop()
		node.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(node.geometry().points()), 24 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( set(nameAttr.strings()), set([ '/a/b', '/a/c', '/a/d' ]) )
		bPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/b' ]
		cPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/c' ]
		dPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/d' ]
		self.assertEqual( len(bPrims), 6 )
		self.assertEqual( len(cPrims), 6 )
		self.assertEqual( len(dPrims), 6 )
		self.assertEqual( bPrims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( cPrims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( dPrims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( bPrims[0].attribValue( "Cd" ), ( 1, 0, 0 ) )
		self.assertEqual( cPrims[0].attribValue( "Cd" ), ( 1, 0, 0 ) )
		self.assertEqual( dPrims[0].attribValue( "Cd" ), ( 1, 0, 0 ) )
		
		hou.setTime( 1 - spf )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 13 )
		self.assertEqual( len(node.geometry().points()), 20 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( set(nameAttr.strings()), set([ '/a/b', '/a/c', '/a/d' ]) )
		bPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/b' ]
		cPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/c' ]
		dPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/d' ]
		self.assertEqual( len(bPrims), 1 )
		self.assertEqual( len(cPrims), 6 )
		self.assertEqual( len(dPrims), 6 )
		self.assertEqual( bPrims[0].vertex( 0 ).point().position(), hou.Vector3( -1, 1, 0 ) )
		self.assertEqual( cPrims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( dPrims[0].vertex( 0 ).point().position(), hou.Vector3( 2, 2, 2 ) )
		self.assertEqual( bPrims[0].attribValue( "Cd" ), ( 0, 0, 0 ) )
		self.assertEqual( cPrims[0].attribValue( "Cd" ), ( 1, 0, 0 ) )
		self.assertEqual( dPrims[0].attribValue( "Cd" ), ( 0, 1, 0 ) )
		
		hou.setTime( 0.5 - spf )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 13 )
		self.assertEqual( len(node.geometry().points()), 20 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( set(nameAttr.strings()), set([ '/a/b', '/a/c', '/a/d' ]) )
		bPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/b' ]
		cPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/c' ]
		dPrims = [ x for x in prims if x.attribValue( nameAttr ) == '/a/d' ]
		self.assertEqual( len(bPrims), 1 )
		self.assertEqual( len(cPrims), 6 )
		self.assertEqual( len(dPrims), 6 )
		self.assertEqual( bPrims[0].vertex( 0 ).point().position(), hou.Vector3( -1, 1, 0 ) )
		self.assertEqual( cPrims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( dPrims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 1, 1 ) )
		self.assertEqual( bPrims[0].attribValue( "Cd" ), ( 0, 0, 0 ) )
		self.assertEqual( cPrims[0].attribValue( "Cd" ), ( 1, 0, 0 ) )
		self.assertEqual( dPrims[0].attribValue( "Cd" ), ( 0.5, 0.5, 0 ) )
	
	def testSaveLoadCortexObjects( self ) :
		
		self.writeSCC()
		sop = self.sop()
		sop.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		null = sop.createOutputNode( "null" )
		nullPath = null.path()
		prims = null.geometry().prims()
		self.assertEqual( len(prims), 3 )
		for i in range( 0, 3 ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
		
		# make sure they survive the locks
		null.setHardLocked( True )
		null.setInput( 0, None )
		prims = null.geometry().prims()
		self.assertEqual( len(prims), 3 )
		for i in range( 0, 3 ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
		
		# make sure they survive a scene save/load
		hou.hipFile.save( TestSceneCache.__testHip )
		hou.hipFile.load( TestSceneCache.__testHip )
		null = hou.node( nullPath )
		prims = null.geometry().prims()
		self.assertEqual( len(prims), 3 )
		for i in range( 0, 3 ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
		
		# make sure they survive bgeo caching
		writer = null.createOutputNode( "file" )
		writer.parm( "file" ).set( TestSceneCache.__testBgeo )
		writer.parm( "filemode" ).set( 2 ) # write
		writer.cook( force = True )
		reader = null.parent().createNode( "file" )
		reader.parm( "file" ).set( TestSceneCache.__testBgeo )
		prims = reader.geometry().prims()
		self.assertEqual( len(prims), 3 )
		for i in range( 0, 3 ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
		
		# make sure they survive bgeo.gz caching
		writer.parm( "file" ).set( TestSceneCache.__testBgeoGz )
		writer.cook( force = True )
		reader = null.parent().createNode( "file" )
		reader.parm( "file" ).set( TestSceneCache.__testBgeoGz )
		prims = reader.geometry().prims()
		self.assertEqual( len(prims), 3 )
		for i in range( 0, 3 ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
		
		# make sure they survive geo caching
		writer.parm( "file" ).set( TestSceneCache.__testGeo )
		writer.cook( force = True )
		reader = null.parent().createNode( "file" )
		reader.parm( "file" ).set( TestSceneCache.__testGeo )
		prims = reader.geometry().prims()
		self.assertEqual( len(prims), 3 )
		for i in range( 0, 3 ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
	
	def testStashing( self ) :
		
		self.writeSCC()
		sop = self.sop()
		writer = sop.createOutputNode( "file" )
		writer.parm( "file" ).set( TestSceneCache.__testBgeo )
		writer.parm( "filemode" ).set( 2 ) # write
		writer.cook( force = True )
		reader = sop.parent().createNode( "file" )
		reader.parm( "file" ).set( TestSceneCache.__testBgeo )
		reader.cook( force = True )
		reader.cook( force = True )
	
	def testNonPrimitiveCortexObjects( self ) :
		
		scene = self.writeAnimSCC()
		coordChild = scene.child( "1" ).createChild( "coord" )
		coord = IECore.CoordinateSystem()
		coord.setName( "testing" )
		coord.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 1, 2, 3 ) ) ) )
		coordChild.writeObject( coord, 0 )
		coord.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 1, 5, 5 ) ) ) )
		coordChild.writeObject( coord, 1 )
		intsChild = scene.createChild( "ints" )
		intsChild.writeObject( IECore.IntVectorData( [ 1, 10, 20, 30 ] ), 0 )
		
		del scene, coordChild, intsChild
		
		spf = 1.0 / hou.fps()
		hou.setTime( 0 - spf )
		sop = self.sop()
		sop.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 5 )
		
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( sorted(nameAttr.strings()), ['/1', '/1/2', '/1/2/3', '/1/coord', '/ints'] )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		
		for i in range( 0, 5 ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
		
		aPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/1' ][0]
		bPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/1/2' ][0]
		cPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/1/2/3' ][0]
		coordPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/1/coord' ][0]
		intsPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/ints' ][0]
		
		self.assertEqual( aPrim.vertices()[0].point().position(), hou.Vector3( 1.5, 0.5, 0.5 ) )
		self.assertEqual( bPrim.vertices()[0].point().position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		self.assertEqual( cPrim.vertices()[0].point().position(), hou.Vector3( 6.5, 0.5, 0.5 ) )
		self.assertEqual( coordPrim.vertices()[0].point().position(), hou.Vector3( 2, 2, 3 ) )
		self.assertEqual( intsPrim.vertices()[0].point().position(), hou.Vector3( 0, 0, 0 ) )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop )
		self.assertTrue( isinstance( converter, IECoreHoudini.FromHoudiniCompoundObjectConverter ) )
		result = converter.convert()
		self.assertTrue( isinstance( result, IECore.CompoundObject ) )
		self.assertEqual( sorted( result.keys() ), ['/1', '/1/2', '/1/2/3', '/1/coord', '/ints'] )
		self.assertTrue( isinstance( result["/1"], IECore.MeshPrimitive ) )
		self.assertTrue( isinstance( result["/1/2"], IECore.MeshPrimitive ) )
		self.assertTrue( isinstance( result["/1/2/3"], IECore.MeshPrimitive ) )
		self.assertTrue( isinstance( result["/1/coord"], IECore.CoordinateSystem ) )
		self.assertEqual( result["/ints"], IECore.IntVectorData( [ 1, 10, 20, 30 ] ) )
		
		hou.setTime( 1 - spf )
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 5 )
		
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( sorted(nameAttr.strings()), ['/1', '/1/2', '/1/2/3', '/1/coord', '/ints'] )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 1 )
		
		for i in range( 0, 5 ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
		
		aPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/1' ][0]
		bPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/1/2' ][0]
		cPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/1/2/3' ][0]
		coordPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/1/coord' ][0]
		intsPrim = [ x for x in prims if x.attribValue( nameAttr ) == '/ints' ][0]
		
		self.assertEqual( aPrim.vertices()[0].point().position(), hou.Vector3( 1.5, 1.5, 0.5 ) )
		self.assertEqual( bPrim.vertices()[0].point().position(), hou.Vector3( 3.5, 2.5, 0.5 ) )
		self.assertEqual( cPrim.vertices()[0].point().position(), hou.Vector3( 6.5, 3.5, 0.5 ) )
		self.assertEqual( coordPrim.vertices()[0].point().position(), hou.Vector3( 2, 6, 5 ) )
		self.assertEqual( intsPrim.vertices()[0].point().position(), hou.Vector3( 0, 0, 0 ) )
		
		sop.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 18 )
		self.assertTrue( "/1/coord" in sop.warnings() )
		self.assertTrue( "/ints" in sop.warnings() )
		
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 1, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 2, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 3, 0 ) )
	
	def testCoordinateSystemNoTransform( self ) :
		
		scene = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Write )
		coordChild = scene.createChild( "coord")
		coord = IECore.CoordinateSystem()
		coord.setName( "testing" )
		coordChild.writeObject( coord, 0 )
		coordChild2 = coordChild.createChild( "other")
		coordChild2.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 2, 3 ) ) ), 0 )
		coordChild2.writeObject( coord, 0 )
		
		del scene, coordChild, coordChild2
		
		sop = self.sop()
		sop.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Cortex )
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 2 )
		
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( sorted(nameAttr.strings()), [ "/coord", "/coord/other" ] )
		self.assertEqual( prims[0].attribValue( "name" ), "/coord" )
		self.assertEqual( prims[0].type(), hou.primType.Custom )
		self.assertEqual( prims[0].vertices()[0].point().number(), 0 )
		self.assertEqual( prims[0].vertices()[0].point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[1].attribValue( "name" ), "/coord/other" )
		self.assertEqual( prims[1].type(), hou.primType.Custom )
		self.assertEqual( prims[1].vertices()[0].point().number(), 1 )
		self.assertEqual( prims[1].vertices()[0].point().position(), hou.Vector3( 1, 2, 3 ) )
	
	def testObjectMerge( self ) :
		
		self.writeSCC()
		xform = self.xform()
		xform.parm( "expand" ).pressButton()
		origSop = hou.node( xform.path()+"/1/2/geo/2" )
		merge = hou.node( "/obj" ).createNode( "geo" ).createNode( "object_merge" )
		merge.parm( "objpath1" ).set( origSop.path() )
		
		# not transformed because we haven't set "Into this Object"
		geo = merge.geometry()
		self.assertEqual( geo.points()[0].position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
		self.assertEqual( geo.boundingBox(), hou.BoundingBox( 0, 0, 0, 1, 1, 1 ) )
		
		# transformed to its world position
		merge.parm( "xformtype" ).set( 1 ) # "Into this Object"
		geo = merge.geometry()
		self.assertEqual( geo.points()[0].position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		self.assertEqual( geo.boundingBox(), hou.BoundingBox( 3, 0, 0, 4, 1, 1 ) )
		mesh = IECoreHoudini.FromHoudiniGeometryConverter.create( merge ).convert()
		self.assertEqual( mesh.bound(), IECore.Box3f( IECore.V3f( 3, 0, 0 ), IECore.V3f( 4, 1, 1 ) ) )
		
		# didn't affect the original SOP because it stores it's own copy of the prim
		geo = origSop.geometry()
		self.assertEqual( geo.points()[0].position(), hou.Vector3( 0.5, 0.5, 0.5 ) )
		self.assertEqual( geo.boundingBox(), hou.BoundingBox( 0, 0, 0, 1, 1, 1 ) )
		mesh = IECoreHoudini.FromHoudiniGeometryConverter.create( origSop ).convert()
		self.assertEqual( mesh.bound(), IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1 ) ) )
		
		# transformable at the SOP level as well
		sopXform = merge.createOutputNode( "xform" )
		sopXform.parm( "ty" ).set( 7 )
		geo = sopXform.geometry()
		self.assertEqual( geo.points()[0].position(), hou.Vector3( 3.5, 7.5, 0.5 ) )
		self.assertEqual( geo.boundingBox(), hou.BoundingBox( 3, 7, 0, 4, 8, 1 ) )
		mesh = IECoreHoudini.FromHoudiniGeometryConverter.create( sopXform ).convert()
		self.assertEqual( mesh.bound(), IECore.Box3f( IECore.V3f( 3, 7, 0 ), IECore.V3f( 4, 8, 1 ) ) )
		
		# didn't affect the input SOP because it stores it's own copy of the prim
		geo = merge.geometry()
		self.assertEqual( geo.points()[0].position(), hou.Vector3( 3.5, 0.5, 0.5 ) )
		self.assertEqual( geo.boundingBox(), hou.BoundingBox( 3, 0, 0, 4, 1, 1 ) )
		mesh = IECoreHoudini.FromHoudiniGeometryConverter.create( merge ).convert()
		self.assertEqual( mesh.bound(), IECore.Box3f( IECore.V3f( 3, 0, 0 ), IECore.V3f( 4, 1, 1 ) ) )
	
	def testPointsDontAccumulate( self ) :
		
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		box = geo.createNode( "box" )
		facet = geo.createNode( "facet" )
		facet.parm("postnml").set(True)
		points = geo.createNode( "scatter" )
		facet.setInput( 0, box )
		points.setInput( 0, facet )
		points.setRenderFlag( True )
		points.setDisplayFlag( True )
		
		rop = self.rop( geo )
		rop.parm( "trange" ).set( 1 )
		rop.parmTuple( "f" ).set( ( 1, 10, 1 ) )
		rop.parm( "execute" ).pressButton()
		
		sop = self.sop()
		sop.parm( "file" ).set( TestSceneCache.__testOutFile )
		sop.parm( "geometryType" ).set( IECoreHoudini.SceneCacheNode.GeometryType.Houdini )
		
		for t in range( 0, 10 ) :
			hou.setTime( t )
			self.assertEqual( len(sop.geometry().points()), 5000 )
			self.assertEqual( len(sop.geometry().prims()), 1 )
	
	def testTimeDependent( self ) :
		
		self.writeSCC()
		xform = self.xform()
		xform.parm( "expand" ).pressButton()
		self.assertFalse( hou.node( xform.path()+"/1" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/geo" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/geo/1" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/geo" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/geo/2" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3/geo" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3/geo/3" ).isTimeDependent() )
		
		scene = self.writeSCC()
		sc1 = scene.child( str( 1 ) )
		sc2 = sc1.child( str( 2 ) )
		sc3 = sc2.child( str( 3 ) )
		
		for time in [ 0.5, 1, 1.5, 2, 5, 10 ] :
			
			matrix = IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) )
			sc1.writeTransform( IECore.M44dData( matrix ), time )
			
			matrix = IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) )
			sc3.writeTransform( IECore.M44dData( matrix ), time )
		
		del scene, sc1, sc2, sc3
		
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1" ).isTimeDependent() )
		self.assertTrue( hou.node( xform.path()+"/1/geo" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/geo/1" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2" ).isTimeDependent() )
		self.assertTrue( hou.node( xform.path()+"/1/2/geo" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/geo/2" ).isTimeDependent() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3" ).isTimeDependent() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3/geo" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3/geo/3" ).isTimeDependent() )
		
		scene = self.writeSCC()
		sc1 = scene.child( str( 1 ) )
		sc2 = sc1.child( str( 2 ) )
		sc3 = sc2.child( str( 3 ) )
		
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		for time in [ 0.5, 1, 1.5, 2, 5, 10 ] :
			
			matrix = IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) )
			sc1.writeTransform( IECore.M44dData( matrix ), time )
			
			mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( time, 1, 0 ) ] * 6 ) )
			sc2.writeObject( mesh, time )
		
		del scene, sc1, sc2, sc3
		
		xform.parm( "collapse" ).pressButton()
		xform.parm( "expand" ).pressButton()
		self.assertTrue( hou.node( xform.path()+"/1" ).isTimeDependent() )
		self.assertTrue( hou.node( xform.path()+"/1/geo" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/geo/1" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2" ).isTimeDependent() )
		self.assertTrue( hou.node( xform.path()+"/1/2/geo" ).isTimeDependent() )
		self.assertTrue( hou.node( xform.path()+"/1/2/geo/2" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3" ).isTimeDependent() )
		self.assertTrue( hou.node( xform.path()+"/1/2/3/geo" ).isTimeDependent() )
		self.assertFalse( hou.node( xform.path()+"/1/2/3/geo/3" ).isTimeDependent() )
	
	def tearDown( self ) :
		
		for f in [ TestSceneCache.__testFile, TestSceneCache.__testOutFile, TestSceneCache.__testLinkedOutFile, TestSceneCache.__testHip, TestSceneCache.__testBgeo, TestSceneCache.__testBgeoGz, TestSceneCache.__testGeo ] :
			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
    unittest.main()
