##########################################################################
#
#  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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
import time

import IECore
import IECoreImage
import IECoreRI
				
class DspyTest( IECoreRI.TestCase ) :

	def testRenderDirectToImagePrimitive( self ) :
		
		r = IECoreRI.Renderer( "" )
		
		# write one image direct to memory
		r.display( "test", "ie", "rgba",
			{
				"driverType" : IECore.StringData( "ImageDisplayDriver" ),
				"handle" : IECore.StringData( "myLovelySphere" ),
				"quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		# write another to disk the usual way
		r.display( "test/IECoreRI/output/sphere.tif", "tiff", "rgba",
			{
				"quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
	
			r.sphere( 1, -1, 1, 360, {} )
			
		# check that they're the same
		
		i = IECoreImage.ImageDisplayDriver.removeStoredImage( "myLovelySphere" )
		i2 = IECore.Reader.create( "test/IECoreRI/output/sphere.tif" ).read()
		
		i.blindData().clear()
		i2.blindData().clear()
		
		self.failIf( IECoreImage.ImageDiffOp()( imageA = i, imageB = i2, maxError = 0.001 ).value )
	
	def testDisplayDriver( self ) :
	
		server = IECoreImage.DisplayDriverServer( 1559 )
		time.sleep( 2 )
		
		rib = """
		Option "searchpath" "string display" "@:./src/rmanDisplays/ieDisplay"
		
		Display "test" "ieTestDisplay" "rgba"
			"quantize" [ 0 0 0 0 ]
			"string driverType" "ClientDisplayDriver"
			"string displayHost" "localhost"
			"string displayPort" "1559"
			"string remoteDisplayType" "ImageDisplayDriver"
			"string handle" "myLovelySphere"
			
		Display "+test/IECoreRI/output/sphere.tif" "tiff" "rgba" "quantize" [ 0 0 0 0 ]
			
		Projection "perspective" "float fov" [ 40 ]
		
		WorldBegin
		
			Translate 0 0 5			
			Sphere 1 -1 1 360

		WorldEnd
		"""
		
		ribFile = open( "test/IECoreRI/output/display.rib", "w" )
		ribFile.write( rib )
		ribFile.close()
		
		os.system( "renderdl test/IECoreRI/output/display.rib" )
		
		i = IECoreImage.ImageDisplayDriver.removeStoredImage( "myLovelySphere" )
		i2 = IECore.Reader.create( "test/IECoreRI/output/sphere.tif" ).read()
		
		i.blindData().clear()
		i2.blindData().clear()
		
		self.failIf( IECoreImage.ImageDiffOp()( imageA = i, imageB = i2, maxError = 0.001 ).value )
				
if __name__ == "__main__":
    unittest.main()
