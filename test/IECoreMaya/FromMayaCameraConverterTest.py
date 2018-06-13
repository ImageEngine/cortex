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

import IECore
import IECoreScene
import IECoreMaya

import imath

INCH_TO_MM = 25.400051

class FromMayaCameraConverterTest( IECoreMaya.TestCase ) :
	def assertIECoreCamAndMayaCamEqual( self, coreCam, mayaCam ) :

		self.assertEqual( coreCam.parameters()["clippingPlanes"].value, imath.V2f( maya.cmds.getAttr( mayaCam+".nearClipPlane" ), maya.cmds.getAttr( mayaCam+".farClipPlane" ) ) )
		self.assertEqual( coreCam.getApertureOffset(), imath.V2f( maya.cmds.getAttr( mayaCam+".horizontalFilmOffset" ), maya.cmds.getAttr( mayaCam+".verticalFilmOffset" ) ) * INCH_TO_MM )

		sel = maya.OpenMaya.MSelectionList()
		sel.add( mayaCam )
		dag = maya.OpenMaya.MDagPath()
		sel.getDagPath( 0, dag )
		fn = maya.OpenMaya.MFnCamera( dag )

		if coreCam.parameters()["projection"].value == "perspective" :
			self.assertFalse( maya.cmds.getAttr(  mayaCam+".orthographic" ) )
			self.assertAlmostEqual( coreCam.getFocalLength(), fn.focalLength() )
			self.assertAlmostEqual( coreCam.getAperture()[0], fn.horizontalFilmAperture() * INCH_TO_MM, places = 6 )
			self.assertAlmostEqual( coreCam.getAperture()[1], fn.verticalFilmAperture() * INCH_TO_MM, places = 6 )
		else :
			self.assertTrue( maya.cmds.getAttr(  mayaCam+".orthographic" ) )
			self.assertEqual( coreCam.getAperture(), imath.V2f( maya.cmds.getAttr( mayaCam+".orthographicWidth" ) ) )

		# Check that the actual frustum computed by Maya matches the frustum computed by Cortex
		utils = [ maya.OpenMaya.MScriptUtil() for i in range(4)]
		utilPtrs = [ i.asDoublePtr() for i in utils ]
		fn.getFilmFrustum( 1, *utilPtrs )
		mayaFrustum = [ maya.OpenMaya.MScriptUtil.getDouble( i ) for i in utilPtrs ]

		cortexFrustum = coreCam.frustum( IECoreScene.Camera.FilmFit.Distort )
		self.assertAlmostEqual( mayaFrustum[0], cortexFrustum.size()[0], places = 6 )
		self.assertAlmostEqual( mayaFrustum[1], cortexFrustum.size()[1], places = 6 )

		# Note the ridiculous conversion factor because the offset returned by getFilmFrustum
		# isn't actually relative to the focalLength you have to pass in ( Thanks, Maya )
		self.assertAlmostEqual( mayaFrustum[2] * INCH_TO_MM / fn.focalLength(), cortexFrustum.center()[0] )
		self.assertAlmostEqual( mayaFrustum[3] * INCH_TO_MM / fn.focalLength(), cortexFrustum.center()[1] )


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
		self.assertIECoreCamAndMayaCamEqual( camera, "perspShape" )

	def testConstructor( self ) :

		converter = IECoreMaya.FromMayaCameraConverter( "perspShape" )
		camera = converter.convert()
		self.assert_( camera.isInstanceOf( IECoreScene.Camera.staticTypeId() ) )

	def testPerspective( self ) :

		camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()

		self.assertFalse( camera.hasResolution() )
		self.assertEqual( camera.getClippingPlanes(), imath.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
		self.assertEqual( camera.getProjection(), "perspective" )
		self.assertEqual( camera.getAperture(), imath.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) * INCH_TO_MM )
		self.assertAlmostEqual( camera.getFocalLength(), maya.cmds.getAttr( "perspShape.focalLength" ) )
		self.assertIECoreCamAndMayaCamEqual( camera, "perspShape" )

	def testOrthographic( self ) :

		camera = IECoreMaya.FromMayaCameraConverter( "topShape" ).convert()

		self.assertFalse( camera.hasResolution() )
		self.assertEqual( camera.getClippingPlanes(), imath.V2f( maya.cmds.getAttr( "topShape.nearClipPlane" ), maya.cmds.getAttr( "topShape.farClipPlane" ) ) )
		self.assertEqual( camera.getProjection(), "orthographic" )
		self.assertEqual( camera.getAperture(), imath.V2f( maya.cmds.getAttr( "topShape.orthographicWidth" ) ) )
		self.assertIECoreCamAndMayaCamEqual( camera, "topShape" )

	def testOverrideResolution( self ) :

		camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		self.assertFalse( camera.hasResolution() )
		self.assertFalse( camera.hasFilmFit() )
		self.assertFalse( camera.hasPixelAspectRatio() )

		maya.cmds.addAttr( "perspShape", ln="ieCamera_overrideResolution", at="long2" )
		maya.cmds.addAttr( "perspShape", ln="ieCamera_overrideResolutionX", at="long", p = "ieCamera_overrideResolution", dv = 1024 )
		maya.cmds.addAttr( "perspShape", ln="ieCamera_overrideResolutionY", at="long", p = "ieCamera_overrideResolution", dv = 778 )
		camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		self.assertTrue( camera.hasResolution() )
		self.assertEqual( camera.getResolution(), imath.V2i( 1024, 778 ) )
		self.assertFalse( camera.hasPixelAspectRatio() )
		self.assertFalse( camera.hasFilmFit() )

		maya.cmds.deleteAttr( "perspShape", attribute= "ieCamera_overrideResolution" )
		maya.cmds.addAttr( "perspShape", ln= "ieCamera_overridePixelAspectRatio", at="float", dv=2.0 )
		camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		self.assertFalse( camera.hasResolution() )
		self.assertTrue( camera.hasPixelAspectRatio() )
		self.assertEqual( camera.getPixelAspectRatio(), 2.0 )
		self.assertFalse( camera.hasFilmFit() )

		maya.cmds.deleteAttr( "perspShape", attribute= "ieCamera_overridePixelAspectRatio" )
		fitModeNames = IECoreScene.Camera.FilmFit.names.keys()
		maya.cmds.addAttr( "perspShape", ln= "ieCamera_overrideFilmFit", at="enum", en=":".join( fitModeNames ) )


		for i in range( len( fitModeNames ) ):
			maya.cmds.setAttr( "perspShape.ieCamera_overrideFilmFit", i )
			camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
			self.assertFalse( camera.hasResolution() )
			self.assertFalse( camera.hasPixelAspectRatio() )
			self.assertTrue( camera.hasFilmFit() )
			self.assertEqual( camera.getFilmFit(), IECoreScene.Camera.FilmFit.names[ fitModeNames[i] ] )

		maya.cmds.deleteAttr( "perspShape", attribute= "ieCamera_overrideFilmFit" )
		camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		self.assertFalse( camera.hasResolution() )
		self.assertFalse( camera.hasFilmFit() )
		self.assertFalse( camera.hasPixelAspectRatio() )

	def testFilmOffset( self ) :

		for x in [ -0.5, -0.25, 0, 0.25, 0.5 ] :

			for y in [ -0.5, -0.25, 0, 0.25, 0.5 ] :

				maya.cmds.setAttr( "perspShape.horizontalFilmOffset", x )
				maya.cmds.setAttr( "perspShape.verticalFilmOffset", y )
				camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()

				self.assertFalse( camera.hasResolution() )
				self.assertEqual( camera.getClippingPlanes(), imath.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
				self.assertEqual( camera.getProjection(), "perspective" )
				self.assertEqual( camera.getAperture(), INCH_TO_MM * imath.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) )
				self.assertEqual( camera.getApertureOffset(), INCH_TO_MM * imath.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmOffset" ), maya.cmds.getAttr( "perspShape.verticalFilmOffset" ) ) )
				self.assertIECoreCamAndMayaCamEqual( camera, "perspShape" )


if __name__ == "__main__":
	IECoreMaya.TestProgram()
