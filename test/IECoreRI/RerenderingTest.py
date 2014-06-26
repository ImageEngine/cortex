##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import os
import unittest
import time
import gc

import weakref

import IECore
import IECoreRI
				
class RerenderingTest( unittest.TestCase ) :
	
	def testEditLight( self ) :
		
		# start an editable render with one light colour
		
		r = IECoreRI.Renderer( "" )
		
		r.setOption( "editable", True )
		
		r.display( "test", "ie", "rgba",
			{
				"driverType" : IECore.StringData( "ImageDisplayDriver" ),
				"handle" : IECore.StringData( "myLovelySphere" ),
				"quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		with IECore.WorldBlock( r ) :
		
			r.light(
				"pointlight",
				"myLovelyLight",
				{
					"lightcolor" : IECore.Color3f( 1, 0.5, 0.25 ),
				}
			)
					
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
	
			r.shader( "surface", "matte", {} )
			r.sphere( 1, -1, 1, 360, {} )
				
		# give it a bit of time to finish
		
		time.sleep( 1 )
		
		# check we get the colour we expected
		
		i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
		e = IECore.ImagePrimitiveEvaluator( i )
		er = e.createResult()

		e.pointAtUV( IECore.V2f( 0.5 ), er )
		c = IECore.Color3f( er.floatPrimVar( i["R"] ), er.floatPrimVar( i["G"] ), er.floatPrimVar( i["B"] ) )
		self.assertEqual( c / c[0], IECore.Color3f( 1, 0.5, 0.25 ) )
		
		# make an edit to the light color and check the colour has changed
		
		r.editBegin( "light", {} )
		
		r.light(
				"pointlight",
				"myLovelyLight",
				{
					"lightcolor" : IECore.Color3f( 0.25, 0.5, 1 ),
				}
			)
		
		r.editEnd()
		
		time.sleep( 1 )
		
		i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
		e = IECore.ImagePrimitiveEvaluator( i )
		er = e.createResult()
		
		c = IECore.Color3f( er.floatPrimVar( i["R"] ), er.floatPrimVar( i["G"] ), er.floatPrimVar( i["B"] ) )
		self.assertEqual( c / c[2], IECore.Color3f( 0.25, 0.5, 1 ) )
	
	def testEditShader( self ) :
	
		# start an editable render with one light colour
		
		r = IECoreRI.Renderer( "" )
		
		r.setOption( "editable", True )
		
		r.display( "test", "ie", "rgba",
			{
				"driverType" : IECore.StringData( "ImageDisplayDriver" ),
				"handle" : IECore.StringData( "myLovelySphere" ),
				"quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		with IECore.WorldBlock( r ) :
		
			r.light( "pointlight", "myLovelyLight", {} )
			
			with IECore.AttributeBlock( r ) :
			
				r.setAttribute( "name", "/sphere" )
			
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
	
				r.shader( "surface", "matte", { "Kd" : 1 } )
				
				r.sphere( 1, -1, 1, 360, {} )
				
		# give it a bit of time to finish
		
		time.sleep( 1 )
		
		# record the colour produced by the first render
		
		i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
		e = IECore.ImagePrimitiveEvaluator( i )
		er = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5 ), er )
		initialColor = IECore.Color3f( er.floatPrimVar( i["R"] ), er.floatPrimVar( i["G"] ), er.floatPrimVar( i["B"] ) )
		
		# make an edit to the shader and wait for it to take hold
		
		r.editBegin( "attribute", { "scopename" : "/sphere" } )
		r.shader( "surface", "matte", { "Kd" : 0.5 } )
		r.editEnd()
		time.sleep( 1 )
		
		# check the ratio of the two colours is as expected.
		
		i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
		e = IECore.ImagePrimitiveEvaluator( i )
		er = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5 ), er )
		newColor = IECore.Color3f( er.floatPrimVar( i["R"] ), er.floatPrimVar( i["G"] ), er.floatPrimVar( i["B"] ) )
		
		self.assertEqual( newColor / initialColor, IECore.Color3f( .5 ) )
	
	
	def testRenderStop( self ) :
		
		# Tests a bug where it was impossible to stop an ipr render involving a procedural.
		
		# The bug happened because in ipr mode, 3delight was keeping hold of a "ProceduralData" structure
		# containing the renderer's RendererImplementation and the procedural, until the IPR
		# render was terminated with a call to RiEnd().
		
		# Unfortunately, the only way of calling RiEnd() in IECoreRI is to delete the renderer
		# by reducing its reference count to zero, thereby destroying its implementation. This
		# was impossible to do because 3delight was keeping hold of a reference to it.
		
		# We fixed this by removing the RendererImplementation from the ProceduralData structure and
		# putting other data there instead...
		
		# We're going to excercise this by launching an IPR render of a procedural, then making
		# sure that deleting the renderer kills the procedural. This impliess that the renderer
		# has been successfully killed, which has called RiEnd() and released the contents of
		# the ProceduralData structure.
		
		IECore.initThreads()
		
		r = IECoreRI.Renderer( "" )
		
		r.setOption( "editable", True )
		
		r.display( "test", "ie", "rgba",
			{
				"driverType" : IECore.StringData( "ImageDisplayDriver" ),
				"handle" : IECore.StringData( "myLovelySphere" ),
				"quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		class BlahProcedural( IECore.Renderer.Procedural ) :
			
			def __init__( self ) :

				IECore.Renderer.Procedural.__init__( self )

			def bound( self ) :

				return IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 10 ) )
			
			def render( self, renderer ) :
				renderer.sphere( 1, -1, 1, 360, {} )

			def hash( self ) :
				return IECore.MurmurHash()

		
		with IECore.WorldBlock( r ) :
		
			with IECore.AttributeBlock( r ) :
			
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
				p = BlahProcedural()
				r.procedural( p )
		
		# give it time to finish
		time.sleep( 1 )
		
		# now try and kill a bunch of stuff...
		procref = weakref.ref( p )
		del p
		del r
		
		while gc.collect():
			pass
		
		IECore.RefCounted.collectGarbage()
		
		# and check we've actually killed it:
		self.failUnless( procref() is None )

	def testEditCamera( self ) :
		
		# start an editable render with the camera in one spot
		
		r = IECoreRI.Renderer( "" )
		
		r.setOption( "editable", True )
		
		r.display( "test", "ie", "rgba",
			{
				"driverType" : IECore.StringData( "ImageDisplayDriver" ),
				"handle" : IECore.StringData( "myLovelySphere" ),
				"quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		camera = IECore.Camera(
			"main",
			None,
			{
				"projection" : "orthographic",
				"resolution" : IECore.V2i( 512 ),
				"screenWindow" : IECore.Box2f( IECore.V2f( -0.5 ), IECore.V2f( 0.5 ) ),
			}
		)
		
		with IECore.TransformBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, 5 ) ) )
			camera.render( r )
			
		with IECore.WorldBlock( r ) :
			r.sphere( 0.08, -1, 1, 360, {} )
		
		# give it a bit of time to finish
		
		time.sleep( 2 )
		
		# check we get the sphere where we expected

		def checkResults( results ) :
		
			i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
			
			e = IECore.ImagePrimitiveEvaluator( i )
			er = e.createResult()
			
			for position, value in results :
				e.pointAtUV( position, er )
				self.assertEqual( er.floatPrimVar( i["A"] ), value )
		
		checkResults( [
			( IECore.V2f( 0.5, 0.5 ), 1 ),
			( IECore.V2f( 0.6, 0.5 ), 0 ),
			( IECore.V2f( 0.4, 0.5 ), 0 ),
		] )
		
		# move the camera and check the sphere has moved
		
		r.editBegin( "option", {} )
		
		with IECore.TransformBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0.1, 0, 5 ) ) )
			camera.render( r )
		
		r.editEnd()
		
		time.sleep( 2 )
		
		checkResults( [
			( IECore.V2f( 0.4, 0.5 ), 1 ),
			( IECore.V2f( 0.5, 0.5 ), 0 ),
			( IECore.V2f( 0.3, 0.5 ), 0 ),
		] )
		
if __name__ == "__main__":
    unittest.main()
