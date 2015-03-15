##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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
import unittest
from IECore import *

class TestFileExaminer( unittest.TestCase ) :

	def testNoExaminer( self ) :

		self.assertEqual( FileExaminer.create( "noExtension" ), None )
		self.assertEqual( FileExaminer.create( "extensionNot.registered" ), None )
		self.assertEqual( FileExaminer.allDependencies( "noExtension" ), set() )
		self.assertEqual( FileExaminer.allDependencies( "extensionNot.registered" ), set() )

	def testNuke( self ) :

		e = FileExaminer.create( "test/IECore/data/nukeScripts/dependencies.nk" )

		expectedDependencies = [
			"/tmp/test.####.tif 3-10",
			"/film/grain/scans/AreDependenciesToo.####.tif 2-75",
			"/tmp/filesInAGroupMustBeDetected.#.exr 1",
			"/tmp/testProxy.####.tif 3-10",
			"/tmp/testNoPadding.#.tif 1-101",
			"/tmp/testNoFrameNumber.tif",
			"/tmp/animatedGeo.####.obj 1-101",
			"/tmp/test/geoNoAnimation.obj",
		]

		d = e.dependencies()

		self.assertEqual( len( expectedDependencies ), len( d ) )
		for ed in expectedDependencies :
			self.assert_( ed in d )

	def testNukeWithSpacesInFileNames( self ) :
	
		e = FileExaminer.create( "test/IECore/data/nukeScripts/dependenciesWithSpaces.nk" )
		
		d = e.dependencies()
		
		expectedDependencies = [
			"/tmp/thisFileHas Some Spaces.####.exr 1-10",
			"/tmp/thisFileHas Some SpacesToo.####.exr 1-10",
		]
				
		self.assertEqual( len( expectedDependencies ), len( d ) )
		for ed in expectedDependencies :
			self.assert_( ed in d )

	def testRIB( self ) :

		e = FileExaminer.create( "test/IECore/data/ribFiles/dependencies.rib" )

		expectedDependencies = [
			"aShader.sdl",
			"/a/texture/file.0001.tdl",
			"hello.tdl",
			"aPythonProcedural",
		]

		d = e.dependencies()

		self.assertEqual( len( expectedDependencies ), len( d ) )
		for ed in expectedDependencies :
			self.assert_( ed in d )

if __name__ == "__main__":
	unittest.main()
