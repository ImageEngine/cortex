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

import os
import unittest
import sys
import imath
import IECore

class TestTurbulence( unittest.TestCase ) :

	def testConstructors( self ) :

		t = IECore.TurbulenceV2ff()
		self.assertEqual( t.octaves, 4 )
		self.assertEqual( t.gain, 0.5 )
		self.assertEqual( t.lacunarity, 2 )
		self.assertEqual( t.turbulent, True )

		t = IECore.TurbulenceV2ff( 2, 1, 3, False )
		self.assertEqual( t.octaves, 2 )
		self.assertEqual( t.gain, 1 )
		self.assertEqual( t.lacunarity, 3 )
		self.assertEqual( t.turbulent, False )

		t = IECore.TurbulenceV2ff(
			octaves = 3,
			gain = 1.4,
			lacunarity = 3,
			turbulent = False
		)

		self.assertEqual( t.octaves, 3 )
		self.assertAlmostEqual( t.gain, 1.4 )
		self.assertEqual( t.lacunarity, 3 )
		self.assertEqual( t.turbulent, False )

	def test2d( self ) :

		expected = (
			0.50000, 0.51876, 0.53219, 0.53978, 0.54359, 0.54603, 0.54817, 0.54947, 0.55075, 0.55015,
			0.52756, 0.54607, 0.55913, 0.56621, 0.56943, 0.57121, 0.57268, 0.57323, 0.57335, 0.57130,
			0.55703, 0.57450, 0.58716, 0.59444, 0.59796, 0.59962, 0.60027, 0.59953, 0.59753, 0.59314,
			0.58574, 0.60132, 0.61350, 0.62163, 0.62640, 0.62853, 0.62838, 0.62607, 0.62148, 0.61452,
			0.61106, 0.62433, 0.63581, 0.64489, 0.65119, 0.65417, 0.65355, 0.65003, 0.64343, 0.63479,
			0.63288, 0.64391, 0.65440, 0.66383, 0.67115, 0.67494, 0.67442, 0.67063, 0.66344, 0.65462,
			0.65284, 0.66195, 0.67109, 0.67993, 0.68734, 0.69165, 0.69182, 0.68881, 0.68250, 0.67475,
			0.67045, 0.67779, 0.68540, 0.69318, 0.70017, 0.70478, 0.70589, 0.70414, 0.69938, 0.69326,
			0.68390, 0.68996, 0.69614, 0.70254, 0.70864, 0.71326, 0.71543, 0.71530, 0.71266, 0.70885,
			0.69565, 0.70112, 0.70616, 0.71097, 0.71569, 0.71997, 0.72320, 0.72493, 0.72457, 0.72302,
		)

		t = IECore.TurbulenceV2ff(
			octaves = 4,
			gain = 0.35,
			lacunarity = 2,
			turbulent = False
		)

		width = 10
		height = 10

		for i in range( 0, height ) :
			for j in range( 0, width ) :
				f = 0.5 + t.turbulence( imath.V2f( i/50.0, j/50.0 ) )
				self.assertAlmostEqual( f, expected[i*height + j], 5 )

	def testNaN( self ) :

		t = IECore.TurbulenceV2ff(
			octaves = 28,
			gain = 0.35,
			lacunarity = 2,
			turbulent = True
		)

		f = t.turbulence( imath.V2f( 21.3, 51.2 ) )
		self.assert_( f == f )

if sys.platform == "darwin" :

	# These fail because MacOS uses libc++, and libc++ has a
	# different `std::random_shuffle()` than libstdc++.

	TestTurbulence.test2d = unittest.expectedFailure( TestTurbulence.test2d )

if __name__ == "__main__":
	unittest.main()

