##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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
import imath

import IECore
import IECoreScene
import IECoreAppleseed

import appleseed

class CameraAlgoTest( unittest.TestCase ):

	# Test that after converting into an appleseed camera, the corners of the screenWindow computed
	# by Cortex are the corners of the screenWindow in Appleseed's screen space
	def assertCameraConvertsFrustum( self, camera ):

		appleseedCamera = IECoreAppleseed.CameraAlgo.convert( camera )

		screenWindow = camera.frustum()
		resolution = appleseed.Vector2u(1920, 1080)
		proj = appleseed.ProjectPoints(appleseedCamera, resolution)
		self.assertTrue(proj.is_initialized())

		for x in [ 0, 1 ]:
			for y in [ 0, 1 ]:
				corner = appleseed.Vector3d(
					screenWindow.max().x if x else screenWindow.min().x,
					screenWindow.max().y if y else screenWindow.min().y,
					-1.0
				)
				screenPos = proj.project_camera_space_point( corner )
				self.assertTrue(screenPos != None)

				self.assertAlmostEqual( x , screenPos[0] )
				self.assertAlmostEqual( y , 1.0 - screenPos[1] )

	@unittest.skipIf( not hasattr( appleseed, "Vector2u" ), "Appleseed >= 2.0.5 required for Vector2u" )
	def testPerspectiveCamera( self ) :

		c = IECoreScene.Camera()
		c.setProjection( "perspective" )
		self.assertCameraConvertsFrustum( c )

		c.setFocalLength( 100 )
		self.assertCameraConvertsFrustum( c )

		c.setFocalLength( 1 )
		self.assertCameraConvertsFrustum( c )

		c.setAperture( imath.V2f( 2, 0.1 ) )
		self.assertCameraConvertsFrustum( c )

		c.setApertureOffset( imath.V2f( 0.6, 0.1 ) )
		self.assertCameraConvertsFrustum( c )

	@unittest.skipIf( not hasattr( appleseed, "Vector2u" ), "Appleseed >= 2.0.5 required for Vector2u" )
	def testOrthographicCamera( self ) :

		c = IECoreScene.Camera()
		c.setProjection( "orthographic" )
		self.assertCameraConvertsFrustum( c )

		c.setFocalLength( 100 )
		self.assertCameraConvertsFrustum( c )

		c.setFocalLength( 1 )
		self.assertCameraConvertsFrustum( c )

		c.setAperture( imath.V2f( 2, 0.1 ) )
		self.assertCameraConvertsFrustum( c )

		# TODO - this test can't be used yet because we're not using a version of
		# appleseed yet which supports aperture offsets on ortho cameras.
		#c.setApertureOffset( imath.V2f( 0.6, 0.1 ) )
		#self.assertCameraConvertsFrustum( c )

if __name__ == "__main__":
	unittest.main()
