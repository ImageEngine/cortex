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

class CameraTest( IECoreRI.TestCase ) :
	
	def testParameters( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/testCamera.rib" )

		r.camera( "main", {
			"resolution" : IECore.V2iData( IECore.V2i( 1024, 200 ) ),
			"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) ),
			"cropWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( 0.1, 0.1 ), IECore.V2f( 0.9, 0.9 ) ) ),
			"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
			"projection" : IECore.StringData( "perspective" ),
			"projection:fov" : IECore.FloatData( 45 ),
			"shutter" : IECore.V2fData( IECore.V2f( 0, 0.1 ) ),
		} )

		r.worldBegin()
		r.worldEnd()

		l = "".join( file( "test/IECoreRI/output/testCamera.rib" ).readlines() )
		l = " ".join( l.split() )

		self.assert_( "Format 1024 200 1 " in l )
		self.assert_( "ScreenWindow -1 1 -1 1" in l )
		self.assert_( "CropWindow 0.1 0.9 0.1 0.9" in l )
		self.assert_( ("Clipping 1 1000" in l) or ("Clipping 1 1e3" in l) )
		self.assert_( "Projection \"perspective\" \"float fov\" [ 45 ]" in l )
		self.assert_( "Shutter 0 0.1" in l )

	def testPositioning( self ) :

		# render a plane at z = 0 with the default camera
		r = IECoreRI.Renderer( "" )
		r.display( "test/IECoreRI/output/testCamera.tif", "tiff", "rgba", {} )
		r.worldBegin()
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) ).render( r )
		r.worldEnd()

		# check that nothing appears in the output image
		i = IECore.Reader.create( "test/IECoreRI/output/testCamera.tif" ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 0 )
		del r

		# render a plane at z = 0 with the camera moved back a touch to see it
		r = IECoreRI.Renderer( "" )
		r.display( "test/IECoreRI/output/testCamera.tif", "tiff", "rgba", {} )

		r.transformBegin()
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, 1 ) ) )
		r.camera( "main", {} )
		r.transformEnd()

		r.worldBegin()
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) ).render( r )
		r.worldEnd()

		# check that something appears in the output image
		i = IECore.Reader.create( "test/IECoreRI/output/testCamera.tif" ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 1 )

	def testXYOrientation( self ) :

		# render a red square at x==1, and a green one at y==1

		r = IECoreRI.Renderer( "" )
		r.display( "test/IECoreRI/output/testCamera.tif", "tiff", "rgba", {} )
		r.transformBegin()
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, 1 ) ) )
		r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 512 ) ) } )
		r.transformEnd()

		r.worldBegin()
		r.setAttribute( "color", IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) )
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0.75, -0.25 ), IECore.V2f( 1.25, 0.25 ) ) ).render( r )
		r.setAttribute( "color", IECore.Color3fData( IECore.Color3f( 0, 1, 0 ) ) )
		IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.25, 0.75 ), IECore.V2f( 0.25, 1.25 ) ) ).render( r )
		r.worldEnd()

		# check we get the colors we'd expect where we expect them
		i = IECore.Reader.create( "test/IECoreRI/output/testCamera.tif" ).read()
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

	def testMultipleCameraRIB( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/testCamera.rib" )

		with IECore.TransformBlock( r ) :

			r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 1, 2, 3 ) ) )
		
			r.camera( "first", {
				"projection" : IECore.StringData( "perspective" ),
				"projection:fov" : IECore.FloatData( 45 ),
			} )

		with IECore.TransformBlock( r ) :

			r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 3, 4, 5 ) ) )
		
			r.camera( "second", {
				"projection" : IECore.StringData( "perspective" ),
				"projection:fov" : IECore.FloatData( 50 ),
			} )

		with IECore.WorldBlock( r ) :
			pass
			
		l = "".join( file( "test/IECoreRI/output/testCamera.rib" ).readlines() )
		l = " ".join( l.split() )

		self.assertTrue( "Projection \"perspective\" \"float fov\" [ 45 ]" in l )
		self.assertTrue( "Camera \"first\"" in l )
		self.assertTrue( "Projection \"perspective\" \"float fov\" [ 50 ]" in l )
		self.assertTrue( "Camera \"second\"" in l )
		self.assertEqual( l.count( "Camera" ), 2 )
		
	def testMultipleCameraRender( self ) :

		r = IECoreRI.Renderer( "" )
		r.display( "test/IECoreRI/output/testCamera.tif", "tiff", "rgba", {} )

		with IECore.TransformBlock( r ) :
			r.camera( "iCantSeeAnything", {} )
		
		with IECore.TransformBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, 1 ) ) )
			r.camera( "iCanSeeSomething", {} )
		
		with IECore.WorldBlock( r ) :
			IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) ).render( r )

		# check that something appears in the output image
		i = IECore.Reader.create( "test/IECoreRI/output/testCamera.tif" ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 1 )
	
	def testMotionBlurCameraRender( self ) :
	
		r = IECoreRI.Renderer( "" )
		r.display( "test/IECoreRI/output/testCamera.tif", "tiff", "rgba", {} )
		
		with IECore.TransformBlock( r ) :
			with IECore.MotionBlock( r, [ 0, 1 ] ) :
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 1 ) ) )
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 1 ) ) )
			
			r.camera( "main", { "shutter" : IECore.V2f( 0, 1 ) } )
		
		with IECore.WorldBlock( r ) :
			IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) ).render( r )

		# check that something appears in the output image
		i = IECore.Reader.create( "test/IECoreRI/output/testCamera.tif" ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertTrue( result.floatPrimVar( e.A() ) > 0 ) # something should be there
		self.assertTrue( result.floatPrimVar( e.A() ) < 1 ) # but it should be blurry
	
	def testMotionBlurCameraRib( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testCamera.rib" )
		
		with IECore.TransformBlock( r ) :
			with IECore.MotionBlock( r, [ 0, 1 ] ) :
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 1 ) ) )
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 1 ) ) )
			
			r.camera( "main", { "shutter" : IECore.V2f( 0, 1 ) } )
		
		r.worldBegin()
		r.worldEnd()
		
		l = "".join( file( "test/IECoreRI/output/testCamera.rib" ).readlines() )
		
		self.assert_( "MotionBegin [ 0 1 ]" in l )
	
	def testPixelAspectRatio( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/testCamera.rib" )

		r.camera( "main", {
			"resolution" : IECore.V2i( 100, 200 ),
			"pixelAspectRatio" : 2.0,
		} )

		with IECore.WorldBlock( r ) :
			pass

		l = "".join( file( "test/IECoreRI/output/testCamera.rib" ).readlines() )
		l = " ".join( l.split() )

		self.assertTrue( "Format 100 200 2" in l )
		self.assertTrue( "ScreenWindow -1 1 -1 1" in l )
	
if __name__ == "__main__":
    unittest.main()
