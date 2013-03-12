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
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 6 ) )
		sc.writeObject( mesh, time )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		sc.writeTransform( IECore.M44dData( matrix ), time )
		
		sc = sc.createChild( str( 2 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 1, 0 ) ] * 6 ) )
		sc.writeObject( mesh, time )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		sc.writeTransform( IECore.M44dData( matrix ), time )
		
		sc = sc.createChild( str( 3 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 1 ) ] * 6 ) )
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
			node.parm( "root" ).pressButton()
			self.assertRaises( hou.OperationFailed, IECore.curry( node.cook, True ) )
			self.failUnless( node.errors() )
			node.parm( "root" ).set( "/1/2" )
			node.parm( "root" ).pressButton()
			node.cook( force=True )
			self.failUnless( not node.errors() )
			
			if isinstance( node, hou.ObjNode ) :
				node.parm( "build" ).pressButton()
				self.failUnless( node.children() )
				node.parm( "root" ).set( "/1/fake" )
				node.parm( "root" ).pressButton()
				node.parm( "build" ).pressButton()
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
			node.parm( "root" ).pressButton()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 1, 0, 0 ) )
			node.parm( "root" ).set( "/1/2" )
			node.parm( "root" ).pressButton()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 3, 0, 0 ) )
			node.parm( "root" ).set( "/1/2/3" )
			node.parm( "root" ).pressButton()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 6, 0, 0 ) )

		xform = self.xform()
		geo = self.geometry()
		testSimple( xform )
		testSimple( geo )
		
		self.writeSCC( rotation = IECore.V3d( 0, 0, IECore.degreesToRadians( -30 ) ) )
		
		def testRotated( node ) :
			
			node.parm( "reload" ).pressButton()
			node.parm( "root" ).set( "/" )
			node.parm( "root" ).pressButton()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			self.assertEqual( node.parmTransform().extractRotates(), hou.Vector3( 0, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, 0 ) ) )
			node.parm( "root" ).set( "/1" )
			node.parm( "root" ).pressButton()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 1, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -30 ) ) )
			node.parm( "root" ).set( "/1/2" )
			node.parm( "root" ).pressButton()
			self.failUnless( node.parmTransform().extractTranslates().isAlmostEqual( hou.Vector3( 2.73205, -1, 0 ) ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -60 ) ) )
			node.parm( "root" ).set( "/1/2/3" )
			node.parm( "root" ).pressButton()
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
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "root" ).set( "/1/2" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 12 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "root" ).set( "/1/2/3" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
	
	def testSopSpaceModes( self ) :
		
		self.writeSCC()
		node = self.sop()
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
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Path )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Local )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Object )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		
		node.parm( "root" ).set( "/1" )
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.World )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Path )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 5, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Local )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.SceneCacheNode.Space.Object )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
	
	def testSopShapeFilter( self ) :
		
		self.writeSCC()
		node = self.sop()
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 18 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1', '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "shapeFilter" ).set( "* ^2" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 12 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1','/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "shapeFilter" ).set( "3" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "shapeFilter" ).set( "" )
		self.assertEqual( len(node.geometry().prims()), 0 )
		self.assertEqual( len(node.geometry().primGroups()), 0 )
	
	def testSopAttributeFilter( self ) :
		
		self.writeSCC()
		node = self.sop()
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["Cd", "name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "P" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "* ^Cd" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "Cd" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["Cd", "name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["name"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
	
	def testBuildGeo( self ) :
		
		self.writeSCC()
		geo = self.geometry()
		self.assertEqual( geo.children(), tuple() )
		geo.parm( "build" ).pressButton()
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
		
		geo.parm( "root" ).set( "/1/2" )
		geo.parm( "build" ).pressButton()
		self.assertEqual( len(geo.children()), 1 )
		node = geo.children()[0]
		self.assertEqual( node.name(), "2" )
		node.parm( "root" ).set( "/1/2" )
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 12 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 6, 0, 0 ) )
	
	def cookAll( self, node ) :
		node.cook( force=True )
		for child in node.children() :
			self.cookAll( child )
	
	def testBuildSubNetwork( self ) :
		
		self.writeSCC()
		xform = self.xform()
		self.assertEqual( xform.children(), tuple() )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "build" ).pressButton()
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
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		
		xform.parm( "root" ).set( "/1/2" )
		xform.parm( "build" ).pressButton()
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
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		
		xform.parm( "root" ).set( "/1" )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "build" ).pressButton()
		self.assertEqual( len(xform.children()), 2 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( hou.node( xform.path()+"/geo" ), hou.ObjNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 1, 0, 0 ) )
		
		next = hou.node( xform.path()+"/2" )
		self.failUnless( isinstance( next, hou.ObjNode ) )
		self.assertEqual( len(next.children()), 1 )
		geo = hou.node( xform.path()+"/2/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		
		next.parm( "build" ).pressButton()
		self.assertEqual( len(next.children()), 2 )
		self.failUnless( isinstance( hou.node( xform.path()+"/2/3" ), hou.ObjNode ) )
		
		next = hou.node( xform.path()+"/2/3" )
		self.failUnless( isinstance( next, hou.ObjNode ) )
		self.assertEqual( len(next.children()), 1 )
		self.failUnless( isinstance( hou.node( xform.path()+"/2/3/geo" ), hou.ObjNode ) )
		next.parm( "build" ).pressButton()
		self.assertEqual( len(next.children()), 1 )
		self.failUnless( isinstance( hou.node( xform.path()+"/2/3/geo" ), hou.ObjNode ) )		
	
	def testBuildParenting( self ) :
		
		self.writeSCC()
		xform = self.xform()
		self.assertEqual( xform.children(), tuple() )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "build" ).pressButton()
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
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		
		xform.parm( "root" ).set( "/1/2" )
		xform.parm( "build" ).pressButton()
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
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		
		xform.parm( "root" ).set( "/1" )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "build" ).pressButton()
		self.assertEqual( len(xform.children()), 2 )
		geo = hou.node( xform.path()+"/geo" )
		self.failUnless( isinstance( geo, hou.ObjNode ) )
		self.failUnless( isinstance( geo.children()[0], hou.SopNode ) )
		self.cookAll( xform )
		node = geo.children()[0]
		prims = node.geometry().prims()
		self.assertEqual( len(prims), 6 )
		nameAttr = node.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ '/1' ] ) )
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
	
	def testBuildFlatGeometry( self ) :
		
		self.writeSCC()
		xform = self.xform()
		self.assertEqual( xform.children(), tuple() )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.FlatGeometry )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		xform.parm( "build" ).pressButton()
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
		xform.parm( "root" ).pressButton()
		xform.parm( "build" ).pressButton()
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
		self.assertEqual( nameAttr.strings(), tuple( [ '/1/2', '/1/2/3' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 6, 0, 0 ) )
		
		xform.parm( "root" ).set( "/1" )
		xform.parm( "root" ).pressButton()
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "build" ).pressButton()
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
		self.assertEqual( nameAttr.strings(), tuple( [ '/1' ] ) )
		for name in nameAttr.strings() :
			self.assertEqual( len([ x for x in prims if x.attribValue( "name" ) == name ]), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position() * geo.worldTransform(), hou.Vector3( 1, 0, 0 ) )
	
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
			
			mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( time, 1, 0 ) ] * 6 ) )
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
		
		sop = self.sop()
		for time in times :
			hou.setTime( time )
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
		
		xform = self.xform()
		xform.parm( "build" ).pressButton()
		a = xform.children()[0]
		b = [ x for x in a.children() if x.name() != "geo" ][0]
		c = [ x for x in b.children() if x.name() != "geo" ][0]
		for time in times :
			self.assertEqual( IECore.M44d( list(xform.worldTransformAtTime( time ).asTuple()) ), IECore.M44d() )
			self.assertEqual( IECore.M44d( list(a.worldTransformAtTime( time ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(b.worldTransformAtTime( time ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 2, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(c.worldTransformAtTime( time ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) ) )
		
		for time in times :
			hou.setTime( time )
			self.assertEqual( IECore.M44d( list(xform.parmTransform().asTuple()) ), IECore.M44d() )
			self.assertEqual( IECore.M44d( list(a.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(b.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 2, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(c.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) ) )
	
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "build" ).pressButton()
		a = xform.children()[0]
		b = xform.children()[1]
		c = xform.children()[2]
		for time in times :
			self.assertEqual( IECore.M44d( list(xform.worldTransformAtTime( time ).asTuple()) ), IECore.M44d() )
			self.assertEqual( IECore.M44d( list(a.worldTransformAtTime( time ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(b.worldTransformAtTime( time ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 3, 2*time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(c.worldTransformAtTime( time ).asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 6, 3*time, 0 ) ) )
	
		for time in times :
			hou.setTime( time )
			self.assertEqual( IECore.M44d( list(xform.parmTransform().asTuple()) ), IECore.M44d() )
			self.assertEqual( IECore.M44d( list(a.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 1, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(b.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 2, time, 0 ) ) )
			self.assertEqual( IECore.M44d( list(c.parmTransform().asTuple()) ), IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) ) )
	
	def compareScene( self, a, b, time = 0 ) :
		
		self.assertEqual( a.name(), b.name() )
		self.assertEqual( a.path(), b.path() )
		ab = a.readBound( time )
		bb = b.readBound( time )
		self.assertTrue( ab.min.equalWithAbsError( bb.min, 1e-6 ) )
		self.assertTrue( ab.max.equalWithAbsError( bb.max, 1e-6 ) )
		self.assertEqual( a.readTransformAsMatrix( time ), b.readTransformAsMatrix( time ) )
		self.assertEqual( a.hasObject(), b.hasObject() )
		if a.hasObject() :
			# need to remove the name added by Houdini
			mb = b.readObject( time )
			self.assertTrue( mb.isInstanceOf( IECore.TypeId.Renderable ) )
			del mb.blindData()['name']
			self.assertEqual( a.readObject( time ), mb )
		
		self.assertEqual( a.childNames(), b.childNames() )
		for child in a.childNames() :
			self.compareScene( a.child( child ), b.child( child ) )
	
	def testRop( self ) :
		
		# test a parented xform
		self.writeSCC()
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		xform.parm( "build" ).pressButton()
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
		xform.parm( "build" ).pressButton()
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output )
		
		# test a mixed subnet/parented xform
		os.remove( TestSceneCache.__testOutFile )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "build" ).pressButton()
		a = xform.children()[0]
		a.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.Parenting )
		a.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		a.parm( "build" ).pressButton()
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output )
		
		# test a flat xform
		os.remove( TestSceneCache.__testOutFile )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.FlatGeometry )
		xform.parm( "build" ).pressButton()
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output )
		
		# test a mixed subnet/flat xform
		os.remove( TestSceneCache.__testOutFile )
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "build" ).pressButton()
		a = xform.children()[0]
		a.parm( "build" ).pressButton()
		b = a.children()[1]
		b.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.FlatGeometry )
		b.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		b.parm( "build" ).pressButton()
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output )
		
		# test a OBJ Geo
		os.remove( TestSceneCache.__testOutFile )
		geo = self.geometry()
		geo.parm( "build" ).pressButton()
		rop.parm( "rootObject" ).set( geo.path() )
		rop.parm( "execute" ).pressButton()
		self.assertEqual( rop.errors(), "" )
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		output = IECore.SceneCache( TestSceneCache.__testOutFile, IECore.IndexedIO.OpenMode.Read )
		self.compareScene( orig, output )
	
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
		xform.parm( "build" ).pressButton()
		rop = self.rop( xform )
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
			self.compareScene( orig, output )
	
	def testLiveScene( self ) :
		
		self.writeSCC()
		xform = self.xform()
		xform.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.SubNetworks )
		xform.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.Children )
		xform.parm( "build" ).pressButton()
		a = xform.children()[0]
		a.parm( "build" ).pressButton()
		b = a.children()[1]
		b.parm( "hierarchy" ).set( IECoreHoudini.SceneCacheNode.Hierarchy.FlatGeometry )
		b.parm( "depth" ).set( IECoreHoudini.SceneCacheNode.Depth.AllDescendants )
		b.parm( "build" ).pressButton()
		orig = IECore.SceneCache( TestSceneCache.__testFile, IECore.IndexedIO.OpenMode.Read )
		live = IECoreHoudini.HoudiniScene( xform.path(), rootPath = [ xform.name() ] )
		self.compareScene( orig, live )
	
	def tearDown( self ) :
		
		for f in [ TestSceneCache.__testFile, TestSceneCache.__testOutFile ] :
			if os.path.exists( f ) :
				os.remove( f )
		
		IECoreHoudini.SceneCacheNode.clearCache()

if __name__ == "__main__":
    unittest.main()
