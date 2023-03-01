##########################################################################
#
#  Copyright (c) 2022, Image Engine Design Inc. All rights reserved.
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

import unittest

import nuke

import IECore
import IECoreNuke

class LiveSceneKnobTest( IECoreNuke.TestCase ) :

	def testNameAndLabel( self ) :

		n = nuke.createNode( "ieLiveScene" )

		k = n.knob( "scene" )

		self.assertEqual( k.name(), "scene" )
		self.assertEqual( k.label(), "Scene" )

	def testEmptyScene( self ) :
		import imath

		n = nuke.createNode( "ieLiveScene" )
		liveScene = n.knob( "scene" ).getValue()

		self.assertEqual( liveScene.childNames(), [] )
		self.assertEqual( liveScene.readTransform( 0 ).value.transform, imath.M44d() )
		self.assertEqual( liveScene.readBound( 0 ), imath.Box3d() )
		self.assertTrue( isinstance( liveScene.readObject( 0 ), IECore.NullObject ) )

	def testAccessors( self ) :

		n = nuke.createNode( "ieLiveScene" )

		k = n.knob( "scene" )

		self.assertTrue( isinstance( k, IECoreNuke.LiveSceneKnob ) )

		self.assertTrue( isinstance( k.getValue(), IECoreNuke.LiveScene ) )

	def testQueryScene( self ) :

		sph = nuke.createNode( "Sphere" )
		cub = nuke.createNode( "Cube" )
		sn = nuke.createNode( "Scene")
		sn.setInput( 0, sph )
		sn.setInput( 1, cub )
		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sn )

		k = n.knob( "scene" )

		self.assertTrue( isinstance( k, IECoreNuke.LiveSceneKnob ) )

		self.assertTrue( isinstance( k.getValue(), IECoreNuke.LiveScene ) )

	def testNameAndPath( self ):
		import IECoreScene
		sph = nuke.createNode( "Sphere" )
		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sph )

		liveScene = n.knob( "scene" ).getValue()
		
		self.assertEqual( liveScene.name(), "/" )
		self.assertEqual( liveScene.path(), [] )

		sceneFile = "test/IECoreNuke/scripts/animatedSpheres.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		n.setInput( 0, sceneReader )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/A/a', '/root/B/b'] )

		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		self.assertEqual( liveScene.name(), expectedScene.name() )
		self.assertEqual( liveScene.path(), expectedScene.path() )

		sceneA = liveScene.scene( ["A"] )
		expectedSceneA = expectedScene.scene( ["A"] )

		self.assertEqual( sceneA.name(), expectedSceneA.name() )
		self.assertEqual( sceneA.path(), expectedSceneA.path() )

		sceneBb = liveScene.scene( ["B", "b"] )
		expectedSceneBb = expectedScene.scene( ["B", "b"] )

		self.assertEqual( sceneBb.name(), expectedSceneBb.name() )
		self.assertEqual( sceneBb.path(), expectedSceneBb.path() )

	def testChildNames( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/liveSceneData.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/A/a', '/root/B/b'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()

		self.assertEqual( sorted( liveScene.childNames() ) , sorted( expectedScene.childNames() ) )

		self.assertTrue( liveScene.hasChild( "B" ) )
		self.assertFalse( liveScene.hasChild( "wrongChild" ) )

		self.assertRaises( RuntimeError, liveScene.child, "wrongChild" )
		self.assertFalse( liveScene.child( "wrongChild", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ) )
		self.assertRaises( RuntimeError, liveScene.child, "wrongChild", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

		for subPath in ( ["B"], ["A"] ):
			subScene = liveScene.scene( subPath )
			subExpectedScene = expectedScene.scene( subPath )

			self.assertEqual( subScene.childNames(), subExpectedScene.childNames() )

		self.assertRaises( RuntimeError, liveScene.scene, ["B", "wrongChild"] )
		self.assertFalse( liveScene.scene( ["B", "wrongChild"], IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ) )
		self.assertRaises( RuntimeError, liveScene.scene, ["B", "wrongChild"], IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

	def assertAlmostEqualBound( self, box1, box2 ):
		for dim in range( 3 ):
			self.assertAlmostEqual( box1.size()[dim], box2.size()[dim], 4 )

	def testBounds( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/liveSceneData.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/A/a', '/root/B/b'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()
		for time in (0, 1, 2, 3):
			liveSceneBound = liveScene.readBound( time )
			expectedSceneBound = expectedScene.readBound( time )
			self.assertAlmostEqualBound( liveSceneBound, expectedSceneBound )

		for subPath in ( ["B"], ["A"] ):
			subScene = liveScene.scene( subPath )
			subExpectedScene = expectedScene.scene( subPath )

			self.assertEqual( subScene.readBound(0), subExpectedScene.readBound(0) )

	def testDuplicatedParent( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/duplicateParent.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/groupA/cube','/root/groupA/sphere', '/root/groupB/sphere'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()
		self.assertEqual( set( liveScene.childNames() ), set( expectedScene.childNames() ) )

	def testFilterDifferentParent( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/duplicateParent.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/groupA/cube','/root/groupA/sphere', '/root/groupB/sphere'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()

		for subPath in ( ["groupA"], ["groupB"] ):
			subScene = liveScene.scene( subPath )
			subExpectedScene = expectedScene.scene( subPath )

			self.assertCountEqual( subScene.childNames(), subExpectedScene.childNames() )
		
	def testAnimatedBounds( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/animatedTransform.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/group/cube'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()
		for time in (0, 1, 2):
			liveSceneBound = liveScene.readBound( time )
			expectedSceneBound = expectedScene.readBound( time )
			self.assertAlmostEqualBound( liveSceneBound, expectedSceneBound )

			for subPath in ( ["group"], ["group", "cube"] ):
				subScene = liveScene.scene( subPath )
				subExpectedScene = expectedScene.scene( subPath )

				self.assertEqual( subScene.readBound(0), subExpectedScene.readBound(0) )

	def assertAlmostEqualTransform( self, tran1, tran2 ):
		for x in range( 4 ):
			for y in range( 4 ):
				self.assertAlmostEqual( tran1[x][y], tran2[x][y], 4 )

	def _checkNukeSceneTransform( self, liveScene, expectedScene, matrix, time ):
		import imath
		# combine the matrix of each parent as the SceneCacheReader does
		matrix = expectedScene.readTransform( time ).value * matrix
		if liveScene.childNames():
			# each parent location have an identity matrix
			self.assertEqual( liveScene.readTransform( time ).value.transform, imath.M44d() )
			for childName in liveScene.childNames():
				self._checkNukeSceneTransform( liveScene.child( childName ), expectedScene.child( childName ), matrix, time )
		else:
			# check the leaf matrix is the same as the combined parents' matrix
			self.assertAlmostEqualTransform( liveScene.readTransform( time ).value.transform, matrix )

	def testTransform( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/liveSceneData.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/A/a', '/root/B/b'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()
		for time in (0, 1, 2, 3):
			liveSceneTransform = liveScene.readTransform( time )
			expectedSceneTransform = expectedScene.readTransform( time )
			self.assertEqual( liveSceneTransform.value.transform, expectedSceneTransform.value )

		# check the leaf has the world matrix
		for childName in liveScene.childNames():
			# root transform
			matrix = expectedScene.readTransform( 0 ).value
			self._checkNukeSceneTransform( liveScene.child( childName ), expectedScene.child( childName ), matrix, 0 )

	def testAnimatedTransform( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/animatedTransform.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/group/cube'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()
		for time in (0, 1, 2):
			liveSceneTransform = liveScene.readTransform( time )
			expectedSceneTransform = expectedScene.readTransform( time )
			self.assertEqual( liveSceneTransform.value.transform, expectedSceneTransform.value )

			# check the leaf has the world matrix
			for childName in liveScene.childNames():
				# root transform
				matrix = expectedScene.readTransform( 0 ).value
				self._checkNukeSceneTransform( liveScene.child( childName ), expectedScene.child( childName ), matrix, 0 )

	def testHasObject( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/liveSceneData.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/A/a', '/root/B/b'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()
		self.assertFalse( liveScene.hasObject() )

		for subPath in ( ["A"], ["B"] ):
			self.assertFalse( liveScene.scene( subPath ).hasObject() )

		for subPath in ( ["A", "a"], ["B", "b"] ):
			self.assertTrue( liveScene.scene( subPath ).hasObject() )

	def testReadObjet( self ):
		import IECoreScene

		sceneFile = "test/IECoreNuke/scripts/data/liveSceneData.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/A/a', '/root/B/b'] )

		n = nuke.createNode( "ieLiveScene" )
		n.setInput( 0, sceneReader )

		liveScene = n.knob( "scene" ).getValue()
		objectA = liveScene.scene( [ "A", "a" ] ).readObject( 0 )
		expectedObjectA = expectedScene.scene( [ "A", "a" ] ).readObject( 0 )

		self.assertEqual( objectA.topologyHash(), expectedObjectA.topologyHash() )
		self.assertEqual( objectA.keys(), [ "P", "uv" ] )


if __name__ == "__main__":
	unittest.main()

