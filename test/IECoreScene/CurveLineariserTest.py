##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
import imath
import os
import tempfile
import shutil
import IECore
import IECoreScene

class CurveLineariserTest( unittest.TestCase ) :

	def runTest( self, curves ) :

		curves2 = IECoreScene.CurveLineariser()( input=curves, verticesPerSegment=1000 )

		self.assertTrue( not curves.isSame( curves2 ) )
		self.assertEqual( curves2.numCurves(), curves.numCurves() )
		self.assertEqual( curves2.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( curves2.periodic(), curves.periodic() )
		self.assertEqual( curves.keys(), curves2.keys() )
		self.assertTrue( curves2.arePrimitiveVariablesValid() )

		e = IECoreScene.CurvesPrimitiveEvaluator( curves )
		r = e.createResult()

		e2 = IECoreScene.CurvesPrimitiveEvaluator( curves2 )
		r2 = e.createResult()

		curves["constantwidth"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.001 ) )
		curves2["constantwidth"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.001 ) )
		tempDir = tempfile.mkdtemp()
		IECore.ObjectWriter( curves, os.path.join( tempDir, "c.cob" ) ).write()
		IECore.ObjectWriter( curves2, os.path.join( tempDir, "c2.cob" ) ).write()

		for curveIndex in range( 0, curves.numCurves() ) :

			for i in range( 0, 100 ) :

				v = float( i ) / 99

				s = e.pointAtV( curveIndex, v, r )
				s2 = e2.pointAtV( curveIndex, v, r2 )

				self.assertTrue( s )
				self.assertTrue( s2 )

				for k in curves.keys() :

					pv = r.primVar( curves[k] )
					pv2 = r2.primVar( curves2[k] )

					if isinstance( pv, ( float, int ) ) :
						self.assertAlmostEqual( pv, pv2 )
					elif isinstance( pv, ( imath.V3f, imath.Color3f ) ) :
						self.assertTrue( pv.equalWithAbsError( pv2, 0.005 ) )
					else :
						self.assertEqual( pv, pv2 )

		shutil.rmtree( tempDir )

	def test3SegmentBSpline( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def test3SegmentBSplineDoubledEndpoints( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def test2Curve3SegmentBSpline( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def testPeriodicBSpline( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def test2CurvePeriodicBSpline( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def test3SegmentLinear( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def test3SegmentLinearDoubledEndpoints( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def test2Curve3SegmentLinear( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def test3SegmentPeriodicLinear( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

	def test2Curve3SegmentPeriodicLinear( self ) :

		v = imath.V3f
		c = IECoreScene.CurvesPrimitive(

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

		self.runTest( c )

if __name__ == "__main__":
	unittest.main()

