##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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
import os.path
import shutil

import IECore
import IECoreScene
import IECoreImage

import IECoreGL
IECoreGL.init( False )

# workaround lack of skipIf decorator for
# python < 2.7.
## \todo If we had an IECoreTest module we could
# put this there, along with the expectedFailure
# workaround from GafferTest.
try :
	skipIf = unittest.skipIf
except AttributeError :
	def skipIf( condition, reason ) :
		if condition :
			def ignoreFunction( f ) :
				def noOp( *args ) :
					pass
			return ignoreFunction
		else :
			def callFunction( f ) :
				return f
			return callFunction

class CurvesPrimitiveTest( unittest.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/testCurves.tif"

	def showColorShader( self ) :

		fs = """
		#include "IECoreGL/FragmentShader.h"

		IECOREGL_FRAGMENTSHADER_IN vec3 fragmentCs;
		void main()
		{
			gl_FragColor = vec4( fragmentCs, 1 );
		}
		"""

		s = IECoreScene.Shader( "showColor", "surface" )
		s.parameters["gl:fragmentSource"] = IECore.StringData( fs )

		return s

	def performTest( self, curvesPrimitive, attributes=[], testPixels=[], testImage=None, shader=None, diffImage=None ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			for a in attributes :
				r.setAttribute( a[0], a[1] )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			if shader :
				shader.render( r )
			else :
				r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 0, 0, 1 ) ) } )

			curvesPrimitive.render( r )

		i = IECore.Reader.create( self.outputFileName ).read()
		dimensions = i.dataWindow.size() + IECore.V2i( 1 )

		for t in testPixels :

			xOffset = 1 if t[0].x == 1 else 0
			yOffset = 1 if t[0].y == 1 else 0
			index = dimensions.x * int(dimensions.y * t[0].y - yOffset) + int(dimensions.x * t[0].x) - xOffset
			c = IECore.Color4f(
				i["R"][index],
				i["G"][index],
				i["B"][index],
				i["A"][index],
			)

			self.assertEqual( c, t[1] )

		if testImage :

			# blue where there must be an object
			# red where we don't mind
			# black where there must be nothing

			a = i["A"]

			i2 = IECore.Reader.create( testImage ).read()
			r2 = i2["R"]
			b2 = i2["B"]
			for i in range( r2.size() ) :

				if b2[i] > 0.5 :
					self.assertEqual( a[i], 1 )
				elif r2[i] < 0.5 :
					self.assertEqual( a[i], 0 )

		if diffImage :

			expectedImage = IECore.Reader.create( diffImage ).read()

			self.assertEqual( IECoreImage.ImageDiffOp()( imageA = expectedImage, imageB = i, maxError = 0.05 ).value, False )


	def testAttributes( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )

		r.worldBegin()

		self.assertEqual( r.getAttribute( "gl:curvesPrimitive:useGLLines" ), IECore.BoolData( False ) )
		self.assertEqual( r.getAttribute( "gl:curvesPrimitive:glLineWidth" ), IECore.FloatData( 1 ) )
		self.assertEqual( r.getAttribute( "gl:curvesPrimitive:ignoreBasis" ), IECore.BoolData( False ) )

		r.setAttribute( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) )
		self.assertEqual( r.getAttribute( "gl:curvesPrimitive:useGLLines" ), IECore.BoolData( True ) )

		r.setAttribute( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 2 ) )
		self.assertEqual( r.getAttribute( "gl:curvesPrimitive:glLineWidth" ), IECore.FloatData( 2 ) )

		r.setAttribute( "gl:curvesPrimitive:ignoreBasis", IECore.BoolData( True ) )
		self.assertEqual( r.getAttribute( "gl:curvesPrimitive:ignoreBasis" ), IECore.BoolData( True ) )

		r.worldEnd()

	def testLinearNonPeriodicAsLines( self ) :

		self.performTest(

			IECoreScene.CurvesPrimitive(

				IECore.IntVectorData( [ 4, 4 ] ),
				IECore.CubicBasisf.linear(),
				False,
				IECore.V3fVectorData(
					[
						IECore.V3f( 1, 0, 0 ),
						IECore.V3f( 0, 0, 0 ),
						IECore.V3f( 0, 0.5, 0 ),
						IECore.V3f( 0.5, 0.5, 0 ),

						IECore.V3f( 0.5, 0.5, 0 ),
						IECore.V3f( 1, 0.5, 0 ),
						IECore.V3f( 1, 1, 0 ),
						IECore.V3f( 0, 1, 0 ),
					]
				)

			),
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 4 ) ),
				( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) ),
			],
			[
				( IECore.V2f( 0, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0.25 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 0.25 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 0.75 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0.75 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 1, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
			]
		)

	def testOverriddenLinearPeriodicAsLines( self ) :

		self.performTest(

			IECoreScene.CurvesPrimitive(

				IECore.IntVectorData( [ 4 ] ),
				IECore.CubicBasisf.bSpline(),
				True,
				IECore.V3fVectorData(
					[
						IECore.V3f( 1, 0, 0 ),
						IECore.V3f( 0, 0, 0 ),
						IECore.V3f( 0, 1, 0 ),
						IECore.V3f( 1, 1, 0 ),
					]
				)

			),
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 4 ) ),
				( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) ),
				( "gl:curvesPrimitive:ignoreBasis", IECore.BoolData( True ) ),
			],
			[
				( IECore.V2f( 0, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.1, 0.1 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.9, 0.1 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.9, 0.9 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.1, 0.9 ), IECore.Color4f( 0, 0, 0, 0 ) ),

			]
		)

	@skipIf( IECoreGL.glslVersion() >= 150, "Modern GLSL version, no need to test fallback" )
	def testFallbackLinearPeriodicAsLines( self ) :

		# when the GLSL version is < 150, we should fall back
		# to rendering everything as linear lines.

		self.performTest(

			IECoreScene.CurvesPrimitive(

				IECore.IntVectorData( [ 4 ] ),
				IECore.CubicBasisf.bSpline(),
				True,
				IECore.V3fVectorData(
					[
						IECore.V3f( 1, 0, 0 ),
						IECore.V3f( 0, 0, 0 ),
						IECore.V3f( 0, 1, 0 ),
						IECore.V3f( 1, 1, 0 ),
					]
				)

			),
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 4 ) ),
			],
			[
				( IECore.V2f( 0, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.1, 0.1 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.9, 0.1 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.9, 0.9 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.1, 0.9 ), IECore.Color4f( 0, 0, 0, 0 ) ),

			]
		)

	def testLinearPeriodicAsLines( self ) :

		self.performTest(

			IECoreScene.CurvesPrimitive(

				IECore.IntVectorData( [ 4 ] ),
				IECore.CubicBasisf.linear(),
				True,
				IECore.V3fVectorData(
					[
						IECore.V3f( 1, 0, 0 ),
						IECore.V3f( 0, 0, 0 ),
						IECore.V3f( 0, 1, 0 ),
						IECore.V3f( 1, 1, 0 ),
					]
				)

			),
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 4 ) ),
				( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) ),
			],
			[
				( IECore.V2f( 0, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 1, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 1 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0, 0.5 ), IECore.Color4f( 0, 0, 1, 1 ) ),
				( IECore.V2f( 0.5, 0.5 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.1, 0.1 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.9, 0.1 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.9, 0.9 ), IECore.Color4f( 0, 0, 0, 0 ) ),
				( IECore.V2f( 0.1, 0.9 ), IECore.Color4f( 0, 0, 0, 0 ) ),

			]
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testBSplinePeriodicAsLines( self ) :

		self.performTest(

			IECoreScene.CurvesPrimitive(

				IECore.IntVectorData( [ 4 ] ),
				IECore.CubicBasisf.bSpline(),
				True,
				IECore.V3fVectorData(
					[
						IECore.V3f( 1, 0, 0 ),
						IECore.V3f( 0, 0, 0 ),
						IECore.V3f( 0, 1, 0 ),
						IECore.V3f( 1, 1, 0 ),
					]
				)

			),
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 2 ) ),
				( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/periodicBSpline.tif"
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testBSplinePeriodicAsRibbons( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.bSpline(),
			True,
			IECore.V3fVectorData(
				[
					IECore.V3f( 1, 0, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 1, 0 ),
					IECore.V3f( 1, 1, 0 ),
				]
			)

		)
		c["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performTest(

			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/bSplineCircle.tif"
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testBezierAsRibbons( self ) :

		c = IECoreScene.CurvesPrimitive(

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
		c["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performTest(
			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/bezierHorseShoe.tif"
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testLinearRibbons( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.linear(),
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
		c["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.035 ) )

		self.performTest(
			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/linearHorseShoeRibbon.tif"
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testLinearPeriodicRibbons( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.linear(),
			True,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.8, 0.2, 0 ),
					IECore.V3f( 0.2, 0.2, 0 ),
					IECore.V3f( 0.2, 0.8, 0 ),
					IECore.V3f( 0.8, 0.8, 0 ),
				]
			)

		)
		c["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performTest(
			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/linearPeriodicRibbon.tif"
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testSeveralBSplineRibbons( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4, 4 ] ),
			IECore.CubicBasisf.bSpline(),
			True,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.4, 0.2, 0 ),
					IECore.V3f( 0.2, 0.2, 0 ),
					IECore.V3f( 0.2, 0.4, 0 ),
					IECore.V3f( 0.4, 0.4, 0 ),

					IECore.V3f( 0.8, 0.6, 0 ),
					IECore.V3f( 0.6, 0.6, 0 ),
					IECore.V3f( 0.6, 0.8, 0 ),
					IECore.V3f( 0.8, 0.8, 0 ),
				]
			)

		)
		c["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.035 ) )

		self.performTest(
			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/twoBSplineCircles.tif"
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testSeveralBSplineLines( self ) :

		self.performTest(
			IECoreScene.CurvesPrimitive(

				IECore.IntVectorData( [ 4, 4 ] ),
				IECore.CubicBasisf.bSpline(),
				True,
				IECore.V3fVectorData(
					[
						IECore.V3f( 0.4, 0.2, 0 ),
						IECore.V3f( 0.2, 0.2, 0 ),
						IECore.V3f( 0.2, 0.4, 0 ),
						IECore.V3f( 0.4, 0.4, 0 ),

						IECore.V3f( 0.8, 0.6, 0 ),
						IECore.V3f( 0.6, 0.6, 0 ),
						IECore.V3f( 0.6, 0.8, 0 ),
						IECore.V3f( 0.8, 0.8, 0 ),
					]
				)

			),
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 1 ) ),
				( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) ),
			],
			[
			],
			diffImage = os.path.dirname( __file__ ) + "/expectedOutput/twoBSplineCirclesAsLines.tif"
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testRibbonWindingOrder( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.bSpline(),
			True,
			IECore.V3fVectorData(
				[
					IECore.V3f( 1, 0, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 1, 0 ),
					IECore.V3f( 1, 1, 0 ),
				]
			)

		)
		c["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )


		self.performTest(
			c,
			[
				( "doubleSided", IECore.BoolData( False ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/bSplineCircle.tif"
		)

	@skipIf( IECoreGL.glslVersion() < 150, "Insufficient GLSL version" )
	def testLinearRibbonWindingOrder( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.linear(),
			True,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.8, 0.2, 0 ),
					IECore.V3f( 0.2, 0.2, 0 ),
					IECore.V3f( 0.2, 0.8, 0 ),
					IECore.V3f( 0.8, 0.8, 0 ),
				]
			)

		)
		c["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performTest(
			c,
			[
				( "doubleSided", IECore.BoolData( False ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/linearPeriodicRibbon.tif"
		)

	def testLinearLinesWithVertexColor( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4, 4 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 1, 0, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 0.5, 0 ),
					IECore.V3f( 0.5, 0.5, 0 ),

					IECore.V3f( 0.5, 0.5, 0 ),
					IECore.V3f( 1, 0.5, 0 ),
					IECore.V3f( 1, 1, 0 ),
					IECore.V3f( 0, 1, 0 ),
				]
			)

		)
		c["Cs"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.Color3fVectorData(
				[
					IECore.Color3f( 1, 0, 0 ),
					IECore.Color3f( 0, 1, 0 ),
					IECore.Color3f( 0, 0, 1 ),
					IECore.Color3f( 0, 1, 0 ),

					IECore.Color3f( 1, 0, 0 ),
					IECore.Color3f( 0, 1, 0 ),
					IECore.Color3f( 0, 0, 1 ),
					IECore.Color3f( 0, 1, 0 ),
				]
			)
		)

		self.performTest(

			c,
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 4 ) ),
				( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) ),
			],
			diffImage = os.path.dirname( __file__ ) + "/expectedOutput/linearLinesWithVertexColor.tif",
			shader = self.showColorShader(),
		)

	def testLinearLinesWithUniformColor( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4, 4 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 1, 0, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 0.5, 0 ),
					IECore.V3f( 0.5, 0.5, 0 ),

					IECore.V3f( 0.5, 0.5, 0 ),
					IECore.V3f( 1, 0.5, 0 ),
					IECore.V3f( 1, 1, 0 ),
					IECore.V3f( 0, 1, 0 ),
				]
			)

		)
		c["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ IECore.Color3f( 1, 0, 0 ), IECore.Color3f( 0, 1, 0 ) ] ) )

		self.performTest(

			c,
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 4 ) ),
				( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) ),
			],
			diffImage = os.path.dirname( __file__ ) + "/expectedOutput/linearLinesWithUniformColor.tif",
			shader = self.showColorShader(),
		)

	def testLinearLinesWithConstantColor( self ) :

		c = IECoreScene.CurvesPrimitive(

			IECore.IntVectorData( [ 4, 4 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 1, 0, 0 ),
					IECore.V3f( 0, 0, 0 ),
					IECore.V3f( 0, 0.5, 0 ),
					IECore.V3f( 0.5, 0.5, 0 ),

					IECore.V3f( 0.5, 0.5, 0 ),
					IECore.V3f( 1, 0.5, 0 ),
					IECore.V3f( 1, 1, 0 ),
					IECore.V3f( 0, 1, 0 ),
				]
			)

		)
		c["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) )

		self.performTest(

			c,
			[
				( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 4 ) ),
				( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) ),
			],
			diffImage = os.path.dirname( __file__ ) + "/expectedOutput/linearLinesWithConstantColor.tif",
			shader = self.showColorShader(),
		)

	def setUp( self ) :

		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )

	def tearDown( self ) :

		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )

if __name__ == "__main__":
    unittest.main()
