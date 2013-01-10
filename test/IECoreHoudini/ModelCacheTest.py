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

class TestModelCache( IECoreHoudini.TestCase ) :
	
	__testFile = "test/test.mdc"
	
	def sop( self, parent=None ) :
		if not parent :
			parent = hou.node( "/obj" ).createNode( "geo", run_init_scripts=False )
		sop = parent.createNode( "ieModelCacheSource" )
		sop.parm( "file" ).set( TestModelCache.__testFile )
		return sop
	
	def xform( self, parent=None ) :
		if not parent :
			parent = hou.node( "/obj" )
		xform = parent.createNode( "ieModelCacheTransform" )
		xform.parm( "file" ).set( TestModelCache.__testFile )
		return xform
	
	def geometry( self, parent=None ) :
		if not parent :
			parent = hou.node( "/obj" )
		geometry = parent.createNode( "ieModelCacheGeometry" )
		geometry.parm( "file" ).set( TestModelCache.__testFile )
		return geometry
	
	def writeMDC( self, rotation=IECore.V3d( 0, 0, 0 ) ) :
		
		m = IECore.ModelCache( TestModelCache.__testFile, IECore.IndexedIOOpenMode.Write )
		
		mc = m.writableChild( str( 1 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 6 ) )
		mc.writeObject( mesh )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		mc.writeTransform( matrix )
		
		mc = mc.writableChild( str( 2 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 1, 0 ) ] * 6 ) )
		mc.writeObject( mesh )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		mc.writeTransform( matrix )
		
		mc = mc.writableChild( str( 3 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 1 ) ] * 6 ) )
		mc.writeObject( mesh )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 3, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		mc.writeTransform( matrix )
	
	def testBadFile( self ) :
		
		def testNode( node ) :
			
			node.parm( "file" ).set( "" )
			self.assertRaises( hou.OperationFailed, IECore.curry( node.cook, True ) )
			self.failUnless( node.errors() )		
			node.parm( "file" ).set( "/tmp/fake" )
			self.assertRaises( hou.OperationFailed, IECore.curry( node.cook, True ) )
			self.failUnless( node.errors() )
			node.parm( "file" ).set( TestModelCache.__testFile )
			self.assertRaises( hou.OperationFailed, IECore.curry( node.cook, True ) )
			self.failUnless( node.errors() )
			self.writeMDC()
			node.cook( force=True )
			self.failUnless( not node.errors() )
		
		testNode( self.sop() )
		os.remove( TestModelCache.__testFile )
		testNode( self.xform() )
		os.remove( TestModelCache.__testFile )
		testNode( self.geometry() )
	
	def testObjSubPaths( self ) :
		
		self.writeMDC()
		
		def testSimple( node ) :
			
			node.parm( "reload" ).pressButton()
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			node.parm( "root" ).set( "/1" )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 1, 0, 0 ) )
			node.parm( "root" ).set( "/1/2" )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 3, 0, 0 ) )
			node.parm( "root" ).set( "/1/2/3" )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 6, 0, 0 ) )

		xform = self.xform()
		geo = self.geometry()
		testSimple( xform )
		testSimple( geo )
		
		self.writeMDC( rotation = IECore.V3d( 0, 0, IECore.degreesToRadians( -30 ) ) )
		
		def testRotated( node ) :
			
			node.parm( "reload" ).pressButton()
			node.parm( "root" ).set( "/" )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			self.assertEqual( node.parmTransform().extractRotates(), hou.Vector3( 0, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, 0 ) ) )
			node.parm( "root" ).set( "/1" )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 1, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -30 ) ) )
			node.parm( "root" ).set( "/1/2" )
			node.cook()
			self.failUnless( node.parmTransform().extractTranslates().isAlmostEqual( hou.Vector3( 2.73205, -1, 0 ) ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -60 ) ) )
			node.parm( "root" ).set( "/1/2/3" )
			node.cook()
			self.failUnless( node.parmTransform().extractTranslates().isAlmostEqual( hou.Vector3( 4.23205, -3.59808, 0 ) ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -90 ) ) )
		
		testRotated( xform )
		testRotated( geo )
	
	def testObjSpaceModes( self ) :
		
		self.writeMDC()
		
		def testSimple( node ) :
			
			node.parm( "reload" ).pressButton()
			node.parm( "root" ).set( "/1/2" )
			self.assertEqual( node.parm( "space" ).eval(), IECoreHoudini.ModelCacheNode.Space.World )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 3, 0, 0 ) )
			node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Path )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Leaf )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 2, 0, 0 ) )
			node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Object )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
		
		xform = self.xform()
		geo = self.geometry()
		testSimple( xform )
		testSimple( geo )
		
		self.writeMDC( rotation = IECore.V3d( 0, 0, IECore.degreesToRadians( -30 ) ) )
		
		def testRotated( node ) :
			
			node.parm( "reload" ).pressButton()
			node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.World )
			node.cook()
			self.failUnless( node.parmTransform().extractTranslates().isAlmostEqual( hou.Vector3( 2.73205, -1, 0 ) ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -60 ) ) )
			node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Path )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, 0 ) ) )
			node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Leaf )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 2, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, -30 ) ) )
			node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Object )
			node.cook()
			self.assertEqual( node.parmTransform().extractTranslates(), hou.Vector3( 0, 0, 0 ) )
			self.failUnless( node.parmTransform().extractRotates().isAlmostEqual( hou.Vector3( 0, 0, 0 ) ) )
		
		testRotated( xform )
		testRotated( geo )
	
	def testSopSubPaths( self ) :
		
		self.writeMDC()
		node = self.sop()
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "root" ).set( "/1" )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "root" ).set( "/1/2" )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 12 )
		self.assertEqual( len(primGroups), 2 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "root" ).set( "/1/2/3" )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 6 )
		self.assertEqual( len(primGroups), 1 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
	
	def testSopSpaceModes( self ) :
		
		self.writeMDC()
		node = self.sop()
		self.assertEqual( node.parm( "space" ).eval(), IECoreHoudini.ModelCacheNode.Space.World )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Path )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Leaf )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Object )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		
		node.parm( "root" ).set( "/1" )
		node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.World )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Path )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 5, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Leaf )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 2, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		
		node.parm( "space" ).set( IECoreHoudini.ModelCacheNode.Space.Object )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 0, 0, 0 ) )
	
	def testSopShapeFilter( self ) :
		
		self.writeMDC()
		node = self.sop()
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 18 )
		self.assertEqual( len(primGroups), 3 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 3, 0, 0 ) )
		self.assertEqual( prims[12].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "shapeFilter" ).set( "* ^2" )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 12 )
		self.assertEqual( len(primGroups), 2 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1', '_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 1, 0, 0 ) )
		self.assertEqual( prims[6].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "shapeFilter" ).set( "3" )
		prims = node.geometry().prims()
		primGroups = node.geometry().primGroups()
		self.assertEqual( len(prims), 6 )
		self.assertEqual( len(primGroups), 1 )
		self.assertEqual( sorted( [ x.name() for x in primGroups ] ), ['_1_2_3'] )
		for group in primGroups :
			self.assertEqual( len(group.prims()), 6 )
		self.assertEqual( prims[0].vertex( 0 ).point().position(), hou.Vector3( 6, 0, 0 ) )
		
		node.parm( "shapeFilter" ).set( "" )
		self.assertEqual( len(node.geometry().prims()), 0 )
		self.assertEqual( len(node.geometry().primGroups()), 0 )
	
	def testSopAttributeFilter( self ) :
		
		self.writeMDC()
		node = self.sop()
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["Cd"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "P" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( node.geometry().primAttribs(), tuple() )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "* ^Cd" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( node.geometry().primAttribs(), tuple() )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "Cd" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().primAttribs() ] ), ["Cd"] )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
		
		node.parm( "attributeFilter" ).set( "" )
		self.assertEqual( len(node.geometry().prims()), 18 )
		self.assertEqual( sorted( [ x.name() for x in node.geometry().pointAttribs() ] ), ["P", "Pw"] )
		self.assertEqual( node.geometry().primAttribs(), tuple() )
		self.assertEqual( node.geometry().vertexAttribs(), tuple() )
		self.assertEqual( node.geometry().globalAttribs(), tuple() )
	
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
		
		self.writeMDC()
		testNode( self.sop() )
		testNode( self.xform() )
		testNode( self.geometry() )
	
	def tearDown( self ) :
		if os.path.exists( TestModelCache.__testFile ) :
			os.remove( TestModelCache.__testFile )

if __name__ == "__main__":
    unittest.main()

