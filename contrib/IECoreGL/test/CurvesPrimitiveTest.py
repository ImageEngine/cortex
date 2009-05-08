##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import IECore

import IECoreGL
IECoreGL.init( False )

class CurvesPrimitiveTest( unittest.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/testCurves.tif"

	def performTest( self, curvesPrimitive, attributes=[], testPixels=[], testImage=None ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )

		r.worldBegin()

		for a in attributes :
			r.setAttribute( a[0], a[1] )

		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 0, 0, 1 ) ) } )
		curvesPrimitive.render( r )

		r.worldEnd()

		i = IECore.Reader.create( self.outputFileName ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		r = e.R()
		g = e.G()
		b = e.B()

		for t in testPixels :

			e.pointAtUV( t[0], result )
			c = IECore.Color4f(
				result.floatPrimVar( r ),
				result.floatPrimVar( g ),
				result.floatPrimVar( b ),
				result.floatPrimVar( a )
			)

			self.assertEqual( c, t[1] )

		if testImage :

			# blue where there must be an object
			# red where we don't mind
			# black where there must be nothing

			a = i["A"].data

			i2 = IECore.Reader.create( testImage ).read()
			r2 = i2["R"].data
			b2 = i2["B"].data
			for i in range( r2.size() ) :

				if b2[i] > 0.5 :
					self.assertEqual( a[i], 1 )
				elif r2[i] < 0.5 :
					self.assertEqual( a[i], 0 )

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

			IECore.CurvesPrimitive(

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

			IECore.CurvesPrimitive(

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

	def testLinearPeriodicAsLines( self ) :

		self.performTest(

			IECore.CurvesPrimitive(

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

	def testBSplinePeriodicAsLines( self ) :

		self.performTest(

			IECore.CurvesPrimitive(

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

	def testBSplinePeriodicAsRibbons( self ) :

		c = IECore.CurvesPrimitive(

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
		c["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performTest(

			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/bSplineCircle.tif"
		)

	def testBezierAsRibbons( self ) :

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

		self.performTest(
			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/bezierHorseShoe.tif"
		)

	def testLinearRibbons( self ) :

		c = IECore.CurvesPrimitive(

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
		c["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.035 ) )

		self.performTest(
			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/linearHorseShoeRibbon.tif"
		)

	def testLinearPeriodicRibbons( self ) :

		c = IECore.CurvesPrimitive(

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
		c["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performTest(
			c,
			[
				( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/linearPeriodicRibbon.tif"
		)

	def testSeveralBSplineRibbons( self ) :

		c = IECore.CurvesPrimitive(

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
		c["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.035 ) )

		self.performTest(
			c,
			[
				#( "gl:primitive:wireframe", IECore.BoolData( True ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/twoBSplineCircles.tif"
		)

	def testSeveralBSplineLines( self ) :

		self.performTest(
			IECore.CurvesPrimitive(

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
			os.path.dirname( __file__ ) + "/images/twoBSplineCirclesAsLines.tif"
		)

	def testRibbonWindingOrder( self ) :

		c = IECore.CurvesPrimitive(

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
		c["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )


		self.performTest(
			c,
			[
				( "doubleSided", IECore.BoolData( False ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/bSplineCircle.tif"
		)

	def testLinearRibbonWindingOrder( self ) :

		c = IECore.CurvesPrimitive(

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
		c["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.05 ) )

		self.performTest(
			c,
			[
				( "doubleSided", IECore.BoolData( False ) ),
			],
			[
			],
			os.path.dirname( __file__ ) + "/images/linearPeriodicRibbon.tif"
		)

	def tearDown( self ) :

		if os.path.exists( self.outputFileName ) :
			os.remove( self.outputFileName )

if __name__ == "__main__":
    unittest.main()
