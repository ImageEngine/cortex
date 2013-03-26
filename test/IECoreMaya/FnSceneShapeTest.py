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

	def writeSCC( self ) :
		
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

		self.writeSCC()
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
	
		fn = IECoreMaya.FnSceneShape.create( "bob" )
		self.assertEqual( fn.fullPathName(), u"|bob|bobShape" )

		fn = IECoreMaya.FnSceneShape.create( "bob1")
		self.assertEqual( fn.fullPathName(), u"|bob1|bobShape1" )

		fn = IECoreMaya.FnSceneShape.create( "bob" )
		self.assertEqual( fn.fullPathName(), u"|bob2|bobShape2" )
	
	def testCreationSetup( self ) :
		
		fn = IECoreMaya.FnSceneShape.create( "test" )
		
		self.assertTrue( maya.cmds.sets( fn.fullPathName(), isMember="initialShadingGroup" ) )
		self.assertTrue( maya.cmds.getAttr( fn.fullPathName()+".objectOnly", l=True ) )
		self.assertFalse( maya.cmds.getAttr( fn.fullPathName()+".objectOnly" ) )
		self.assertTrue( maya.cmds.isConnected( "time1.outTime", fn.fullPathName()+".time" ) )

	def testExpandScene( self ) :
		
		self.writeSCC()
		fn = IECoreMaya.FnSceneShape.create( "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )
		
		result = fn.expandScene()
		
		self.assertTrue( maya.cmds.getAttr( fn.fullPathName()+".objectOnly" ) )
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".queryPaths[0]" ), "/1" )
		
		self.assertTrue( len(result) == 1 )
		childFn = result[0]
		self.assertTrue( isinstance( childFn, IECoreMaya.FnSceneShape ) )
		self.assertEqual( childFn.fullPathName(), "|test|sceneShape_1|sceneShape_Shape1" )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".file" ), FnSceneShapeTest.__testFile )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".root" ), "/1" )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".objectTransform[0].objectTranslate", "|test|sceneShape_1.translate" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".objectTransform[0].objectRotate", "|test|sceneShape_1.rotate" ) )
		self.assertTrue( maya.cmds.isConnected( fn.fullPathName()+".objectTransform[0].objectScale", "|test|sceneShape_1.scale" ) )
		
		maya.cmds.setAttr( childFn.fullPathName()+".drawGeometry", 1 )
		result = childFn.expandScene()
		
		self.assertTrue( maya.cmds.getAttr( childFn.fullPathName()+".objectOnly" ) )
		self.assertEqual( maya.cmds.getAttr( childFn.fullPathName()+".queryPaths[0]" ), "/child" )
		
		self.assertTrue( len(result) == 1 )
		self.assertTrue( isinstance( result[0], IECoreMaya.FnSceneShape ) )
		self.assertEqual( result[0].fullPathName(), "|test|sceneShape_1|child|childShape" )
		self.assertEqual( maya.cmds.getAttr( result[0].fullPathName()+".file" ), FnSceneShapeTest.__testFile )
		self.assertEqual( maya.cmds.getAttr( result[0].fullPathName()+".root" ), "/1/child" )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".objectTransform[0].objectTranslate", "|test|sceneShape_1|child.translate" ) )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".objectTransform[0].objectRotate", "|test|sceneShape_1|child.rotate" ) )
		self.assertTrue( maya.cmds.isConnected( childFn.fullPathName()+".objectTransform[0].objectScale", "|test|sceneShape_1|child.scale" ) )
		self.assertEqual( maya.cmds.getAttr( result[0].fullPathName()+".drawGeometry"), 1 )
		
		
	def testCollapseScene( self ) :
		
		self.writeSCC()
		fn = IECoreMaya.FnSceneShape.create( "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )
		
		result = fn.expandScene()
		result[0].expandScene()
		
		children = set( ["|test|testShape", "|test|sceneShape_1", "|test|sceneShape_1|sceneShape_Shape1", "|test|sceneShape_1|child", "|test|sceneShape_1|child|childShape"] )
		self.assertEqual( set(maya.cmds.listRelatives( "|test", ad=True, f=True )), children )
		
		fn.collapseScene()
		self.assertEqual( maya.cmds.listRelatives( "|test", ad=True, f=True ), ["|test|testShape"] )
		
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".objectOnly" ), 0 )
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".visibility" ), 1 )
	
	def testConvertToGeometry( self ):
		
		self.writeSCC()
		fn = IECoreMaya.FnSceneShape.create( "test" )
		maya.cmds.setAttr( fn.fullPathName()+'.file', FnSceneShapeTest.__testFile,type='string' )
		
		fn.convertToGeometry()
		
		children = set( ["|test|testShape", "|test|sceneShape_1"] )
		self.assertEqual( set(maya.cmds.listRelatives( "|test", f=True )), children )
		self.assertEqual( maya.cmds.getAttr( fn.fullPathName()+".visibility" ), 0 )
		
		children = set( ["|test|sceneShape_1|sceneShape_Shape1", "|test|sceneShape_1|child", "|test|sceneShape_1|sceneShape_1_mesh"] )
		self.assertEqual( set(maya.cmds.listRelatives( "|test|sceneShape_1", f=True )), children )
		self.assertEqual( maya.cmds.getAttr( "|test|sceneShape_1|sceneShape_Shape1.visibility" ), 0 )
		self.assertEqual( maya.cmds.nodeType( "|test|sceneShape_1|sceneShape_1_mesh" ), "mesh")
		
		self.assertEqual( maya.cmds.getAttr( "|test|sceneShape_1|sceneShape_Shape1.queryPaths[1]" ), "/" )
		self.assertTrue( maya.cmds.isConnected( "|test|sceneShape_1|sceneShape_Shape1.outputObjects[1]", "|test|sceneShape_1|sceneShape_1_mesh.inMesh" ) )
		
		

	
	def tearDown( self ) :
		
		if os.path.exists( FnSceneShapeTest.__testFile ) :
			os.remove( FnSceneShapeTest.__testFile )
		
		
if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
