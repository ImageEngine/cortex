##########################################################################
#
#  Copyright (c) 2012-2015, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreScene
import IECoreMaya
import unittest

import imath
import random

INCH_TO_MM = 25.400051

class ToMayaCameraConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		converter = IECoreMaya.ToMayaObjectConverter.create( IECoreScene.Camera() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.ToMayaCameraConverter.staticTypeId() ) )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.ToMayaCameraConverter ) ) )

	def assertMayaCamsEqual( self, camA, camB ) :

		self.assertAlmostEqual( maya.cmds.getAttr( camA+".nearClipPlane" ), maya.cmds.getAttr( camB+".nearClipPlane" ) )
		self.assertAlmostEqual( maya.cmds.getAttr( camA+".farClipPlane" ), maya.cmds.getAttr( camB+".farClipPlane" ) )
		self.assertEqual( maya.cmds.getAttr( camA+".orthographic" ), maya.cmds.getAttr( camB+".orthographic" ) )

		if maya.cmds.getAttr( camA+".orthographic" ):
			self.assertAlmostEqual( maya.cmds.getAttr( camA+".orthographicWidth" ), maya.cmds.getAttr( camB+".orthographicWidth" ) )
		else:
			self.assertAlmostEqual( maya.cmds.getAttr( camA+".horizontalFilmAperture" ), maya.cmds.getAttr( camB+".horizontalFilmAperture" ) )
			self.assertAlmostEqual( maya.cmds.getAttr( camA+".verticalFilmAperture" ), maya.cmds.getAttr( camB+".verticalFilmAperture" ) )
		self.assertAlmostEqual( maya.cmds.getAttr( camA+".horizontalFilmOffset" ), maya.cmds.getAttr( camB+".horizontalFilmOffset" ) )
		self.assertAlmostEqual( maya.cmds.getAttr( camA+".verticalFilmOffset" ), maya.cmds.getAttr( camB+".verticalFilmOffset" ) )

	def assertMayaCamsNotEqual( self, camA, camB ) :

		self.assertNotAlmostEqual( maya.cmds.getAttr( camA+".nearClipPlane" ), maya.cmds.getAttr( camB+".nearClipPlane" ) )
		self.assertNotAlmostEqual( maya.cmds.getAttr( camA+".farClipPlane" ), maya.cmds.getAttr( camB+".farClipPlane" ) )
		self.assertNotAlmostEqual( maya.cmds.getAttr( camA+".orthographic" ), maya.cmds.getAttr( camB+".orthographic" ) )

	def assertIECoreCamsAlmostEqual( self, camA, camB, names=False ) :

		self.assertEqual( camA.parameters().keys(), camB.parameters().keys() )

		for a in camA.parameters().keys():
			if type( camA.parameters()[a] ) == IECore.V2fData:
				self.assertTrue( camA.parameters()[a].value.equalWithRelError( camB.parameters()[a].value, 0.000001 ) )
			elif type( camA.parameters()[a] ) == float:
				self.assertAlmostEqual( camA.parameters()[a], camB.parameters()[a] )
			else:
				self.assertEqual( camA.parameters()[a], camB.parameters()[a] )


	def assertIECoreCamAndMayaCamEqual( self, coreCam, mayaCam ) :

		sel = maya.OpenMaya.MSelectionList()
		sel.add( mayaCam )
		dag = maya.OpenMaya.MDagPath()
		sel.getDagPath( 0, dag )
		fn = maya.OpenMaya.MFnCamera( dag )

		self.assertEqual( coreCam.getClippingPlanes(), imath.V2f( maya.cmds.getAttr( mayaCam+".nearClipPlane" ), maya.cmds.getAttr( mayaCam+".farClipPlane" ) ) )

		# If the focal length is less than 2.5, we can check the specific parameters for matching, since
		# Maya doesn't allow focal lengths less than 2.5, but we can still check that the frustum matches
		if coreCam.getFocalLength() >= 2.5:
			self.assertAlmostEqual( coreCam.getApertureOffset()[0], maya.cmds.getAttr( mayaCam+".horizontalFilmOffset" ) * INCH_TO_MM, places = 6 )
			self.assertAlmostEqual( coreCam.getApertureOffset()[1], maya.cmds.getAttr( mayaCam+".verticalFilmOffset" ) * INCH_TO_MM, places = 6 )


			if coreCam.parameters()["projection"].value == "perspective" :
				self.assertFalse( maya.cmds.getAttr( mayaCam+".orthographic" ) )
				self.assertAlmostEqual( coreCam.getFocalLength(), fn.focalLength() )
				self.assertAlmostEqual( coreCam.getAperture()[0], fn.horizontalFilmAperture() * INCH_TO_MM, places = 5 )
				self.assertAlmostEqual( coreCam.getAperture()[1], fn.verticalFilmAperture() * INCH_TO_MM, places = 5 )
			else :
				self.assertTrue( maya.cmds.getAttr( mayaCam+".orthographic" ) )
				self.assertEqual( coreCam.getAperture(), imath.V2f( maya.cmds.getAttr( mayaCam+".orthographicWidth" ) ) )

		if coreCam.getFStop() > 0.0:
			self.assertTrue( fn.isDepthOfField() )
			self.assertEqual( coreCam.getFStop(), fn.fStop() )
		else:
			self.assertFalse( fn.isDepthOfField() )

		self.assertEqual( coreCam.getFocusDistance(), fn.focusDistance() )

		# Check that the actual frustum computed by Maya matches the frustum computed by Cortex
		utils = [ maya.OpenMaya.MScriptUtil() for i in range(4)]
		utilPtrs = [ i.asDoublePtr() for i in utils ]
		fn.getFilmFrustum( 1, *utilPtrs )
		mayaFrustum = [ maya.OpenMaya.MScriptUtil.getDouble( i ) for i in utilPtrs ]

		cortexFrustum = coreCam.frustum( IECoreScene.Camera.FilmFit.Distort )
		self.assertAlmostEqual( mayaFrustum[0], cortexFrustum.size()[0], places = 5 )
		self.assertAlmostEqual( mayaFrustum[1], cortexFrustum.size()[1], places = 5 )

		# Note the ridiculous conversion factor because the offset returned by getFilmFrustum
		# isn't actually relative to the focalLength you have to pass in ( Thanks, Maya )
		self.assertAlmostEqual( mayaFrustum[2] * INCH_TO_MM / fn.focalLength(), cortexFrustum.center()[0], places = 6 )
		self.assertAlmostEqual( mayaFrustum[3] * INCH_TO_MM / fn.focalLength(), cortexFrustum.center()[1], places = 6 )


	def testExistingCam( self ) :

		maya.cmds.setAttr( "topShape.nearClipPlane", 1 )
		maya.cmds.setAttr( "topShape.farClipPlane", 1000 )
		maya.cmds.setAttr( "topShape.horizontalFilmAperture", 2 )
		maya.cmds.setAttr( "topShape.verticalFilmAperture", 1 )

		self.assertMayaCamsNotEqual( "perspShape", "topShape" )

		persp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		top = IECoreMaya.FromMayaCameraConverter( "topShape" ).convert()
		self.assertNotEqual( persp, top )

		self.failUnless( IECoreMaya.ToMayaCameraConverter( persp ).convert( "topShape" ) )
		self.assertMayaCamsEqual( "perspShape", "topShape" )
		self.assertIECoreCamAndMayaCamEqual( persp, "topShape" )

		newTop = IECoreMaya.FromMayaCameraConverter( "topShape" ).convert()
		self.assertEqual( persp.parameters(), newTop.parameters() )

	def testExistingCamParent( self ) :

		maya.cmds.setAttr( "frontShape.nearClipPlane", 1 )
		maya.cmds.setAttr( "frontShape.farClipPlane", 1000 )
		maya.cmds.setAttr( "frontShape.orthographicWidth", 2 )

		self.assertMayaCamsNotEqual( "frontShape", "perspShape" )

		front = IECoreMaya.FromMayaCameraConverter( "frontShape" ).convert()
		persp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		self.assertNotEqual( front, persp )

		self.failUnless( IECoreMaya.ToMayaCameraConverter( front ).convert( "persp" ) )
		self.assertMayaCamsEqual( "frontShape", "perspShape" )
		self.assertIECoreCamAndMayaCamEqual( front, "perspShape" )

		newPersp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		self.assertEqual( front.parameters(), newPersp.parameters() )

	def testNewCam( self ) :

		persp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()

		parent = maya.cmds.createNode( "transform" )
		self.failUnless( IECoreMaya.ToMayaCameraConverter( persp ).convert( parent ) )

		cams = maya.cmds.listRelatives( parent, children=True, fullPath=True, type="camera" )
		self.assertEqual( len(cams), 1 )
		self.assertMayaCamsEqual( "|persp|perspShape", cams[0] )
		self.assertIECoreCamAndMayaCamEqual( persp, cams[0] )

		newCam = IECoreMaya.FromMayaCameraConverter( cams[0] ).convert()
		self.assertEqual( persp.parameters(), newCam.parameters() )

	def testWrongIECoreObject( self ) :

		converter = IECoreMaya.ToMayaCameraConverter( IECoreScene.MeshPrimitive() )

		messageHandler = IECore.CapturingMessageHandler()
		with messageHandler :
			self.assertFalse( converter.convert( "topShape" ) )

		self.assertEqual( len( messageHandler.messages ), 1 )
		self.assertEqual( messageHandler.messages[0].level, IECore.MessageHandler.Level.Warning )
		self.assertEqual( messageHandler.messages[0].context, "ToMayaCameraConverter::doConversion" )

	def testWrongMayaNode( self ) :

		maya.cmds.polySphere( name="pSphere" )
		persp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		converter = IECoreMaya.ToMayaCameraConverter( persp )

		messageHandler = IECore.CapturingMessageHandler()
		with messageHandler :
			self.assertFalse( converter.convert( "pSphereShape" ) )

		self.assertEqual( len( messageHandler.messages ), 1 )
		self.assertEqual( messageHandler.messages[0].level, IECore.MessageHandler.Level.Warning )
		self.assertEqual( messageHandler.messages[0].context, "ToMayaCameraConverter::doConversion" )

	def testFilmOffset( self ) :
		for x in [ -0.5, -0.25, 0, 0.25, 0.5 ] :

			maya.cmds.setAttr( "perspShape.horizontalFilmOffset", x )
			self.assertNotAlmostEqual( maya.cmds.getAttr( "frontShape.horizontalFilmOffset" ), maya.cmds.getAttr( "perspShape.horizontalFilmOffset" ) )

			for y in [ -0.5, -0.25, 0, 0.25, 0.5 ] :

				maya.cmds.setAttr( "perspShape.verticalFilmOffset", y )
				self.assertNotAlmostEqual( maya.cmds.getAttr( "frontShape.verticalFilmOffset" ), maya.cmds.getAttr( "perspShape.verticalFilmOffset" ) )

				front = IECoreMaya.FromMayaCameraConverter( "frontShape" ).convert()
				persp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
				self.assertNotEqual( front, persp )

				self.failUnless( IECoreMaya.ToMayaCameraConverter( persp ).convert( "front" ) )
				self.assertMayaCamsEqual( "frontShape", "perspShape" )
				self.assertIECoreCamAndMayaCamEqual( persp, "frontShape" )

				newFront = IECoreMaya.FromMayaCameraConverter( "frontShape" ).convert()
				self.assertEqual( persp.parameters(), newFront.parameters() )

	def testRandomCameras( self ) :

		random.seed( 42 )
		for i in range( 40 ):
			randomCamera = maya.cmds.camera()[0]
			cortexCamera = IECoreScene.Camera()
			projection = "orthographic" if random.random() > 0.5 else "perspective"
			cortexCamera.setProjection( projection )
			cortexCamera.setAperture( imath.V2f( random.uniform( 1, 20 ), random.uniform( 1, 20 ) ) )
			cortexCamera.setApertureOffset( imath.V2f( random.uniform( -10, 10 ), random.uniform( -10, 10 ) ) )
			if projection != "orthographic":
				cortexCamera.setFocalLength( random.uniform( 2.5, 200 ) )

			cortexCamera.setClippingPlanes( random.uniform( 1, 1000 ) + imath.V2f( 0, random.uniform( 1, 1000 ) ) )

			cortexCamera.setFStop( random.uniform( 1, 10 ) )
			cortexCamera.setFocusDistance( random.uniform( 0, 1000 ) )

			if cortexCamera.getProjection() == "orthographic":
				# Remove settings which we know will fail on ortho cameras in Maya ( see test below )
				del cortexCamera.parameters()["apertureOffset"]

				# Only square apertures supported
				cortexCamera.setAperture( imath.V2f( cortexCamera.getAperture()[0] ) )

			self.failUnless( IECoreMaya.ToMayaCameraConverter( cortexCamera ).convert( randomCamera ) )
			self.assertIECoreCamAndMayaCamEqual( cortexCamera, randomCamera )
			roundTrip = IECoreMaya.FromMayaCameraConverter( randomCamera ).convert()
			self.assertIECoreCamsAlmostEqual( cortexCamera, roundTrip )

			maya.cmds.delete( randomCamera )

	# Maya doesn't allow ortho cameras with rectangular apertures or aperture offsets
	# That means that this test can never pass - it's just here to document that we can't successfully
	# round trip all cameras
	@unittest.expectedFailure
	def testUnrepresentable( self ) :

			testCamera = maya.cmds.camera()[0]
			cortexCamera = IECoreScene.Camera()
			cortexCamera.setProjection( "orthographic" )
			cortexCamera.setAperture( imath.V2f( 1, 2 ) )
			cortexCamera.setClippingPlanes( imath.V2f( 1, 100 ) )

			self.failUnless( IECoreMaya.ToMayaCameraConverter( cortexCamera ).convert( testCamera ) )
			self.assertIECoreCamAndMayaCamEqual( cortexCamera, testCamera )
			roundTrip = IECoreMaya.FromMayaCameraConverter( testCamera ).convert()
			self.assertIECoreCamsAlmostEqual( cortexCamera, roundTrip )

			cortexCamera.setApertureOffset( imath.V2f( 3, 4 ) )

			self.failUnless( IECoreMaya.ToMayaCameraConverter( cortexCamera ).convert( testCamera ) )
			self.assertIECoreCamAndMayaCamEqual( cortexCamera, testCamera )
			roundTrip = IECoreMaya.FromMayaCameraConverter( testCamera ).convert()
			self.assertIECoreCamsAlmostEqual( cortexCamera, roundTrip )

			maya.cmds.delete( testCamera )

	# For small focal lengths, we have to clamp focalLength to at least 2.5, and adjust aperture accordingly
	# ( Due to silly limitation of Maya )
	def testSmallFocalLengths( self ) :

		random.seed( 42 )
		for i in range( 40 ):
			randomCamera = maya.cmds.camera()[0]
			cortexCamera = IECoreScene.Camera()
			cortexCamera.setProjection( "perspective" )
			cortexCamera.setAperture( imath.V2f( random.uniform( 0.1, 0.5 ), random.uniform( 0.1, 0.5 ) ) )
			cortexCamera.setApertureOffset( imath.V2f( random.uniform( -0.5, 0.5 ), random.uniform( -0.5, 0.5 ) ) )
			cortexCamera.setFocalLength( random.uniform( 0.1, 1 ) )

			cortexCamera.setClippingPlanes( random.uniform( 1, 1000 ) + imath.V2f( 0, random.uniform( 1, 1000 ) ) )

			self.failUnless( IECoreMaya.ToMayaCameraConverter( cortexCamera ).convert( randomCamera ) )
			self.assertIECoreCamAndMayaCamEqual( cortexCamera, randomCamera )

			maya.cmds.delete( randomCamera )

	def testFocalLengthWorldScale( self ) :

			testCamera = maya.cmds.camera()[0]
			cortexCamera = IECoreScene.Camera()
			cortexCamera.setProjection( "perspective" )
			cortexCamera.setAperture( imath.V2f( 36, 24 ) )
			cortexCamera.setFocalLength( 35 )
			cortexCamera.setClippingPlanes( imath.V2f( 1, 100 ) )
			cortexCamera.setApertureOffset( imath.V2f( 0 ) )
			cortexCamera.setFocusDistance( 1 )

			self.failUnless( IECoreMaya.ToMayaCameraConverter( cortexCamera ).convert( testCamera ) )
			self.assertIECoreCamAndMayaCamEqual( cortexCamera, testCamera )
			roundTrip = IECoreMaya.FromMayaCameraConverter( testCamera ).convert()
			self.assertIECoreCamsAlmostEqual( cortexCamera, roundTrip )

			for scale in [ 1.0, 0.01 ]:
				cortexCamera.setFocalLengthWorldScale( scale )

				self.failUnless( IECoreMaya.ToMayaCameraConverter( cortexCamera ).convert( testCamera ) )

				sel = maya.OpenMaya.MSelectionList()
				sel.add( testCamera )
				dag = maya.OpenMaya.MDagPath()
				sel.getDagPath( 0, dag )
				fn = maya.OpenMaya.MFnCamera( dag )
				self.assertAlmostEqual( fn.focalLength(), cortexCamera.getFocalLength() * scale / 0.1, places = 6 )

				roundTrip = IECoreMaya.FromMayaCameraConverter( testCamera ).convert()
				self.assertAlmostEqual( roundTrip.getFocalLength(), cortexCamera.getFocalLength() * scale / 0.1, places = 6 )

				# Check that despite using different units for focalLength and aperture, we still get the
				# same frustum
				self.assertEqual( roundTrip.frustum(), cortexCamera.frustum() )

			maya.cmds.delete( testCamera )


if __name__ == "__main__":
	IECoreMaya.TestProgram()
