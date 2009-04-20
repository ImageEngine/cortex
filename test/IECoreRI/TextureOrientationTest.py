##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
import IECore
import IECoreRI
import os.path
import os

class TextureOrientationTest( unittest.TestCase ) :

	def test( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/tex.sdl test/IECoreRI/shaders/tex.sl" ), 0 )

		r = IECoreRI.Renderer( "" )
		r.display( "test/IECoreRI/output/testTextureOrientation1.tif", "tiff", "rgba", {} )
		r.worldBegin()
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
		r.shader( "surface", "st", {} )
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) ).render( r )
		r.worldEnd()

		del r
		
		r = IECoreRI.Renderer( "" )
		r.display( "test/IECoreRI/output/testTextureOrientation2.tif", "tiff", "rgba", {} )
 		r.setOption( "ri:searchpath:texture", IECore.StringData( "@:." ) )
		r.worldBegin()
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
		r.shader( "surface", "test/IECoreRI/shaders/tex", {} )
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) ).render( r )
		r.worldEnd()
		
		ie = IECore.ImageReader.create( "test/IECoreRI/data/textureOrientationImages/expected.tif" ).read()
		i1 = IECore.ImageReader.create( "test/IECoreRI/output/testTextureOrientation1.tif" ).read()
		i2 = IECore.ImageReader.create( "test/IECoreRI/output/testTextureOrientation2.tif" ).read()
		
		testOp = IECore.ImageDiffOp()
		
		self.assert_( not testOp( imageA=ie, imageB=i1, maxError=0.002 ).value ) 
		self.assert_( not testOp( imageA=ie, imageB=i2, maxError=0.002 ).value ) 
			
	def tearDown( self ) :
		
		files = [
			"test/IECoreRI/output/testTextureOrientation1.tif",
			"test/IECoreRI/output/testTextureOrientation2.tif",
			"test/IECoreRI/shaders/tex.sdl",
		]
		for f in files :
			if os.path.exists( f ) :
				os.remove( f )
				
if __name__ == "__main__":
    unittest.main()   
