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


if __name__ == "__main__":
	unittest.main()


