##########################################################################
#
#  Copyright (c) 2009-2013, Image Engine Design Inc. All rights reserved.
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
import os.path
import os
import time
import IECore
import IECoreImage
import IECoreRI

class PythonProceduralTest( IECoreRI.TestCase ) :

	def test( self ) :

		rib = """
		Option "searchpath" "string procedural" "./src/rmanProcedurals/python"

		Display "test/IECoreRI/output/testPythonProcedural.tif" "tiff" "rgba"

		Projection "perspective" "float fov" [ 40 ]

		WorldBegin

			Translate 0 -1.5 10
			Rotate 270 1 0 0

			Procedural "DynamicLoad" [ "python" "IECoreRI.Renderer().geometry( 'ri:teapot', {}, {} )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]

		WorldEnd
		"""

		ribFile = open( "test/IECoreRI/output/pythonProcedural.rib", "w" )
		ribFile.write( rib )
		ribFile.close()

		os.system( "renderdl test/IECoreRI/output/pythonProcedural.rib" )

		imageCreated = IECore.Reader.create( "test/IECoreRI/output/testPythonProcedural.tif" ).read()
		expectedImage = IECore.Reader.create( "test/IECoreRI/data/testPythonProcedural.tif" ).read()

		self.assertEqual( IECoreImage.ImageDiffOp()( imageA=imageCreated, imageB=expectedImage, maxError=0.01 ), IECore.BoolData( False ) )

	def testThreading( self ) :

		## Checks that we're doing the right basic things in the python procedural to allow multiple threads to
		# share access to the interpreter. This is deliberately only executing pure python (and not cortex) code
		# to test only the python procedural itself - cortex issues will be dealt with separately.

		rib = """
		Option "searchpath" "string procedural" "./src/rmanProcedurals/python"

		Display "test/IECoreRI/output/testPythonProcedural.tif" "tiff" "rgba"

		Projection "perspective" "float fov" [ 40 ]

		WorldBegin

			Translate 0 -1.5 10
			Rotate 270 1 0 0

			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 100000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 100000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 1000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 1000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 100 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 1000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 1000000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 1000000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 100 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 1000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 1000000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 1000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "l = list( range( 0, 10000 ) )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]

		WorldEnd
		"""

		ribFile = open( "test/IECoreRI/output/pythonProcedural.rib", "w" )
		ribFile.write( rib )
		ribFile.close()

		# give it a bunch of chances to go wrong
		for i in range( 0, 20 ) :

			os.system( "renderdl test/IECoreRI/output/pythonProcedural.rib" )

	def testThreadingGains( self ) :

		## Checks that there is actually some potential benefit to allowing python procedurals
		# to run in multiple threads. This test is entirely artificial. It tries to simulate a bunch
		# of procedurals which block waiting for io and therefore give the others a chance at using
		# the python interpreter during that interval.

		rib = """
		Option "searchpath" "string procedural" "./src/rmanProcedurals/python"

		Display "test/IECoreRI/output/testPythonProcedural.tif" "tiff" "rgba"

		Projection "perspective" "float fov" [ 40 ]

		WorldBegin

			Translate 0 -1.5 10
			Rotate 270 1 0 0

			Attribute "procedural" "integer reentrant" [ %d ]

			Procedural "DynamicLoad" [ "python" "import time; time.sleep( 2 )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]
			Procedural "DynamicLoad" [ "python" "import time; time.sleep( 2 )" ] [ -3.75 3.75 -3.75 3.75 -3.75 3.75 ]

		WorldEnd
		"""

		# first try running with multiple concurrent procedurals

		ribFile = open( "test/IECoreRI/output/pythonProcedural.rib", "w" )
		ribFile.write( rib % 1 )
		ribFile.close()

		tStart = time.time()
		os.system( "renderdl test/IECoreRI/output/pythonProcedural.rib" )
		threadedTime = time.time() - tStart

		# and then with serialised procedurals

		ribFile = open( "test/IECoreRI/output/pythonProcedural.rib", "w" )
		ribFile.write( rib % 0 )
		ribFile.close()

		tStart = time.time()
		os.system( "renderdl test/IECoreRI/output/pythonProcedural.rib" )
		nonThreadedTime = time.time() - tStart

		self.assert_( threadedTime < nonThreadedTime ) # might fail on single core machines or machines under other load

if __name__ == "__main__":
    unittest.main()
