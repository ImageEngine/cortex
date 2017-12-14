##########################################################################
#
#  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

import appleseed
import imath

import IECore
import IECoreScene
import IECoreImage
import IECoreAppleseed

import AppleseedTest

class AttributeTest( AppleseedTest.TestCase ):

	def testVisbility( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "plane" ) )
		r.setAttribute( "as:visibility:camera", IECore.BoolData( False ) )
		r.setAttribute( "as:visibility:diffuse", IECore.BoolData( False ) )
		m.render( r )
		r.attributeEnd()

		ass = self._getMainAssembly( r )
		obj_instance = ass.assembly_instances()['plane_assembly_instance']
		params = obj_instance.get_parameters()

		self.failUnless( params['visibility']['camera'] == False )
		self.failUnless( params['visibility']['diffuse'] == False )

	def testShadingSamples( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		self._createDefaultShader( r )

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "plane1" ) )
		r.setAttribute( "as:shading_samples", IECore.IntData( 4 ) )
		m.render( r )
		r.attributeEnd()

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "plane2" ) )
		r.setAttribute( "as:shading_samples", IECore.IntData( 2 ) )
		m.render( r )
		r.attributeEnd()

		ass = self._getMainAssembly( r )
		sshader = ass.surface_shaders()[0]
		self.failUnless( sshader.get_parameters()['front_lighting_samples'] == 4 )
		self.failUnless( sshader.get_parameters()['back_lighting_samples'] == 4 )

		sshader = ass.surface_shaders()[1]
		self.failUnless( sshader.get_parameters()['front_lighting_samples'] == 2 )
		self.failUnless( sshader.get_parameters()['back_lighting_samples'] == 2 )

	def testAlphaMaps( self ) :

		r = IECoreAppleseed.Renderer()

		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )

		r.setOption( "as:cfg:shading_engine:override_shading:mode", IECore.StringData( "object_instances" ) )

		with IECoreScene.WorldBlock( r ) :

			r.attributeBegin()
			r.setAttribute( "name", IECore.StringData( "plane" ) )
			r.setAttribute( "as:alpha_map", IECore.StringData( os.path.dirname( __file__ ) + "/data/textures/leaf.exr" ) )
			r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -5 ) ) )
			IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -4 ), imath.V2f( 4 ) ) ).render( r )
			r.attributeEnd()

		del r

		image = IECoreImage.ImageDisplayDriver.removeStoredImage( "testHandle" )
		expectedImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/data/referenceImages/expectedAlphaMaps.exr" ).read()

		for channel in ( "R", "G", "B" ) :
			# Appleseed's default shading appears to have changed since
			# expectedAlphaMaps.exr was created, so since we're only interested
			# in the alpha channel, we delete the other channels before comparing
			# the images.
			del image[channel]
			del expectedImage[channel]
		self.assertIn( "A", image )

		self.failIf( IECoreImage.ImageDiffOp()( imageA=image, imageB=expectedImage, maxError=0.003 ).value )

	def testMediumPriorities( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		self._createDefaultShader( r )

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "plane1" ) )
		r.setAttribute( "as:medium_priority", IECore.IntData( 4 ) )
		m.render( r )
		r.attributeEnd()

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "plane2" ) )
		r.setAttribute( "as:medium_priority", IECore.IntData( 2 ) )
		m.render( r )
		r.attributeEnd()

		ass = self._getMainAssembly( r )

		self.failUnless( len( ass.assemblies() ) == 2 )
		self.failUnless( len( ass.assembly_instances() ) == 2 )

		objAss = ass.assemblies()["plane1_assembly"]
		objInst = objAss.object_instances()[0]
		self.failUnless( objInst.get_parameters()['medium_priority'] == 4 )

		objAss = ass.assemblies()["plane2_assembly"]
		objInst = objAss.object_instances()[0]
		self.failUnless( objInst.get_parameters()['medium_priority'] == 2 )

	def testNestedAttributeBlock( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "object_name" ) )
		r.setAttribute( "as:medium_priority", IECore.IntData( 7 ) )
		r.attributeBegin()

		self.failUnless( r.getAttribute( "name" ) == IECore.StringData( "object_name" ) )
		self.failUnless( r.getAttribute( "as:medium_priority" ) == IECore.IntData( 7 ) )

if __name__ == "__main__":
	unittest.main()
