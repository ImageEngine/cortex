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

from __future__ import with_statement

import os
import maya.cmds

import IECore
import IECoreMaya

class FnSceneShapeTest( IECoreMaya.TestCase ) :

	__testFile = "test/test.scc"

	def setUp( self ) :

		scene = IECore.SceneCache( FnSceneShapeTest.__testFile, IECore.IndexedIO.OpenMode.Write )
		sc = scene.createChild( str(1) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 6 ) )
		sc.writeObject( mesh, 0.0 )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0.0 )

		sc = sc.createChild( "child" )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 1, 0 ) ] * 6 ) )
		sc.writeObject( mesh, 0.0 )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0.0 )

		sc = sc.createChild( str( 3 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		mesh["Cd"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 1 ) ] * 6 ) )
		sc.writeObject( mesh, 0.0 )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 3, 0, 0 ) )
		sc.writeTransform( IECore.M44dData( matrix ), 0.0 )

		return scene


	def testSceneInterface( self ) :

		maya.cmds.file( new=True, f=True )
		node = maya.cmds.createNode( "ieSceneShape" )
		maya.cmds.setAttr( node+'.file', FnSceneShapeTest.__testFile,type='string' )

		fn = IECoreMaya.FnSceneShape( node )

		# Check scene for a wrong path
		maya.cmds.setAttr( node+'.root', 'blabla', type='string' )
		scene = fn.sceneInterface()
		self.assertEqual( scene, None )

		maya.cmds.setAttr( node+'.root', '/', type='string' )
		scene = fn.sceneInterface()
		self.assertTrue( isinstance( scene, IECore.SceneCache ) )
		self.assertEqual( scene.childNames(), ['1'] )
		self.assertFalse( scene.hasObject() )

		maya.cmds.setAttr( node+'.root', '/1', type='string' )
		scene = fn.sceneInterface()
		self.assertTrue( isinstance( scene, IECore.SceneCache ) )
		self.assertEqual( scene.childNames(), ['child'] )
		self.assertTrue( scene.hasObject() )

	def testCreationName( self ) :

		maya.cmds.file( new=True, f=True )
		fn = IECoreMaya.FnSceneShape.create( "bob" )
		self.assertEqual( fn.fullPathName(), u"|bob|bobSceneShape" )

		fn = IECoreMaya.FnSceneShape.create( "bob1")
		self.assertEqual( fn.fullPathName(), u"|bob1|bobSceneShape1" )

		fn = IECoreMaya.FnSceneShape.create( "bob" )
		self.assertEqual( fn.fullPathName(), u"|bob2|bobSceneShape2" )

	def testCreationSetup( self ) :

		maya.cmds.file( new=True, f=True )
		fn = IECoreMaya.FnSceneShape.create( "test" )

		self.assertTrue( maya.cmds.sets( fn.fullPathName(), isMember="initialShadingGroup" ) )
		self.assertTrue( maya.cmds.getAttr( fn.fullPathName()+".objectOnly", l=True ) )
		self.assertFalse( maya.cmds.getAttr( fn.fullPathName()+".objectOnly" ) )
		self.assertTrue( maya.cmds.isConnected( "time1.outTime", fn.fullPathName()+".time" ) )

	def testExpandOnce( self ) :

		maya.cmds.file( new=True, f=True )
		fn = IECoreMaya.FnSceneShape.create( "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )

		result = fn.expandOnce()

		self.assertTrue( maya.cmds.getAttr( fn.fullPathName()+".objectOnly" ) )
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".queryPaths[0]" ), "/1" )

		self.assertTrue( len(result) == 1 )
		childFn = result[0]
		self.assertTrue( isinstance( childFn, IECoreMaya.FnSceneShape ) )
		self.assertEqual( childFn.fullPathName(), "|test|sceneShape_1|sceneShape_SceneShape1" )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".file" ), FnSceneShapeTest.__testFile )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".root" ), "/1" )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outTranslate", "|test|sceneShape_1.translate" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outRotate", "|test|sceneShape_1.rotate" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outScale", "|test|sceneShape_1.scale" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTime", childFn.fullPathName()+".time" ) )

		maya.cmds.setAttr( childFn.fullPathName()+".drawGeometry", 1 )
		result = childFn.expandOnce()

		self.assertTrue( maya.cmds.getAttr( childFn.fullPathName()+".objectOnly" ) )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".queryPaths[0]" ), "/child" )

		self.assertTrue( len(result) == 1 )
		self.assertTrue( isinstance( result[0], IECoreMaya.FnSceneShape ) )
		self.assertEqual( result[0].fullPathName(), "|test|sceneShape_1|child|childSceneShape" )
		self.assertEqual( maya.cmds.getAttr( result[0].fullPathName()+".file" ), FnSceneShapeTest.__testFile )
		self.assertEqual( maya.cmds.getAttr( result[0].fullPathName()+".root" ), "/1/child" )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outTranslate", "|test|sceneShape_1|child.translate" ) )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outRotate", "|test|sceneShape_1|child.rotate" ) )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outScale", "|test|sceneShape_1|child.scale" ) )
		self.assertEqual( maya.cmds.getAttr( result[0].fullPathName()+".drawGeometry"), 1 )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTime", result[0].fullPathName()+".time" ) )

	def testExpandOnceNamespace( self ) :

		maya.cmds.file( new=True, f=True )

		namespace = "INPUT"

		if not maya.cmds.namespace( exists=namespace ):
			maya.cmds.namespace( addNamespace=namespace )

		def addnamespace( path ):
			return path.replace( "|", "|" + namespace + ":" )

		fn = IECoreMaya.FnSceneShape.create( namespace + ":" + "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile, type='string' )

		result = fn.expandOnce( preserveNamespace=True )
		self.assertTrue( len(result) == 1 )

		childFn = result[ 0 ]
		self.assertTrue( isinstance( childFn, IECoreMaya.FnSceneShape ) )
		self.assertEqual( childFn.fullPathName(), addnamespace ( "|test|sceneShape_1|sceneShape_SceneShape1" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outTranslate", addnamespace ( "|test|sceneShape_1.translate" ) ) )

	def testExpandAll( self ) :

		maya.cmds.file( new=True, f=True )
		fn = IECoreMaya.FnSceneShape.create( "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )
		maya.cmds.setAttr( fn.fullPathName()+".drawGeometry", 1 )

		result = fn.expandAll()

		self.assertTrue( maya.cmds.getAttr( fn.fullPathName()+".objectOnly" ) )
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".queryPaths[0]" ), "/1" )

		self.assertTrue( len(result) == 3 )
		childFn = result[0]
		self.assertTrue( isinstance( childFn, IECoreMaya.FnSceneShape ) )
		self.assertEqual( childFn.fullPathName(), "|test|sceneShape_1|sceneShape_SceneShape1" )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".file" ), FnSceneShapeTest.__testFile )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".root" ), "/1" )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outTranslate", "|test|sceneShape_1.translate" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outRotate", "|test|sceneShape_1.rotate" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outScale", "|test|sceneShape_1.scale" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTime", childFn.fullPathName()+".time" ) )

		self.assertTrue( maya.cmds.getAttr( childFn.fullPathName()+".objectOnly" ) )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".queryPaths[0]" ), "/child" )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".drawGeometry"), 1 )

		self.assertTrue( isinstance( result[1], IECoreMaya.FnSceneShape ) )
		self.assertEqual( result[1].fullPathName(), "|test|sceneShape_1|child|childSceneShape" )
		self.assertEqual( maya.cmds.getAttr( result[1].fullPathName()+".file" ), FnSceneShapeTest.__testFile )
		self.assertEqual( maya.cmds.getAttr( result[1].fullPathName()+".root" ), "/1/child" )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outTranslate", "|test|sceneShape_1|child.translate" ) )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outRotate", "|test|sceneShape_1|child.rotate" ) )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outScale", "|test|sceneShape_1|child.scale" ) )
		self.assertEqual( maya.cmds.getAttr( result[1].fullPathName()+".drawGeometry"), 1 )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTime", result[1].fullPathName()+".time" ) )

	def testExpandAllNamespace( self ) :

		namespace = "INPUT"

		if not maya.cmds.namespace( exists=namespace ):
			maya.cmds.namespace( addNamespace=namespace )

		def addnamespace( path ):
			return path.replace( "|", "|" + namespace + ":" )

		maya.cmds.file( new=True, f=True )
		fn = IECoreMaya.FnSceneShape.create( namespace + ":" + "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )
		maya.cmds.setAttr( fn.fullPathName()+".drawGeometry", 1 )

		result = fn.expandAll( preserveNamespace=True )

		self.assertTrue( maya.cmds.getAttr( fn.fullPathName()+".objectOnly" ) )
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".queryPaths[0]" ), "/1" )

		self.assertTrue( len(result) == 3 )
		childFn = result[0]
		self.assertTrue( isinstance( childFn, IECoreMaya.FnSceneShape ) )
		self.assertEqual( childFn.fullPathName(), addnamespace( "|test|sceneShape_1|sceneShape_SceneShape1" ) )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".file" ), FnSceneShapeTest.__testFile )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".root" ), "/1" )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outTranslate", addnamespace( "|test|sceneShape_1.translate" ) ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outRotate", addnamespace( "|test|sceneShape_1.rotate" ) ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTransform[0].outScale", addnamespace( "|test|sceneShape_1.scale" ) ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".outTime", childFn.fullPathName()+".time" ) )

		self.assertTrue( maya.cmds.getAttr( childFn.fullPathName()+".objectOnly" ) )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".queryPaths[0]" ), "/child" )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".drawGeometry"), 1 )

		self.assertTrue( isinstance( result[1], IECoreMaya.FnSceneShape ) )
		self.assertEqual( result[1].fullPathName(), addnamespace( "|test|sceneShape_1|child|childSceneShape" ) )
		self.assertEqual( maya.cmds.getAttr( result[1].fullPathName()+".file" ), FnSceneShapeTest.__testFile )
		self.assertEqual( maya.cmds.getAttr( result[1].fullPathName()+".root" ), "/1/child" )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outTranslate", addnamespace( "|test|sceneShape_1|child.translate" ) ) )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outRotate", addnamespace( "|test|sceneShape_1|child.rotate" ) ) )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTransform[0].outScale", addnamespace( "|test|sceneShape_1|child.scale" ) ) )
		self.assertEqual( maya.cmds.getAttr( result[1].fullPathName()+".drawGeometry"), 1 )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".outTime", result[1].fullPathName()+".time" ) )

	def testCollapse( self ) :

		maya.cmds.file( new=True, f=True )
		fn = IECoreMaya.FnSceneShape.create( "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )

		result = fn.expandOnce()
		result[0].expandOnce()

		children = set( ["|test|testSceneShape", "|test|sceneShape_1", "|test|sceneShape_1|sceneShape_SceneShape1", "|test|sceneShape_1|child", "|test|sceneShape_1|child|childSceneShape"] )
		self.assertEqual( set(maya.cmds.listRelatives( "|test", ad=True, f=True )), children )

		fn.collapse()
		self.assertEqual( maya.cmds.listRelatives( "|test", ad=True, f=True ), ["|test|testSceneShape"] )

		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".objectOnly" ), 0 )
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".visibility" ), 1 )

	def testConvertAllToGeometry( self ):

		maya.cmds.file( new=True, f=True )
		fn = IECoreMaya.FnSceneShape.create( "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )

		fn.convertAllToGeometry()

		children = ["|test|testSceneShape", "|test|sceneShape_1"]
		self.assertEqual( maya.cmds.listRelatives( "|test", f=True ), children )
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".intermediateObject" ), 1 )

		children = ["|test|sceneShape_1|sceneShape_SceneShape1", "|test|sceneShape_1|child", "|test|sceneShape_1|sceneShape_Shape1"]
		self.assertEqual( maya.cmds.listRelatives( "|test|sceneShape_1", f=True ), children )
		self.assertEqual( maya.cmds.getAttr( "|test|sceneShape_1|sceneShape_SceneShape1.intermediateObject" ), 1 )
		self.assertEqual( maya.cmds.nodeType( "|test|sceneShape_1|sceneShape_Shape1" ), "mesh")

		self.assertEqual( maya.cmds.getAttr( "|test|sceneShape_1|sceneShape_SceneShape1.queryPaths[1]" ), "/" )
		self.assertTrue( maya.cmds.isConnected( "|test|sceneShape_1|sceneShape_SceneShape1.outObjects[1]", "|test|sceneShape_1|sceneShape_Shape1.inMesh" ) )

	def testComponentNames( self ):

		maya.cmds.file( new=True, f=True )
		fn = IECoreMaya.FnSceneShape.create( "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )
		maya.cmds.setAttr( fn.fullPathName()+".drawGeometry", 0 )
		self.assertEqual( fn.componentNames(), [] )

		maya.cmds.setAttr( fn.fullPathName()+".drawGeometry", 1 )
		self.assertEqual( fn.componentNames(), ['/', '/1', '/1/child', '/1/child/3'] )

		fn.selectComponentNames( ['/', '/1', '/1/child/3'] )
		self.assertEqual( fn.selectedComponentNames(), set( ['/', '/1', '/1/child/3'] ) )

	def testQuery( self ):

		maya.cmds.file( new=True, f=True )

		def createSceneFile():
		    scene = IECore.SceneCache( FnSceneShapeTest.__testFile, IECore.IndexedIO.OpenMode.Write )
		    sc = scene.createChild( str(1) )
		    curves = IECore.CurvesPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1))) # 6 curves.
		    sc.writeObject( curves, 0.0 )
		    matrix = IECore.M44d.createTranslated( IECore.V3d( 0, 0, 0 ) )
		    sc.writeTransform( IECore.M44dData( matrix ), 0.0 )

		createSceneFile()

		node = maya.cmds.createNode( "ieSceneShape" )
		maya.cmds.setAttr( node+'.file', FnSceneShapeTest.__testFile,type='string' )
		maya.cmds.setAttr( node+'.root', '/',type='string' )
		fn = IECoreMaya.FnSceneShape( node )

		self.assertEqual( maya.cmds.getAttr(fn.fullPathName()+".outObjects[0]", type=True), None )
		self.assertEqual( maya.cmds.getAttr(fn.fullPathName()+".outObjects[1]", type=True), None )

		maya.cmds.setAttr( fn.fullPathName()+".queryPaths[0]" , "/1", type="string")
		maya.cmds.setAttr( fn.fullPathName()+".queryPaths[1]" , "/1", type="string")

		maya.cmds.setAttr( fn.fullPathName()+".queryConvertParameters[0]", "-index 0", type="string" ) # Set it to output 0 th box curve.
		maya.cmds.setAttr( fn.fullPathName()+".queryConvertParameters[1]", "-index 1", type="string" ) # Set it to output 1 th box curve.

		self.assertEqual( maya.cmds.getAttr(fn.fullPathName()+".outObjects[0]", type=True), "nurbsCurve" )
		self.assertEqual( maya.cmds.getAttr(fn.fullPathName()+".outObjects[1]", type=True), "nurbsCurve" )

		curveShape0 = maya.cmds.createNode( "nurbsCurve" )
		curveShape1 = maya.cmds.createNode( "nurbsCurve" )
		maya.cmds.connectAttr( fn.fullPathName()+ ".outObjects[0]", curveShape0 + '.create' )
		maya.cmds.connectAttr( fn.fullPathName()+ ".outObjects[1]", curveShape1 + '.create' )

		self.assertNotEqual( maya.cmds.pointPosition(curveShape0 + '.cv[0]' ), maya.cmds.pointPosition(curveShape1 + '.cv[0]' ) )

		maya.cmds.setAttr( fn.fullPathName()+".queryConvertParameters[1]", "-index 0", type="string" )

		self.assertEqual( maya.cmds.pointPosition(curveShape0 + '.cv[0]' ), maya.cmds.pointPosition(curveShape1 + '.cv[0]' ) )

	def tearDown( self ) :

		if os.path.exists( FnSceneShapeTest.__testFile ) :
			os.remove( FnSceneShapeTest.__testFile )


if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
