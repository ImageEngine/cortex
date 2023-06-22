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
import os
import shutil
import tempfile

import nuke

import imath

import IECore
import IECoreScene
import IECoreNuke

class SceneCacheWriterTest( IECoreNuke.TestCase ) :

	def setUp( self ) :
		self.__temporaryDirectory = None

	def tearDown( self ) :
		if self.__temporaryDirectory is not None :
			shutil.rmtree( self.__temporaryDirectory )

	def temporaryDirectory( self ) :

		if self.__temporaryDirectory is None :
			self.__temporaryDirectory = tempfile.mkdtemp( prefix = "ieCoreNukeTest" )

		return self.__temporaryDirectory

	def testWriteEmptyScene( self ):

		outputFile = os.path.join( self.temporaryDirectory(), "empty.scc" )

		writer = nuke.createNode( "WriteGeo" )
		writer["file"].fromScript( outputFile )

		nuke.execute( writer, 1001, 1001 )

		self.assertTrue( os.path.exists( outputFile ) )

		scene = IECoreScene.SharedSceneInterfaces.get( outputFile )
		self.assertEqual( scene.childNames(), [] )

	def testWriteSimpleSphere( self ):

		outputFile = os.path.join( self.temporaryDirectory(), "sphere.scc" )

		sphere = nuke.createNode( "Sphere" )
		writer = nuke.createNode( "WriteGeo" )
		writer["file"].fromScript( outputFile )

		nuke.execute( writer, 1001, 1001 )

		self.assertTrue( os.path.exists( outputFile ) )

		scene = IECoreScene.SharedSceneInterfaces.get( outputFile )

		self.assertEqual( scene.childNames(), ["object0"] )
		self.assertEqual( scene.readTransform( 0 ).value, imath.M44d() )

		liveSceneHolder = nuke.createNode( "ieLiveScene" )
		liveSceneHolder.setInput( 0, sphere )
		
		liveScene = liveSceneHolder["scene"].getValue()

		liveSceneMesh = liveScene.scene( ["object0"] ).readObject( 0 )
		mesh = scene.scene( ["object0"] ).readObject( 0 )

		self.assertEqual( mesh.topologyHash(), liveSceneMesh.topologyHash() )

	def testWriteSceneCacheReader( self ):
		import random
		import IECoreScene

		outputFile = os.path.join( self.temporaryDirectory(), "scene.scc" )

		sceneFile = "test/IECoreNuke/scripts/data/liveSceneData.scc"
		sceneReader = nuke.createNode( "ieSceneCacheReader" )
		sceneReader.knob( "file" ).setValue( sceneFile )
		expectedScene = IECoreScene.SharedSceneInterfaces.get( sceneFile )

		sceneReader.forceValidate()
		widget = sceneReader.knob( "sceneView" )
		widget.setSelectedItems( ['/root/A/a', '/root/B/b'] )

		writer = nuke.createNode( "WriteGeo" )
		writer["file"].fromScript( outputFile )

		nuke.execute( writer, 1, 48 )

		scene = IECoreScene.SharedSceneInterfaces.get( outputFile )

		self.assertEqual( scene.childNames(), expectedScene.childNames() )
		for time in range( 0, 3 ):
			self.assertAlmostEqual( scene.readBound( time ).min(), expectedScene.readBound( time ).min() )
			mesh = scene.scene( ["B", "b"] ).readObject( time )
			expectedMesh = expectedScene.scene( ["B", "b"] ).readObject( time )
			random.seed( 12 )
			for i in range( 12 ):
				pointIndex = random.choice( range( len( mesh["P"].data ) ) )
				self.assertAlmostEqual( mesh["P"].data[pointIndex], expectedMesh["P"].data[pointIndex], 4 )

	def testWriteParticle( self ):

		outputFile = os.path.join( self.temporaryDirectory(), "particle.scc" )

		noise = nuke.createNode( "Noise")
		card = nuke.createNode( "Card2" )
		card.setInput( 0, noise )
		particle = nuke.createNode( "ParticleEmitter" )
		particle.setInput( 1, card )
		particle["size_variation"].setValue( 2 )
		particle["color_from_texture"].setValue( True )
		particle["spread"].setValue( .3 )
		writer = nuke.createNode( "WriteGeo" )
		writer["file"].fromScript( outputFile )

		nuke.execute( writer, 0, 24 )

		self.assertTrue( os.path.exists( outputFile ) )

		scene = IECoreScene.SharedSceneInterfaces.get( outputFile )

		self.assertEqual( scene.childNames(), ["object0"] )

		pointsPrim = scene.scene( ["object0",] ).readObject( 1 )
		self.assertEqual( set( pointsPrim.keys() ), set( ["Cs", "P", "pid", "width", "velocity", "alpha"] ) )

		self.assertEqual( pointsPrim.numPoints, 100 )
		self.assertEqual( scene.scene( ["object0",] ).readObject( 0.04 ).numPoints, 10 )
		self.assertEqual( scene.scene( ["object0",] ).readObject( 0.5 ).numPoints, 100 )

		self.assertAlmostEqual( pointsPrim["P"].data[12], imath.V3f(-0.559, 1.797, 1.677), delta=.015 )
		self.assertAlmostEqual( pointsPrim["Cs"].data[21], imath.Color4f(0.241325, 0.241325, 0.241325, 1), delta=.015 )
		self.assertAlmostEqual( pointsPrim["alpha"].data[72], 1.0, delta=.015 )
		self.assertAlmostEqual( pointsPrim["width"].data[99], .105, delta=.015 )
		self.assertAlmostEqual( pointsPrim["pid"].data[92], 197, delta=.015 )
		self.assertAlmostEqual( pointsPrim["velocity"].data[72], imath.V3f(-18.424, 4.602, 14.675), delta=.015 )

	def assertAlmostEqual( self, left, right, delta=None ):
		if isinstance( left, ( imath.V3f, imath.Color3f, imath.Color4f ) ):
			for index, _ in enumerate( left ):
				super( SceneCacheWriterTest, self ).assertAlmostEqual( left[index], right[index], delta=delta )
		else:
			super( SceneCacheWriterTest, self ).assertAlmostEqual( left, right, delta=delta )

if __name__ == "__main__":
	unittest.main()


