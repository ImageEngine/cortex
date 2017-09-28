##########################################################################
#
#  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
#  Copyright (c) 2012, John Haddon. All rights reserved.
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

class CameraControllerTest( unittest.TestCase ) :

	def testUnprojectOrthographic( self ) :

		camera = IECore.Camera()
		camera.parameters()["clippingPlanes"] = IECore.V2f( .1, 10 )
		camera.parameters()["screenWindow"] = IECore.Box2f( IECore.V2f( -2, -1 ), IECore.V2f( 2, 1 ) )
		camera.parameters()["resolution"] = IECore.V2i( 500, 250 )
		camera.parameters()["projection"] = "orthographic"

		controller = IECore.CameraController( camera )

		near, far = controller.unproject( IECore.V2f( 0, 0 ) )
		self.assertEqual( near, IECore.V3f( -2, 1, -.1 ) )
		self.assertEqual( far, IECore.V3f( -2, 1, -10 ) )

		near, far = controller.unproject( IECore.V2f( 500, 250 ) )
		self.assertEqual( near, IECore.V3f( 2, -1, -.1 ) )
		self.assertEqual( far, IECore.V3f( 2, -1, -10 ) )

		near, far = controller.unproject( IECore.V2f( 250, 125 ) )
		self.assertEqual( near, IECore.V3f( 0, 0, -.1 ) )
		self.assertEqual( far, IECore.V3f( 0, 0, -10 ) )

	def testUnprojectPerspective( self ) :

		camera = IECore.Camera()
		camera.parameters()["clippingPlanes"] = IECore.V2f( .1, 10 )
		camera.parameters()["screenWindow"] = IECore.Box2f( IECore.V2f( -1, -.5 ), IECore.V2f( 1, .5 ) )
		camera.parameters()["resolution"] = IECore.V2i( 500, 250 )
		camera.parameters()["projection"] = "perspective"
		camera.parameters()["projection:fov"] = 90

		controller = IECore.CameraController( camera )

		near, far = controller.unproject( IECore.V2f( 0, 0 ) )
		self.assertEqual( near, IECore.V3f( -.1, .05, -.1 ) )
		self.assertEqual( far, IECore.V3f( -10, 5, -10 ) )

		near, far = controller.unproject( IECore.V2f( 500, 250 ) )
		self.assertEqual( near, IECore.V3f( .1, -.05, -.1 ) )
		self.assertEqual( far, IECore.V3f( 10, -5, -10 ) )

		near, far = controller.unproject( IECore.V2f( 250, 125 ) )
		self.assertEqual( near, IECore.V3f( 0, 0, -.1 ) )
		self.assertEqual( far, IECore.V3f( 0, 0, -10 ) )

	def testProjectOrthographic( self ) :

		camera = IECore.Camera()
		camera.parameters()["clippingPlanes"] = IECore.V2f( .1, 10 )
		camera.parameters()["screenWindow"] = IECore.Box2f( IECore.V2f( -2, -1 ), IECore.V2f( 2, 1 ) )
		camera.parameters()["resolution"] = IECore.V2i( 500, 250 )
		camera.parameters()["projection"] = IECore.StringData( "orthographic" )

		controller = IECore.CameraController( camera )

		r = IECore.Rand48()
		for i in range( 0, 100 ) :
			rasterPosition = IECore.V2f( r.nexti() % 500, r.nexti() % 250 )
			near, far = controller.unproject( rasterPosition )
			nearProjected = controller.project( near )
			farProjected = controller.project( far )
			self.failUnless( nearProjected.equalWithAbsError( farProjected, 0.0001 ) )
			self.failUnless(
				IECore.V2f( rasterPosition.x, rasterPosition.y ).equalWithAbsError(
					nearProjected, 0.0001
				)
			)

	def testProjectPerspective( self ) :

		camera = IECore.Camera()
		camera.parameters()["clippingPlanes"] = IECore.V2f( .1, 10 )
		camera.parameters()["screenWindow"] = IECore.Box2f( IECore.V2f( -1, -.5 ), IECore.V2f( 1, .5 ) )
		camera.parameters()["resolution"] = IECore.V2i( 500, 250 )
		camera.parameters()["projection"] = IECore.StringData( "perspective" )
		camera.parameters()["projection:fov"] = IECore.FloatData( 50 )

		controller = IECore.CameraController( camera )

		r = IECore.Rand48()
		for i in range( 0, 100 ) :
			rasterPosition = IECore.V2f( r.nexti() % 500, r.nexti() % 250 )
			near, far = controller.unproject( rasterPosition )
			nearProjected = controller.project( near )
			farProjected = controller.project( far )
			self.failUnless( nearProjected.equalWithAbsError( farProjected, 0.0001 ) )
			self.failUnless(
				IECore.V2f( rasterPosition.x, rasterPosition.y ).equalWithAbsError(
					nearProjected, 0.0001
				)
			)

	def testCameraWithExistingTransform( self ) :

		camera = IECore.Camera()
		transform = IECore.MatrixTransform( IECore.M44f() )
		originalMatrix = transform.transform()
		camera.setTransform( transform )

		controller = IECore.CameraController( camera )

		controller.motionStart( controller.MotionType.Track, IECore.V2f( 0 ) )
		controller.motionEnd( IECore.V2f( 10 ) )

		self.assertNotEqual( transform.transform(), originalMatrix )

	def testSetResolutionScaling( self ) :

		camera = IECore.Camera()
		camera.parameters()["resolution"] = IECore.V2i( 200, 100 )
		camera.parameters()["screenWindow"] = IECore.Box2f( IECore.V2f( -2, -1 ), IECore.V2f( 2, 1 ) )

		controller = IECore.CameraController( camera )

		controller.setResolution( IECore.V2i( 200, 200 ) )
		self.assertEqual( camera.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )

		controller.setResolution( IECore.V2i( 200, 100 ), controller.ScreenWindowAdjustment.ScaleScreenWindow )
		self.assertEqual( camera.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -2, -1 ), IECore.V2f( 2, 1 ) ) )

	def testSetResolutionCropping( self ) :

		camera = IECore.Camera()
		camera.parameters()["resolution"] = IECore.V2i( 200, 100 )
		camera.parameters()["screenWindow"] = IECore.Box2f( IECore.V2f( -2, -1 ), IECore.V2f( 2, 1 ) )

		controller = IECore.CameraController( camera )

		controller.setResolution( IECore.V2i( 100, 100 ), controller.ScreenWindowAdjustment.CropScreenWindow )
		self.assertEqual( camera.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )

		controller.setResolution( IECore.V2i( 100, 200 ), controller.ScreenWindowAdjustment.CropScreenWindow )
		self.assertEqual( camera.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -1, -2 ), IECore.V2f( 1, 2 ) ) )

	def testSetResolutionCroppingWithOffsetCenter( self ) :

		camera = IECore.Camera()
		camera.parameters()["resolution"] = IECore.V2i( 200, 100 )
		camera.parameters()["screenWindow"] = IECore.Box2f( IECore.V2f( -2, -1 ), IECore.V2f( 0, 0 ) )

		controller = IECore.CameraController( camera )

		controller.setResolution( IECore.V2i( 100, 100 ), controller.ScreenWindowAdjustment.CropScreenWindow )
		self.assertEqual( camera.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -1.5, -1 ), IECore.V2f( -0.5, 0  ) ) )

		controller.setResolution( IECore.V2i( 100, 200 ), controller.ScreenWindowAdjustment.CropScreenWindow )
		self.assertEqual( camera.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -1.5, -1.5 ), IECore.V2f( -0.5, 0.5 ) ) )

if __name__ == "__main__":
        unittest.main()
