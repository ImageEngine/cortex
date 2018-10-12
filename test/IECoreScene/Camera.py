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

import unittest
import os.path

import IECore
import IECoreScene

import imath

class TestCamera( unittest.TestCase ) :

	def assertBox2fEqual( self, box, x1, y1, x2, y2 ):
		self.assertAlmostEqual( box.min().x, x1 )
		self.assertAlmostEqual( box.min().y, y1 )
		self.assertAlmostEqual( box.max().x, x2 )
		self.assertAlmostEqual( box.max().y, y2 )

	def test( self ) :

		c = IECoreScene.Camera()
		self.assertEqual( c.parameters(), IECore.CompoundData() )

		cc = c.copy()
		self.assertEqual( cc.parameters(), IECore.CompoundData() )
		self.assertEqual( cc, c )

		IECore.Writer.create( cc, "test/IECore/data/camera.cob" ).write()
		ccc = IECore.Reader.create( "test/IECore/data/camera.cob" ).read()

		self.assertEqual( c, ccc )

		c.setFocalLength( 5 )
		self.assertEqual( c.getFocalLength(), 5 )

		# test copying and saving with some parameters
		cc = c.copy()
		self.assertEqual( cc, c )

		IECore.Writer.create( cc, "test/IECore/data/camera.cob" ).write()
		ccc = IECore.Reader.create( "test/IECore/data/camera.cob" ).read()
		self.assertEqual( ccc, c )

	def testCameraParameters( self ) :

		c = IECoreScene.Camera()

		# Defaults
		self.assertEqual( c.getProjection(), "orthographic" )
		self.assertEqual( c.getAperture(), imath.V2f( 2, 2 ) )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 0, 0 ) )
		self.assertEqual( c.getFocalLength(), 1 )
		self.assertEqual( c.getClippingPlanes(), imath.V2f( 0.01, 100000 ) )
		self.assertEqual( c.getFStop(), 0 )
		self.assertAlmostEqual( c.getFocalLengthWorldScale(), 0.1 )
		self.assertEqual( c.getFocusDistance(), 1 )

		# Set some values
		c.setProjection("perspective" )
		c.setAperture(imath.V2f( 36, 24 ) )
		c.setApertureOffset(imath.V2f( 1, -1 ) )
		c.setFocalLength( 35 )
		c.setClippingPlanes(imath.V2f( -10, 42 ) )
		c.setFStop( 3.0 )
		c.setFocalLengthWorldScale( 0.001 )
		c.setFocusDistance( 12.0 )

		# Test that we've got the new values
		self.assertEqual( c.getProjection(), "perspective" )
		self.assertEqual( c.getAperture(), imath.V2f( 36, 24 ) )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 1, -1 ) )
		self.assertEqual( c.getFocalLength(), 35 )
		self.assertEqual( c.getClippingPlanes(), imath.V2f( -10, 42 ) )
		self.assertEqual( c.getFStop(), 3.0 )
		self.assertAlmostEqual( c.getFocalLengthWorldScale(), 0.001 )
		self.assertEqual( c.getFocusDistance(), 12.0 )

	def testDefaultApertureFromObseleteFovAndScreenWindow( self ) :

		c = IECoreScene.Camera()
		c.parameters()["resolution"] = imath.V2i( 100, 100 )
		c.parameters()["projection"] = "perspective"

		self.assertEqual( c.getAperture(), imath.V2f( 2, 2 ) )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 0, 0 ) )

		c.parameters()["projection:fov"] = 90.0

		self.assertEqual( c.getAperture(), imath.V2f( 2, 2 ) )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 0, 0 ) )

		c.parameters()["projection:fov"] = 60.0
		self.assertAlmostEqual( c.getAperture()[0], 1 / (3 ** 0.5) * 2, places = 6 )
		self.assertAlmostEqual( c.getAperture()[1], 1 / (3 ** 0.5) * 2, places = 6 )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 0, 0 ) )

		c.parameters()["projection:fov"] = 90.0
		c.setResolution( imath.V2i( 200, 100 ) )

		self.assertEqual( c.getAperture(), imath.V2f( 4, 2 ) )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 0, 0 ) )

		c.setPixelAspectRatio( 2 )
		self.assertEqual( c.getAperture(), imath.V2f( 8, 2 ) )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 0, 0 ) )

		c.parameters()["projection:fov"] = 90.0
		c.setResolution( imath.V2i( 100, 100 ) )
		c.setPixelAspectRatio( 1 )
		c.parameters()["screenWindow"] = imath.Box2f( imath.V2f( 10, -3 ), imath.V2f( 11, 5 ) )
		self.assertEqual( c.getAperture(), imath.V2f( 1, 8 ) )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 10.5, 1 ) )

		c.parameters()["screenWindow"] = imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) )
		c.setFocalLength( 35 )

		self.assertEqual( c.getAperture(), imath.V2f( 70, 70 ) )
		self.assertEqual( c.getApertureOffset(), imath.V2f( 0, 0 ) )

	def testRenderOverrides( self ):
		c = IECoreScene.Camera()

		self.assertEqual( c.hasFilmFit(), False )
		self.assertEqual( c.hasResolution(), False )
		self.assertEqual( c.hasPixelAspectRatio(), False )
		self.assertEqual( c.hasResolutionMultiplier(), False )
		self.assertEqual( c.hasOverscan(), False )
		self.assertEqual( c.hasOverscanLeft(), False )
		self.assertEqual( c.hasOverscanRight(), False )
		self.assertEqual( c.hasOverscanTop(), False )
		self.assertEqual( c.hasOverscanBottom(), False )
		self.assertEqual( c.hasCropWindow(), False )
		self.assertEqual( c.hasShutter(), False )

		c.setFilmFit( IECoreScene.Camera.FilmFit.Vertical )
		c.setResolution( imath.V2i( 1280, 720 ) )
		c.setPixelAspectRatio( 2 )
		c.setResolutionMultiplier( 0.5 )
		c.setOverscan( True )
		c.setOverscanLeft( 0.2 )
		c.setOverscanRight( 0.1 )
		c.setOverscanTop( 0.3 )
		c.setOverscanBottom( 0.4 )
		c.setCropWindow( imath.Box2f( imath.V2f( 0.1, 0.2 ), imath.V2f( 0.8, 0.9 ) ) )
		c.setShutter( imath.V2f( -0.7, 0.3 ) )

		self.assertEqual( c.hasFilmFit(), True )
		self.assertEqual( c.hasResolution(), True )
		self.assertEqual( c.hasPixelAspectRatio(), True )
		self.assertEqual( c.hasResolutionMultiplier(), True )
		self.assertEqual( c.hasOverscan(), True )
		self.assertEqual( c.hasOverscanLeft(), True )
		self.assertEqual( c.hasOverscanRight(), True )
		self.assertEqual( c.hasOverscanTop(), True )
		self.assertEqual( c.hasOverscanBottom(), True )
		self.assertEqual( c.hasCropWindow(), True )
		self.assertEqual( c.hasShutter(), True )

		self.assertEqual( c.getFilmFit(), IECoreScene.Camera.FilmFit.Vertical )
		self.assertEqual( c.getResolution(), imath.V2i( 1280, 720 ) )
		self.assertEqual( c.getPixelAspectRatio(), 2 )
		self.assertEqual( c.getResolutionMultiplier(), 0.5 )
		self.assertEqual( c.getOverscan(), True )
		self.assertAlmostEqual( c.getOverscanLeft(), 0.2 )
		self.assertAlmostEqual( c.getOverscanRight(), 0.1 )
		self.assertAlmostEqual( c.getOverscanTop(), 0.3 )
		self.assertAlmostEqual( c.getOverscanBottom(), 0.4 )
		self.assertBox2fEqual( c.getCropWindow(), 0.1, 0.2, 0.8, 0.9 )
		self.assertAlmostEqual( c.getShutter(), imath.V2f( -0.7, 0.3 ) )

		c.removeFilmFit()
		c.removeResolution()
		c.removePixelAspectRatio()
		c.removeResolutionMultiplier()
		c.removeOverscan()
		c.removeOverscanLeft()
		c.removeOverscanRight()
		c.removeOverscanTop()
		c.removeOverscanBottom()
		c.removeCropWindow()
		c.removeShutter()

		self.assertEqual( c.hasFilmFit(), False )
		self.assertEqual( c.hasResolution(), False )
		self.assertEqual( c.hasPixelAspectRatio(), False )
		self.assertEqual( c.hasResolutionMultiplier(), False )
		self.assertEqual( c.hasOverscan(), False )
		self.assertEqual( c.hasOverscanLeft(), False )
		self.assertEqual( c.hasOverscanRight(), False )
		self.assertEqual( c.hasOverscanTop(), False )
		self.assertEqual( c.hasOverscanBottom(), False )
		self.assertEqual( c.hasCropWindow(), False )
		self.assertEqual( c.hasShutter(), False )

	def testHash( self ) :
		c = IECoreScene.Camera()
		h = c.hash()

		c.setFocalLength( 12 )
		self.assertNotEqual( c.hash(), h )
		h = c.hash()

	def testNormalizedScreenWindow( self ):
		c = IECoreScene.Camera()
		self.assertBox2fEqual( c.frustum(), -1, -0.75, 1, 0.75 )
		c.setFocalLength( 2 )
		self.assertBox2fEqual( c.frustum(), -1, -0.75, 1, 0.75 )
		c.setProjection("perspective" )
		self.assertBox2fEqual( c.frustum(), -0.5, -0.375, 0.5, 0.375 )
		c.setAperture(imath.V2f( 4, 4 ) )
		self.assertBox2fEqual( c.frustum(), -1, -0.75, 1, 0.75 )
		c.setApertureOffset(imath.V2f( 1, 1 ) )
		self.assertBox2fEqual( c.frustum(), -0.5, -0.25, 1.5, 1.25 )
		c.setFocalLength( 1 )
		self.assertBox2fEqual( c.frustum(), -1, -0.5, 3, 2.5 )
		c.setResolution(imath.V2i( 100, 100 ) )
		self.assertBox2fEqual( c.frustum(), -1, -1, 3, 3 )

	def testRenderImageSpec( self ):
		def B( x1, y1, x2, y2 ):
			return imath.Box2i( imath.V2i( x1, y1 ), imath.V2i( x2, y2 ) )

		c = IECoreScene.Camera()
		self.assertEqual( c.renderResolution(), imath.V2i( 640, 480 ) )
		self.assertEqual( c.renderRegion(), B( 0, 0, 640, 480 ) )
		c.setResolution( imath.V2i( 1920, 1080 ) )
		self.assertEqual( c.renderResolution(), imath.V2i( 1920, 1080 ) )
		self.assertEqual( c.renderRegion(), B( 0, 0, 1920, 1080 ) )
		c.setOverscanLeft( 0.1 )
		self.assertEqual( c.renderResolution(), imath.V2i( 1920, 1080 ) )
		self.assertEqual( c.renderRegion(), B( 0, 0, 1920, 1080 ) )
		c.setOverscan( True )
		self.assertEqual( c.renderResolution(), imath.V2i( 1920, 1080 ) )
		self.assertEqual( c.renderRegion(), B( -192, 0, 1920, 1080 ) )
		c.setOverscanRight( 1.0 )
		c.setOverscanTop( 0.5 )
		c.setOverscanBottom( 0.25 )
		self.assertEqual( c.renderResolution(), imath.V2i( 1920, 1080 ) )
		self.assertEqual( c.renderRegion(), B( -192, -270, 3840, 1620 ) )
		c.setCropWindow( imath.Box2f( imath.V2f( 0, 0 ), imath.V2f( 1, 1 ) ) )
		self.assertEqual( c.renderResolution(), imath.V2i( 1920, 1080 ) )
		self.assertEqual( c.renderRegion(), B( 0, 0, 1920, 1080 ) )
		c.setCropWindow( imath.Box2f( imath.V2f( 0.2, 0.3 ), imath.V2f( 0.8, 0.5 ) ) )
		self.assertEqual( c.renderResolution(), imath.V2i( 1920, 1080 ) )
		self.assertEqual( c.renderRegion(), B( 384, 1080 - 540, 1536, 1080 - 324 ) )

	def testFitWindow( self ):
		def B( x1, y1, x2, y2 ):
			return imath.Box2f( imath.V2f( x1, y1 ), imath.V2f( x2, y2 ) )

		FitMode = IECoreScene.Camera.FilmFit
		cc = IECoreScene.Camera
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Horizontal, 1.0 ), -1, -1, 1, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Vertical,   1.0 ), -1, -1, 1, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Fit,        1.0 ), -1, -1, 1, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Fill,       1.0 ), -1, -1, 1, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Distort,    1.0 ), -1, -1, 1, 1 )

		self.assertBox2fEqual( cc.fitWindow( B(-2, -1, 2, 1), FitMode.Horizontal, 1.0 ), -2, -2, 2, 2 )
		self.assertBox2fEqual( cc.fitWindow( B(-2, -1, 2, 1), FitMode.Vertical,   1.0 ), -1, -1, 1, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-2, -1, 2, 1), FitMode.Fit,        1.0 ), -2, -2, 2, 2 )
		self.assertBox2fEqual( cc.fitWindow( B(-2, -1, 2, 1), FitMode.Fill,       1.0 ), -1, -1, 1, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-2, -1, 2, 1), FitMode.Distort,    1.0 ), -2, -1, 2, 1 )

		self.assertBox2fEqual( cc.fitWindow( B(-1, -2, 1, 2), FitMode.Horizontal, 1.0 ), -1, -1, 1, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -2, 1, 2), FitMode.Vertical,   1.0 ), -2, -2, 2, 2 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -2, 1, 2), FitMode.Fit,        1.0 ), -2, -2, 2, 2 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -2, 1, 2), FitMode.Fill,       1.0 ), -1, -1, 1, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -2, 1, 2), FitMode.Distort,    1.0 ), -1, -2, 1, 2 )

		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Horizontal, 0.5 ), -1,   -2, 1,   2 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Vertical,   0.5 ), -0.5, -1, 0.5, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Fit,        0.5 ), -1,   -2, 1,   2 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Fill,       0.5 ), -0.5, -1, 0.5, 1 )
		self.assertBox2fEqual( cc.fitWindow( B(-1, -1, 1, 1), FitMode.Distort,    0.5 ), -1,   -1, 1,   1 )


	def tearDown( self ) :

		if os.path.isfile( "test/IECore/data/camera.cob" ) :
			os.remove( "test/IECore/data/camera.cob" )

if __name__ == "__main__":
        unittest.main()
