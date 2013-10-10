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

import IECore
import IECoreGL

IECoreGL.init( False )

class ShadingTest( unittest.TestCase ) :

	__imageFileName = os.path.dirname( __file__ ) + "/output/test.exr"

	def mesh( self ) :
	
		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.1 ), IECore.V2f( 0.1 ) ) )
		m["N"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0, 0, 1 ) ] * 4 ) )
		return m
	
	def constantShader( self ) :
	
		s = IECore.Shader( "test", "gl:surface" )
		
		s.parameters["gl:fragmentSource"] = """
			
			in vec3 fragmentCs;
						
			void main()
			{
				gl_FragColor = vec4( fragmentCs, 1.0 );
			}
		"""
		
		return s
	
	def colorShader( self ) :
	
		s = IECore.Shader( "test", "gl:surface" )
		
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
	
		s = IECore.Shader( "test", "gl:surface" )
		
		s.parameters["gl:fragmentSource"] = """
			
			uniform sampler2D sampler;
			
			varying vec2 fragmentst;
						
			void main()
			{
				gl_FragColor = vec4( texture2D( sampler, fragmentst ).rgb, 1 );
			}
		"""
		
		return s
		
	def geometryShader( self ) :
	
		s = IECore.Shader( "test", "gl:surface" )
		
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
		
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
			
			group.render( r )
			
		return IECore.Reader.create( self.__imageFileName ).read()

	def assertImageValues( self, image, tests ) :
	
		evaluator = IECore.ImagePrimitiveEvaluator( image )
		result = evaluator.createResult()
		
		for t in tests :
		
			evaluator.pointAtUV( t[0], result )
			c = IECore.Color4f(
				result.floatPrimVar( evaluator.R() ),
				result.floatPrimVar( evaluator.G() ),
				result.floatPrimVar( evaluator.B() ),
				result.floatPrimVar( evaluator.A() )
			)
			
			self.assertEqual( c, t[1] )

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
	
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["rB"] = IECore.BoolData( 1 )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["rF"] = IECore.FloatData( 1 )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["gI"] = IECore.FloatData( 1 )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["rgF"] = IECore.V2fData( IECore.V2f( 0, 1 ) )
		c2.state()[0].parameters["rgF"] = IECore.V2fData( IECore.V2f( 1, 0 ) )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["rgF"] = IECore.V2iData( IECore.V2i( 0, 1 ) )
		c2.state()[0].parameters["rgF"] = IECore.V2iData( IECore.V2i( 1, 0 ) )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["rgbF"] = IECore.V3fData( IECore.V3f( 0, 1, 1 ) )
		c2.state()[0].parameters["rgbF"] = IECore.V3fData( IECore.V3f( 1, 1, 0 ) )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["rgbI"] = IECore.V3iData( IECore.V3i( 0, 1, 1 ) )
		c2.state()[0].parameters["rgbI"] = IECore.V3iData( IECore.V3i( 1, 1, 0 ) )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
		
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["rgbF"] = IECore.V3iData( IECore.V3i( 0, 1, 1 ) )
		c1.children()[0]["rgbF"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fData( IECore.V3f( 1, 1, 0 ) ) )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.textureShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["sampler"] = self.splineGradient( IECore.Color3f( 1, 0, 0 ), IECore.Color3f( 1, 0, 0 ) )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.textureShader() )
		
		c2 = c1.copy()
		c1.state()[0].parameters["sampler"] = IECore.StringData( os.path.dirname( __file__ ) + "/images/yellow.exr" )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.textureShader() )
		
		yellowImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/images/yellow.exr" ).read()
		yellowCompoundData = IECore.CompoundData( {
			"dataWindow" : IECore.Box2iData( yellowImage.dataWindow ),
			"displayWindow" : IECore.Box2iData( yellowImage.displayWindow ),
			"channels" : {
				"R" : yellowImage["R"].data,
				"G" : yellowImage["G"].data,
				"B" : yellowImage["B"].data,
			}
		} )
		
		
		c2 = c1.copy()
		c1.state()[0].parameters["sampler"] = yellowCompoundData
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		g = IECore.Group()
		g.addChild( self.mesh() )
		
		g.addState(
			IECore.AttributeState(
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
	
		c1 = IECore.Group()
		c1.addChild( self.mesh() )
		c1.addState( self.colorShader() )
		c1.state()[0].parameters["rgbF"] = IECore.V3fData( IECore.V3f( 0, 1, 1 ) )		
		
		c2 = IECore.Group()
		c2.addChild( self.mesh() )
		c2.addState( self.textureShader() )
		c2.state()[0].parameters["sampler"] = IECore.StringData( os.path.dirname( __file__ ) + "/images/yellow.exr" )
		
		c1.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 0.2, 0, 0 ) ) ) )
		c2.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -0.2, 0, 0 ) ) ) )
		
		g = IECore.Group()
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
	
		g = IECore.Group()
		
		p = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) )
		p["type"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "gl:point" ) )
		g.addChild( p )
		
		g.addState( IECore.AttributeState( { "gl:pointsPrimitive:glPointWidth" : IECore.FloatData( 4 ) } ) )
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
	
		g = IECore.Group()
		
		p = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) )
		p["type"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "gl:point" ) )
		g.addChild( p )
		
		g.addState( IECore.AttributeState( { "gl:pointsPrimitive:glPointWidth" : IECore.FloatData( 4 ) } ) )
		g.addState( IECore.Shader( "pointTripler", "gl:surface" ) )
		
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
		
		g = IECore.Group()
		g.addChild( self.mesh() )
		
		g.addState( IECore.AttributeState( { "color" : IECore.Color3f( 1, 0, 0 ) } ) )
		g.addState( self.constantShader() )
		
		image = self.renderImage( g )
		self.assertImageValues( image, [ ( IECore.V2f( 0.5 ), IECore.Color4f( 1, 0, 0, 1 ) ) ] )
		
		# but if there is a Cs parameter, it should override the colour
		# from the attribute state.
		
		g = IECore.Group()
		g.addChild( self.mesh() )
		
		g.addState( IECore.AttributeState( { "color" : IECore.Color3f( 1, 0, 0 ) } ) )
		
		s = self.constantShader()
		s.parameters["Cs"] = IECore.Color3f( 0, 1, 0 )
		g.addState( s )
		
		image = self.renderImage( g )
		self.assertImageValues( image, [ ( IECore.V2f( 0.5 ), IECore.Color4f( 0, 1, 0, 1 ) ) ] )
	
	def testColorAttributeDoesntAffectWireframe( self ) :
	
		g = IECore.Group()
		g.addChild( self.mesh() )
		
		g.addState(
			IECore.AttributeState(
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
				
	def setUp( self ) :
		
		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )
	
	def tearDown( self ) :
		
		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )
		
if __name__ == "__main__":
    unittest.main()
