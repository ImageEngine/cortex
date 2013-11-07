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

import math
import unittest
import IECoreNuke
import IECore
import nuke
import os

class LensDisortTest( IECoreNuke.TestCase ) :

	def __outputPath( self ) :	
		return "test/IECoreNuke/nukeLensDistortOutput.exr"

	def __paths( self ) :
		paths = {
			'path' : "test/IECoreNuke/nukeLensDistortInput.exr",
			'croppedPath' : "test/IECoreNuke/nukeLensDistortInputCropped.exr",
			'croppedOffsetPath' : "test/IECoreNuke/nukeLensDistortInputCroppedOffset.exr",
			'offsetPath' : "test/IECoreNuke/nukeLensDistortInputOffset.exr"
		}
		return paths

	def setUp( self ) :
		
		paths = self.__paths()
		for p in paths.keys() :
			if os.path.exists( p ) :
				os.remove( p )
		
		# Set the default format to be something fun.
		nuke.root()["format"].fromScript("1144 862 0 0 1144 862 2 CortexTestAlexaProxyAnamorphic(2.66)")
		
		# Create a colourful test image that we will distort.
		n1 = nuke.createNode("ColorWheel")
		n2 = nuke.createNode("ColorBars")
		n3 = nuke.createNode("CheckerBoard2")
		m1 = nuke.createNode("Merge2")
		m2 = nuke.createNode("Merge2")
		m1.setInput( 0, n1 )
		m1.setInput( 1, n2 )
		m2.setInput( 0, m1 )
		m2.setInput( 1, n3 )
		m2["operation"].setValue(1)

		# Create a write node so that we can save the image to disk.
		w = nuke.createNode("Write")
		w.setInput( 0, m2 )
		w["file"].setText( paths['path'] )
	
		# Crop the image and generate another test image.
		c = nuke.createNode("Crop")
		c["box"].setValue( ( 29, -74, 374, 448 ) )
		c.setInput( 0, m2 )

		w2 = nuke.createNode("Write")
		w2.setInput( 0, c )
		w2["file"].setText( paths['croppedPath'] )
		
		# Create the test files.
		nuke.execute( w, 1, 1 )
		nuke.execute( w2, 1, 1 )
	
		# Finally, read back the images and offset their display windows to make the
		# tests even more interesting...
		offsetImg = IECore.Reader.create( paths['path'] ).read()
		offsetDisplayWindow = IECore.Box2i( offsetImg.displayWindow.min + IECore.V2i( -261, 172 ), offsetImg.displayWindow.max + IECore.V2i( -261, 172 ) ) 
		offsetImg.displayWindow = offsetDisplayWindow
		IECore.Writer.create( offsetImg, paths['offsetPath'] ).write()
		
		croppedOffsetImg = IECore.Reader.create( paths['croppedPath'] ).read()
		offsetDisplayWindow = IECore.Box2i( croppedOffsetImg.displayWindow.min + IECore.V2i( 120, -100 ), croppedOffsetImg.displayWindow.max + IECore.V2i( 120, -100 ) ) 
		croppedOffsetImg.displayWindow = offsetDisplayWindow
		IECore.Writer.create( croppedOffsetImg, paths['croppedOffsetPath'] ).write()

	def __testLens( self ) :
		# Create a nice asymetric lens distortion.
		o = IECore.Reader.create("test/IECore/data/StandardRadialLens.cob").read() 
		return o

	def testLensDistortAgainstLensDistortOp( self ) :
		"""Test that the output of the Cortex LensDistortOp and the LensDistort node are the same.\n"""
		
		paths = self.__paths()
		for p in paths.keys() :
			self.assertTrue( os.path.exists( paths[p] ) )
		
		outputPath = self.__outputPath()
		if os.path.exists( outputPath ) :
			os.remove( outputPath )
			
		# Set the default format to be something fun.
		nuke.root()["format"].fromScript("1144 862 0 0 1144 862 2 CortexTestAlexaProxyAnamorphic(2.66)")
		
		r = nuke.createNode("Read")
		
		# Create a LensDistort node.
		l = nuke.createNode("ieLensDistort")
		l["mode"].setValue( IECore.LensModel.Undistort )
		l.setInput( 0, r )
		
		# Set the parameters of the lens distort node.
		l['lensFileSequence'].fromScript( os.path.abspath( "test/IECore/data/StandardRadialLens.cob" ) )

		# Create a write node.
		w = nuke.createNode("Write")
		w.setInput( 0, l )
		w["file"].setText( outputPath )
	
		# Create the op that we will compare the result of the nuke LensDistort node with.
		lensDistortOp = IECore.LensDistortOp()
		lensDistortOp["mode"].setValue( IECore.LensModel.Undistort )
		lensDistortOp['lensModel'].setValue( self.__testLens() )
		
		for path in paths.keys() :
			# Update the read node.	
			r["file"].setText( paths[path] )

			if path == 'path' :
				# When the format is the same as the data window, nuke doesn't create a black border around the image.
				# As a result, we shouldn't create one using our LensDistortOp either.
				lensDistortOp["boundMode"].setValue( IECore.WarpOp.BoundMode.Clamp )
			else :
				lensDistortOp["boundMode"].setValue( IECore.WarpOp.BoundMode.SetToBlack )

			# Write out the result of the LensDistort so that we can compare it to the output of the cortex op.	
			nuke.execute( w, 1, 1 )
		
			img = IECore.Reader.create( paths[path] ).read()
			lensDistortOp["input"].setValue( img )
		
			cortexImg = lensDistortOp()
			nukeImg = IECore.Reader.create( outputPath ).read()
		
			# Assert that the two images are almost identical. 
			# We expect a little bit of error as the cortex op uses a different sampling filter to the nuke node.
			res = IECore.ImageDiffOp()(
				imageA = cortexImg,
				imageB = nukeImg,
			)
			self.assertFalse( res.value )	
							
	def tearDown( self ) :
		paths = self.__paths()
		paths['output'] = self.__outputPath()

		for p in paths.keys() :
			if os.path.exists( paths[p] ) :
				os.remove( p )

if __name__ == "__main__":
    unittest.main()

