##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
import random
import os
import shutil

import IECore
import IECoreScene
import IECoreImage
import IECoreGL

IECoreGL.init( False )

class TestPointsPrimitive( unittest.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/testPoints.exr"

	def testStateComponentsInstantiation( self ):

		IECoreGL.PointsPrimitive.UseGLPoints( IECoreGL.GLPointsUsage.ForPointsOnly )
		IECoreGL.PointsPrimitive.GLPointWidth( 1.5 )

	def testStateComponentsUsage( self ):

		g = IECoreGL.Group()
		g.getState().add( IECoreGL.PointsPrimitive.UseGLPoints( IECoreGL.GLPointsUsage.ForPointsAndDisks ) )
		g.getState().add( IECoreGL.PointsPrimitive.GLPointWidth( 2.3 ) )

	def testVertexAttributes( self ) :

		# if rendering points and requiring custom vertex attributes to be passed
		# to the fragment shader, you are now required to provide your own vertex shader.
		# however, handy macros exist to make the instancing and aiming easy.
		vertexSource = """
		#include \"IECoreGL/PointsPrimitive.h\"
		#include \"IECoreGL/VertexShader.h\"

		IECOREGL_POINTSPRIMITIVE_DECLAREVERTEXPARAMETERS

		IECOREGL_VERTEXSHADER_IN vec3 instanceP;
		// although we don't need to pass st to the fragment shader, everything
		// ceases to work on os x if we don't.
		IECOREGL_VERTEXSHADER_IN vec2 instancest;
		IECOREGL_VERTEXSHADER_IN float vertexgreyTo255;

		IECOREGL_VERTEXSHADER_OUT float fragmentGrey;
		IECOREGL_VERTEXSHADER_OUT vec2 fragmentst;

		void main()
		{
			mat4 instanceMatrix = IECOREGL_POINTSPRIMITIVE_INSTANCEMATRIX;
			vec4 pCam = instanceMatrix * vec4( instanceP, 1 );
			gl_Position = gl_ProjectionMatrix * pCam;
			fragmentst = instancest;
			fragmentGrey = float( vertexgreyTo255 ) / 255.0;
		}
		"""

		fragmentSource = """
		#include \"IECoreGL/FragmentShader.h\"

		IECOREGL_FRAGMENTSHADER_IN float fragmentGrey;

		void main()
		{
			gl_FragColor = vec4( fragmentGrey, fragmentGrey, fragmentGrey, 1.0 );
		}
		"""

		numPoints = 100
		p = IECore.V3fVectorData( numPoints )
		g = IECore.IntVectorData( numPoints )
		random.seed( 0 )
		for i in range( 0, numPoints ) :
			p[i] = IECore.V3f( random.random() * 4, random.random() * 4, random.random() * 4 )
			g[i] = int( random.uniform( 0.0, 255.0 ) )
		p = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, p )
		g = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, g )

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( self.outputFileName, "exr", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -2, -2, -10 ) ) )
			r.shader( "surface", "grey", { "gl:vertexSource" : vertexSource, "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )
			r.points( numPoints, { "P" : p, "greyTo255" : g } )

		reader = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/pointVertexAttributes.tif" )
		reader["rawChannels"].setTypedValue( True )
		expectedImage = reader.read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()

		self.assertEqual( IECoreImage.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.05 ).value, False )

	def testEmptyPointsPrimitive( self ):

		fragmentSource = """
		uniform int greyTo255;

		void main()
		{
			float g = float( greyTo255 ) / 255.0;
			gl_FragColor = vec4( g, g, g, 1 );
		}
		"""
		p = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData() )
		g = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData() )
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )
		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( self.outputFileName, "exr", "rgba", {} )
		with IECoreScene.WorldBlock( r ) :
			r.shader( "surface", "grey", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )
			r.points( 0, { "P" : p, "greyTo255" : g } )		# it should not crash rendering 0 points.

	def performAimTest( self, projection, expectedImage, particleType ) :

		fragmentSource = """
		void main()
		{
			gl_FragColor = vec4( 1, 1, 1, 1 );
		}
		"""

		p = IECore.V3fVectorData()
		for x in range( -2, 3 ) :
			for y in range( -2, 3 ) :
				p.append( IECore.V3f( x, y, 0 ) )

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( projection ),
				"projection:fov" : IECore.FloatData( 20 ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( self.outputFileName, "exr", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )

			r.shader( "surface", "white", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )
			r.points( p.size(), {
					"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, p ),
					"constantwidth" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.75 ) ),
					"type" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( particleType ) )
				}
			)

		expectedImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/" + expectedImage ).read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()

		self.assertEqual( IECoreImage.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.08 ).value, False )

	def testPerspectiveAimedPoints( self ) :

		self.performAimTest( "perspective", "aimedPerspectivePoints.tif", "particle" )

	def testOrthographicAimedPoints( self ) :

		self.performAimTest( "orthographic", "aimedOrthographicPoints.tif", "particle" )

	def testPerspectiveAimedPatches( self ) :

		self.performAimTest( "perspective", "aimedPerspectivePatches.tif", "patch" )

	def testOrthographicAimedPatches( self ) :

		self.performAimTest( "orthographic", "aimedOrthographicPatches.tif", "patch" )

	def testGLPoints( self ) :

		fragmentSource = """
		void main()
		{
			gl_FragColor = vec4( 1, 1, 1, 1 );
		}
		"""

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"projection:fov" : IECore.FloatData( 20 ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )

			r.shader( "surface", "white", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )

			with IECoreScene.AttributeBlock( r ) :

				r.setAttribute( "gl:pointsPrimitive:glPointWidth", IECore.FloatData( 20 ) )

				r.points( 1, {
						"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) ),
						"type" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "gl:point" ) )
					}
				)

			with IECoreScene.AttributeBlock( r ) :

				r.setAttribute( "gl:pointsPrimitive:glPointWidth", IECore.FloatData( 10 ) )

				r.points( 1, {
						"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] ) ),
						"type" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "gl:point" ) )
					}
				)

		expectedImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/glPoints.tif" ).read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()

		self.assertEqual( IECoreImage.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.05 ).value, False )

	def testTexturing( self ) :

		fragmentSource = """
		uniform sampler2D texture;
		varying vec2 fragmentuv;

		void main()
		{
			gl_FragColor = texture2D( texture, fragmentuv );
		}
		"""

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:texture", IECore.StringData( "./" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )
		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 1024 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -.5 ), IECore.V2f( .5 ) ) )
			}
		)
		r.display( self.outputFileName, "exr", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )

			r.shader(
				"surface", "test",
				{
					"gl:fragmentSource" : IECore.StringData( fragmentSource ),
					"texture" : "test/IECoreGL/images/numbers.exr",
				}
			)

			r.points( 1, {
				"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) ),
				"type" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "patch" ) )
			} )

		expectedImage = IECore.Reader.create( "test/IECoreGL/images/numbers.exr" ).read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()
		self.assertEqual( IECoreImage.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.05 ).value, False )

	def testWindingOrder( self ) :

		def performTest( rightHandedOrientation, expectedAlpha ) :

			r = IECoreGL.Renderer()
			r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
			r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )
			r.camera( "main", {
					"projection" : IECore.StringData( "orthographic" ),
					"resolution" : IECore.V2iData( IECore.V2i( 1024 ) ),
					"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
					"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -.5 ), IECore.V2f( .5 ) ) )
				}
			)
			r.display( self.outputFileName, "exr", "rgba", {} )

			with IECoreScene.WorldBlock( r ) :

				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )
				r.setAttribute( "doubleSided", IECore.BoolData( False ) )
				r.setAttribute( "rightHandedOrientation", IECore.BoolData( rightHandedOrientation ) )

				r.points( 1, {
					"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) ),
				} )

			i = IECore.Reader.create( self.outputFileName ).read()
			dimensions = i.dataWindow.size() + IECore.V2i( 1 )
			index = dimensions.x * int(dimensions.y * 0.5) + int(dimensions.x * 0.5)
			self.assertEqual( i["A"][index], expectedAlpha )

		performTest( True, 1 )
		performTest( False, 0 )

	def testAspectAndRotation( self ) :

		fragmentSource = """
		void main()
		{
			gl_FragColor = vec4( 1, 1, 1, 1 );
		}
		"""

		random = IECore.Rand48()

		p = IECore.V3fVectorData()
		r = IECore.FloatVectorData()
		w = IECore.FloatVectorData()
		a = IECore.FloatVectorData()
		for x in range( -2, 3 ) :
			for y in range( -2, 3 ) :
				p.append( IECore.V3f( x, y, 0 ) )
				r.append( random.nextf( 0, 360 ) )
				w.append( random.nextf( 0.25, 0.5 ) )
				a.append( random.nextf( 0.5, 2 ) )

		renderer = IECoreGL.Renderer()
		renderer.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		renderer.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )

		renderer.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 512 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		renderer.display( self.outputFileName, "exr", "rgba", { "quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ) } )

		with IECoreScene.WorldBlock( renderer ) :

			renderer.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )

			renderer.shader( "surface", "white", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )
			renderer.points( p.size(), {
					"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, p ),
					"patchrotation" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, r ),
					"width" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, w ),
					"patchaspectratio" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, a ),
					"type" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "patch" ) ),
				}
			)

		expectedImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/rotatedPointPatches.exr" ).read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()

		self.assertEqual( IECoreImage.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.08 ).value, False )

	def testBound( self ) :

		p = IECoreGL.PointsPrimitive( IECoreGL.PointsPrimitive.Type.Point )

		self.assertEqual( p.bound(), IECore.Box3f() )

		p.addPrimitiveVariable(
			"P",
			IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Vertex,
				IECore.V3fVectorData( [ IECore.V3f( -1 ), IECore.V3f( 10 ) ] )
			)
		)

		self.assertEqual( p.bound(), IECore.Box3f( IECore.V3f( -1.5 ), IECore.V3f( 10.5 ) ) )

		p.addPrimitiveVariable(
			"constantwidth",
			IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Constant,
				IECore.FloatData( 2 )
			)
		)

		self.assertEqual( p.bound(), IECore.Box3f( IECore.V3f( -2 ), IECore.V3f( 11 ) ) )

		p.addPrimitiveVariable(
			"width",
			IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Vertex,
				IECore.FloatVectorData( [ .5, 2 ] )
			)
		)

		self.assertEqual( p.bound(), IECore.Box3f( IECore.V3f( -1.5 ), IECore.V3f( 12 ) ) )

		p.addPrimitiveVariable(
			"patchaspectratio",
			IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Vertex,
				IECore.FloatVectorData( [ .5, 4 ] )
			)
		)

		self.assertEqual( p.bound(), IECore.Box3f( IECore.V3f( -2 ), IECore.V3f( 12 ) ) )

	def testUniformShaderParameters( self ) :

		fragmentSource = """
		uniform vec3 myColor;
		void main()
		{
			gl_FragColor = vec4( myColor, 1 );
		}
		"""

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )
		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 1024 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -.5 ), IECore.V2f( .5 ) ) )
			}
		)
		r.display( self.outputFileName, "exr", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )

			r.shader( "surface", "test", { "gl:fragmentSource" : fragmentSource, "myColor" : IECore.Color3f( 1, 0, 0 ) } )

			r.points( 1, {
				"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) ),
			} )

		i = IECore.Reader.create( self.outputFileName ).read()
		dimensions = i.dataWindow.size() + IECore.V2i( 1 )
		index = dimensions.x * int(dimensions.y * 0.5) + int(dimensions.x * 0.5)
		self.assertEqual( i["A"][index], 1 )
		self.assertEqual( i["R"][index], 1 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 0 )

	def testWireframeShading( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera(
			"main",
			{
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 1024 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -.5 ), IECore.V2f( .5 ) ) )
			}
		)

		r.display( self.outputFileName, "exr", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )
			r.setAttribute( "gl:primitive:wireframe", True )
			r.setAttribute( "gl:primitive:wireframeColor", IECore.Color4f( 1, 0, 0, 1 ) )
			r.setAttribute( "gl:primitive:wireframeWidth", 5.0 )
			r.setAttribute( "gl:primitive:solid", False )

			r.points( 1, {
				"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) ),
			} )

		i = IECore.Reader.create( self.outputFileName ).read()
		dimensions = i.dataWindow.size() + IECore.V2i( 1 )
		index = dimensions.x * int(dimensions.y * 0.5) + int(dimensions.x * 0.5)
		self.assertEqual( i["A"][index], 1 )
		self.assertEqual( i["R"][index], 1 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 0 )

	def testVertexCs( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera(
			"main",
			{
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 1024 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1), IECore.V2f( 1 ) ) )
			}
		)

		r.display( self.outputFileName, "exr", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )

			r.points( 2, {
				"P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( -.5 ), IECore.V3f( .5 ) ] ) ),
				"Cs" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.Color3fVectorData( [ IECore.Color3f( 1, 0, 0 ), IECore.Color3f( 0, 1, 0 ) ] ) ),
			} )

		i = IECore.Reader.create( self.outputFileName ).read()
		dimensions = i.dataWindow.size() + IECore.V2i( 1 )
		index = dimensions.x * int(dimensions.y * 0.75) + int(dimensions.x * 0.25)
		self.assertEqual( i["A"][index], 1 )
		self.assertEqual( i["R"][index], 1 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 0 )

		index = dimensions.x * int(dimensions.y * 0.25) + int(dimensions.x * 0.75)
		self.assertEqual( i["A"][index], 1 )
		self.assertEqual( i["R"][index], 0 )
		self.assertEqual( i["G"][index], 1 )
		self.assertEqual( i["B"][index], 0 )

	def setUp( self ) :

		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )

	def tearDown( self ) :

		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )

if __name__ == "__main__":
    unittest.main()
