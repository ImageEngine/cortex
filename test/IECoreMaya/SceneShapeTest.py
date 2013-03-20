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
import IECore
import IECoreMaya
import maya.cmds
import unittest

class SceneShapeTest( IECoreMaya.TestCase ) :
	
	__testFile = "test/test.scc"
	__testPlugFile = "test/testPlug.scc"
	__testPlugAnimFile = "test/testPlugAnim.scc"

	def writeSCC( self, file, rotation=IECore.V3d( 0, 0, 0 ), time=0 ) :
		
		scene = IECore.SceneCache( file, IECore.IndexedIO.OpenMode.Write )
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
	
	def writeAnimSCC( self, file ) :
		
		scene = self.writeSCC( file )
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
	

	def testComputePlugs( self ) :
			
		self.writeSCC( file = SceneShapeTest.__testFile )
		
		maya.cmds.file( new=True, f=True )
		node = maya.cmds.createNode( 'ieSceneShape' )
		maya.cmds.setAttr( node+'.sceneFile', SceneShapeTest.__testFile,type='string' )
		maya.cmds.setAttr( node+'.sceneRoot',"/",type='string' )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".objectBound", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outputObjects", mi=True ), None)
		
		maya.cmds.setAttr( node+".sceneQueries[0]", "/1", type="string")
		maya.cmds.setAttr( node+".sceneQueries[1]", "/1/2", type="string")
		maya.cmds.setAttr( node+".sceneQueries[2]", "/1/2/3", type="string")
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".objectBound", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outputObjects", mi=True ), None)
		
		# Check only the plugs we trigger get computed
		maya.cmds.getAttr( node+".objectTransform[0].objectTranslate" )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform", mi=True ), [0])
		self.assertEqual( maya.cmds.getAttr( node+".objectBound", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outputObjects", mi=True ), None)
		
		maya.cmds.getAttr( node+".objectTransform[2].objectTranslate" )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform", mi=True ), [0, 2])
		self.assertEqual( maya.cmds.getAttr( node+".objectBound", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outputObjects", mi=True ), None)
		
		maya.cmds.getAttr( node+".objectTransform[1].objectTranslate" )
		maya.cmds.getAttr( node+".objectBound[1].objectBoundCenter" )
		
		mesh = maya.cmds.createNode("mesh")
		maya.cmds.connectAttr( node+'.outputObjects[2]', mesh+".inMesh" )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform", mi=True ), [0, 1, 2])
		self.assertEqual( maya.cmds.getAttr( node+".objectBound", mi=True ), [1])
		self.assertEqual( maya.cmds.getAttr( node+".outputObjects", mi=True ), [2])
		
		maya.cmds.setAttr( node+".sceneQueries[3]", "/", type="string");
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform", mi=True ), [0, 1, 2])
		self.assertEqual( maya.cmds.getAttr( node+".objectBound", mi=True ), [1])
		self.assertEqual( maya.cmds.getAttr( node+".outputObjects", mi=True ), [2])


	def testPlugValues( self ) :
		
		self.writeSCC( file=SceneShapeTest.__testPlugFile, rotation = IECore.V3d( 0, 0, IECore.degreesToRadians( -30 ) ) )
		maya.cmds.file( new=True, f=True )

		node = maya.cmds.createNode( 'ieSceneShape' )
		maya.cmds.setAttr( node+'.sceneFile', SceneShapeTest.__testPlugFile,type='string' )
		maya.cmds.setAttr( node+'.sceneRoot',"/",type='string' )

		maya.cmds.setAttr( node+".sceneQueries[0]", "/1", type="string")
		maya.cmds.setAttr( node+".sceneQueries[1]", "/1/2", type="string")
		maya.cmds.setAttr( node+".sceneQueries[2]", "/1/2/3", type="string")

		# World space
		maya.cmds.setAttr( node+".querySpace", 0)
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate.objectTranslateX"), 2.732050895 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate.objectTranslateY"), -1.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate.objectTranslateZ"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateZ")), -60.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate.objectTranslateX"), 4.232050895 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate.objectTranslateY"), -3.598076105 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate.objectTranslateZ"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateZ")), -90.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )

		# Local space
		maya.cmds.setAttr( node+".querySpace", 1)
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate"), [(2.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate"), [(3.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )

		# Change the root path
		maya.cmds.setAttr( node+'.sceneRoot', "/1",type='string' )
		
		maya.cmds.setAttr( node+".sceneQueries[0]", "/", type="string")
		maya.cmds.setAttr( node+".sceneQueries[1]", "/2", type="string")
		maya.cmds.setAttr( node+".sceneQueries[2]", "/2/3", type="string")
		
		# World space
		maya.cmds.setAttr( node+".querySpace", 0)
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(0.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateZ")), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate.objectTranslateX"), 2.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate.objectTranslateY"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate.objectTranslateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate.objectTranslateX"), 4.5980763 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate.objectTranslateY"), -1.5 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate.objectTranslateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateZ")), -60.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )

		# Local space
		maya.cmds.setAttr( node+".querySpace", 1)
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[0].objectRotate.objectRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate"), [(2.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[1].objectRotate.objectRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate"), [(3.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".objectTransform[2].objectRotate.objectRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )
		
		
	def testAnimPlugValues( self ) :
		
		self.writeAnimSCC( file=SceneShapeTest.__testPlugAnimFile )

		maya.cmds.file( new=True, f=True )
		node = maya.cmds.createNode( 'ieSceneShape' )
		maya.cmds.connectAttr( "time1.outTime", node+".time" )
		maya.cmds.setAttr( node+'.sceneFile', SceneShapeTest.__testPlugAnimFile,type='string' )

		maya.cmds.setAttr( node+'.sceneRoot',"/",type='string' )
		
		maya.cmds.setAttr( node+".sceneQueries[0]", "/1", type="string")
		maya.cmds.setAttr( node+".sceneQueries[1]", "/1/2", type="string")
		maya.cmds.setAttr( node+".sceneQueries[2]", "/1/2/3", type="string")

		maya.cmds.currentTime( 0 )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate"), [(3.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate"), [(6.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.currentTime( 48 )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(1.0, 2.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate"), [(3.0, 4.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate"), [(6.0, 6.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.currentTime( 60 )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(1.0, 2.5, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate"), [(3.0, 5.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate"), [(6.0, 7.5, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.currentTime( 0 )
		maya.cmds.setAttr( node+".querySpace", 1)

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate"), [(2.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate"), [(3.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.currentTime( 48 )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectTranslate"), [(1.0, 2.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[0].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[0].objectScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectTranslate"), [(2.0, 2.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[1].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[1].objectScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectTranslate"), [(3.0, 2.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".objectTransform[2].objectRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".objectTransform[2].objectScale"), [(1.0, 1.0, 1.0)] )

	def tearDown( self ) :
		
		for f in [ SceneShapeTest.__testFile, SceneShapeTest.__testPlugFile, SceneShapeTest.__testPlugAnimFile ] :
			if os.path.exists( f ) :
				os.remove( f )

			

if __name__ == "__main__":
    unittest.main()
