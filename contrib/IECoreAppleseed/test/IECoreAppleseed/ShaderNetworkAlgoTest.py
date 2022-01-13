##########################################################################
#
#  Copyright (c) 2021, Image Engine Design Inc. All rights reserved.
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
import imath

import IECore
import IECoreScene
import IECoreAppleseed

class ShaderNetworkAlgoTest( unittest.TestCase ):

	# Just one half-hearted test for splines, since I don't want to break this, but also adding Appleseed
	# tests is annoying
	def testSplines( self ) :

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"test" : IECoreScene.Shader( "test", "test",
					IECore.CompoundData(
						{
							"testColorSpline" : IECore.SplinefColor3fData( IECore.SplinefColor3f( IECore.CubicBasisf.linear(), ( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ), ( 20, imath.Color3f(0) ) ) ) ),
							"testFloatSpline" : IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.bezier(), ( ( 0, 0 ), ( 1, 1 ), ( 2, 0 ), ( 3, 1 ) ) ) ),
						}
					)
				),
			},
			output = ( "test", "" )
		)

		net = IECoreAppleseed.ShaderNetworkAlgo.convert( n )

		self.assertEqual(
			IECoreAppleseed.ShaderNetworkAlgo.introspectAppleseedNetwork( net ),
			IECore.CompoundData( {
				"test" : IECore.CompoundData( {
					'testColorSplineBasis':IECore.StringData( 'string linear' ),
					'testColorSplinePositions':IECore.StringData( 'float[] 0 0 10 20 20 ' ),
					'testColorSplineValues' : IECore.StringData( 'color[] 1 1 1 1 1 1 2 2 2 0 0 0 0 0 0 ' ),
					'testFloatSplineBasis':IECore.StringData( 'string bezier' ),
					'testFloatSplinePositions':IECore.StringData( 'float[] 0 1 2 3 ' ),
					'testFloatSplineValues':IECore.StringData( 'float[] 0 1 0 1 ' )
				} )
			} )
		)


if __name__ == "__main__":
	unittest.main()
