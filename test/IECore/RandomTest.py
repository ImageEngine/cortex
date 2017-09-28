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

import unittest
import IECore

class RandomTest( unittest.TestCase ) :

	def testCosineHemisphere( self ) :

		r = IECore.Rand32()

		v = r.cosineHemispherefVector( 1000 )

		for i in range( 0, v.size() ) :

			self.assert_( v[i].z >= 0 )
			self.assertAlmostEqual( v[i].length(), 1, 6 )

	def testBarycentric( self ) :

		r = IECore.Rand32()

		f = r.barycentricf()
		self.assert_( ( f[0] + f[1] + f[2] ) == 1.0 )

		d = r.barycentricd()
		self.assert_( ( d[0] + d[1] + d[2] ) == 1.0 )

		fvs = r.barycentricfVector( IECore.IntVectorData( [ 1, 2, 3, 4, 5 ] ) )
		for i in range( 0, fvs.size() ) :
			self.assert_( ( fvs[i][0] + fvs[i][1] + fvs[i][2] ) == 1.0 )

		fv = r.barycentricfVector( 5 )
		for i in range( 0, fv.size() ) :
			self.assert_( ( fv[i][0] + fv[i][1] + fv[i][2] ) == 1.0 )

		dvs = r.barycentricdVector( IECore.IntVectorData( [ 1, 2, 3, 4, 5 ] ) )
		for i in range( 0, dvs.size() ) :
			self.assert_( ( dvs[i][0] + dvs[i][1] + dvs[i][2] ) == 1.0 )

		dv = r.barycentricdVector( 5 )
		for i in range( 0, dv.size() ) :
			self.assert_( ( dv[i][0] + dv[i][1] + dv[i][2] ) == 1.0 )

if __name__ == "__main__":
	unittest.main()

