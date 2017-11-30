##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

import random
import unittest
import os
import IECore

class TestInverseDistanceWeightedInterpolation(unittest.TestCase):

	def testSimple( self ):

		p = IECore.V3fVectorData()
		v = IECore.FloatVectorData()

		p.append( IECore.V3f( -1,  1, 0 ) )
		p.append( IECore.V3f( -1, -1, 0 ) )
		p.append( IECore.V3f(  1,  1, 0 ) )
		p.append( IECore.V3f(  1, -1, 0 ) )

		v.append( 1 )
		v.append( 2 )
		v.append( 3 )
		v.append( 4 )

		idw = IECore.InverseDistanceWeightedInterpolationV3ff( p, v, 1 )

		for i in range( 0, 4 ):

			res = idw( p[i] )

			self.assertAlmostEqual( res, v[i] )

	def testRandomPoints( self ):

		random.seed( 1 )

		p = IECore.V2fVectorData()
		v = IECore.FloatVectorData()

		size = 256
		numPoints = 100

		for i in range( 0, numPoints ):

			p.append( IECore.V2f( random.uniform( 0, size ), random.uniform( 0, size ) ) )
			v.append( random.uniform( 0, 1 ) )

		idw = IECore.InverseDistanceWeightedInterpolationV2ff( p, v, 10 )

		expected = (
			0.18944, 0.18686, 0.18517, 0.18489, 0.18669, 0.19138, 0.19995, 0.21342, 0.23276, 0.25867,
			0.18253, 0.17896, 0.17615, 0.17465, 0.17522, 0.17881, 0.18661, 0.19994, 0.22011, 0.24815,
			0.17659, 0.17212, 0.16824, 0.16554, 0.16485, 0.16725, 0.17415, 0.18721, 0.20818, 0.23848,
			0.17190, 0.16668, 0.16188, 0.15811, 0.15626, 0.15751, 0.16350, 0.17625, 0.19802, 0.23077,
			0.16867, 0.16297, 0.15751, 0.15294, 0.15018, 0.15051, 0.15574, 0.16832, 0.19104, 0.22641,
			0.16704, 0.16120, 0.15547, 0.15050, 0.14726, 0.14709, 0.15198, 0.16478, 0.18876, 0.22699,
			0.16697, 0.16144, 0.15593, 0.15109, 0.14791, 0.14787, 0.15313, 0.16680, 0.19254, 0.23382,
			0.16821, 0.16350, 0.15880, 0.15473, 0.15234, 0.15321, 0.15969, 0.17500, 0.20298, 0.24726,
			0.17022, 0.16681, 0.16351, 0.16094, 0.16014, 0.16277, 0.17126, 0.18882, 0.21923, 0.26605,
			0.17207, 0.17031, 0.16891, 0.16843, 0.16990, 0.17497, 0.18600, 0.20605, 0.23867, 0.28713,
		)

		for i in range( 0, 10 ):
			for j in range( 0, 10 ) :
				r =  idw( IECore.V2f( i, j ) )
				self.assertAlmostEqual( r, expected[i*10 + j], 5 )

	def testVectorQueries( self ):

		random.seed( 1 )

		p = IECore.V2fVectorData()
		v = IECore.FloatVectorData()

		size = 1024
		numPoints = 10000

		for i in range( 0, numPoints ):

			p.append( IECore.V2f( random.uniform( 0, size ), random.uniform( 0, size ) ) )
			v.append( random.uniform( 0, 1 ) )

		idw = IECore.InverseDistanceWeightedInterpolationV2ff( p, v, 10 )

		queryPoints = IECore.V2fVectorData()
		for i in range( 0, size ):
			for j in range( 0, size ) :
				queryPoints.append( IECore.V2f( i, j ) )

		f = idw( queryPoints )

		expected = (
			0.19844, 0.19087, 0.18747, 0.18713, 0.18954, 0.19380, 0.19915, 0.20507, 0.21172, 0.22308,
			0.19582, 0.18663, 0.18254, 0.18296, 0.18653, 0.19222, 0.19917, 0.20616, 0.21279, 0.22258,
			0.19699, 0.18465, 0.17956, 0.18030, 0.18578, 0.19412, 0.20304, 0.21075, 0.21731, 0.22494,
			0.20597, 0.18909, 0.18150, 0.18247, 0.18999, 0.20225, 0.21449, 0.22408, 0.22882, 0.23116,
			0.23220, 0.20992, 0.19815, 0.19823, 0.20791, 0.22246, 0.23758, 0.24799, 0.25151, 0.24737,
			0.29168, 0.26577, 0.25006, 0.24703, 0.25415, 0.26680, 0.27718, 0.28169, 0.27858, 0.26721,
			0.39600, 0.37585, 0.35828, 0.34773, 0.34324, 0.34070, 0.33604, 0.32591, 0.30787, 0.28097,
			0.52160, 0.52027, 0.50894, 0.48644, 0.45900, 0.43126, 0.40132, 0.36646, 0.32886, 0.28795,
			0.62477, 0.63906, 0.63500, 0.61053, 0.56471, 0.50782, 0.45051, 0.38819, 0.32783, 0.27786,
			0.68951, 0.71161, 0.71069, 0.68313, 0.63113, 0.55610, 0.46952, 0.38273, 0.30192, 0.24791,
		)

		for i in range( 0, 10 ) :
			for j in range( 0, 10 ) :
				self.assertAlmostEqual( f[i*size + j], expected[i*10 + j], 5 )

if __name__ == "__main__":
	unittest.main()
