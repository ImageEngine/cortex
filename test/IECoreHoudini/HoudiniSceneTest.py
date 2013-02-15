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

import hou

import IECore
import IECoreHoudini

class HoudiniSceneTest( IECoreHoudini.TestCase ) :
	
	def testFactory( self ) :
		
		scene = IECore.SceneInterface.create( "x.hip", IECore.IndexedIO.OpenMode.Read )
		self.failUnless( isinstance( scene, IECoreHoudini.HoudiniScene ) )
		self.assertEqual( scene.typeId(), IECoreHoudini.TypeId.HoudiniScene )
	
	def buildScene( self ) :
		
		obj = hou.node( "/obj" )
		sub1 = obj.createNode( "subnet", "sub1" )
		sub2 = obj.createNode( "subnet", "sub2" )
		box1 = sub1.createOutputNode( "geo", "box1", run_init_scripts=False )
		box1.createNode( "box", "actualBox" )
		box2 = obj.createNode( "geo", "box2", run_init_scripts=False )
		box2.createNode( "box", "actualBox" )
		torus1 = sub1.createNode( "geo", "torus1", run_init_scripts=False )
		torus1.createNode( "torus", "actualTorus" )
		torus2 = torus1.createOutputNode( "geo", "torus2", run_init_scripts=False )
		torus2.createNode( "torus", "actualTorus" )
		
		return IECoreHoudini.HoudiniScene()
	
	def testChildNames( self ) :
	
		scene = self.buildScene()
		self.assertEqual( sorted( scene.childNames() ), [ "box2", "sub1", "sub2" ] )
		
		child = scene.child( "sub1" )
		self.assertEqual( sorted( child.childNames() ), [ "box1", "torus1" ] )
		
		child2 = child.child( "torus1" )
		self.assertEqual( sorted( child2.childNames() ), [ "torus2" ] )
		
		child3 = child2.child( "torus2" )
		self.assertEqual( sorted( child3.childNames() ), [] )
		
		self.assertEqual( sorted( child.child( "box1" ).childNames() ), [] )
		self.assertEqual( sorted( scene.child( "box2" ).childNames() ), [] )
		self.assertEqual( sorted( scene.child( "sub2" ).childNames() ), [] )
	
	def testHasChild( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.hasChild( "box2" ), True )
		self.assertEqual( scene.hasChild( "sub1" ), True )
		self.assertEqual( scene.hasChild( "sub2" ), True )
		self.assertEqual( scene.hasChild( "box1" ), False )
		self.assertEqual( scene.hasChild( "fake" ), False )
		
		child = scene.child( "sub1" )
		self.assertEqual( child.hasChild( "torus1" ), True )
		self.assertEqual( child.hasChild( "torus2" ), False )
		self.assertEqual( child.child( "torus1" ).hasChild( "torus2" ), True )
		self.assertEqual( child.hasChild( "fake" ), False )
	
	def testNames( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.name(), "/" )
		self.assertEqual( scene.child( "box2" ).name(), "box2" )
		self.assertEqual( scene.child( "sub2" ).name(), "sub2" )
		
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.name(), "sub1" )
		self.assertEqual( sub1.child( "box1" ).name(), "box1" )
		
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.name(), "torus1" )
		self.assertEqual( torus1.child( "torus2" ).name(), "torus2" )
	
	def testPaths( self ) :
		
		scene = self.buildScene()
		self.assertEqual( scene.path(), [] )
		self.assertEqual( scene.pathAsString(), "/" )
		self.assertEqual( scene.child( "box2" ).path(), [ "box2" ] )
		self.assertEqual( scene.child( "box2" ).pathAsString(), "/box2" )
		self.assertEqual( scene.child( "sub2" ).path(), [ "sub2" ] )
		self.assertEqual( scene.child( "sub2" ).pathAsString(), "/sub2" )
		
		sub1 = scene.child( "sub1" )
		self.assertEqual( sub1.path(), [ "sub1" ] )
		self.assertEqual( sub1.pathAsString(), "/sub1" )
		self.assertEqual( sub1.child( "box1" ).path(), [ "sub1", "box1" ] )
		self.assertEqual( sub1.child( "box1" ).pathAsString(), "/sub1/box1" )
		
		torus1 = sub1.child( "torus1" )
		self.assertEqual( torus1.path(), [ "sub1", "torus1" ] )
		self.assertEqual( torus1.pathAsString(), "/sub1/torus1" )
		self.assertEqual( torus1.child( "torus2" ).path(), [ "sub1", "torus1", "torus2" ] )
		self.assertEqual( torus1.child( "torus2" ).pathAsString(), "/sub1/torus1/torus2" )
		
		## \todo: add a harder test. 3 connections at the same level, and 3 one level deep
		
		self.assertRaises( RuntimeError, scene.child, "idontexist" )
		self.assertEqual( scene.child( "idontexist", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		
		# test the node that does exist according to houdini, but is actually a grandchild in our world
		self.assertRaises( RuntimeError, scene.child, "box1" )
		self.assertEqual( scene.child( "box1", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )		
		
	def testSceneMethod( self ) :
		
		scene = self.buildScene()
		
		self.assertEqual( scene.scene( [ "sub2" ] ).pathAsString(), "/sub2" )
		
		torus1 = scene.scene( [ "sub1", "torus1" ] )
		self.assertEqual( torus1.name(), "torus1" )
		self.assertEqual( torus1.path(), [ "sub1", "torus1" ] )
		self.assertEqual( torus1.pathAsString(), "/sub1/torus1" )
		self.assertEqual( torus1.hasChild( "torus2" ), True )
		self.assertEqual( torus1.childNames(), [ "torus2" ] )
		
		# does it still return absolute paths if we've gone to another location?
		sub1 = scene.scene( [ "sub1" ] )
		self.assertEqual( sub1.scene( [] ).name(), "/" )
		self.assertEqual( sub1.scene( [ "sub1", "torus1", "torus2" ] ).pathAsString(), "/sub1/torus1/torus2" )
		self.assertEqual( sub1.scene( [ "box2" ] ).pathAsString(), "/box2" )
		
		self.assertRaises( RuntimeError, scene.scene, [ "idontexist" ] )
		self.assertEqual( scene.scene( [ "idontexist" ], IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		
		# test the node that does exist according to houdini, but is actually a grandchild in our world
		self.assertRaises( RuntimeError, scene.scene, [ "box1" ] )
		self.assertEqual( scene.scene( [ "box1" ], IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		
if __name__ == "__main__":
	IECoreHoudini.TestProgram()
