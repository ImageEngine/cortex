##########################################################################
#
#  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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
import shutil
import inspect

import IECore
import IECoreScene
import IECoreImage
import IECoreGL

IECoreGL.init( False )

class ShadingTest( unittest.TestCase ) :

	__imageFileName = os.path.dirname( __file__ ) + "/output/test.tif"

	def mesh( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) )
		m["N"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 1 ) ] * 4 ) )
		return m

	def constantShader( self ) :

		s = IECoreScene.Shader( "test", "gl:surface" )

		s.parameters["gl:fragmentSource"] = """
			#include "IECoreGL/FragmentShader.h"

			IECOREGL_FRAGMENTSHADER_IN vec3 fragmentCs;

			void main()
			{
				gl_FragColor = vec4( fragmentCs, 1.0 );
			}
		"""

		return s

	def colorShader( self ) :

		s = IECoreScene.Shader( "test", "gl:surface" )

		s.parameters["gl:fragmentSource"] = """

			uniform bool rB;

			uniform float rF;
			uniform int gI;

			uniform vec2 rgF;
			uniform ivec2 rgI;

			uniform vec3 rgbF;
			uniform ivec3 rgbI;

			void main()
			{
				gl_FragColor = vec4(
					float( rB ) + rF + rgF.r + float( rgI.r ) + rgbF.r + float( rgbI.r ),
					float( gI ) + rgF.g + float( rgI.g ) + rgbF.g + float( rgbI.g ),
					rgbF.b + float( rgbI.b ),
					1.0
				);
			}
		"""

		return s

	def textureShader( self ) :

		s = IECoreScene.Shader( "test", "gl:surface" )

		s.parameters["gl:fragmentSource"] = """

			uniform sampler2D sampler;

			varying vec2 fragmentuv;

			void main()
			{
				gl_FragColor = vec4( texture2D( sampler, fragmentuv ).rgb, 1 );
			}
		"""

		return s

	def geometryShader( self ) :

		s = IECoreScene.Shader( "test", "gl:surface" )

		s.parameters["gl:geometrySource"] = """
#version 150

			layout( points ) in;
			layout( points, max_vertices=3 ) out;

			void main()
			{
				for( int i = -1; i<2; i++ )
				{
					gl_Position = gl_in[0].gl_Position + vec4( 0.5 * i, 0, 0, 0 );
					EmitVertex();
				}
			}
		"""

		s.parameters["gl:fragmentSource"] = """
			void main()
			{
				gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
			}
		"""

		return s

	def floatArrayShader( self ) :

		s = IECoreScene.Shader( "test", "gl:surface" )

		s.parameters["gl:fragmentSource"] = """

			uniform float f[8];
			uniform vec2 f2[8];
			uniform vec3 f3[8];
			uniform vec4 f4[8];

			void main()
			{
				vec3 accum = vec3( 0 );
				for( int i = 0; i < 8; i += 1 ) accum += 0.01 * ( vec3(f[i], 0, 0) + vec3(f2[i], 0) + f3[i] + f4[i].xyz );
				gl_FragColor = vec4( accum, 1.0);
			}
		"""

		return s

	def intArrayShader( self ) :

		s = IECoreScene.Shader( "test", "gl:surface" )

		s.parameters["gl:fragmentSource"] = """

			uniform int i[8];
			uniform ivec2 i2[8];
			uniform ivec3 i3[8];

			void main()
			{
				vec3 accum = vec3( 0 );
				for( int j = 0; j < 8; j += 1 ) accum += 0.01 * ( vec3(i[j], 0, 0) + vec3(i2[j], 0) + vec3( i3[j] ) );
				gl_FragColor = vec4( accum, 1.0);
			}
		"""

		return s

	def matrixShader( self ) :

		s = IECoreScene.Shader( "test", "gl:surface" )

		s.parameters["gl:fragmentSource"] = """

			uniform mat3 m3;
			uniform mat3 m3v[8];
			uniform mat4 m4;
			uniform mat4 m4v[8];

			void main()
			{
				vec3 accum = vec3( 0 );
				for( int i = 0; i < 8; i += 1 ) accum += m3v[i] * vec3( 0, 1, 0 ) + (m4v[i] * vec4( 0, 1, 0, 0 )).xyz;
				gl_FragColor = vec4( m3 * vec3( 0, 0, 1) + (m4 * vec4( 0, 0, 1, 1 )).xyz + accum, 1.0);
			}
		"""

		return s

	def offsetShader( self, offset ) :

		s = IECoreScene.Shader( "test", "gl:surface" )

		s.parameters["gl:vertexSource"] = inspect.cleandoc( """

			#version 120
			#if __VERSION__ <= 120
				#define in attribute
			#endif

			uniform vec3 offset;

			in vec3 vertexP;
			void main()
			{
				vec3 translatedVertexP = vertexP + offset;
				vec4 pCam = gl_ModelViewMatrix * vec4( translatedVertexP, 1 );
				gl_Position = gl_ProjectionMatrix * pCam;
			}

		""" )

		s.parameters["gl:fragmentSource"] = inspect.cleandoc( """

			void main()
			{

				gl_FragColor = vec4( 1, 0, 0, 1 );

			}

		""" )

		s.parameters["offset"] = offset

		return s

	def renderImage( self, group ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera(
			"main", {
				"projection" : "orthographic" ,
				"resolution" : IECore.V2i( 256 ),
				"clippingPlanes" : IECore.V2f( 1, 1000 ),
				"screenWindow" : IECore.Box2f( IECore.V2f( -0.5 ), IECore.V2f( 0.5 ) )
			}
		)
		r.display( self.__imageFileName, "tif", "rgba", {} )
		r.setOption( "gl:searchPath:texture", IECore.StringData( "./" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( "./test/IECoreGL/shaders" ) )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			group.render( r )

		return IECore.Reader.create( self.__imageFileName ).read()

	def assertImageValues( self, image, tests ) :

		dimensions = image.dataWindow.size() + IECore.V2i( 1 )

		for t in tests :

			index = dimensions.x * int(dimensions.y * t[0].y) + int(dimensions.x * t[0].x)

			c = IECore.Color4f(
				image["R"][index],
				image["G"][index],
				image["B"][index],
				image["A"][index],
			)

			for i in range( 4 ):
				if abs( c[i] - t[1][i] ) > 0.01:
					raise AssertionError( repr( c ) + " != " + repr( t[1] ) )

	def splineGradient( self, color0, color1 ) :

		return IECore.SplinefColor3fData(
			IECore.SplinefColor3f(
				IECore.CubicBasisf.catmullRom(),
				(
					( 0, color0 ),
					( 0, color0 ),
					( 1, color1 ),
					( 1, color1 ),
				),
			),
		)

	def testBasicPositioning( self ) :

		g = IECoreScene.Group()
		g.addChild( self.mesh() )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.5 ), IECore.Color4f( 1, 1, 1, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
			]
		)

	def testUniformBoolParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["rB"] = IECore.BoolData( 1 )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 0, 0, 1 ) ),
			]
		)

	def testUniformFloatParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["rF"] = IECore.FloatData( 1 )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 0, 0, 1 ) ),
			]
		)

	def testUniformIntParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["gI"] = IECore.FloatData( 1 )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 0, 0, 1 ) ),
			]
		)

	def testUniform2fParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["rgF"] = IECore.V2fData( IECore.V2f( 0, 1 ) )
		c2.state()[0].parameters["rgF"] = IECore.V2fData( IECore.V2f( 1, 0 ) )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
			]
		)

	def testUniform2iParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["rgF"] = IECore.V2iData( IECore.V2i( 0, 1 ) )
		c2.state()[0].parameters["rgF"] = IECore.V2iData( IECore.V2i( 1, 0 ) )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
			]
		)

	def testUniform3fParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["rgbF"] = IECore.V3fData( IECore.V3f( 0, 1, 1 ) )
		c2.state()[0].parameters["rgbF"] = IECore.V3fData( IECore.V3f( 1, 1, 0 ) )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 0, 1, 1, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 1, 1, 0, 1 ) ),
			]
		)

	def testUniform3iParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["rgbI"] = IECore.V3iData( IECore.V3i( 0, 1, 1 ) )
		c2.state()[0].parameters["rgbI"] = IECore.V3iData( IECore.V3i( 1, 1, 0 ) )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 0, 1, 1, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 1, 1, 0, 1 ) ),
			]
		)

	def testConstantPrimVarTemporarilyOverridesShaderParameter( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["rgbF"] = IECore.V3iData( IECore.V3i( 0, 1, 1 ) )
		c1.children()[0]["rgbF"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.V3fData( IECore.V3f( 1, 1, 0 ) ) )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 1, 1, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 0, 0, 1 ) ),
			]
		)

	def testSplineAsTexture( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.textureShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["sampler"] = self.splineGradient( IECore.Color3f( 1, 0, 0 ), IECore.Color3f( 1, 0, 0 ) )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 0, 0, 1 ) ),
			]
		)

	def testImageFileAsTexture( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.textureShader() )

		c2 = c1.copy()
		c1.state()[0].parameters["sampler"] = IECore.StringData( os.path.dirname( __file__ ) + "/images/yellow.exr" )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 1, 1, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 0, 0, 1 ) ),
			]
		)


	def testCompoundDataAsTexture( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.textureShader() )

		yellowImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/images/yellow.exr" ).read()
		yellowCompoundData = IECore.CompoundData( {
			"dataWindow" : IECore.Box2iData( yellowImage.dataWindow ),
			"displayWindow" : IECore.Box2iData( yellowImage.displayWindow ),
			"channels" : {
				"R" : yellowImage["R"],
				"G" : yellowImage["G"],
				"B" : yellowImage["B"],
			}
		} )


		c2 = c1.copy()
		c1.state()[0].parameters["sampler"] = yellowCompoundData

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 1, 1, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 0, 0, 1 ) ),
			]
		)

	def testWireframe( self ) :

		g = IECoreScene.Group()
		g.addChild( self.mesh() )

		g.addState(
			IECoreScene.AttributeState(
				{
					"gl:primitive:solid" : IECore.BoolData( False ),
					"gl:primitive:wireframe" : IECore.BoolData( True ),
					"gl:primitive:wireframeColor" : IECore.Color4f( 1, 0, 0, 1 ),
					"gl:primitive:wireframeWidth" : 6.0,
				}
			)
		)

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
				( IECore.V2f( 0.55, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
			]
		)

	def testSameMeshTwoShaders( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		c1.state()[0].parameters["rgbF"] = IECore.V3fData( IECore.V3f( 0, 1, 1 ) )

		c2 = IECoreScene.Group()
		c2.addChild( self.mesh() )
		c2.addState( self.textureShader() )
		c2.state()[0].parameters["sampler"] = IECore.StringData( os.path.dirname( __file__ ) + "/images/yellow.exr" )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 0, 1, 1, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 1, 1, 0, 1 ) ),
			]
		)

	def testGeometryShaderViaParameters( self ) :

		if IECoreGL.glslVersion() < 150 :
			# no point testing unavailable functionality
			return

		g = IECoreScene.Group()

		p = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) )
		p["type"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "gl:point" ) )
		g.addChild( p )

		g.addState( IECoreScene.AttributeState( { "gl:pointsPrimitive:glPointWidth" : IECore.FloatData( 4 ) } ) )
		g.addState( self.geometryShader() )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.125, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.25, 0.5 ), IECore.Color4f( 1, 1, 1, 1 ) ),
				( IECore.V2f( 0.375, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 1, 1, 1, 1 ) ),
				( IECore.V2f( 0.625, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.75, 0.5 ), IECore.Color4f( 1, 1, 1, 1 ) ),
				( IECore.V2f( 0.875, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
			]
		)

	def testGeometryShaderViaFile( self ) :

		if IECoreGL.glslVersion() < 150 :
			# no point testing unavailable functionality
			return

		g = IECoreScene.Group()

		p = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) )
		p["type"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "gl:point" ) )
		g.addChild( p )

		g.addState( IECoreScene.AttributeState( { "gl:pointsPrimitive:glPointWidth" : IECore.FloatData( 4 ) } ) )
		g.addState( IECoreScene.Shader( "pointTripler", "gl:surface" ) )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.125, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.25, 0.5 ), IECore.Color4f( 1, 1, 1, 1 ) ),
				( IECore.V2f( 0.375, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 1, 1, 1, 1 ) ),
				( IECore.V2f( 0.625, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.75, 0.5 ), IECore.Color4f( 1, 1, 1, 1 ) ),
				( IECore.V2f( 0.875, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
			]
		)

	def testCsParameterTrumpsColorAttribute( self ) :

		# if there is no Cs parameter value specified then we should get
		# Cs from the attribute state.

		g = IECoreScene.Group()
		g.addChild( self.mesh() )

		g.addState( IECoreScene.AttributeState( { "color" : IECore.Color3f( 1, 0, 0 ) } ) )
		g.addState( self.constantShader() )

		image = self.renderImage( g )
		self.assertImageValues( image, [ ( IECore.V2f( 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ) ] )

		# but if there is a Cs parameter, it should override the colour
		# from the attribute state.

		g = IECoreScene.Group()
		g.addChild( self.mesh() )

		g.addState( IECoreScene.AttributeState( { "color" : IECore.Color3f( 1, 0, 0 ) } ) )

		s = self.constantShader()
		s.parameters["Cs"] = IECore.Color3f( 0, 1, 0 )
		g.addState( s )

		image = self.renderImage( g )
		self.assertImageValues( image, [ ( IECore.V2f( 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ) ] )

	def testColorAttributeDoesntAffectWireframe( self ) :

		g = IECoreScene.Group()
		g.addChild( self.mesh() )

		g.addState(
			IECoreScene.AttributeState(
				{
					"color" : IECore.Color3f( 1, 0, 0 ),
					"gl:primitive:solid" : IECore.BoolData( False ),
					"gl:primitive:wireframe" : IECore.BoolData( True ),
					"gl:primitive:wireframeColor" : IECore.Color4f( 0, 1, 0, 1 ),
					"gl:primitive:wireframeWidth" : 6.0,
				}
			)
		)

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
			]
		)

	def testVertexCsDoesntAffectWireframe( self ) :

		m = self.mesh()
		m["Cs"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.Color3fVectorData(
				[ IECore.Color3f( 0 ) ] * len( m["P"].data )
			)
		)

		g = IECoreScene.Group()
		g.addChild( m )

		g.addState(
			IECoreScene.AttributeState(
				{
					"gl:primitive:solid" : IECore.BoolData( True ),
					"gl:primitive:wireframe" : IECore.BoolData( True ),
					"gl:primitive:wireframeColor" : IECore.Color4f( 0, 1, 0, 1 ),
					"gl:primitive:wireframeWidth" : 6.0,
				}
			)
		)

		image = self.renderImage( g )
		# wireframe is green, and vertex Cs is black,
		# so there should be no contribution from
		# wireframe or solid shading in the red channel.
		self.assertEqual( sum( image["R"] ), 0 )
		# black vertex colour should have no effect on
		# green wireframe, so we should have some wireframe
		# contribution in the green channel.
		self.assertTrue( sum( image["G"] ) > 0 )

	def testUniformFloatArrayParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.floatArrayShader() )

		c2 = c1.copy()
		c3 = c1.copy()
		c4 = c1.copy()
		c1.state()[0].parameters["f"] = IECore.FloatVectorData( [ i for i in range( 8 ) ] )
		c2.state()[0].parameters["f2"] = IECore.V2fVectorData( [ IECore.V2f( i, 100.0 / 16) for i in range( 8 ) ] )
		c3.state()[0].parameters["f3"] = IECore.V3fVectorData( [ IECore.V3f( i, 100.0 / 16, 100 * 0.5 ** ( i + 2 )) for i in range( 8 ) ] )
		c4.state()[0].parameters["f4"] = IECore.Color4fVectorData( [ IECore.Color4f( i, 100.0 / 32, 100 * 0.5 ** ( i + 2), 0.0 ) for i in range( 8 ) ] )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0.2, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0.2, 0 ) ) ) )
		c3.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, -0.2, 0 ) ) ) )
		c4.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, -0.2, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )
		g.addChild( c3 )
		g.addChild( c4 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.3, 0.3 ), IECore.Color4f( 0.28, 0, 0, 1 ) ),
				( IECore.V2f( 0.7, 0.3 ), IECore.Color4f( 0.28, 0.5, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.7 ), IECore.Color4f( 0.28, 0.5, 0.5, 1 ) ),
				( IECore.V2f( 0.7, 0.7 ), IECore.Color4f( 0.28, 0.25, 0.5, 1 ) ),
			]
		)

	def testUniformIntArrayParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.intArrayShader() )

		c2 = c1.copy()
		c3 = c1.copy()
		c1.state()[0].parameters["i"] = IECore.FloatVectorData( [ i for i in range( 8 ) ] )
		c2.state()[0].parameters["i2"] = IECore.V2fVectorData( [ IECore.V2f( i, 100.0 / 16) for i in range( 8 ) ] )
		c3.state()[0].parameters["i3"] = IECore.V3fVectorData( [ IECore.V3f( i, 100.0 / 16, 100 * 0.5 ** ( i + 2 )) for i in range( 8 ) ] )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0.2, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0.2, 0 ) ) ) )
		c3.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, -0.2, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )
		g.addChild( c3 )

		image = self.renderImage( g )
		shutil.copy( self.__imageFileName, "/tmp/foo.exr" )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.3, 0.3 ), IECore.Color4f( 0.28, 0, 0, 1 ) ),
				( IECore.V2f( 0.7, 0.3 ), IECore.Color4f( 0.28, 0.48, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.7 ), IECore.Color4f( 0.28, 0.48, 0.47, 1 ) ),
			]
		)

	def testMatrixParameters( self ) :

		c1 = IECoreScene.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.matrixShader() )

		c2 = c1.copy()
		c3 = c1.copy()
		c4 = c1.copy()
		c1.state()[0].parameters["m3"] = IECore.M33fData( IECore.M33f( [ i * 0.1 for i in range( 9 ) ] ) )
		c2.state()[0].parameters["m4"] = IECore.M44fData( IECore.M44f( [ i * 0.01 for i in range( 16 ) ] ) )
		c3.state()[0].parameters["m3v"] = IECore.M33fVectorData( [ IECore.M33f( [0,0,0, i * 0.01, i * 0.02, i * 0.03, 0,0,0] ) for i in range( 8 ) ] )
		c4.state()[0].parameters["m4v"] = IECore.M44fVectorData( [ IECore.M44f( [0,0,0,0, i * 0.005, i * 0.01, i * 0.02, 0, 0,0,0,0,0,0,0,0 ] ) for i in range( 8 ) ] )

		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0.2, 0 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0.2, 0 ) ) ) )
		c3.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, -0.2, 0 ) ) ) )
		c4.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, -0.2, 0 ) ) ) )

		g = IECoreScene.Group()
		g.addChild( c1 )
		g.addChild( c2 )
		g.addChild( c3 )
		g.addChild( c4 )

		image = self.renderImage( g )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.3, 0.3 ), IECore.Color4f( 0.6, 0.7, 0.8, 1 ) ),
				( IECore.V2f( 0.7, 0.3 ), IECore.Color4f( 0.20, 0.22, 0.24, 1 ) ),
				( IECore.V2f( 0.3, 0.7 ), IECore.Color4f( 0.28, 0.56, 0.84, 1 ) ),
				( IECore.V2f( 0.7, 0.7 ), IECore.Color4f( 0.14, 0.28, 0.56, 1 ) ),
			]
		)

	def testWireframeWithCustomVertexShader( self ) :

		def renderOffsetImage( offset ) :

			m = self.mesh()

			g = IECoreScene.Group()
			g.addChild( m )

			g.addState(
				IECoreScene.AttributeState(
					{
						"gl:primitive:solid" : IECore.BoolData( True ),
						"gl:primitive:wireframe" : IECore.BoolData( True ),
						"gl:primitive:wireframeColor" : IECore.Color4f( 0, 1, 0, 1 ),
						"gl:primitive:wireframeWidth" : 6.0,
					}
				)
			)
			g.addState( self.offsetShader( offset = offset ) )

			return self.renderImage( g )

		# Offset shader offsets in X in the vertex shader,
		# and renders red in the fragment shader. The wireframe
		# shading should inherit the offset but not the red.

		image = renderOffsetImage( offset = IECore.V3f( 0.2, 0, 0 ) )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.6, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
				( IECore.V2f( 0.65, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
				( IECore.V2f( 0.7, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
				( IECore.V2f( 0.75, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
				( IECore.V2f( 0.8, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
			]
		)

		image = renderOffsetImage( offset = IECore.V3f( -0.2, 0, 0 ) )

		self.assertImageValues(
			image,
			[
				( IECore.V2f( 0.2, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
				( IECore.V2f( 0.25, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
				( IECore.V2f( 0.3, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
				( IECore.V2f( 0.35, 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ),
				( IECore.V2f( 0.4, 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ),
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
			]
		)

	def setUp( self ) :

		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )

	def tearDown( self ) :

		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )

if __name__ == "__main__":
    unittest.main()
