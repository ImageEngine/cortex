##########################################################################
#
#  Copyright (c) 2025, Image Engine Design Inc. All rights reserved.
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

import math
import unittest
import random
import imath
import IECore

class RampTest( unittest.TestCase ) :

	def testConstructor( self ) :

		s = IECore.Rampff()
		self.assertEqual( s.interpolation, IECore.RampInterpolation.CatmullRom )
		self.assertEqual( s.points(), () )

		s = IECore.Rampff( (), IECore.RampInterpolation.MonotoneCubic )
		self.assertEqual( s.interpolation, IECore.RampInterpolation.MonotoneCubic )
		self.assertEqual( s.points(), () )

		s = IECore.Rampff( ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 20, 0 ), ( 21, 2 ) ), IECore.RampInterpolation.BSpline )
		self.assertEqual( s.interpolation, IECore.RampInterpolation.BSpline )
		self.assertEqual( s.points(), ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 20, 0 ), ( 21, 2 ) ) )

	def testRepr( self ) :

		s = IECore.Rampff( ( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 20, 0 ), ( 21, 2 ) ), IECore.RampInterpolation.BSpline )

		ss = eval( repr( s ) )
		self.assertEqual( s, ss )

		s = IECore.RampfColor3f( ( ( 0, imath.Color3f( 1, 0, 0 ) ), ( 10, imath.Color3f( 0, 1, 0 ) ), ( 20, imath.Color3f( 1, 1, 0 ) ), ( 20, imath.Color3f( 0, 0, 1 ) ) ), IECore.RampInterpolation.Linear )

		ss = eval( repr( s ) )
		self.assertEqual( s, ss )

	def testEvaluator( self ) :

		s = IECore.Rampff( ( ( 0, 1 ), ( 1, 2 ), ( 2, 2 ), ( 3, 3 ) ), IECore.RampInterpolation.Linear )
		self.assertEqual(
			s.evaluator(),
			IECore.Splineff( IECore.CubicBasisf.linear(), ( ( 0, 1 ), ( 1, 2 ), ( 2, 2 ), ( 3, 3 ) ) )
		)

		s = IECore.Rampff( ( ( 0, 1 ), ( 1, 2 ), ( 2, 2 ), ( 3, 3 ) ), IECore.RampInterpolation.CatmullRom )
		self.assertEqual(
			s.evaluator(),
			IECore.Splineff( IECore.CubicBasisf.catmullRom(),
				( ( 0, 1 ), ( 0, 1 ), ( 1, 2 ), ( 2, 2 ), ( 3, 3 ), ( 3, 3 ) )
			)
		)

		s = IECore.RampfColor3f(
			( ( 0, imath.Color3f(1) ), ( 1, imath.Color3f(2) ), ( 2, imath.Color3f(2) ), ( 3, imath.Color3f(3) ) ),
			IECore.RampInterpolation.Constant
		)
		self.assertEqual(
			s.evaluator(),
			IECore.SplinefColor3f( IECore.CubicBasisf.constant(),
				( ( 0, imath.Color3f(1) ), ( 1, imath.Color3f(2) ), ( 2, imath.Color3f(2) ), ( 3, imath.Color3f(3) ) )
			)
		)

		s = IECore.Rampff( ( ( 0, 1 ), ( 1, 2 ), ( 2, 2 ), ( 3, 3 ) ), IECore.RampInterpolation.BSpline )
		self.assertEqual(
			s.evaluator(),
			IECore.Splineff( IECore.CubicBasisf.bSpline(),
				( ( 0, 1 ), ( 0, 1 ), ( 0, 1 ), ( 1, 2 ), ( 2, 2 ), ( 3, 3 ), ( 3, 3 ), ( 3, 3 ) )
			)
		)

		s = IECore.Rampff( ( ( 0, 1 ), ( 1, 2 ), ( 2, 2 ), ( 3, 3 ) ), IECore.RampInterpolation.MonotoneCubic )

		# This ought to be an assertAlmostEqual, but it's quicker to just hardcode the value of 5/3 that
		# doesn't work out exactly right
		self.assertEqual(
			s.evaluator(),
			IECore.Splineff( IECore.CubicBasisf.bezier(), (
				( 0, 1), ( 1/3, 1), ( 2/3, 2 ),
				( 1, 2 ), ( 4/3, 2 ), ( 1.6666667461395264, 2 ),
				( 2, 2 ), ( 7/3, 2 ), ( 8/3, 3 ),
				( 3, 3 )
			) )
		)

if __name__ == "__main__":
    unittest.main()

