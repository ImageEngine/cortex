##########################################################################
#
#  Copyright (c) 2008-2015, Image Engine Design Inc. All rights reserved.
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

import maya.cmds
import maya.OpenMaya
import imath

import IECore
import IECoreScene
import IECoreMaya

class FromMayaCameraConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape" )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaCameraConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape", IECoreScene.Camera.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaCameraConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape", IECoreScene.Renderable.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaCameraConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape", IECore.Writer.staticTypeId() )
		self.assertEqual( converter, None )

	def test( self ) :

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape" )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCameraConverter ) ) )

		camera = converter.convert()
		self.assert_( camera.isInstanceOf( IECoreScene.Camera.staticTypeId() ) )

	def testConstructor( self ) :

		converter = IECoreMaya.FromMayaCameraConverter( "perspShape" )
		camera = converter.convert()
		self.assert_( camera.isInstanceOf( IECoreScene.Camera.staticTypeId() ) )

	def testPerspective( self ) :

		camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()

		self.assertEqual( camera.getName(), "perspShape" )
		self.assertEqual( camera.getTransform().transform(), imath.M44f( *maya.cmds.getAttr( "persp.worldMatrix[0]" ) ) )
		self.assertEqual( camera.parameters()["resolution"].value, imath.V2i( maya.cmds.getAttr( "defaultResolution.width" ), maya.cmds.getAttr( "defaultResolution.height" ) ) )
		self.assertEqual( camera.parameters()["clippingPlanes"].value, imath.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
		self.assertEqual( camera.parameters()["projection"].value, "perspective" )
		self.assertEqual( camera.blindData()["maya"]["aperture"].value, imath.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) )

		sel = maya.OpenMaya.MSelectionList()
		sel.add( "perspShape" )
		dag = maya.OpenMaya.MDagPath()
		sel.getDagPath( 0, dag )
		fn = maya.OpenMaya.MFnCamera( dag )
		self.assertAlmostEqual( camera.parameters()["projection:fov"].value, IECore.radiansToDegrees( fn.horizontalFieldOfView() ), 5 )

	def testOrthographic( self ) :

		camera = IECoreMaya.FromMayaCameraConverter( "topShape" ).convert()

		self.assertEqual( camera.getName(), "topShape" )
		self.assertEqual( camera.getTransform().transform(), imath.M44f( *maya.cmds.getAttr( "top.worldMatrix[0]" ) ) )
		self.assertEqual( camera.parameters()["resolution"].value, imath.V2i( maya.cmds.getAttr( "defaultResolution.width" ), maya.cmds.getAttr( "defaultResolution.height" ) ) )
		self.assertEqual( camera.parameters()["clippingPlanes"].value, imath.V2f( maya.cmds.getAttr( "topShape.nearClipPlane" ), maya.cmds.getAttr( "topShape.farClipPlane" ) ) )
		self.assertEqual( camera.parameters()["projection"].value, "orthographic" )
		self.assertEqual( camera.parameters()["screenWindow"].value.max().x - camera.parameters()["screenWindow"].value.min().x, maya.cmds.getAttr( "topShape.orthographicWidth" ) )
		self.assertEqual( camera.blindData()["maya"]["aperture"].value, imath.V2f( maya.cmds.getAttr( "topShape.horizontalFilmAperture" ), maya.cmds.getAttr( "topShape.verticalFilmAperture" ) ) )

	def testCustomResolution( self ) :

		converter = IECoreMaya.FromMayaCameraConverter( "perspShape" )
		converter.parameters()["resolutionMode"].setValue( "specified" )
		converter.parameters()["resolution"].setValue( "1K" )
		camera = converter.convert()

		self.assertEqual( camera.getName(), "perspShape" )
		self.assertEqual( camera.getTransform().transform(), imath.M44f( *maya.cmds.getAttr( "persp.worldMatrix[0]" ) ) )
		self.assertEqual( camera.parameters()["resolution"].value, imath.V2i( 1024, 778 ) )
		self.assertEqual( camera.parameters()["clippingPlanes"].value, imath.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
		self.assertEqual( camera.parameters()["projection"].value, "perspective" )
		self.assertEqual( camera.blindData()["maya"]["aperture"].value, imath.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) )

		sel = maya.OpenMaya.MSelectionList()
		sel.add( "perspShape" )
		dag = maya.OpenMaya.MDagPath()
		sel.getDagPath( 0, dag )
		fn = maya.OpenMaya.MFnCamera( dag )
		self.assertAlmostEqual( camera.parameters()["projection:fov"].value, IECore.radiansToDegrees( fn.horizontalFieldOfView() ), 5 )

	def testPixelAspectRatio( self ) :

		def verify( camera, resolution, pixelAspectRatio, expectedScreenWindow ) :

			self.assertEqual( camera.getName(), "perspShape" )
			self.assertEqual( camera.getTransform().transform(), imath.M44f( *maya.cmds.getAttr( "persp.worldMatrix[0]" ) ) )
			self.assertEqual( camera.parameters()["resolution"].value, resolution )
			self.assertEqual( camera.parameters()["clippingPlanes"].value, imath.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
			self.assertEqual( camera.parameters()["projection"].value, "perspective" )
			self.assertTrue( camera.parameters()["screenWindow"].value.min().equalWithAbsError( expectedScreenWindow.min(), 1e-6 ) )
			self.assertTrue( camera.parameters()["screenWindow"].value.max().equalWithAbsError( expectedScreenWindow.max(), 1e-6 ) )
			self.assertEqual( camera.blindData()["maya"]["aperture"].value, imath.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) )

			sel = maya.OpenMaya.MSelectionList()
			sel.add( "perspShape" )
			dag = maya.OpenMaya.MDagPath()
			sel.getDagPath( 0, dag )
			fn = maya.OpenMaya.MFnCamera( dag )
			self.assertAlmostEqual( camera.parameters()["projection:fov"].value, IECore.radiansToDegrees( fn.horizontalFieldOfView() ), 5 )

		converter = IECoreMaya.FromMayaCameraConverter( "perspShape" )

		# from the render globals
		converter.parameters()["resolutionMode"].setValue( "renderGlobals" )

		# this crazy 3 step approach seem to be the only way to set
		# the pixel aspect ratio without altering the resolution
		# settings. this was essentially cribbed from Maya's
		# changeMayaSoftwareResolution() mel function.
		def setGlobalPixelAspectRatio( pixelAspectRatio ) :
			maya.cmds.setAttr( "defaultResolution.deviceAspectRatio", float(pixelAspectRatio) / ( float(maya.cmds.getAttr( "defaultResolution.height" )) / float(maya.cmds.getAttr( "defaultResolution.width" )) ) )
			maya.cmds.setAttr( "defaultResolution.lockDeviceAspectRatio", 0 )
			maya.cmds.setAttr( "defaultResolution.pixelAspect", pixelAspectRatio )

		globalRes = imath.V2i( maya.cmds.getAttr( "defaultResolution.width" ), maya.cmds.getAttr( "defaultResolution.height" ) )
		self.assertEqual( globalRes, imath.V2i( 960, 540 ) )
		globalScreenWindow = imath.Box2f( imath.V2f( -1, -0.5625 ), imath.V2f( 1, 0.5625 ) )
		setGlobalPixelAspectRatio( 1 )
		verify( converter.convert(), resolution = globalRes, pixelAspectRatio = 1, expectedScreenWindow = globalScreenWindow )
		setGlobalPixelAspectRatio( 2 )
		verify( converter.convert(), resolution = globalRes, pixelAspectRatio = 2, expectedScreenWindow = imath.Box2f( globalScreenWindow.min() / imath.V2f( 1, 2 ), globalScreenWindow.max() / imath.V2f( 1, 2 ) ) )
		setGlobalPixelAspectRatio( 3 )
		verify( converter.convert(), resolution = globalRes, pixelAspectRatio = 3, expectedScreenWindow = imath.Box2f( globalScreenWindow.min() / imath.V2f( 1, 3 ), globalScreenWindow.max() / imath.V2f( 1, 3 ) ) )

		# from the parameters
		customRes = imath.V2i( 1024, 778 )
		customScreenWindow = imath.Box2f( imath.V2f( -1, -0.759765 ), imath.V2f( 1, 0.759765 ) )
		converter.parameters()["resolutionMode"].setValue( "specified" )
		converter.parameters()["resolution"].setTypedValue( customRes )
		converter.parameters()["pixelAspectRatio"].setTypedValue( 1 )
		verify( converter.convert(), resolution = customRes, pixelAspectRatio = 1, expectedScreenWindow = customScreenWindow )
		converter.parameters()["pixelAspectRatio"].setTypedValue( 2 )
		verify( converter.convert(), resolution = customRes, pixelAspectRatio = 2, expectedScreenWindow = imath.Box2f( customScreenWindow.min() / imath.V2f( 1, 2 ), customScreenWindow.max() / imath.V2f( 1, 2 ) ) )
		converter.parameters()["pixelAspectRatio"].setTypedValue( 3 )
		verify( converter.convert(), resolution = customRes, pixelAspectRatio = 3, expectedScreenWindow = imath.Box2f( customScreenWindow.min() / imath.V2f( 1, 3 ), customScreenWindow.max() / imath.V2f( 1, 3 ) ) )

	def testFilmOffset( self ) :

		for x in [ -0.5, -0.25, 0, 0.25, 0.5 ] :

			for y in [ -0.5, -0.25, 0, 0.25, 0.5 ] :

				maya.cmds.setAttr( "perspShape.horizontalFilmOffset", x )
				maya.cmds.setAttr( "perspShape.verticalFilmOffset", y )
				camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()

				self.assertEqual( camera.getName(), "perspShape" )
				self.assertEqual( camera.getTransform().transform(), imath.M44f( *maya.cmds.getAttr( "persp.worldMatrix[0]" ) ) )
				self.assertEqual( camera.parameters()["resolution"].value, imath.V2i( maya.cmds.getAttr( "defaultResolution.width" ), maya.cmds.getAttr( "defaultResolution.height" ) ) )
				self.assertEqual( camera.parameters()["clippingPlanes"].value, imath.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
				self.assertEqual( camera.parameters()["projection"].value, "perspective" )
				self.assertEqual( camera.blindData()["maya"]["aperture"].value, imath.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) )

				sel = maya.OpenMaya.MSelectionList()
				sel.add( "perspShape" )
				dag = maya.OpenMaya.MDagPath()
				sel.getDagPath( 0, dag )
				fn = maya.OpenMaya.MFnCamera( dag )
				self.assertAlmostEqual( camera.parameters()["projection:fov"].value, IECore.radiansToDegrees( fn.horizontalFieldOfView() ), 5 )

if __name__ == "__main__":
	IECoreMaya.TestProgram()
