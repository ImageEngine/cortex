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
	__testPlugAttrFile = "test/testPlugAttr.scc"

	def setUp( self ) :

		maya.cmds.file( new=True, f=True )

	def writeSCC( self, file, rotation=IECore.V3d( 0, 0, 0 ), time=0 ) :
		
		scene = IECore.SceneCache( file, IECore.IndexedIO.OpenMode.Write )
		sc = scene.createChild( str( 1 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		sc.writeObject( mesh, time )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		sc.writeTransform( IECore.M44dData( matrix ), time )
		
		sc = sc.createChild( str( 2 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
		sc.writeObject( mesh, time )
		matrix = IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) )
		matrix = matrix.rotate( rotation )
		sc.writeTransform( IECore.M44dData( matrix ), time )
		
		sc = sc.createChild( str( 3 ) )
		mesh = IECore.MeshPrimitive.createBox(IECore.Box3f(IECore.V3f(0),IECore.V3f(1)))
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

			sc2.writeObject( mesh, time )
			matrix = IECore.M44d.createTranslated( IECore.V3d( 2, time, 0 ) )
			sc2.writeTransform( IECore.M44dData( matrix ), time )
			
			matrix = IECore.M44d.createTranslated( IECore.V3d( 3, time, 0 ) )
			sc3.writeTransform( IECore.M44dData( matrix ), time )

		return scene

	def writeAttributeSCC( self, file ) :
		
		scene = self.writeSCC( file )
		sc1 = scene.child( str( 1 ) )
		sc2 = sc1.child( str( 2 ) )
		sc3 = sc2.child( str( 3 ) )

		sc1.writeAttribute( "boolAttr", IECore.BoolData( True ), 0.0 )
		sc1.writeAttribute( "floatAttr", IECore.FloatData( 5.20 ), 0.0 )
		
		sc2.writeAttribute( "boolAttr", IECore.BoolData( False ), 0.0 )
		sc2.writeAttribute( "floatAttr", IECore.FloatData( 2.0 ), 0.0 )

		sc3.writeAttribute( "intAttr", IECore.IntData( 12 ), 0.0 )
		sc3.writeAttribute( "strAttr", IECore.StringData( "blah" ), 0.0 )
		sc3.writeAttribute( "doubleAttr", IECore.DoubleData( 1.2 ), 0.0 )
		
		return scene
	
	def writeTagSCC( self, file ) :
		
		scene = self.writeSCC( file )
		sc1 = scene.child( str( 1 ) )
		sc2 = sc1.child( str( 2 ) )
		sc3 = sc2.child( str( 3 ) )
		sc1.writeTags( [ "a" ] )
		sc2.writeTags( [ "b" ] )
		sc3.writeTags( [ "c" ] )
		
		return scene
	
	def testComputePlugs( self ) :
			
		self.writeSCC( file = SceneShapeTest.__testFile )
		
		maya.cmds.file( new=True, f=True )
		node = maya.cmds.createNode( 'ieSceneShape' )
		maya.cmds.setAttr( node+'.file', SceneShapeTest.__testFile,type='string' )
		maya.cmds.setAttr( node+'.root',"/",type='string' )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outBound", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outObjects", mi=True ), None)
		
		maya.cmds.setAttr( node+".queryPaths[0]", "/1", type="string")
		maya.cmds.setAttr( node+".queryPaths[1]", "/1/2", type="string")
		maya.cmds.setAttr( node+".queryPaths[2]", "/1/2/3", type="string")
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outBound", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outObjects", mi=True ), None)
		
		# Check only the plugs we trigger get computed
		maya.cmds.getAttr( node+".outTransform[0].outTranslate" )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform", mi=True ), [0])
		self.assertEqual( maya.cmds.getAttr( node+".outBound", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outObjects", mi=True ), None)
		
		maya.cmds.getAttr( node+".outTransform[2].outTranslate" )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform", mi=True ), [0, 2])
		self.assertEqual( maya.cmds.getAttr( node+".outBound", mi=True ), None)
		self.assertEqual( maya.cmds.getAttr( node+".outObjects", mi=True ), None)
		
		maya.cmds.getAttr( node+".outTransform[1].outTranslate" )
		maya.cmds.getAttr( node+".outBound[1].outBoundCenter" )
		
		mesh = maya.cmds.createNode("mesh")
		maya.cmds.connectAttr( node+'.outObjects[2]', mesh+".inMesh" )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform", mi=True ), [0, 1, 2])
		self.assertEqual( maya.cmds.getAttr( node+".outBound", mi=True ), [1])
		self.assertEqual( maya.cmds.getAttr( node+".outObjects", mi=True ), [2])
		
		maya.cmds.setAttr( node+".queryPaths[3]", "/", type="string");
		self.assertEqual( maya.cmds.getAttr( node+".outTransform", mi=True ), [0, 1, 2])
		self.assertEqual( maya.cmds.getAttr( node+".outBound", mi=True ), [1])
		self.assertEqual( maya.cmds.getAttr( node+".outObjects", mi=True ), [2])


	def testPlugValues( self ) :
		
		self.writeSCC( file=SceneShapeTest.__testPlugFile, rotation = IECore.V3d( 0, 0, IECore.degreesToRadians( -30 ) ) )
		maya.cmds.file( new=True, f=True )

		node = maya.cmds.createNode( 'ieSceneShape' )
		maya.cmds.setAttr( node+'.file', SceneShapeTest.__testPlugFile,type='string' )
		maya.cmds.setAttr( node+'.root',"/",type='string' )

		maya.cmds.setAttr( node+".queryPaths[0]", "/1", type="string")
		maya.cmds.setAttr( node+".queryPaths[1]", "/1/2", type="string")
		maya.cmds.setAttr( node+".queryPaths[2]", "/1/2/3", type="string")

		# World space
		maya.cmds.setAttr( node+".querySpace", 0)
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate.outTranslateX"), 2.732050895 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate.outTranslateY"), -1.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate.outTranslateZ"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateZ")), -60.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate.outTranslateX"), 4.232050895 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate.outTranslateY"), -3.598076105 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate.outTranslateZ"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateZ")), -90.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )

		# Local space
		maya.cmds.setAttr( node+".querySpace", 1)
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate"), [(2.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate"), [(3.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )

		# Change the root path
		maya.cmds.setAttr( node+'.root', "/1",type='string' )
		
		maya.cmds.setAttr( node+".queryPaths[0]", "/", type="string")
		maya.cmds.setAttr( node+".queryPaths[1]", "/2", type="string")
		maya.cmds.setAttr( node+".queryPaths[2]", "/2/3", type="string")
		
		# World space
		maya.cmds.setAttr( node+".querySpace", 0)
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(0.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateZ")), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate.outTranslateX"), 2.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate.outTranslateY"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate.outTranslateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate.outTranslateX"), 4.5980763 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate.outTranslateY"), -1.5 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate.outTranslateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateZ")), -60.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )

		# Local space
		maya.cmds.setAttr( node+".querySpace", 1)
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[0].outRotate.outRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate"), [(2.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[1].outRotate.outRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate"), [(3.0, 0.0, 0.0)] )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateX"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateY"), 0.0 )
		self.assertEqual( round(maya.cmds.getAttr( node+".outTransform[2].outRotate.outRotateZ")), -30.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.setAttr( node+'.time', 5 )
		self.assertEqual( maya.cmds.getAttr( node+".outTime" ), 5 )
		
		
	def testAnimPlugValues( self ) :
		
		self.writeAnimSCC( file=SceneShapeTest.__testPlugAnimFile )

		maya.cmds.file( new=True, f=True )
		node = maya.cmds.createNode( 'ieSceneShape' )
		maya.cmds.connectAttr( "time1.outTime", node+".time" )
		maya.cmds.setAttr( node+'.file', SceneShapeTest.__testPlugAnimFile,type='string' )

		maya.cmds.setAttr( node+'.root',"/",type='string' )
		
		maya.cmds.setAttr( node+".queryPaths[0]", "/1", type="string")
		maya.cmds.setAttr( node+".queryPaths[1]", "/1/2", type="string")
		maya.cmds.setAttr( node+".queryPaths[2]", "/1/2/3", type="string")

		maya.cmds.currentTime( 0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTime" ), 0 )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate"), [(3.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate"), [(6.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.currentTime( 48 )
		self.assertEqual( maya.cmds.getAttr( node+".outTime" ), 48 )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(1.0, 2.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate"), [(3.0, 4.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate"), [(6.0, 6.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.currentTime( 60 )
		self.assertEqual( maya.cmds.getAttr( node+".outTime" ), 60 )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(1.0, 2.5, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate"), [(3.0, 5.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate"), [(6.0, 7.5, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.currentTime( 0 )
		maya.cmds.setAttr( node+".querySpace", 1)
		self.assertEqual( maya.cmds.getAttr( node+".outTime" ), 0 )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(1.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate"), [(2.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate"), [(3.0, 0.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )
		
		maya.cmds.currentTime( 48 )
		self.assertEqual( maya.cmds.getAttr( node+".outTime" ), 48 )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outTranslate"), [(1.0, 2.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[0].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[0].outScale"), [(1.0, 1.0, 1.0)] )

		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outTranslate"), [(2.0, 2.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[1].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[1].outScale"), [(1.0, 1.0, 1.0)] )
		
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outTranslate"), [(3.0, 2.0, 0.0)] )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateX"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateY"), 0.0 )
		self.assertAlmostEqual( maya.cmds.getAttr( node+".outTransform[2].outRotateZ"), 0.0 )
		self.assertEqual( maya.cmds.getAttr( node+".outTransform[2].outScale"), [(1.0, 1.0, 1.0)] )
	
	
	def testqueryAttributes( self ) :
		
		self.writeAttributeSCC( file=SceneShapeTest.__testPlugAttrFile )

		maya.cmds.file( new=True, f=True )
		node = maya.cmds.createNode( 'ieSceneShape' )
		maya.cmds.setAttr( node+'.file', SceneShapeTest.__testPlugAttrFile,type='string' )
		
		maya.cmds.setAttr( node+".queryPaths[0]", "/1", type="string")
		maya.cmds.setAttr( node+".queryPaths[1]", "/1/2", type="string")
		maya.cmds.setAttr( node+".queryPaths[2]", "/1/2/3", type="string")
		
		maya.cmds.setAttr( node+".queryAttributes[0]", "boolAttr", type="string")
		maya.cmds.setAttr( node+".queryAttributes[1]", "floatAttr", type="string")
		maya.cmds.setAttr( node+".queryAttributes[2]", "intAttr", type="string")
		maya.cmds.setAttr( node+".queryAttributes[3]", "strAttr", type="string")
		maya.cmds.setAttr( node+".queryAttributes[4]", "blablAttr", type="string")
		maya.cmds.setAttr( node+".queryAttributes[5]", "doubleAttr", type="string")

		self.assertEqual( maya.cmds.getAttr( node+".attributes[0].attributeValues[0]" ), True )
		self.assertEqual( round(maya.cmds.getAttr( node+".attributes[0].attributeValues[1]"), 6 ), 5.2 )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[0].attributeValues[2]" ), None )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[0].attributeValues[3]" ), None )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[0].attributeValues[4]" ), None )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[0].attributeValues[5]" ), None )
		
		self.assertEqual( maya.cmds.getAttr( node+".attributes[1].attributeValues[0]" ), False )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[1].attributeValues[1]" ), 2.0 )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[1].attributeValues[2]" ), None )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[1].attributeValues[3]" ), None )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[1].attributeValues[4]" ), None )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[1].attributeValues[5]" ), None )
		
		self.assertEqual( maya.cmds.getAttr( node+".attributes[2].attributeValues[0]" ), None )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[2].attributeValues[1]" ), None )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[2].attributeValues[2]" ), 12 )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[2].attributeValues[3]" ), "blah" )
		self.assertEqual( maya.cmds.getAttr( node+".attributes[2].attributeValues[4]" ), None )
		self.assertEqual( round(maya.cmds.getAttr( node+".attributes[2].attributeValues[5]" ), 6), 1.2 )

	def testTags( self ) :
		
		self.writeTagSCC( file=SceneShapeTest.__testFile )
		
		maya.cmds.file( new=True, f=True )
		node = maya.cmds.createNode( 'ieSceneShape' )
		fn = IECoreMaya.FnSceneShape( node )
		transform = str(maya.cmds.listRelatives( node, parent=True )[0])
		maya.cmds.setAttr( node+'.file', SceneShapeTest.__testFile, type='string' )
		
		scene = IECoreMaya.MayaScene().child( transform )
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.TagFilter.EveryTag ) ]), [ "ObjectType:MeshPrimitive", "a", "b", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags() ]), [] )
		for tag in scene.readTags(IECore.SceneInterface.TagFilter.EveryTag) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.TagFilter.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.TagFilter.EveryTag ) )
		
		# double expanding because the first level has all the same tags
		childFn = fn.expandOnce()[0].expandOnce()[0]
		scene = childFn.sceneInterface()
		self.assertEqual( set([ str(x) for x in scene.readTags( IECore.SceneInterface.TagFilter.DescendantTag|IECore.SceneInterface.TagFilter.LocalTag ) ]), set([ "ObjectType:MeshPrimitive", "b", "c" ]) )
		self.assertEqual( sorted([ str(x) for x in scene.readTags() ]), [ "ObjectType:MeshPrimitive", "b" ] )
		for tag in scene.readTags(IECore.SceneInterface.TagFilter.EveryTag) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.TagFilter.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.TagFilter.EveryTag ) )
		
		childFn = childFn.expandOnce()[0]
		scene = childFn.sceneInterface()
		self.assertEqual( sorted([ str(x) for x in scene.readTags( IECore.SceneInterface.TagFilter.DescendantTag|IECore.SceneInterface.TagFilter.LocalTag ) ]), [ "ObjectType:MeshPrimitive", "c" ] )
		self.assertEqual( sorted([ str(x) for x in scene.readTags() ]), [ "ObjectType:MeshPrimitive", "c" ] )
		for tag in scene.readTags(IECore.SceneInterface.TagFilter.EveryTag) :
			self.assertTrue( scene.hasTag( tag, IECore.SceneInterface.TagFilter.EveryTag ) )
		self.assertFalse( scene.hasTag( "fakeTag", IECore.SceneInterface.TagFilter.EveryTag ) )
	
	def tearDown( self ) :
		
		for f in [ SceneShapeTest.__testFile, SceneShapeTest.__testPlugFile, SceneShapeTest.__testPlugAnimFile, SceneShapeTest.__testPlugAttrFile ] :
			if os.path.exists( f ) :
				os.remove( f )

			

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
