##########################################################################
#
#  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

import math
import unittest
import IECoreNuke
import IECore
import nuke
import os
import random

## \todo:
## Add dtex and rat files to this test and compare them against the deep exr.

## \todo:
## Fix this test. It currently is failing becuase the call to deepSample() isn't returning the value of the deep pixel.
## When testing this inside nuke, it appears that a correct result is given on occasion and in the other cases, the
## cache needs to be refreshed before the deepSample returns a correct result. This may be due to a bug in nuke
## or an error in the IECoreNuke::DeepImageReader code. Further investigation is required.
class DeepImageReaderTest( IECoreNuke.TestCase ) :
	
	__smokeWithoutZBack = "test/IECoreRI/data/exr/smokeWithoutZBack.dexr"
	__output = "test/IECoreRI/data/exr/smokeWithZBack.exr"

	def __inputPaths( self ) :	
		return { "exr" : "test/IECoreRI/data/exr/primitives.exr", "shw" : "test/IECoreRI/data/shw/primitives.shw" }

	def testCreationOfZBack( self ) :

		if not IECore.withDeepEXR :
			return
				
		# Create a DeepReader to read the deep EXR.
		reader = nuke.createNode( "DeepRead" )
		reader["file"].setText( DeepImageReaderTest.__smokeWithoutZBack )

		# Write it back out. We do this because nuke's DeepSample node is un
		writer = nuke.createNode( "DeepWrite" )
		writer["file"].setText( DeepImageReaderTest.__output )
		nuke.execute( writer, 1, 1 )
		
		# Read the image back in and check the values of it's ZBack channel.
		reader = IECore.EXRDeepImageReader( DeepImageReaderTest.__output )
		header = reader.readHeader()
		resolution = reader['dataWindow'].size() + IECore.V2i( 1 )
			
		self.assertEqual( set( header['channelNames'] ), set( [ 'R', 'G', 'B', 'A', 'ZBack' ] ) )

		for y in range( 0, resolution[0] ) :
			for x in range( 0, resolution[1] ) :
				
				p = reader.readPixel( x, y )
				n = p.numSamples()
				zBackIndex = p.channelIndex( 'ZBack' )

				if n >= 1 :
					n = n - 1

				for s in range( 0, n ) : 
					front = p.getDepth( s )
					back = p.getDepth( s+1 )
					actualBack = p.channelData( s )[ zBackIndex ]
					self.assertEqual( back, actualBack )

	def testReadOfShwAgainstExr( self ) :
		import IECoreRI
	
		# Create a DeepReader to read a deep EXR.	
		reader = nuke.createNode( "DeepRead" )
		reader["file"].setText( self.__inputPaths()["exr"] )

		# Create an ieDeepImageReader to read the deep SHW.
		readerShw = nuke.createNode("DeepRead")
		readerShw["file"].setText( self.__inputPaths()["shw"] )
		
		random.seed( 1 )

		# Randomly sample 200 points and check that they are the same in both images.
		for i in range( 0, 200 ) :
			x = random.randint( 0, 511 )
			y = random.randint( 0, 511 )
			
			# Check that both image have the same number of samples.
			nSamplesExr = reader.deepSampleCount( x, y )
			nSamplesShw = readerShw.deepSampleCount( x, y )
			self.assertEqual( nSamplesExr, nSamplesShw )
		
			for channel in [ "front", "back", "A" ] :
				for idx in range( 0, nSamplesExr ) :
					frontExr = reader.deepSample( channel, x, y, idx )
					frontShw = readerShw.deepSample( channel, x, y, idx )
					self.assertEqual( frontExr, frontShw )
	
	def tearDown( self ) :
		
		if os.path.isfile( DeepImageReaderTest.__output ) :
			os.remove( DeepImageReaderTest.__output )
							
if __name__ == "__main__":
    unittest.main()

