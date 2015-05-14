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
import IECoreMaya

class FromMayaCameraConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :
	
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape" )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaCameraConverter.staticTypeId() ) )
				
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape", IECore.Camera.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaCameraConverter.staticTypeId() ) )
		
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape", IECore.Renderable.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaCameraConverter.staticTypeId() ) )
		
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape", IECore.Writer.staticTypeId() )
		self.assertEqual( converter, None )
		
	def test( self ) :

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "perspShape" )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaCameraConverter ) ) )

		camera = converter.convert()
		self.assert_( camera.isInstanceOf( IECore.Camera.staticTypeId() ) )
		
	def testConstructor( self ) :
	
		converter = IECoreMaya.FromMayaCameraConverter( "perspShape" )
		camera = converter.convert()
		self.assert_( camera.isInstanceOf( IECore.Camera.staticTypeId() ) )	
		
	def testPerspective( self ) :
		
		camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
		
		self.assertEqual( camera.getName(), "perspShape" )
		self.assertEqual( camera.getTransform().transform(), IECore.M44f( maya.cmds.getAttr( "persp.worldMatrix[0]" ) ) )
		self.assertEqual( camera.parameters()["resolution"].value, IECore.V2i( maya.cmds.getAttr( "defaultResolution.width" ), maya.cmds.getAttr( "defaultResolution.height" ) ) )
		self.assertEqual( camera.parameters()["clippingPlanes"].value, IECore.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
		self.assertEqual( camera.parameters()["projection"].value, "perspective" )
		self.assertEqual( camera.blindData()["maya"]["aperture"].value, IECore.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) )
		
		sel = maya.OpenMaya.MSelectionList()
		sel.add( "perspShape" )
		dag = maya.OpenMaya.MDagPath()
		sel.getDagPath( 0, dag )
		fn = maya.OpenMaya.MFnCamera( dag )
		self.assertAlmostEqual( camera.parameters()["projection:fov"].value, IECore.radiansToDegrees( fn.horizontalFieldOfView() ), 5 )
	
	def testOrthographic( self ) :
		
		camera = IECoreMaya.FromMayaCameraConverter( "topShape" ).convert()
		
		self.assertEqual( camera.getName(), "topShape" )
		self.assertEqual( camera.getTransform().transform(), IECore.M44f( maya.cmds.getAttr( "top.worldMatrix[0]" ) ) )
		self.assertEqual( camera.parameters()["resolution"].value, IECore.V2i( maya.cmds.getAttr( "defaultResolution.width" ), maya.cmds.getAttr( "defaultResolution.height" ) ) )
		self.assertEqual( camera.parameters()["clippingPlanes"].value, IECore.V2f( maya.cmds.getAttr( "topShape.nearClipPlane" ), maya.cmds.getAttr( "topShape.farClipPlane" ) ) )
		self.assertEqual( camera.parameters()["projection"].value, "orthographic" )
		self.assertEqual( camera.parameters()["screenWindow"].value.max.x - camera.parameters()["screenWindow"].value.min.x, maya.cmds.getAttr( "topShape.orthographicWidth" ) )
		self.assertEqual( camera.blindData()["maya"]["aperture"].value, IECore.V2f( maya.cmds.getAttr( "topShape.horizontalFilmAperture" ), maya.cmds.getAttr( "topShape.verticalFilmAperture" ) ) )
	
	def testCustomResolution( self ) :
		
		converter = IECoreMaya.FromMayaCameraConverter( "perspShape" )
		converter.parameters()["resolutionMode"].setValue( "specified" )
		converter.parameters()["resolution"].setValue( "1K" )
		camera = converter.convert()
		
		self.assertEqual( camera.getName(), "perspShape" )
		self.assertEqual( camera.getTransform().transform(), IECore.M44f( maya.cmds.getAttr( "persp.worldMatrix[0]" ) ) )
		self.assertEqual( camera.parameters()["resolution"].value, IECore.V2i( 1024, 778 ) )
		self.assertEqual( camera.parameters()["clippingPlanes"].value, IECore.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
		self.assertEqual( camera.parameters()["projection"].value, "perspective" )
		self.assertEqual( camera.blindData()["maya"]["aperture"].value, IECore.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) )
		
		sel = maya.OpenMaya.MSelectionList()
		sel.add( "perspShape" )
		dag = maya.OpenMaya.MDagPath()
		sel.getDagPath( 0, dag )
		fn = maya.OpenMaya.MFnCamera( dag )
		self.assertAlmostEqual( camera.parameters()["projection:fov"].value, IECore.radiansToDegrees( fn.horizontalFieldOfView() ), 5 )
	
	def testFilmOffset( self ) :
		
		for x in [ -0.5, -0.25, 0, 0.25, 0.5 ] :
			
			for y in [ -0.5, -0.25, 0, 0.25, 0.5 ] :
				
				maya.cmds.setAttr( "perspShape.horizontalFilmOffset", x )
				maya.cmds.setAttr( "perspShape.verticalFilmOffset", y )
				camera = IECoreMaya.FromMayaCameraConverter( "perspShape" ).convert()
				
				self.assertEqual( camera.getName(), "perspShape" )
				self.assertEqual( camera.getTransform().transform(), IECore.M44f( maya.cmds.getAttr( "persp.worldMatrix[0]" ) ) )
				self.assertEqual( camera.parameters()["resolution"].value, IECore.V2i( maya.cmds.getAttr( "defaultResolution.width" ), maya.cmds.getAttr( "defaultResolution.height" ) ) )
				self.assertEqual( camera.parameters()["clippingPlanes"].value, IECore.V2f( maya.cmds.getAttr( "perspShape.nearClipPlane" ), maya.cmds.getAttr( "perspShape.farClipPlane" ) ) )
				self.assertEqual( camera.parameters()["projection"].value, "perspective" )
				self.assertEqual( camera.blindData()["maya"]["aperture"].value, IECore.V2f( maya.cmds.getAttr( "perspShape.horizontalFilmAperture" ), maya.cmds.getAttr( "perspShape.verticalFilmAperture" ) ) )
				
				sel = maya.OpenMaya.MSelectionList()
				sel.add( "perspShape" )
				dag = maya.OpenMaya.MDagPath()
				sel.getDagPath( 0, dag )
				fn = maya.OpenMaya.MFnCamera( dag )
				self.assertAlmostEqual( camera.parameters()["projection:fov"].value, IECore.radiansToDegrees( fn.horizontalFieldOfView() ), 5 )

if __name__ == "__main__":
	IECoreMaya.TestProgram()
