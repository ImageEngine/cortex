##########################################################################
#
#  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

import time
import threading
import math
import unittest
import IECore

## \todo: add tests for Varying prim vars

class CurvesPrimitiveEvaluatorTest( unittest.TestCase ) :

	def runPointAtVTest( self, curvesPrimitive, expectedPositions=None, expectedLengths=None, visualTest=False, printPoints=False ) :

		e = IECore.CurvesPrimitiveEvaluator( curvesPrimitive )
		r = e.createResult()

		p = IECore.V3fVectorData()
		for ci in range( 0, curvesPrimitive.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) ) :
			numSamples = 50
			for i in range( 0, numSamples ) :

				s = e.pointAtV( ci, float( i ) / float( numSamples - 1 ), r )
				self.failUnless( s )

				p.append( r.point() )

		if expectedPositions :

			self.assertEqual( len( p ), len( expectedPositions ) )
			for i in range( 0, len( p ) ) :
				self.failUnless( p[i].equalWithAbsError( expectedPositions[i], 0.00001 ) )

		if expectedLengths :

			self.assertEqual( curvesPrimitive.numCurves(), len( expectedLengths ) )
			linearNonPeriodic = ( curvesPrimitive.basis() == IECore.CubicBasisf.linear() ) and not curvesPrimitive.periodic()
			for i in range( 0, curvesPrimitive.numCurves() ) :
				if linearNonPeriodic :
					self.assertEqual( e.curveLength( i ), expectedLengths[i] )
				else :
					self.assertAlmostEqual( e.curveLength( i ), expectedLengths[i], 5 )

		if printPoints :

			print repr( p ).replace( "),", "),\n" )

		if visualTest :

			import IECoreGL
			IECoreGL.init( False )

			r = IECoreGL.Renderer()
			r.setOption( "gl:mode", IECore.StringData( "deferred" ) )

			pointsPrimitive = IECore.PointsPrimitive( p )
			pointsPrimitive["constantwidth"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.1 ) )

			with IECore.WorldBlock( r ) :

				w = IECoreGL.SceneViewer( "scene", r.scene() )

				with IECore.AttributeBlock( r ) :

					r.shader( "surface", "red", { "gl:fragmentSource" : IECore.StringData( "void main() { gl_FragColor = vec4( 1, 0, 0, 1 ); }" ) } )
					r.setAttribute( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ) )
					curvesPrimitive.render( r )

				with IECore.AttributeBlock( r ) :

					r.shader( "surface", "blue", { "gl:fragmentSource" : IECore.StringData( "void main() { gl_FragColor = vec4( 0, 0, 1, 1 ); }" ) } )
					pointsPrimitive.render( r )

			w.start()

	def testLinearSubsetLength( self ) :

		pts = IECore.V3fVectorData()
		pts.append( IECore.V3f( 0,0,0 ) )
		pts.append( IECore.V3f( 1,0,0 ) )
		pts.append( IECore.V3f( 1,1,0 ) )
		pts.append( IECore.V3f( 2,1,0 ) )
		pts.append( IECore.V3f( 2,1,1 ) )

		c = IECore.CurvesPrimitive( IECore.IntVectorData( [len( pts )] ), IECore.CubicBasisf.linear(), False, pts )
		e = IECore.CurvesPrimitiveEvaluator( c )

		self.assertEqual( e.curveLength( 0, 0.125, 0.875 ), 3 )


	def testCurveLengthAccuracy( self ) :

		pts = IECore.V3fVectorData()
		pts.append( IECore.V3f( 0,0,0 ) )
		pts.append( IECore.V3f( 0,0,0 ) )
		pts.append( IECore.V3f( 0,0,0 ) )
		pts.append( IECore.V3f( 1000,0,0 ) )
		pts.append( IECore.V3f( 1000,0,0 ) )
		pts.append( IECore.V3f( 1000,0,0 ) )

		c = IECore.CurvesPrimitive( IECore.IntVectorData( [len( pts )] ), IECore.CubicBasisf.bSpline(), False, pts )
		e = IECore.CurvesPrimitiveEvaluator( c )

		self.assertAlmostEqual( e.curveLength( 0 ), 1000.0, 3 )

		# measure the length of a semicircle:
		pts = IECore.V3fVectorData()
		pts.append( IECore.V3f( 1,0,0 ) )
		pts.append( IECore.V3f( 1,0,0 ) )

		for i in range( 0,201 ):
			angle = math.pi * float(i) / 200
			pts.append( IECore.V3f( math.cos( angle ), math.sin( angle ), 0 ) )

		pts.append( IECore.V3f( -1,0,0 ) )
		pts.append( IECore.V3f( -1,0,0 ) )

		c = IECore.CurvesPrimitive( IECore.IntVectorData( [len( pts )] ), IECore.CubicBasisf.bSpline(), False, pts )
		e = IECore.CurvesPrimitiveEvaluator( c )

		self.assertAlmostEqual( e.curveLength( 0 ), 3.1415926, 3 )

		# measure the perimeter of a circle:
		pts = IECore.V3fVectorData()

		for i in range( 0,401 ):
			angle = 2 * math.pi * float(i) / 400
			pts.append( IECore.V3f( math.cos( angle ), math.sin( angle ), 0 ) )

		c = IECore.CurvesPrimitive( IECore.IntVectorData( [len( pts )] ), IECore.CubicBasisf.bSpline(), True, pts )
		e = IECore.CurvesPrimitiveEvaluator( c )

		self.assertAlmostEqual( e.curveLength( 0 ), 2 * 3.1415926, 3 )


	def testCurveLengthFreeze( self ) :

		# this test freezes because of a precision bug...
		pts = IECore.V3fVectorData()

		for i in range( 0,1008 ):
			pts.append( IECore.V3f( i,0,0 ) )

		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 1008 ] ),
			IECore.CubicBasisf.bSpline(),
			False,
			pts
		)

		e = IECore.CurvesPrimitiveEvaluator( c )

		e.curveLength( 0, ( 3008.0 - 1.0 ) / 3008, 1 )

	def test3SegmentBSpline( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 6 ] ),
			IECore.CubicBasisf.bSpline(),
			False,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 2, 1, 0 ),
					v( 2, 0, 0 )
				]
			)
		)

		expected = IECore.V3fVectorData( [
			IECore.V3f( 0.166667, 0.166667, 0 ),
			IECore.V3f( 0.199077, 0.137929, 0 ),
			IECore.V3f( 0.234776, 0.112939, 0 ),
			IECore.V3f( 0.273306, 0.0916979, 0 ),
			IECore.V3f( 0.314207, 0.0742052, 0 ),
			IECore.V3f( 0.357021, 0.0604609, 0 ),
			IECore.V3f( 0.401288, 0.0504651, 0 ),
			IECore.V3f( 0.44655, 0.0442177, 0 ),
			IECore.V3f( 0.492347, 0.0417187, 0 ),
			IECore.V3f( 0.538221, 0.0429682, 0 ),
			IECore.V3f( 0.583712, 0.0479661, 0 ),
			IECore.V3f( 0.628362, 0.0567125, 0 ),
			IECore.V3f( 0.671711, 0.0692073, 0 ),
			IECore.V3f( 0.713301, 0.0854505, 0 ),
			IECore.V3f( 0.752673, 0.105442, 0 ),
			IECore.V3f( 0.789366, 0.129182, 0 ),
			IECore.V3f( 0.822924, 0.156671, 0 ),
			IECore.V3f( 0.852931, 0.187885, 0 ),
			IECore.V3f( 0.879502, 0.222539, 0 ),
			IECore.V3f( 0.903089, 0.260176, 0 ),
			IECore.V3f( 0.924152, 0.300338, 0 ),
			IECore.V3f( 0.943149, 0.342566, 0 ),
			IECore.V3f( 0.960539, 0.386399, 0 ),
			IECore.V3f( 0.976783, 0.431381, 0 ),
			IECore.V3f( 0.992337, 0.47705, 0 ),
			IECore.V3f( 1.00766, 0.52295, 0 ),
			IECore.V3f( 1.02322, 0.568619, 0 ),
			IECore.V3f( 1.03946, 0.613601, 0 ),
			IECore.V3f( 1.05685, 0.657435, 0 ),
			IECore.V3f( 1.07585, 0.699662, 0 ),
			IECore.V3f( 1.09691, 0.739824, 0 ),
			IECore.V3f( 1.1205, 0.777461, 0 ),
			IECore.V3f( 1.14707, 0.812115, 0 ),
			IECore.V3f( 1.17708, 0.843329, 0 ),
			IECore.V3f( 1.21063, 0.870818, 0 ),
			IECore.V3f( 1.24733, 0.894558, 0 ),
			IECore.V3f( 1.2867, 0.914549, 0 ),
			IECore.V3f( 1.32829, 0.930793, 0 ),
			IECore.V3f( 1.37164, 0.943287, 0 ),
			IECore.V3f( 1.41629, 0.952034, 0 ),
			IECore.V3f( 1.46178, 0.957032, 0 ),
			IECore.V3f( 1.50765, 0.958281, 0 ),
			IECore.V3f( 1.55345, 0.955782, 0 ),
			IECore.V3f( 1.59871, 0.949535, 0 ),
			IECore.V3f( 1.64298, 0.939539, 0 ),
			IECore.V3f( 1.68579, 0.925795, 0 ),
			IECore.V3f( 1.72669, 0.908302, 0 ),
			IECore.V3f( 1.76522, 0.887061, 0 ),
			IECore.V3f( 1.80092, 0.862071, 0 ),
			IECore.V3f( 1.83333, 0.833333, 0 )
		] )

		lengths = IECore.FloatVectorData( [ 2.2106194496154785 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def test3SegmentBSplineDoubledEndpoints( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 8 ] ),
			IECore.CubicBasisf.bSpline(),
			False,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 2, 1, 0 ),
					v( 2, 0, 0 ),
					v( 2, 0, 0 )
				]
			)
		)

		expected = IECore.V3fVectorData( [
			 IECore.V3f( 0, 0.833333, 0 ),
			 IECore.V3f( 0.00017708, 0.777461, 0 ),
			 IECore.V3f( 0.00141664, 0.713301, 0 ),
			 IECore.V3f( 0.00478117, 0.642979, 0 ),
			 IECore.V3f( 0.0113331, 0.568619, 0 ),
			 IECore.V3f( 0.0221351, 0.492347, 0 ),
			 IECore.V3f( 0.0382494, 0.416288, 0 ),
			 IECore.V3f( 0.0607386, 0.342566, 0 ),
			 IECore.V3f( 0.0906652, 0.273306, 0 ),
			 IECore.V3f( 0.129092, 0.210634, 0 ),
			 IECore.V3f( 0.177076, 0.156671, 0 ),
			 IECore.V3f( 0.234776, 0.112939, 0 ),
			 IECore.V3f( 0.300338, 0.0796196, 0 ),
			 IECore.V3f( 0.371638, 0.0567125, 0 ),
			 IECore.V3f( 0.44655, 0.0442177, 0 ),
			 IECore.V3f( 0.52295, 0.0421352, 0 ),
			 IECore.V3f( 0.598712, 0.0504651, 0 ),
			 IECore.V3f( 0.671711, 0.0692073, 0 ),
			 IECore.V3f( 0.739824, 0.0983618, 0 ),
			 IECore.V3f( 0.800923, 0.137929, 0 ),
			 IECore.V3f( 0.852931, 0.187885, 0 ),
			 IECore.V3f( 0.89553, 0.247327, 0 ),
			 IECore.V3f( 0.930691, 0.314207, 0 ),
			 IECore.V3f( 0.960539, 0.386399, 0 ),
			 IECore.V3f( 0.987201, 0.461779, 0 ),
			 IECore.V3f( 1.0128, 0.538221, 0 ),
			 IECore.V3f( 1.03946, 0.613601, 0 ),
			 IECore.V3f( 1.06931, 0.685793, 0 ),
			 IECore.V3f( 1.10447, 0.752673, 0 ),
			 IECore.V3f( 1.14707, 0.812115, 0 ),
			 IECore.V3f( 1.19908, 0.862071, 0 ),
			 IECore.V3f( 1.26018, 0.901638, 0 ),
			 IECore.V3f( 1.32829, 0.930793, 0 ),
			 IECore.V3f( 1.40129, 0.949535, 0 ),
			 IECore.V3f( 1.47705, 0.957865, 0 ),
			 IECore.V3f( 1.55345, 0.955782, 0 ),
			 IECore.V3f( 1.62836, 0.943288, 0 ),
			 IECore.V3f( 1.69966, 0.92038, 0 ),
			 IECore.V3f( 1.76522, 0.887061, 0 ),
			 IECore.V3f( 1.82292, 0.843329, 0 ),
			 IECore.V3f( 1.87091, 0.789366, 0 ),
			 IECore.V3f( 1.90933, 0.726694, 0 ),
			 IECore.V3f( 1.93926, 0.657435, 0 ),
			 IECore.V3f( 1.96175, 0.583712, 0 ),
			 IECore.V3f( 1.97786, 0.507653, 0 ),
			 IECore.V3f( 1.98867, 0.431381, 0 ),
			 IECore.V3f( 1.99522, 0.357021, 0 ),
			 IECore.V3f( 1.99858, 0.286699, 0 ),
			 IECore.V3f( 1.99982, 0.222539, 0 ),
			 IECore.V3f( 2, 0.166667, 0 ) ] )

		lengths = IECore.FloatVectorData( [ 3.61808 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def test2Curve3SegmentBSpline( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 6, 6 ] ),
			IECore.CubicBasisf.bSpline(),
			False,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 2, 1, 0 ),
					v( 2, 0, 0 ),
					v( 0, 2, 0 ),
					v( 0, 1, 0 ),
					v( 1, 1, 0 ),
					v( 1, 2, 0 ),
					v( 2, 2, 0 ),
					v( 2, 1, 0 )
				]
			)
		)

		expected = IECore.V3fVectorData( [
			IECore.V3f( 0.166667, 0.166667, 0 ),
			IECore.V3f( 0.199077, 0.137929, 0 ),
			IECore.V3f( 0.234776, 0.112939, 0 ),
			IECore.V3f( 0.273306, 0.0916979, 0 ),
			IECore.V3f( 0.314207, 0.0742052, 0 ),
			IECore.V3f( 0.357021, 0.0604609, 0 ),
			IECore.V3f( 0.401288, 0.0504651, 0 ),
			IECore.V3f( 0.44655, 0.0442177, 0 ),
			IECore.V3f( 0.492347, 0.0417187, 0 ),
			IECore.V3f( 0.538221, 0.0429682, 0 ),
			IECore.V3f( 0.583712, 0.0479661, 0 ),
			IECore.V3f( 0.628362, 0.0567125, 0 ),
			IECore.V3f( 0.671711, 0.0692073, 0 ),
			IECore.V3f( 0.713301, 0.0854505, 0 ),
			IECore.V3f( 0.752673, 0.105442, 0 ),
			IECore.V3f( 0.789366, 0.129182, 0 ),
			IECore.V3f( 0.822924, 0.156671, 0 ),
			IECore.V3f( 0.852931, 0.187885, 0 ),
			IECore.V3f( 0.879502, 0.222539, 0 ),
			IECore.V3f( 0.903089, 0.260176, 0 ),
			IECore.V3f( 0.924152, 0.300338, 0 ),
			IECore.V3f( 0.943149, 0.342566, 0 ),
			IECore.V3f( 0.960539, 0.386399, 0 ),
			IECore.V3f( 0.976783, 0.431381, 0 ),
			IECore.V3f( 0.992337, 0.47705, 0 ),
			IECore.V3f( 1.00766, 0.52295, 0 ),
			IECore.V3f( 1.02322, 0.568619, 0 ),
			IECore.V3f( 1.03946, 0.613601, 0 ),
			IECore.V3f( 1.05685, 0.657435, 0 ),
			IECore.V3f( 1.07585, 0.699662, 0 ),
			IECore.V3f( 1.09691, 0.739824, 0 ),
			IECore.V3f( 1.1205, 0.777461, 0 ),
			IECore.V3f( 1.14707, 0.812115, 0 ),
			IECore.V3f( 1.17708, 0.843329, 0 ),
			IECore.V3f( 1.21063, 0.870818, 0 ),
			IECore.V3f( 1.24733, 0.894558, 0 ),
			IECore.V3f( 1.2867, 0.914549, 0 ),
			IECore.V3f( 1.32829, 0.930793, 0 ),
			IECore.V3f( 1.37164, 0.943287, 0 ),
			IECore.V3f( 1.41629, 0.952034, 0 ),
			IECore.V3f( 1.46178, 0.957032, 0 ),
			IECore.V3f( 1.50765, 0.958281, 0 ),
			IECore.V3f( 1.55345, 0.955782, 0 ),
			IECore.V3f( 1.59871, 0.949535, 0 ),
			IECore.V3f( 1.64298, 0.939539, 0 ),
			IECore.V3f( 1.68579, 0.925795, 0 ),
			IECore.V3f( 1.72669, 0.908302, 0 ),
			IECore.V3f( 1.76522, 0.887061, 0 ),
			IECore.V3f( 1.80092, 0.862071, 0 ),
			IECore.V3f( 1.83333, 0.833333, 0 ),
			IECore.V3f( 0.166667, 1.16667, 0 ),
			IECore.V3f( 0.199077, 1.13793, 0 ),
			IECore.V3f( 0.234776, 1.11294, 0 ),
			IECore.V3f( 0.273306, 1.0917, 0 ),
			IECore.V3f( 0.314207, 1.07421, 0 ),
			IECore.V3f( 0.357021, 1.06046, 0 ),
			IECore.V3f( 0.401288, 1.05047, 0 ),
			IECore.V3f( 0.44655, 1.04422, 0 ),
			IECore.V3f( 0.492347, 1.04172, 0 ),
			IECore.V3f( 0.538221, 1.04297, 0 ),
			IECore.V3f( 0.583712, 1.04797, 0 ),
			IECore.V3f( 0.628362, 1.05671, 0 ),
			IECore.V3f( 0.671711, 1.06921, 0 ),
			IECore.V3f( 0.713301, 1.08545, 0 ),
			IECore.V3f( 0.752673, 1.10544, 0 ),
			IECore.V3f( 0.789366, 1.12918, 0 ),
			IECore.V3f( 0.822924, 1.15667, 0 ),
			IECore.V3f( 0.852931, 1.18789, 0 ),
			IECore.V3f( 0.879502, 1.22254, 0 ),
			IECore.V3f( 0.903089, 1.26018, 0 ),
			IECore.V3f( 0.924152, 1.30034, 0 ),
			IECore.V3f( 0.943149, 1.34257, 0 ),
			IECore.V3f( 0.960539, 1.3864, 0 ),
			IECore.V3f( 0.976783, 1.43138, 0 ),
			IECore.V3f( 0.992337, 1.47705, 0 ),
			IECore.V3f( 1.00766, 1.52295, 0 ),
			IECore.V3f( 1.02322, 1.56862, 0 ),
			IECore.V3f( 1.03946, 1.6136, 0 ),
			IECore.V3f( 1.05685, 1.65743, 0 ),
			IECore.V3f( 1.07585, 1.69966, 0 ),
			IECore.V3f( 1.09691, 1.73982, 0 ),
			IECore.V3f( 1.1205, 1.77746, 0 ),
			IECore.V3f( 1.14707, 1.81211, 0 ),
			IECore.V3f( 1.17708, 1.84333, 0 ),
			IECore.V3f( 1.21063, 1.87082, 0 ),
			IECore.V3f( 1.24733, 1.89456, 0 ),
			IECore.V3f( 1.2867, 1.91455, 0 ),
			IECore.V3f( 1.32829, 1.93079, 0 ),
			IECore.V3f( 1.37164, 1.94329, 0 ),
			IECore.V3f( 1.41629, 1.95203, 0 ),
			IECore.V3f( 1.46178, 1.95703, 0 ),
			IECore.V3f( 1.50765, 1.95828, 0 ),
			IECore.V3f( 1.55345, 1.95578, 0 ),
			IECore.V3f( 1.59871, 1.94954, 0 ),
			IECore.V3f( 1.64298, 1.93954, 0 ),
			IECore.V3f( 1.68579, 1.92579, 0 ),
			IECore.V3f( 1.72669, 1.9083, 0 ),
			IECore.V3f( 1.76522, 1.88706, 0 ),
			IECore.V3f( 1.80092, 1.86207, 0 ),
			IECore.V3f( 1.83333, 1.83333, 0 )
		] )

		lengths = IECore.FloatVectorData( [ 2.2106194496154785, 2.2106194496154785 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def testPeriodicBSpline( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 4 ] ),
			IECore.CubicBasisf.bSpline(),
			True,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
				]
			)
		)

		expected = IECore.V3fVectorData( [
			IECore.V3f( 0.166667, 0.166667, 0 ),
			IECore.V3f( 0.210634, 0.129182, 0 ),
			IECore.V3f( 0.260176, 0.0983618, 0 ),
			IECore.V3f( 0.314207, 0.0742052, 0 ),
			IECore.V3f( 0.371638, 0.0567125, 0 ),
			IECore.V3f( 0.431381, 0.0458837, 0 ),
			IECore.V3f( 0.492347, 0.0417187, 0 ),
			IECore.V3f( 0.55345, 0.0442177, 0 ),
			IECore.V3f( 0.613601, 0.0533805, 0 ),
			IECore.V3f( 0.671711, 0.0692073, 0 ),
			IECore.V3f( 0.726694, 0.0916979, 0 ),
			IECore.V3f( 0.777461, 0.120852, 0 ),
			IECore.V3f( 0.822924, 0.156671, 0 ),
			IECore.V3f( 0.862071, 0.199077, 0 ),
			IECore.V3f( 0.894558, 0.247328, 0 ),
			IECore.V3f( 0.92038, 0.300338, 0 ),
			IECore.V3f( 0.939539, 0.357021, 0 ),
			IECore.V3f( 0.952034, 0.416288, 0 ),
			IECore.V3f( 0.957865, 0.47705, 0 ),
			IECore.V3f( 0.957032, 0.538221, 0 ),
			IECore.V3f( 0.949535, 0.598712, 0 ),
			IECore.V3f( 0.935374, 0.657434, 0 ),
			IECore.V3f( 0.914549, 0.713301, 0 ),
			IECore.V3f( 0.887061, 0.765224, 0 ),
			IECore.V3f( 0.852909, 0.812115, 0 ),
			IECore.V3f( 0.812115, 0.852908, 0 ),
			IECore.V3f( 0.765224, 0.887061, 0 ),
			IECore.V3f( 0.713301, 0.914549, 0 ),
			IECore.V3f( 0.657434, 0.935374, 0 ),
			IECore.V3f( 0.598712, 0.949535, 0 ),
			IECore.V3f( 0.538221, 0.957032, 0 ),
			IECore.V3f( 0.47705, 0.957865, 0 ),
			IECore.V3f( 0.416288, 0.952034, 0 ),
			IECore.V3f( 0.357021, 0.939539, 0 ),
			IECore.V3f( 0.300338, 0.92038, 0 ),
			IECore.V3f( 0.247327, 0.894558, 0 ),
			IECore.V3f( 0.199077, 0.862071, 0 ),
			IECore.V3f( 0.156671, 0.822924, 0 ),
			IECore.V3f( 0.120852, 0.777461, 0 ),
			IECore.V3f( 0.0916979, 0.726694, 0 ),
			IECore.V3f( 0.0692073, 0.671711, 0 ),
			IECore.V3f( 0.0533805, 0.613601, 0 ),
			IECore.V3f( 0.0442177, 0.55345, 0 ),
			IECore.V3f( 0.0417187, 0.492347, 0 ),
			IECore.V3f( 0.0458837, 0.431381, 0 ),
			IECore.V3f( 0.0567125, 0.371638, 0 ),
			IECore.V3f( 0.0742052, 0.314207, 0 ),
			IECore.V3f( 0.0983618, 0.260176, 0 ),
			IECore.V3f( 0.129182, 0.210634, 0 ),
			IECore.V3f( 0.166667, 0.166667, 0 )
		] )

		lengths = IECore.FloatVectorData( [ 2.917539119 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def test2CurvePeriodicBSpline( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 4, 4 ] ),
			IECore.CubicBasisf.bSpline(),
			True,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 0, 2, 0 ),
					v( 0, 1, 0 ),
					v( 1, 1, 0 ),
					v( 1, 2, 0 ),
				]
			)
		)

		expected = IECore.V3fVectorData( [
			IECore.V3f( 0.166667, 0.166667, 0 ),
			IECore.V3f( 0.210634, 0.129182, 0 ),
			IECore.V3f( 0.260176, 0.0983618, 0 ),
			IECore.V3f( 0.314207, 0.0742052, 0 ),
			IECore.V3f( 0.371638, 0.0567125, 0 ),
			IECore.V3f( 0.431381, 0.0458837, 0 ),
			IECore.V3f( 0.492347, 0.0417187, 0 ),
			IECore.V3f( 0.55345, 0.0442177, 0 ),
			IECore.V3f( 0.613601, 0.0533805, 0 ),
			IECore.V3f( 0.671711, 0.0692073, 0 ),
			IECore.V3f( 0.726694, 0.0916979, 0 ),
			IECore.V3f( 0.777461, 0.120852, 0 ),
			IECore.V3f( 0.822924, 0.156671, 0 ),
			IECore.V3f( 0.862071, 0.199077, 0 ),
			IECore.V3f( 0.894558, 0.247328, 0 ),
			IECore.V3f( 0.92038, 0.300338, 0 ),
			IECore.V3f( 0.939539, 0.357021, 0 ),
			IECore.V3f( 0.952034, 0.416288, 0 ),
			IECore.V3f( 0.957865, 0.47705, 0 ),
			IECore.V3f( 0.957032, 0.538221, 0 ),
			IECore.V3f( 0.949535, 0.598712, 0 ),
			IECore.V3f( 0.935374, 0.657434, 0 ),
			IECore.V3f( 0.914549, 0.713301, 0 ),
			IECore.V3f( 0.887061, 0.765224, 0 ),
			IECore.V3f( 0.852909, 0.812115, 0 ),
			IECore.V3f( 0.812115, 0.852908, 0 ),
			IECore.V3f( 0.765224, 0.887061, 0 ),
			IECore.V3f( 0.713301, 0.914549, 0 ),
			IECore.V3f( 0.657434, 0.935374, 0 ),
			IECore.V3f( 0.598712, 0.949535, 0 ),
			IECore.V3f( 0.538221, 0.957032, 0 ),
			IECore.V3f( 0.47705, 0.957865, 0 ),
			IECore.V3f( 0.416288, 0.952034, 0 ),
			IECore.V3f( 0.357021, 0.939539, 0 ),
			IECore.V3f( 0.300338, 0.92038, 0 ),
			IECore.V3f( 0.247327, 0.894558, 0 ),
			IECore.V3f( 0.199077, 0.862071, 0 ),
			IECore.V3f( 0.156671, 0.822924, 0 ),
			IECore.V3f( 0.120852, 0.777461, 0 ),
			IECore.V3f( 0.0916979, 0.726694, 0 ),
			IECore.V3f( 0.0692073, 0.671711, 0 ),
			IECore.V3f( 0.0533805, 0.613601, 0 ),
			IECore.V3f( 0.0442177, 0.55345, 0 ),
			IECore.V3f( 0.0417187, 0.492347, 0 ),
			IECore.V3f( 0.0458837, 0.431381, 0 ),
			IECore.V3f( 0.0567125, 0.371638, 0 ),
			IECore.V3f( 0.0742052, 0.314207, 0 ),
			IECore.V3f( 0.0983618, 0.260176, 0 ),
			IECore.V3f( 0.129182, 0.210634, 0 ),
			IECore.V3f( 0.166667, 0.166667, 0 ),
			IECore.V3f( 0.166667, 1.16667, 0 ),
			IECore.V3f( 0.210634, 1.12918, 0 ),
			IECore.V3f( 0.260176, 1.09836, 0 ),
			IECore.V3f( 0.314207, 1.07421, 0 ),
			IECore.V3f( 0.371638, 1.05671, 0 ),
			IECore.V3f( 0.431381, 1.04588, 0 ),
			IECore.V3f( 0.492347, 1.04172, 0 ),
			IECore.V3f( 0.55345, 1.04422, 0 ),
			IECore.V3f( 0.613601, 1.05338, 0 ),
			IECore.V3f( 0.671711, 1.06921, 0 ),
			IECore.V3f( 0.726694, 1.0917, 0 ),
			IECore.V3f( 0.777461, 1.12085, 0 ),
			IECore.V3f( 0.822924, 1.15667, 0 ),
			IECore.V3f( 0.862071, 1.19908, 0 ),
			IECore.V3f( 0.894558, 1.24733, 0 ),
			IECore.V3f( 0.92038, 1.30034, 0 ),
			IECore.V3f( 0.939539, 1.35702, 0 ),
			IECore.V3f( 0.952034, 1.41629, 0 ),
			IECore.V3f( 0.957865, 1.47705, 0 ),
			IECore.V3f( 0.957032, 1.53822, 0 ),
			IECore.V3f( 0.949535, 1.59871, 0 ),
			IECore.V3f( 0.935374, 1.65743, 0 ),
			IECore.V3f( 0.914549, 1.7133, 0 ),
			IECore.V3f( 0.887061, 1.76522, 0 ),
			IECore.V3f( 0.852909, 1.81211, 0 ),
			IECore.V3f( 0.812115, 1.85291, 0 ),
			IECore.V3f( 0.765224, 1.88706, 0 ),
			IECore.V3f( 0.713301, 1.91455, 0 ),
			IECore.V3f( 0.657434, 1.93537, 0 ),
			IECore.V3f( 0.598712, 1.94954, 0 ),
			IECore.V3f( 0.538221, 1.95703, 0 ),
			IECore.V3f( 0.47705, 1.95786, 0 ),
			IECore.V3f( 0.416288, 1.95203, 0 ),
			IECore.V3f( 0.357021, 1.93954, 0 ),
			IECore.V3f( 0.300338, 1.92038, 0 ),
			IECore.V3f( 0.247327, 1.89456, 0 ),
			IECore.V3f( 0.199077, 1.86207, 0 ),
			IECore.V3f( 0.156671, 1.82292, 0 ),
			IECore.V3f( 0.120852, 1.77746, 0 ),
			IECore.V3f( 0.0916979, 1.72669, 0 ),
			IECore.V3f( 0.0692073, 1.67171, 0 ),
			IECore.V3f( 0.0533805, 1.6136, 0 ),
			IECore.V3f( 0.0442177, 1.55345, 0 ),
			IECore.V3f( 0.0417187, 1.49235, 0 ),
			IECore.V3f( 0.0458837, 1.43138, 0 ),
			IECore.V3f( 0.0567125, 1.37164, 0 ),
			IECore.V3f( 0.0742052, 1.31421, 0 ),
			IECore.V3f( 0.0983618, 1.26018, 0 ),
			IECore.V3f( 0.129182, 1.21063, 0 ),
			IECore.V3f( 0.166667, 1.16667, 0 )
		] )

		lengths = IECore.FloatVectorData( [ 2.917539119, 2.917539119 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def test3SegmentLinear( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 6 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 2, 1, 0 ),
					v( 2, 0, 0 )
				]
			)
		)

		expected = IECore.V3fVectorData( [ IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 0.897959, 0 ),
			 IECore.V3f( 0, 0.795918, 0 ),
			 IECore.V3f( 0, 0.693878, 0 ),
			 IECore.V3f( 0, 0.591837, 0 ),
			 IECore.V3f( 0, 0.489796, 0 ),
			 IECore.V3f( 0, 0.387755, 0 ),
			 IECore.V3f( 0, 0.285714, 0 ),
			 IECore.V3f( 0, 0.183674, 0 ),
			 IECore.V3f( 0, 0.0816326, 0 ),
			 IECore.V3f( 0.0204082, 0, 0 ),
			 IECore.V3f( 0.122449, 0, 0 ),
			 IECore.V3f( 0.22449, 0, 0 ),
			 IECore.V3f( 0.326531, 0, 0 ),
			 IECore.V3f( 0.428571, 0, 0 ),
			 IECore.V3f( 0.530612, 0, 0 ),
			 IECore.V3f( 0.632653, 0, 0 ),
			 IECore.V3f( 0.734694, 0, 0 ),
			 IECore.V3f( 0.836735, 0, 0 ),
			 IECore.V3f( 0.938776, 0, 0 ),
			 IECore.V3f( 1, 0.0408163, 0 ),
			 IECore.V3f( 1, 0.142857, 0 ),
			 IECore.V3f( 1, 0.244898, 0 ),
			 IECore.V3f( 1, 0.346939, 0 ),
			 IECore.V3f( 1, 0.44898, 0 ),
			 IECore.V3f( 1, 0.55102, 0 ),
			 IECore.V3f( 1, 0.653061, 0 ),
			 IECore.V3f( 1, 0.755102, 0 ),
			 IECore.V3f( 1, 0.857143, 0 ),
			 IECore.V3f( 1, 0.959184, 0 ),
			 IECore.V3f( 1.06122, 1, 0 ),
			 IECore.V3f( 1.16327, 1, 0 ),
			 IECore.V3f( 1.26531, 1, 0 ),
			 IECore.V3f( 1.36735, 1, 0 ),
			 IECore.V3f( 1.46939, 1, 0 ),
			 IECore.V3f( 1.57143, 1, 0 ),
			 IECore.V3f( 1.67347, 1, 0 ),
			 IECore.V3f( 1.77551, 1, 0 ),
			 IECore.V3f( 1.87755, 1, 0 ),
			 IECore.V3f( 1.97959, 1, 0 ),
			 IECore.V3f( 2, 0.918367, 0 ),
			 IECore.V3f( 2, 0.816327, 0 ),
			 IECore.V3f( 2, 0.714286, 0 ),
			 IECore.V3f( 2, 0.612245, 0 ),
			 IECore.V3f( 2, 0.510204, 0 ),
			 IECore.V3f( 2, 0.408164, 0 ),
			 IECore.V3f( 2, 0.306122, 0 ),
			 IECore.V3f( 2, 0.204082, 0 ),
			 IECore.V3f( 2, 0.102041, 0 ),
			 IECore.V3f( 2, 0, 0 ) ] )

		lengths = IECore.FloatVectorData( [ 5.0 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def test3SegmentLinearDoubledEndpoints( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 8 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 2, 1, 0 ),
					v( 2, 0, 0 ),
					v( 2, 0, 0 )
				]
			)
		)

		expected = IECore.V3fVectorData( [
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 0.857143, 0 ),
			 IECore.V3f( 0, 0.714286, 0 ),
			 IECore.V3f( 0, 0.571429, 0 ),
			 IECore.V3f( 0, 0.428571, 0 ),
			 IECore.V3f( 0, 0.285714, 0 ),
			 IECore.V3f( 0, 0.142857, 0 ),
			 IECore.V3f( 0, 0, 0 ),
			 IECore.V3f( 0.142857, 0, 0 ),
			 IECore.V3f( 0.285714, 0, 0 ),
			 IECore.V3f( 0.428571, 0, 0 ),
			 IECore.V3f( 0.571429, 0, 0 ),
			 IECore.V3f( 0.714286, 0, 0 ),
			 IECore.V3f( 0.857143, 0, 0 ),
			 IECore.V3f( 1, 0, 0 ),
			 IECore.V3f( 1, 0.142857, 0 ),
			 IECore.V3f( 1, 0.285714, 0 ),
			 IECore.V3f( 1, 0.428571, 0 ),
			 IECore.V3f( 1, 0.571429, 0 ),
			 IECore.V3f( 1, 0.714286, 0 ),
			 IECore.V3f( 1, 0.857143, 0 ),
			 IECore.V3f( 1, 1, 0 ),
			 IECore.V3f( 1.14286, 1, 0 ),
			 IECore.V3f( 1.28571, 1, 0 ),
			 IECore.V3f( 1.42857, 1, 0 ),
			 IECore.V3f( 1.57143, 1, 0 ),
			 IECore.V3f( 1.71429, 1, 0 ),
			 IECore.V3f( 1.85714, 1, 0 ),
			 IECore.V3f( 2, 1, 0 ),
			 IECore.V3f( 2, 0.857143, 0 ),
			 IECore.V3f( 2, 0.714286, 0 ),
			 IECore.V3f( 2, 0.571429, 0 ),
			 IECore.V3f( 2, 0.428572, 0 ),
			 IECore.V3f( 2, 0.285714, 0 ),
			 IECore.V3f( 2, 0.142857, 0 ),
			 IECore.V3f( 2, 0, 0 ),
			 IECore.V3f( 2, 0, 0 ),
			 IECore.V3f( 2, 0, 0 ),
			 IECore.V3f( 2, 0, 0 ),
			 IECore.V3f( 2, 0, 0 ),
			 IECore.V3f( 2, 0, 0 ),
			 IECore.V3f( 2, 0, 0 ),
			 IECore.V3f( 2, 0, 0 ) ] )

		lengths = IECore.FloatVectorData( [ 5.0 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def test2Curve3SegmentLinear( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 6, 6 ] ),
			IECore.CubicBasisf.linear(),
			False,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 2, 1, 0 ),
					v( 2, 0, 0 ),

					v( 0, 2, 0 ),
					v( 0, 1, 0 ),
					v( 1, 1, 0 ),
					v( 1, 2, 0 ),
					v( 2, 2, 0 ),
					v( 2, 1, 0 )
				]
			)
		)

		expected = IECore.V3fVectorData( [ IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 0.897959, 0 ),
			 IECore.V3f( 0, 0.795918, 0 ),
			 IECore.V3f( 0, 0.693878, 0 ),
			 IECore.V3f( 0, 0.591837, 0 ),
			 IECore.V3f( 0, 0.489796, 0 ),
			 IECore.V3f( 0, 0.387755, 0 ),
			 IECore.V3f( 0, 0.285714, 0 ),
			 IECore.V3f( 0, 0.183674, 0 ),
			 IECore.V3f( 0, 0.0816326, 0 ),
			 IECore.V3f( 0.0204082, 0, 0 ),
			 IECore.V3f( 0.122449, 0, 0 ),
			 IECore.V3f( 0.22449, 0, 0 ),
			 IECore.V3f( 0.326531, 0, 0 ),
			 IECore.V3f( 0.428571, 0, 0 ),
			 IECore.V3f( 0.530612, 0, 0 ),
			 IECore.V3f( 0.632653, 0, 0 ),
			 IECore.V3f( 0.734694, 0, 0 ),
			 IECore.V3f( 0.836735, 0, 0 ),
			 IECore.V3f( 0.938776, 0, 0 ),
			 IECore.V3f( 1, 0.0408163, 0 ),
			 IECore.V3f( 1, 0.142857, 0 ),
			 IECore.V3f( 1, 0.244898, 0 ),
			 IECore.V3f( 1, 0.346939, 0 ),
			 IECore.V3f( 1, 0.44898, 0 ),
			 IECore.V3f( 1, 0.55102, 0 ),
			 IECore.V3f( 1, 0.653061, 0 ),
			 IECore.V3f( 1, 0.755102, 0 ),
			 IECore.V3f( 1, 0.857143, 0 ),
			 IECore.V3f( 1, 0.959184, 0 ),
			 IECore.V3f( 1.06122, 1, 0 ),
			 IECore.V3f( 1.16327, 1, 0 ),
			 IECore.V3f( 1.26531, 1, 0 ),
			 IECore.V3f( 1.36735, 1, 0 ),
			 IECore.V3f( 1.46939, 1, 0 ),
			 IECore.V3f( 1.57143, 1, 0 ),
			 IECore.V3f( 1.67347, 1, 0 ),
			 IECore.V3f( 1.77551, 1, 0 ),
			 IECore.V3f( 1.87755, 1, 0 ),
			 IECore.V3f( 1.97959, 1, 0 ),
			 IECore.V3f( 2, 0.918367, 0 ),
			 IECore.V3f( 2, 0.816327, 0 ),
			 IECore.V3f( 2, 0.714286, 0 ),
			 IECore.V3f( 2, 0.612245, 0 ),
			 IECore.V3f( 2, 0.510204, 0 ),
			 IECore.V3f( 2, 0.408164, 0 ),
			 IECore.V3f( 2, 0.306122, 0 ),
			 IECore.V3f( 2, 0.204082, 0 ),
			 IECore.V3f( 2, 0.102041, 0 ),
			 IECore.V3f( 2, 0, 0 ),
			 IECore.V3f( 0, 2, 0 ),
			 IECore.V3f( 0, 1.89796, 0 ),
			 IECore.V3f( 0, 1.79592, 0 ),
			 IECore.V3f( 0, 1.69388, 0 ),
			 IECore.V3f( 0, 1.59184, 0 ),
			 IECore.V3f( 0, 1.4898, 0 ),
			 IECore.V3f( 0, 1.38776, 0 ),
			 IECore.V3f( 0, 1.28571, 0 ),
			 IECore.V3f( 0, 1.18367, 0 ),
			 IECore.V3f( 0, 1.08163, 0 ),
			 IECore.V3f( 0.0204082, 1, 0 ),
			 IECore.V3f( 0.122449, 1, 0 ),
			 IECore.V3f( 0.22449, 1, 0 ),
			 IECore.V3f( 0.326531, 1, 0 ),
			 IECore.V3f( 0.428571, 1, 0 ),
			 IECore.V3f( 0.530612, 1, 0 ),
			 IECore.V3f( 0.632653, 1, 0 ),
			 IECore.V3f( 0.734694, 1, 0 ),
			 IECore.V3f( 0.836735, 1, 0 ),
			 IECore.V3f( 0.938776, 1, 0 ),
			 IECore.V3f( 1, 1.04082, 0 ),
			 IECore.V3f( 1, 1.14286, 0 ),
			 IECore.V3f( 1, 1.2449, 0 ),
			 IECore.V3f( 1, 1.34694, 0 ),
			 IECore.V3f( 1, 1.44898, 0 ),
			 IECore.V3f( 1, 1.55102, 0 ),
			 IECore.V3f( 1, 1.65306, 0 ),
			 IECore.V3f( 1, 1.7551, 0 ),
			 IECore.V3f( 1, 1.85714, 0 ),
			 IECore.V3f( 1, 1.95918, 0 ),
			 IECore.V3f( 1.06122, 2, 0 ),
			 IECore.V3f( 1.16327, 2, 0 ),
			 IECore.V3f( 1.26531, 2, 0 ),
			 IECore.V3f( 1.36735, 2, 0 ),
			 IECore.V3f( 1.46939, 2, 0 ),
			 IECore.V3f( 1.57143, 2, 0 ),
			 IECore.V3f( 1.67347, 2, 0 ),
			 IECore.V3f( 1.77551, 2, 0 ),
			 IECore.V3f( 1.87755, 2, 0 ),
			 IECore.V3f( 1.97959, 2, 0 ),
			 IECore.V3f( 2, 1.91837, 0 ),
			 IECore.V3f( 2, 1.81633, 0 ),
			 IECore.V3f( 2, 1.71429, 0 ),
			 IECore.V3f( 2, 1.61225, 0 ),
			 IECore.V3f( 2, 1.5102, 0 ),
			 IECore.V3f( 2, 1.40816, 0 ),
			 IECore.V3f( 2, 1.30612, 0 ),
			 IECore.V3f( 2, 1.20408, 0 ),
			 IECore.V3f( 2, 1.10204, 0 ),
			 IECore.V3f( 2, 1, 0 ) ] )

		lengths = IECore.FloatVectorData( [ 5.0, 5.0 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def test3SegmentPeriodicLinear( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 6 ] ),
			IECore.CubicBasisf.linear(),
			True,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 2, 1, 0 ),
					v( 2, 0, 0 )
				]
			)
		)

		expected = IECore.V3fVectorData( [ IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 0.877551, 0 ),
			 IECore.V3f( 0, 0.755102, 0 ),
			 IECore.V3f( 0, 0.632653, 0 ),
			 IECore.V3f( 0, 0.510204, 0 ),
			 IECore.V3f( 0, 0.387755, 0 ),
			 IECore.V3f( 0, 0.265306, 0 ),
			 IECore.V3f( 0, 0.142857, 0 ),
			 IECore.V3f( 0, 0.0204082, 0 ),
			 IECore.V3f( 0.102041, 0, 0 ),
			 IECore.V3f( 0.22449, 0, 0 ),
			 IECore.V3f( 0.346939, 0, 0 ),
			 IECore.V3f( 0.469388, 0, 0 ),
			 IECore.V3f( 0.591837, 0, 0 ),
			 IECore.V3f( 0.714286, 0, 0 ),
			 IECore.V3f( 0.836735, 0, 0 ),
			 IECore.V3f( 0.959184, 0, 0 ),
			 IECore.V3f( 1, 0.0816326, 0 ),
			 IECore.V3f( 1, 0.204082, 0 ),
			 IECore.V3f( 1, 0.32653, 0 ),
			 IECore.V3f( 1, 0.44898, 0 ),
			 IECore.V3f( 1, 0.571429, 0 ),
			 IECore.V3f( 1, 0.693877, 0 ),
			 IECore.V3f( 1, 0.816327, 0 ),
			 IECore.V3f( 1, 0.938776, 0 ),
			 IECore.V3f( 1.06122, 1, 0 ),
			 IECore.V3f( 1.18367, 1, 0 ),
			 IECore.V3f( 1.30612, 1, 0 ),
			 IECore.V3f( 1.42857, 1, 0 ),
			 IECore.V3f( 1.55102, 1, 0 ),
			 IECore.V3f( 1.67347, 1, 0 ),
			 IECore.V3f( 1.79592, 1, 0 ),
			 IECore.V3f( 1.91837, 1, 0 ),
			 IECore.V3f( 2, 0.959184, 0 ),
			 IECore.V3f( 2, 0.836735, 0 ),
			 IECore.V3f( 2, 0.714286, 0 ),
			 IECore.V3f( 2, 0.591837, 0 ),
			 IECore.V3f( 2, 0.469388, 0 ),
			 IECore.V3f( 2, 0.346939, 0 ),
			 IECore.V3f( 2, 0.22449, 0 ),
			 IECore.V3f( 2, 0.102041, 0 ),
			 IECore.V3f( 1.95918, 0.0204082, 0 ),
			 IECore.V3f( 1.71429, 0.142857, 0 ),
			 IECore.V3f( 1.46939, 0.265306, 0 ),
			 IECore.V3f( 1.22449, 0.387755, 0 ),
			 IECore.V3f( 0.979592, 0.510204, 0 ),
			 IECore.V3f( 0.734694, 0.632653, 0 ),
			 IECore.V3f( 0.489796, 0.755102, 0 ),
			 IECore.V3f( 0.244898, 0.877551, 0 ),
			 IECore.V3f( 0, 1, 0 ) ] )

		lengths = IECore.FloatVectorData( [ 7.23606777 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def test2Curve3SegmentPeriodicLinear( self ) :

		v = IECore.V3f
		c = IECore.CurvesPrimitive(

			IECore.IntVectorData( [ 6, 6 ] ),
			IECore.CubicBasisf.linear(),
			True,
			IECore.V3fVectorData(
				[
					v( 0, 1, 0 ),
					v( 0, 0, 0 ),
					v( 1, 0, 0 ),
					v( 1, 1, 0 ),
					v( 2, 1, 0 ),
					v( 2, 0, 0 ),
					v( 0, 2, 0 ),
					v( 0, 1, 0 ),
					v( 1, 1, 0 ),
					v( 1, 2, 0 ),
					v( 2, 2, 0 ),
					v( 2, 1, 0 )
				]
			)
		)

		expected = IECore.V3fVectorData( [ IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 0.877551, 0 ),
			 IECore.V3f( 0, 0.755102, 0 ),
			 IECore.V3f( 0, 0.632653, 0 ),
			 IECore.V3f( 0, 0.510204, 0 ),
			 IECore.V3f( 0, 0.387755, 0 ),
			 IECore.V3f( 0, 0.265306, 0 ),
			 IECore.V3f( 0, 0.142857, 0 ),
			 IECore.V3f( 0, 0.0204082, 0 ),
			 IECore.V3f( 0.102041, 0, 0 ),
			 IECore.V3f( 0.22449, 0, 0 ),
			 IECore.V3f( 0.346939, 0, 0 ),
			 IECore.V3f( 0.469388, 0, 0 ),
			 IECore.V3f( 0.591837, 0, 0 ),
			 IECore.V3f( 0.714286, 0, 0 ),
			 IECore.V3f( 0.836735, 0, 0 ),
			 IECore.V3f( 0.959184, 0, 0 ),
			 IECore.V3f( 1, 0.0816326, 0 ),
			 IECore.V3f( 1, 0.204082, 0 ),
			 IECore.V3f( 1, 0.32653, 0 ),
			 IECore.V3f( 1, 0.44898, 0 ),
			 IECore.V3f( 1, 0.571429, 0 ),
			 IECore.V3f( 1, 0.693877, 0 ),
			 IECore.V3f( 1, 0.816327, 0 ),
			 IECore.V3f( 1, 0.938776, 0 ),
			 IECore.V3f( 1.06122, 1, 0 ),
			 IECore.V3f( 1.18367, 1, 0 ),
			 IECore.V3f( 1.30612, 1, 0 ),
			 IECore.V3f( 1.42857, 1, 0 ),
			 IECore.V3f( 1.55102, 1, 0 ),
			 IECore.V3f( 1.67347, 1, 0 ),
			 IECore.V3f( 1.79592, 1, 0 ),
			 IECore.V3f( 1.91837, 1, 0 ),
			 IECore.V3f( 2, 0.959184, 0 ),
			 IECore.V3f( 2, 0.836735, 0 ),
			 IECore.V3f( 2, 0.714286, 0 ),
			 IECore.V3f( 2, 0.591837, 0 ),
			 IECore.V3f( 2, 0.469388, 0 ),
			 IECore.V3f( 2, 0.346939, 0 ),
			 IECore.V3f( 2, 0.22449, 0 ),
			 IECore.V3f( 2, 0.102041, 0 ),
			 IECore.V3f( 1.95918, 0.0204082, 0 ),
			 IECore.V3f( 1.71429, 0.142857, 0 ),
			 IECore.V3f( 1.46939, 0.265306, 0 ),
			 IECore.V3f( 1.22449, 0.387755, 0 ),
			 IECore.V3f( 0.979592, 0.510204, 0 ),
			 IECore.V3f( 0.734694, 0.632653, 0 ),
			 IECore.V3f( 0.489796, 0.755102, 0 ),
			 IECore.V3f( 0.244898, 0.877551, 0 ),
			 IECore.V3f( 0, 1, 0 ),
			 IECore.V3f( 0, 2, 0 ),
			 IECore.V3f( 0, 1.87755, 0 ),
			 IECore.V3f( 0, 1.7551, 0 ),
			 IECore.V3f( 0, 1.63265, 0 ),
			 IECore.V3f( 0, 1.5102, 0 ),
			 IECore.V3f( 0, 1.38776, 0 ),
			 IECore.V3f( 0, 1.26531, 0 ),
			 IECore.V3f( 0, 1.14286, 0 ),
			 IECore.V3f( 0, 1.02041, 0 ),
			 IECore.V3f( 0.102041, 1, 0 ),
			 IECore.V3f( 0.22449, 1, 0 ),
			 IECore.V3f( 0.346939, 1, 0 ),
			 IECore.V3f( 0.469388, 1, 0 ),
			 IECore.V3f( 0.591837, 1, 0 ),
			 IECore.V3f( 0.714286, 1, 0 ),
			 IECore.V3f( 0.836735, 1, 0 ),
			 IECore.V3f( 0.959184, 1, 0 ),
			 IECore.V3f( 1, 1.08163, 0 ),
			 IECore.V3f( 1, 1.20408, 0 ),
			 IECore.V3f( 1, 1.32653, 0 ),
			 IECore.V3f( 1, 1.44898, 0 ),
			 IECore.V3f( 1, 1.57143, 0 ),
			 IECore.V3f( 1, 1.69388, 0 ),
			 IECore.V3f( 1, 1.81633, 0 ),
			 IECore.V3f( 1, 1.93878, 0 ),
			 IECore.V3f( 1.06122, 2, 0 ),
			 IECore.V3f( 1.18367, 2, 0 ),
			 IECore.V3f( 1.30612, 2, 0 ),
			 IECore.V3f( 1.42857, 2, 0 ),
			 IECore.V3f( 1.55102, 2, 0 ),
			 IECore.V3f( 1.67347, 2, 0 ),
			 IECore.V3f( 1.79592, 2, 0 ),
			 IECore.V3f( 1.91837, 2, 0 ),
			 IECore.V3f( 2, 1.95918, 0 ),
			 IECore.V3f( 2, 1.83673, 0 ),
			 IECore.V3f( 2, 1.71429, 0 ),
			 IECore.V3f( 2, 1.59184, 0 ),
			 IECore.V3f( 2, 1.46939, 0 ),
			 IECore.V3f( 2, 1.34694, 0 ),
			 IECore.V3f( 2, 1.22449, 0 ),
			 IECore.V3f( 2, 1.10204, 0 ),
			 IECore.V3f( 1.95918, 1.02041, 0 ),
			 IECore.V3f( 1.71429, 1.14286, 0 ),
			 IECore.V3f( 1.46939, 1.26531, 0 ),
			 IECore.V3f( 1.22449, 1.38775, 0 ),
			 IECore.V3f( 0.979592, 1.5102, 0 ),
			 IECore.V3f( 0.734694, 1.63265, 0 ),
			 IECore.V3f( 0.489796, 1.7551, 0 ),
			 IECore.V3f( 0.244898, 1.87755, 0 ),
			 IECore.V3f( 0, 2, 0 ) ] )

		lengths = IECore.FloatVectorData( [ 7.23606777, 7.23606777 ] )

		self.runPointAtVTest( c, expectedPositions=expected, expectedLengths=lengths, visualTest=False, printPoints=False )

	def testClosestPoint( self ) :

		rand = IECore.Rand32()

		for basis in ( IECore.CubicBasisf.linear(), IECore.CubicBasisf.bezier(), IECore.CubicBasisf.bSpline(), IECore.CubicBasisf.catmullRom() ) :

			for i in range( 0, 10 ) :

				p = IECore.V3fVectorData()
				vertsPerCurve = IECore.IntVectorData()

				numCurves = int( rand.nextf( 1, 10 ) )
				for c in range( 0, numCurves ) :

					numSegments = int( rand.nextf( 1, 10 ) )
					numVerts = 4 + basis.step * ( numSegments - 1 )

					vertsPerCurve.append( numVerts )

					for i in range( 0, numVerts ) :

						p.append( rand.nextV3f() + IECore.V3f( c * 2 ) )

				curves = IECore.CurvesPrimitive( vertsPerCurve, basis, False, p )

				curves["constantwidth"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.01 ) )
				IECore.ObjectWriter( curves, "/tmp/curves.cob" ).write()

				e = IECore.CurvesPrimitiveEvaluator( curves )
				result = e.createResult()

				for c in range( 0, numCurves ) :

					numSteps = 100
					for vi in range( 0, numSteps ) :

						v = float( vi ) / ( numSteps - 1 )

						e.pointAtV( c, v, result )
						p = result.point()

						success = e.closestPoint( p, result )
						self.failUnless( success )

						p2 = result.point()
						c2 = result.curveIndex()
						v2 = result.uv()[1]

						self.failUnless( abs( (p2 - p).length() ) < 0.05 )
						self.assertEqual( c2, c )

	def testTopologyMethods( self ) :

		c = IECore.CurvesPrimitive( IECore.IntVectorData( [ 6, 6 ] ), IECore.CubicBasisf.linear(), False, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] * 12 ) )
		e = IECore.CurvesPrimitiveEvaluator( c )
		self.assertEqual( e.verticesPerCurve(), IECore.IntVectorData( [ 6, 6 ] ) )
		self.assertEqual( e.vertexDataOffsets(), IECore.IntVectorData( [ 0, 6 ] ) )
		self.assertEqual( e.varyingDataOffsets(), IECore.IntVectorData( [ 0, 6 ] ) )

	def testCreate( self ) :

		c = IECore.CurvesPrimitive( IECore.IntVectorData( [ 6, 6 ] ), IECore.CubicBasisf.linear(), False, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] * 12 ) )
		e = IECore.PrimitiveEvaluator.create( c )

		self.failUnless( isinstance( e, IECore.CurvesPrimitiveEvaluator ) )

if __name__ == "__main__":
	unittest.main()

