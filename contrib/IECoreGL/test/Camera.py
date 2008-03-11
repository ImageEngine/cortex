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
import IECoreGL
IECoreGL.init( False )
import os.path
import os

class CameraTest( unittest.TestCase ) :

	def testPositioning( self ) :
	
		# render a plane at z = 0 with the default camera
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.display( os.path.dirname( __file__ ) + "/output/testCamera.tif", "tiff", "rgba", {} )
		
		r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 512 ) ), "projection" : IECore.StringData( "perspective" ) } )
		
		r.worldBegin()
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) } )
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) ).render( r )
		r.worldEnd()
		
		# check that nothing appears in the output image
		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/output/testCamera.tif" ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.G()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 0 )
				
		# render a plane at z = 0 with the camera moved back a touch to see it
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.display( os.path.dirname( __file__ ) + "/output/testCamera.tif", "tiff", "rgba", {} )
				
		r.transformBegin()
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
		r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 512 ) ), "projection" : IECore.StringData( "perspective" ) } )
		r.transformEnd()

		r.worldBegin()
						
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) } )
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) ).render( r )
		r.worldEnd()
		
		# check that something appears in the output image
		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/output/testCamera.tif" ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
				
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 1 )

	def testXYOrientation( self ) :
	
		# render a red square at x==1, and a green one at y==1
		
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.display( os.path.dirname( __file__ ) + "/output/testCamera.tif", "tiff", "rgba", {} )
		r.transformBegin()
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
		r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 512 ) ) } )
		r.transformEnd()
		
		r.worldBegin()
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) } )
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0.75, -0.25 ), IECore.V2f( 1.25, 0.25 ) ) ).render( r )
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 0, 1, 0 ) ) } )
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.25, 0.75 ), IECore.V2f( 0.25, 1.25 ) ) ).render( r )
		r.worldEnd()
		
		# check we get the colors we'd expect where we expect them
		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/output/testCamera.tif" ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		r = e.R()
		g = e.G()
		b = e.B()
		e.pointAtUV( IECore.V2f( 1, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 1 )
		self.assertEqual( result.floatPrimVar( r ), 1 )
		self.assertEqual( result.floatPrimVar( g ), 0 )
		self.assertEqual( result.floatPrimVar( b ), 0 )
		e.pointAtUV( IECore.V2f( 0.5, 0 ), result )
		self.assertEqual( result.floatPrimVar( a ), 1 )
		self.assertEqual( result.floatPrimVar( r ), 0 )
		self.assertEqual( result.floatPrimVar( g ), 1 )
		self.assertEqual( result.floatPrimVar( b ), 0 )
	
	def tearDown( self ) :
	
		files = [
			os.path.dirname( __file__ ) + "/output/testCamera.tif",
		]
		
		for f in files :
			if os.path.exists( f ) :
				os.remove( f )
					
if __name__ == "__main__":
    unittest.main()
