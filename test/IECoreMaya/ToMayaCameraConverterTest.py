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
import imath

import IECore
import IECoreScene
import IECoreMaya

class ToMayaCameraConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		converter = IECoreMaya.ToMayaObjectConverter.create( IECoreScene.Camera() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.ToMayaCameraConverter.staticTypeId() ) )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.ToMayaCameraConverter ) ) )

	def assertMayaCamsEqual( self, camA, camB ) :

		self.assertAlmostEqual( maya.cmds.getAttr( camA+".nearClipPlane" ), maya.cmds.getAttr( camB+".nearClipPlane" ) )
		self.assertAlmostEqual( maya.cmds.getAttr( camA+".farClipPlane" ), maya.cmds.getAttr( camB+".farClipPlane" ) )
		self.assertAlmostEqual( maya.cmds.getAttr( camA+".horizontalFilmAperture" ), maya.cmds.getAttr( camB+".horizontalFilmAperture" ) )
		self.assertAlmostEqual( maya.cmds.getAttr( camA+".verticalFilmAperture" ), maya.cmds.getAttr( camB+".verticalFilmAperture" ) )
		self.assertAlmostEqual( maya.cmds.getAttr( camA+".horizontalFilmOffset" ), maya.cmds.getAttr( camB+".horizontalFilmOffset" ) )
		self.assertAlmostEqual( maya.cmds.getAttr( camA+".verticalFilmOffset" ), maya.cmds.getAttr( camB+".verticalFilmOffset" ) )
		self.assertEqual( maya.cmds.getAttr( camA+".orthographic" ), maya.cmds.getAttr( camB+".orthographic" ) )
		matA = maya.cmds.getAttr( camA+".worldMatrix[0]" )
		matB = maya.cmds.getAttr( camB+".worldMatrix[0]" )
		for i in range( 0, 16 ) :
			self.assertAlmostEqual( matA[i], matB[i], 5 )

	def assertMayaCamsNotEqual( self, camA, camB ) :

		self.assertNotAlmostEqual( maya.cmds.getAttr( camA+".nearClipPlane" ), maya.cmds.getAttr( camB+".nearClipPlane" ) )
		self.assertNotAlmostEqual( maya.cmds.getAttr( camA+".farClipPlane" ), maya.cmds.getAttr( camB+".farClipPlane" ) )
		self.assertNotAlmostEqual( maya.cmds.getAttr( camA+".horizontalFilmAperture" ), maya.cmds.getAttr( camB+".horizontalFilmAperture" ) )
		self.assertNotAlmostEqual( maya.cmds.getAttr( camA+".verticalFilmAperture" ), maya.cmds.getAttr( camB+".verticalFilmAperture" ) )
		self.assertNotAlmostEqual( maya.cmds.getAttr( camA+".orthographic" ), maya.cmds.getAttr( camB+".orthographic" ) )
		self.assertNotEqual( maya.cmds.getAttr( camA+".worldMatrix[0]" ), maya.cmds.getAttr( camB+".worldMatrix[0]" ) )

	def assertIECoreCamsEqual( self, camA, camB, names=False ) :

		self.assertEqual( camA.blindData()["maya"], camB.blindData()["maya"] )
		self.assertEqual( camA.parameters(), camB.parameters() )
		if not names :
			camB.blindData()["name"] = IECore.StringData( camA.getName() )
			camB.setName( camA.getName() )
		self.assertEqual( camA, camB )

	def assertIECoreCamsNotEqual( self, camA, camB ) :

		self.assertNotEqual( camA.blindData()["maya"], camB.blindData()["maya"] )
		self.assertNotEqual( camA.parameters(), camB.parameters() )

	def assertIECoreCamAndMayaCamEqual( self, coreCam, mayaCam ) :

		self.assertEqual( coreCam.getTransform().transform(), imath.M44f( *maya.cmds.getAttr( mayaCam+".worldMatrix[0]" ) ) )
		self.assertEqual( coreCam.parameters()["clippingPlanes"].value, imath.V2f( maya.cmds.getAttr(  mayaCam+".nearClipPlane" ), maya.cmds.getAttr(  mayaCam+".farClipPlane" ) ) )
		self.assertEqual( coreCam.blindData()["maya"]["aperture"].value, imath.V2f( maya.cmds.getAttr(  mayaCam+".horizontalFilmAperture" ), maya.cmds.getAttr(  mayaCam+".verticalFilmAperture" ) ) )
		self.assertEqual( coreCam.blindData()["maya"]["filmOffset"].value, imath.V2f( maya.cmds.getAttr(  mayaCam+".horizontalFilmOffset" ), maya.cmds.getAttr(  mayaCam+".verticalFilmOffset" ) ) )

		if coreCam.parameters()["projection"].value == "perspective" :
			self.assertFalse( maya.cmds.getAttr(  mayaCam+".orthographic" ) )
			sel = maya.OpenMaya.MSelectionList()
			sel.add( mayaCam )
			dag = maya.OpenMaya.MDagPath()
			sel.getDagPath( 0, dag )
			fn = maya.OpenMaya.MFnCamera( dag )
			self.assertAlmostEqual( coreCam.parameters()["projection:fov"].value, IECore.radiansToDegrees( fn.horizontalFieldOfView() ), 5 )
		else :
			self.assertTrue( maya.cmds.getAttr(  mayaCam+".orthographic" ) )
			self.assertEqual( coreCam.parameters()["screenWindow"].value.max().x - coreCam.parameters()["screenWindow"].value.min().x, maya.cmds.getAttr( mayaCam+".orthographicWidth" ) )

	def testExistingCam( self ) :

		maya.cmds.setAttr( "topShape.nearClipPlane", 1 )
		maya.cmds.setAttr( "topShape.farClipPlane", 1000 )
		maya.cmds.setAttr( "topShape.horizontalFilmAperture", 2 )
		maya.cmds.setAttr( "topShape.verticalFilmAperture", 1 )

		self.assertMayaCamsNotEqual( "perspShape", "topShape" )

		persp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		top = IECoreMaya.FromMayaCameraConverter( "topShape" ).convert()
		self.assertIECoreCamsNotEqual( persp, top )

		self.failUnless( IECoreMaya.ToMayaCameraConverter( persp ).convert( "topShape" ) )
		self.assertMayaCamsEqual( "perspShape", "topShape" )
		self.assertIECoreCamAndMayaCamEqual( persp, "topShape" )

		newTop = IECoreMaya.FromMayaCameraConverter( "topShape" ).convert()
		self.assertIECoreCamsEqual( persp, newTop )

	def testExistingCamParent( self ) :

		maya.cmds.setAttr( "frontShape.nearClipPlane", 1 )
		maya.cmds.setAttr( "frontShape.farClipPlane", 1000 )
		maya.cmds.setAttr( "frontShape.horizontalFilmAperture", 2 )
		maya.cmds.setAttr( "frontShape.verticalFilmAperture", 1 )

		self.assertMayaCamsNotEqual( "frontShape", "perspShape" )

		front = IECoreMaya.FromMayaCameraConverter( "frontShape" ).convert()
		persp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		self.assertIECoreCamsNotEqual( front, persp )

		self.failUnless( IECoreMaya.ToMayaCameraConverter( front ).convert( "persp" ) )
		self.assertMayaCamsEqual( "frontShape", "perspShape" )
		self.assertIECoreCamAndMayaCamEqual( front, "perspShape" )

		newPersp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		self.assertIECoreCamsEqual( front, newPersp )

	def testNewCam( self ) :

		persp = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()

		parent = maya.cmds.createNode( "transform" )
		self.failUnless( IECoreMaya.ToMayaCameraConverter( persp ).convert( parent ) )

		cams = maya.cmds.listRelatives( parent, children=True, fullPath=True, type="camera" )
		self.assertEqual( len(cams), 1 )
		self.assertEqual( cams[0].split( "|" )[-1], "perspShape" )
		self.assertMayaCamsEqual( "|persp|perspShape", cams[0] )
		self.assertIECoreCamAndMayaCamEqual( persp, cams[0] )

		newCam = IECoreMaya.FromMayaCameraConverter( cams[0] ).convert()
		self.assertIECoreCamsEqual( persp, newCam, names=True )

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
				self.assertIECoreCamsNotEqual( front, persp )

				self.failUnless( IECoreMaya.ToMayaCameraConverter( persp ).convert( "front" ) )
				self.assertMayaCamsEqual( "frontShape", "perspShape" )
				self.assertIECoreCamAndMayaCamEqual( persp, "frontShape" )

				newFront = IECoreMaya.FromMayaCameraConverter( "frontShape" ).convert()
				self.assertIECoreCamsEqual( persp, newFront )

if __name__ == "__main__":
	IECoreMaya.TestProgram()
