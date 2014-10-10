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

from __future__ import with_statement

import os
import unittest

import arnold

import IECore
import IECoreArnold

class RendererTest( unittest.TestCase ) :

	__displayFileName = "contrib/IECoreArnold/test/IECoreArnold/output.tif"
	__assFileName = "contrib/IECoreArnold/test/IECoreArnold/output.ass"
	
	def testTypeId( self ) :

		self.assertEqual( IECoreArnold.Renderer().typeId(), IECoreArnold.Renderer.staticTypeId() )
		self.assertNotEqual( IECoreArnold.Renderer.staticTypeId(), IECore.Renderer.staticTypeId() )

	def testTypeName( self ) :

		r = IECoreArnold.Renderer()
		self.assertEqual( r.typeName(), "IECoreArnold::Renderer" )

	def testOptions( self ) :
	
		r = IECoreArnold.Renderer()
		
		# check we can set an already existing int
		self.assertEqual( r.getOption( "ai:AA_samples" ), IECore.IntData( 1 ) )
		r.setOption( "ai:AA_samples", IECore.IntData( 11 ) )
		self.assertEqual( r.getOption( "ai:AA_samples" ), IECore.IntData( 11 ) )
		
		# check we can set an already existing float
		self.assertEqual( r.getOption( "ai:auto_transparency_threshold" ), IECore.FloatData( .99 ) )
		r.setOption( "ai:auto_transparency_threshold", IECore.FloatData( .9 ) )
		self.assertEqual( r.getOption( "ai:auto_transparency_threshold" ), IECore.FloatData( .9 ) )
		
		# check tbat trying to set nonexistent options yields a message
		m = IECore.CapturingMessageHandler()
		with m :
		
			r.setOption( "ai:thisIsNotAnArnoldOption", IECore.IntData( 10 ) )
			self.assertEqual( len( m.messages ), 1 )
			self.assertEqual( m.messages[-1].level, IECore.Msg.Level.Warning )
			self.assertEqual( m.messages[-1].message, "Unknown option \"ai:thisIsNotAnArnoldOption\"." )
			
		# check that setting user options works
		r.setOption( "user:myLovelyUserOption", IECore.StringData( "oooh!" ) )
		self.assertEqual( r.getOption( "user:myLovelyUserOption" ), IECore.StringData( "oooh!" ) )
		
		# check that set/get for other renderers is ignored
		r.setOption( "ri:pixelSamples", IECore.V2iData( IECore.V2i( 1, 1 ) ) )
		self.assertEqual( r.getOption( "ri:pixelSamples" ), None )

	def testDisplay( self ) :
	
		r = IECoreArnold.Renderer()
		
		self.failIf( os.path.exists( self.__displayFileName ) )
		r.display( self.__displayFileName, "driver_tiff", "rgba", {} )
		
		with IECore.WorldBlock( r ) :
			
			r.sphere( 1, -1, 1, 360, {} )
			
		self.failUnless( os.path.exists( self.__displayFileName ) )

	def testDisplayTypeMapping( self ) :
	
		r = IECoreArnold.Renderer()
		
		self.failIf( os.path.exists( self.__displayFileName ) )
		r.display( self.__displayFileName, "tiff", "rgba", {} )
		
		with IECore.WorldBlock( r ) :
			
			r.sphere( 1, -1, 1, 360, {} )
			
		self.failUnless( os.path.exists( self.__displayFileName ) )

	def testDisplayDriverIntegration( self ) :
	
		r = IECoreArnold.Renderer()
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )
		
		with IECore.WorldBlock( r ) :
			
			r.sphere( 1, -1, 1, 360, {} )
			
		self.failUnless( IECore.ImageDisplayDriver.removeStoredImage( "testHandle" ) )

	def testASSOutput( self ) :
	
		r = IECoreArnold.Renderer( self.__assFileName )

		self.failIf( os.path.exists( self.__assFileName ) )
		with IECore.WorldBlock( r ) :
		
			r.sphere( 1, -1, 1, 360, {} )
			
		self.failUnless( os.path.exists( self.__assFileName ) )

	def testUserAttributes( self ) :
	
		r = IECoreArnold.Renderer()
		
		r.setAttribute( "user:a", IECore.IntData( 10 ) )
		self.assertEqual( r.getAttribute( "user:a" ), IECore.IntData( 10 ) )
		
		with IECore.WorldBlock( r ) :
		
			self.assertEqual( r.getAttribute( "user:a" ), IECore.IntData( 10 ) )
			
			r.setAttribute( "user:a", IECore.IntData( 20 ) )
			self.assertEqual( r.getAttribute( "user:a" ), IECore.IntData( 20 ) )
			
			with IECore.AttributeBlock( r ) :
			
				r.setAttribute( "user:a", IECore.IntData( 30 ) )
				self.assertEqual( r.getAttribute( "user:a" ), IECore.IntData( 30 ) )
				
			self.assertEqual( r.getAttribute( "user:a" ), IECore.IntData( 20 ) )

	def testShader( self ) :
	
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )

		with IECore.WorldBlock( r ) :
		
			r.shader( "surface", "standard", { "emission" : 1.0, "emission_color" : IECore.Color3f( 1, 0, 0 ) } )
			r.sphere( 1, -1, 1, 360, {} )

		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )

		e = IECore.PrimitiveEvaluator.create( image )
		result = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertAlmostEqual( result.floatPrimVar( e.A() ), 1, 5 )
		self.assertAlmostEqual( result.floatPrimVar( e.R() ), 1, 5 )
		self.assertEqual( result.floatPrimVar( e.G() ), 0 )
		self.assertEqual( result.floatPrimVar( e.B() ), 0 )
		
	def testReferenceExistingShader( self ) :
	
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )

		with IECore.WorldBlock( r ) :
		
			shader = arnold.AiNode( "standard" )
			arnold.AiNodeSetStr( shader, "name", "red_shader" )
			arnold.AiNodeSetFlt( shader, "emission", 1 )
			arnold.AiNodeSetRGB( shader, "emission_color", 1, 0, 0 )
			
			r.shader( "surface", "reference:red_shader", {} )
			r.sphere( 1, -1, 1, 360, {} )

		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )

		e = IECore.PrimitiveEvaluator.create( image )
		result = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertAlmostEqual( result.floatPrimVar( e.A() ), 1, 5 )
		self.assertAlmostEqual( result.floatPrimVar( e.R() ), 1, 5 )
		self.assertEqual( result.floatPrimVar( e.G() ), 0 )
		self.assertEqual( result.floatPrimVar( e.B() ), 0 )	

	def testNonexistentReferencedShader( self ) :
	
		r = IECoreArnold.Renderer()
		
		with IECore.WorldBlock( r ) :
		
			m = IECore.CapturingMessageHandler()
			with m :
				r.shader( "surface", "reference:doesntExist", {} )
				
			self.assertEqual( len( m.messages ), 1 )
			self.failUnless( "Couldn't find shader" in m.messages[0].message )

	def testUnloadableShader( self ) :
	
		r = IECoreArnold.Renderer()
		
		with IECore.WorldBlock( r ) :
		
			m = IECore.CapturingMessageHandler()
			with m :
				r.shader( "surface", "thisShaderDoesNotExist", {} )
				
			self.assertEqual( len( m.messages ), 1 )

	def testUnsupportedShaderType( self ) :
	
		r = IECoreArnold.Renderer()
		
		with IECore.WorldBlock( r ) :
		
			m = IECore.CapturingMessageHandler()
			with m :
				r.shader( "thisShaderTypeDoesntExist", "utility", {} )
				
			self.assertEqual( len( m.messages ), 1 )
	
	def testOtherRendererShaderType( self ) :
	
		r = IECoreArnold.Renderer()
		
		with IECore.WorldBlock( r ) :
		
			m = IECore.CapturingMessageHandler()
			with m :
				r.shader( "ri:surface", "something", {} )
				
			self.assertEqual( len( m.messages ), 0 )			
		
	def testDefaultCamera( self ) :
	
		# render a plane at z==0 and check we can't see it with the default camera
	
		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) )
		
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )
		r.setOption( "ai:AA_samples", IECore.IntData( 3 ) )
	
		with IECore.WorldBlock( r ) :
			m.render( r )
		
		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )
		self.assertEqual( image.dataWindow, IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 639, 479 ) ) )
		
		e = IECore.PrimitiveEvaluator.create( image )
		result = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.failUnless( result.floatPrimVar( image["A"] ) < 0.5 )

		# move the plane back a touch and check we can see it with the default camera
		
		del r # must destroy the existing renderer before making a new one
		
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )
		r.setOption( "ai:AA_samples", IECore.IntData( 3 ) )
	
		with IECore.WorldBlock( r ) :
			with IECore.TransformBlock( r ) :
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
				m.render( r )
		
		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )
		self.assertEqual( image.dataWindow, IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 639, 479 ) ) )
		
		e = IECore.PrimitiveEvaluator.create( image )
		result = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.failUnless( result.floatPrimVar( image["A"] ) > 0.9 )

		# move the camera back a touch and check we can see the plane at z==0
		
		del r # must destroy the existing renderer before making a new one
		
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )
		r.setOption( "ai:AA_samples", IECore.IntData( 3 ) )
	
		with IECore.TransformBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, 1 ) ) )
			r.camera( "main", {} )
			
		with IECore.WorldBlock( r ) :
				m.render( r )
		
		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )
		self.assertEqual( image.dataWindow, IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 639, 479 ) ) )
		
		e = IECore.PrimitiveEvaluator.create( image )
		result = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.failUnless( result.floatPrimVar( image["A"] ) > 0.9 )		

	def testCameraXYOrientation( self ) :

		# render a red square at x==1, and a green one at y==1

		r = IECoreArnold.Renderer()

		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )

		with IECore.TransformBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, 1 ) ) )
			r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 512 ) ) } )

		with IECore.WorldBlock( r ) :
		
			r.shader( "surface", "utility", { "color" : IECore.Color3f( 1, 0, 0 ) } )
			IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0.75, -0.25 ), IECore.V2f( 1.25, 0.25 ) ) ).render( r )
			
			r.shader( "surface", "utility", { "color" : IECore.Color3f( 0, 1, 0 ) } )
			IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.25, 0.75 ), IECore.V2f( 0.25, 1.25 ) ) ).render( r )

		# check we get the colors we'd expect where we expect them
		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )
		self.failUnless( image is not None )

		e = IECore.PrimitiveEvaluator.create( image )
		result = e.createResult()
		a = e.A()
		r = e.R()
		g = e.G()
		b = e.B()
		e.pointAtUV( IECore.V2f( 1, 0.5 ), result )
		self.assertAlmostEqual( result.floatPrimVar( a ), 1, 5 )
		self.assertAlmostEqual( result.floatPrimVar( r ), 1, 5 )
		self.assertEqual( result.floatPrimVar( g ), 0 )
		self.assertEqual( result.floatPrimVar( b ), 0 )
		e.pointAtUV( IECore.V2f( 0.5, 0 ), result )
		self.assertAlmostEqual( result.floatPrimVar( a ), 1, 5 )
		self.assertEqual( result.floatPrimVar( r ), 0 )
		self.assertAlmostEqual( result.floatPrimVar( g ), 1, 5 )
		self.assertEqual( result.floatPrimVar( b ), 0 )

	def testCameraAspectRatio( self ) :
	
		r = IECoreArnold.Renderer()
		
		r.camera( "main", { "resolution" : IECore.V2i( 640, 480 ), "screenWindow" : IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 640, 480 ) ) } )	
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )
					
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
			r.shader( "surface", "utility", { "shading_mode" : "flat", "color" : IECore.Color3f( 1, 0, 0 ) } )
			IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 2 ), IECore.V2f( 638, 478 ) ) ).render( r )
			
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
			r.shader( "surface", "utility", { "shade_mode" : "flat", "color" : IECore.Color3f( 0, 1, 0 ) } )
			IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 640, 480 ) ) ).render( r )
	
		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )
		self.failUnless( image is not None )

		e = IECore.PrimitiveEvaluator.create( image )
		result = e.createResult()
		r = e.R()
		g = e.G()
		
		edges = [
			IECore.V2i( 0 ),
			IECore.V2i( 320, 0 ),
			IECore.V2i( 639, 0 ),			 
			IECore.V2i( 639, 240 ),			 
			IECore.V2i( 639, 479 ),
			IECore.V2i( 320, 479 ),
			IECore.V2i( 0, 479 ),
			IECore.V2i( 0, 240 ),
		]
		
		for point in edges :							
			self.failUnless( e.pointAtPixel( point, result ) )
			self.failUnless( result.floatPrimVar( r ) < 0.1 )
			self.failUnless( result.floatPrimVar( g ) > 0.8 )
		
		innerEdges = [
			IECore.V2i( 3, 3 ),
			IECore.V2i( 320, 3 ),
			IECore.V2i( 637, 3 ),			 
			IECore.V2i( 636, 240 ),			 
			IECore.V2i( 636, 477 ),
			IECore.V2i( 320, 477 ),
			IECore.V2i( 3, 477 ),
			IECore.V2i( 3, 240 ),
		]
		
		for point in innerEdges :
			self.failUnless( e.pointAtPixel( point, result ) )
			self.failUnless( result.floatPrimVar( r ) > 0.8 )
			self.failUnless( result.floatPrimVar( g ) < 0.1 )
				
	def testProcedural( self ) :
	
		attributeValues = []
	
		class TestProcedural( IECore.Renderer.Procedural ) :
		
			def __init__( self ) :
			
				IECore.Renderer.Procedural.__init__( self )
			
			def bound( self ) :
			
				return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )
				
			def render( self, renderer ) :
			
				t = renderer.getAttribute( "user:test" ).value
				attributeValues.append( t )
				renderer.sphere( 1, -1, 1, 360, {} )
			
			def hash( self ):
			
				h = IECore.MurmurHash()
				return h
		
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )
		
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
		
			r.setAttribute( "user:test", IECore.IntData( 0 ) )
			r.procedural( TestProcedural() )

			r.setAttribute( "user:test", IECore.IntData( 1 ) )
			r.procedural( TestProcedural() )
		
		self.assertEqual( len( attributeValues ), 2 )
		self.failUnless( 1 in attributeValues )
		self.failUnless( 0 in attributeValues )
	
	def performCurvesTest( self, curvesPrimitive, expectedImage ) :
	
		r = IECoreArnold.Renderer()
				
		r.setOption( "ai:AA_samples", IECore.IntData( 3 ) )
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )
		
		with IECore.TransformBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, 2 ) ) )
			r.camera(
				"main",
				{
					"resolution" : IECore.V2i( 512 ),
					"projectin" : "orthographic",
					"screenWindow" : IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ),
				}
			)

		with IECore.WorldBlock( r ) :			
			curvesPrimitive.render( r )

		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )
		del image["A"]
		
		# raise blackPoint massively to remove possible watermark
		IECore.Grade()( input = image, copyInput = False, blackPoint = IECore.Color3f( 0.9 ) )
				
		expectedImage = IECore.Reader.create( expectedImage ).read()
		
		self.assertEqual( IECore.ImageDiffOp()( imageA=image, imageB=expectedImage, maxError=0.01 ), IECore.BoolData( False ) )
			
	def testBezierCurves( self ) :
	
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.bezier(),
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.8, 0.2, 0 ),
					IECore.V3f( 0.2, 0.2, 0 ),
					IECore.V3f( 0.2, 0.8, 0 ),
					IECore.V3f( 0.8, 0.8, 0 ),
				]
			)

		)
		c["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performCurvesTest( c, "contrib/IECoreArnold/test/IECoreArnold/data/curveImages/bezier.exr" )
	
	def testBSplineCurves( self ) :
	
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.bSpline(),
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.8, 0.2, 0 ),
					IECore.V3f( 0.2, 0.2, 0 ),
					IECore.V3f( 0.2, 0.8, 0 ),
					IECore.V3f( 0.8, 0.8, 0 ),
				]
			)

		)
		c["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performCurvesTest( c, "contrib/IECoreArnold/test/IECoreArnold/data/curveImages/bSpline.exr" )
	
	def testVisibilityAttributes( self ) :
	
		r = IECoreArnold.Renderer()
		self.assertEqual( r.getAttribute( "ai:visibility:camera" ), IECore.BoolData( True ) )
		self.assertEqual( r.getAttribute( "ai:visibility:shadow" ), IECore.BoolData( True ) )
		self.assertEqual( r.getAttribute( "ai:visibility:reflected" ), IECore.BoolData( True ) )
		self.assertEqual( r.getAttribute( "ai:visibility:refracted" ), IECore.BoolData( True ) )
		self.assertEqual( r.getAttribute( "ai:visibility:diffuse" ), IECore.BoolData( True ) )
		self.assertEqual( r.getAttribute( "ai:visibility:glossy" ), IECore.BoolData( True ) )
		
		r.setAttribute( "ai:visibility:shadow", IECore.BoolData( False ) )
		self.assertEqual( r.getAttribute( "ai:visibility:shadow" ), IECore.BoolData( False ) )
		
	def __displacementRender( self, doDisplacement ) :
		
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )

		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
		
			r.setAttribute( "ai:polymesh:subdiv_iterations", IECore.IntData( 5 ) )
		
			r.shader( "surface", "utility", { "color_mode" : IECore.StringData( "ng" ), "shade_mode" : IECore.StringData( "flat" ) } )	
			if doDisplacement :
				r.shader( "displacement", "noise", {} )
			
			mesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )
			mesh.interpolation = "catmullClark"
			mesh.render( r )

		return IECore.ImageDisplayDriver.removeStoredImage( "test" )
	
	def testDisplacementShader( self ) :
			
		undisplaced1 = self.__displacementRender( doDisplacement = False )
		undisplaced2 = self.__displacementRender( doDisplacement = False )
		
		displaced1 = self.__displacementRender( doDisplacement = True )
		displaced2 = self.__displacementRender( doDisplacement = True )			

		self.assertEqual( IECore.ImageDiffOp()( imageA=undisplaced1, imageB=undisplaced2, maxError=0.001 ), IECore.BoolData( False ) )
		self.assertEqual( IECore.ImageDiffOp()( imageA=displaced1, imageB=displaced2, maxError=0.001 ), IECore.BoolData( False ) )
		
		self.assertEqual( IECore.ImageDiffOp()( imageA=displaced1, imageB=undisplaced1, maxError=0.1 ), IECore.BoolData( True ) )
	
	def __allNodes( self, type = arnold.AI_NODE_ALL ) :
	
		result = []
		i = arnold.AiUniverseGetNodeIterator( type )
		while not arnold.AiNodeIteratorFinished( i ) :
			result.append( arnold.AiNodeIteratorGetNext( i ) )
	
		return result
		
	def testShapeAttributes( self ) :
		
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )

		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
		
			r.setAttribute( "ai:polymesh:subdiv_iterations", IECore.IntData( 10 ) )
			
			mesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )
			mesh.render( r )
			
			shapes = self.__allNodes( type = arnold.AI_NODE_SHAPE )

			self.assertEqual( len( shapes ), 1 )
			self.assertEqual( arnold.AiNodeGetInt( shapes[0], "subdiv_iterations" ), 10 )
	
	def testShaderConnections( self ) :
	
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )

		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
			
			r.shader( "shader", "flat", { "color" : IECore.Color3f( 1, 0, 0  ), "__handle" : "myInputShader" } )
			r.shader( "surface", "standard", { "emission" : 1.0, "emission_color" : "link:myInputShader" } )	
			
			mesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			mesh.render( r )

		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )
		e = IECore.PrimitiveEvaluator.create( image )
 		result = e.createResult()
		
		e.pointAtUV( IECore.V2f( 0.5 ), result )
		self.assertAlmostEqual( result.floatPrimVar( e.R() ), 1, 5 )
		self.assertEqual( result.floatPrimVar( e.G() ), 0 )
		self.assertEqual( result.floatPrimVar( e.B() ), 0 )
	
	def testMissingShaderConnectionWarnings( self ) :
	
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )

		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
			
	 		m = IECore.CapturingMessageHandler()
 			with m :
				r.shader( "shader", "flat", { "color" : IECore.Color3f( 1, 0, 0  ), "__handle" : "myInputShader" } )
				r.shader( "surface", "standard", { "emission" : 1.0, "emission_color" : "link:oopsWrongOne" } )	
		
		self.assertEqual( len( m.messages ), 1 )
		self.assertEqual( m.messages[0].level, IECore.Msg.Level.Warning )
		self.failUnless( "oopsWrongOne" in m.messages[0].message )
	
	def testLight( self ) :
	
		r = IECoreArnold.Renderer()
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "test" } )

		with IECore.WorldBlock( r ) :
		
			r.light( "point_light", "handle", { "intensity" : 1, "color" : IECore.Color3f( 1, 0.5, 0.25 ) } )
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
			
			r.shader( "surface", "standard", {} )	
			
			mesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			mesh.render( r )

		image = IECore.ImageDisplayDriver.removeStoredImage( "test" )
		e = IECore.PrimitiveEvaluator.create( image )
 		result = e.createResult()
				
		e.pointAtUV( IECore.V2f( 0.5 ), result )
		self.assertTrue( result.floatPrimVar( e.R() ) > 0.2 )
		self.assertAlmostEqual( result.floatPrimVar( e.R() ) * 0.5, result.floatPrimVar( e.G() ) )
		self.assertAlmostEqual( result.floatPrimVar( e.R() ) * 0.25, result.floatPrimVar( e.B() ) )

	def testExternalProcedural( self ) :

		r = IECoreArnold.Renderer( self.__assFileName )

		with IECore.WorldBlock( r ) :
		
			r.procedural(
				r.ExternalProcedural(
					"test.so",
					IECore.Box3f(
						IECore.V3f( 1, 2, 3 ),
						IECore.V3f( 4, 5, 6 )
					),
					{
						"colorParm" : IECore.Color3f( 1, 2, 3 ),
						"stringParm" : "test",
						"floatParm" : 1.5,
						"intParm" : 2,
					}
				)
			)

		ass = "".join( file( self.__assFileName ).readlines() )

		self.assertTrue( "procedural" in ass )
		self.assertTrue( "min 1 2 3" in ass )
		self.assertTrue( "max 4 5 6" in ass )
		self.assertTrue( "dso \"test.so\"" in ass )
		self.assertTrue( "declare stringParm constant STRING" in ass )
		self.assertTrue( "declare floatParm constant FLOAT" in ass )
		self.assertTrue( "declare intParm constant INT" in ass )
		self.assertTrue( "declare colorParm constant RGB" in ass )
		self.assertTrue( "stringParm \"test\"" in ass )
		self.assertTrue( "floatParm 1.5" in ass )
		self.assertTrue( "intParm 2" in ass )
		self.assertTrue( "colorParm 1 2 3" in ass )

	def testPixelAspectRatio( self ) :

		r = IECoreArnold.Renderer( self.__assFileName )

		r.camera( "main", { "resolution" : IECore.V2i( 640, 480 ), "pixelAspectRatio" : 2.0 } )

		with IECore.WorldBlock( r ) :
			pass

		ass = "".join( file( self.__assFileName ).readlines() )
		self.assertTrue( "aspect_ratio 0.5" in ass )

	def tearDown( self ) :
			
		for f in [
			self.__displayFileName,
			self.__assFileName,
		] :
			if os.path.exists( f ) :
				os.remove( f )
		
if __name__ == "__main__":
    unittest.main()
