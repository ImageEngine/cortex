##########################################################################
#
#  Copyright (c) 2019, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
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
import functools
import os

import IECoreScene

class SharedSceneInterfacesTest( unittest.TestCase ) :

	def setUp( self ) :

		maxScenes = IECoreScene.SharedSceneInterfaces.getMaxScenes()
		self.addCleanup( functools.partial( IECoreScene.SharedSceneInterfaces.setMaxScenes ), maxScenes )

	def testLimits( self ) :

		IECoreScene.SharedSceneInterfaces.clear()
		self.assertEqual( IECoreScene.SharedSceneInterfaces.numScenes(), 0 )

		# Get more files than there is room for in the cache.

		files = [
			os.path.join( "test", "IECore", "data", "sccFiles", "animatedSpheres.scc" ),
			os.path.join( "test", "IECore", "data", "sccFiles", "attributeAtRoot.scc" ),
			os.path.join( "test", "IECore", "data", "sccFiles", "cube_v6.scc" ),
		]

		IECoreScene.SharedSceneInterfaces.setMaxScenes( len( files ) - 1 )

		scenes = set()
		for f in files :
			scenes.add( IECoreScene.SharedSceneInterfaces.get( f ) )

		# Check that the cache limit wasn't exceeded.

		self.assertEqual( len( scenes ), len( files ) )
		self.assertEqual( IECoreScene.SharedSceneInterfaces.numScenes(), IECoreScene.SharedSceneInterfaces.getMaxScenes() )

		# Get all files again. This should result in being given at
		# least one new interface, due to cache evictions.

		for f in files :
			scenes.add( IECoreScene.SharedSceneInterfaces.get( f ) )

		self.assertGreater( len( scenes ), len( files ) )

if __name__ == "__main__":
	unittest.main()
