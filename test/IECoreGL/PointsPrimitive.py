##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

import unittest
import random
import os

import IECore
import IECoreGL

IECoreGL.init( False )

class TestPointsPrimitive( unittest.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/testPoints.tif"

	def testStateComponentsInstantiation( self ):

		IECoreGL.PointsPrimitive.UseGLPoints( IECoreGL.GLPointsUsage.ForPointsOnly )
		IECoreGL.PointsPrimitive.GLPointWidth( 1.5 )

	def testStateComponentsUsage( self ):

		g = IECoreGL.Group()
		g.getState().add( IECoreGL.PointsPrimitive.UseGLPoints( IECoreGL.GLPointsUsage.ForPointsAndDisks ) )
		g.getState().add( IECoreGL.PointsPrimitive.GLPointWidth( 2.3 ) )


	def testVertexAttributes( self ) :

		fragmentSource = """
		uniform int greyTo255;

		void main()
		{
			float g = float( greyTo255 ) / 255.0;
			gl_FragColor = vec4( g, g, g, 1 );
		}
		"""

		numPoints = 100
		p = IECore.V3fVectorData( numPoints )
		g = IECore.IntVectorData( numPoints )
		random.seed( 0 )
		for i in range( 0, numPoints ) :
			p[i] = IECore.V3f( random.random() * 4, random.random() * 4, random.random() * 4 )
			g[i] = int( random.uniform( 0.0, 255.0 ) )
		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, p )
		g = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, g )

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )
		
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -2, -2, -10 ) ) )
			r.shader( "surface", "grey", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )
			r.points( numPoints, { "P" : p, "greyTo255" : g } )

		reader = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/pointVertexAttributes.tif" )
		reader['colorSpace'] = 'linear'
		expectedImage = reader.read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()
		
		self.assertEqual( IECore.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.05 ).value, False )

	def testEmptyPointsPrimitive( self ):

		fragmentSource = """
		uniform int greyTo255;

		void main()
		{
			float g = float( greyTo255 ) / 255.0;
			gl_FragColor = vec4( g, g, g, 1 );
		}
		"""
		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData() )
		g = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData() )
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )
		with IECore.WorldBlock( r ) :
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

		r.camera( "main", {
				"projection" : IECore.StringData( projection ),
				"projection:fov" : IECore.FloatData( 20 ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )
		
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )
			
			r.shader( "surface", "white", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )
			r.points( p.size(), { 
					"P" : IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, p ),
					"constantwidth" : IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.75 ) ),
					"type" : IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( particleType ) )
				}
			)

		expectedImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/" + expectedImage ).read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()
		
		self.assertEqual( IECore.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.05 ).value, False )

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

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"projection:fov" : IECore.FloatData( 20 ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )
		
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -6 ) ) )
			
			r.shader( "surface", "white", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )
		
			with IECore.AttributeBlock( r ) :
			
				r.setAttribute( "gl:pointsPrimitive:glPointWidth", IECore.FloatData( 20 ) )
			
				r.points( 1, { 
						"P" : IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) ),
						"type" : IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "gl:point" ) )
					}
				)
				
			with IECore.AttributeBlock( r ) :
			
				r.setAttribute( "gl:pointsPrimitive:glPointWidth", IECore.FloatData( 10 ) )
			
				r.points( 1, { 
						"P" : IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] ) ),
						"type" : IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.StringData( "gl:point" ) )
					}
				)	

		expectedImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/glPoints.tif" ).read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()
		
		self.assertEqual( IECore.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.05 ).value, False )
	
	def tearDown( self ) :
		
		if os.path.exists( self.outputFileName ) :
			
			os.remove( self.outputFileName )

if __name__ == "__main__":
    unittest.main()
