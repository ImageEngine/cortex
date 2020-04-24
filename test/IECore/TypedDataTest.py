##########################################################################
#
#  Copyright (c) 2009-2013, Image Engine Design Inc. All rights reserved.
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
import IECore

class TestTypedData( unittest.TestCase ) :

	def testRValueFromT( self ) :

		c = IECore.CompoundData()

		c["a"] = 1
		c["b"] = True
		c["c"] = "hello"
		c["d"] = 10.1
		c["e"] = imath.V3f( 1, 2, 3 )

		self.assertEqual( c["a"], IECore.IntData( 1 ) )
		self.assertEqual( c["b"], IECore.BoolData( True ) )
		self.assertEqual( c["c"], IECore.StringData( "hello" ) )
		self.assertEqual( c["d"], IECore.FloatData( 10.1 ) )
		self.assertEqual( c["e"], IECore.V3fData( imath.V3f( 1, 2, 3 ) ) )

	def testInterpretation( self ) :

		self.assertNotEqual( IECore.V3fData( imath.V3f( 1, 2, 3 ) ), IECore.V3fData( imath.V3f( 1, 2, 3 ), IECore.GeometricData.Interpretation.Point ))
		self.assertEqual( IECore.V3fData( imath.V3f( 1, 2, 3 ) ), IECore.V3fData( imath.V3f( 1, 2, 3 ), IECore.GeometricData.Interpretation.None_ ))
		self.assertEqual( IECore.V3fData( imath.V3f( 1, 2, 3 ) ), IECore.V3fData( imath.V3f( 1, 2, 3 ), IECore.GeometricData.Interpretation.Numeric ))

	def testHash( self ) :

		# although the underlying data is identical, these objects
		# must not hash equal because they have different types.

		self.assertNotEqual( IECore.V3fData( imath.V3f( 1, 2, 3 ) ).hash(), IECore.Color3fData( imath.Color3f( 1, 2, 3 ) ).hash() )
		self.assertNotEqual( IECore.IntVectorData( [ 0, 0, 0 ] ).hash(), IECore.UIntVectorData( [ 0, 0, 0 ] ).hash() )

		a = IECore.V3fData( imath.V3f( 1, 2, 3 ), IECore.GeometricData.Interpretation.Point )
		b = a.copy()
		self.assertEqual( a.hash(), b.hash() )
		b.setInterpretation( IECore.GeometricData.Interpretation.None_ )
		self.assertNotEqual( a.hash(), b.hash() )

		a = IECore.V2dVectorData( [ imath.V2d( 1, 2 ) ] )
		b = a.copy()
		self.assertEqual( a.hash(), b.hash() )
		b.setInterpretation( IECore.GeometricData.Interpretation.Point )
		self.assertNotEqual( a.hash(), b.hash() )

	def testHalfDataConstruction( self ) :

		zeroHalf = IECore.HalfData( 0 )
		self.assertEqual( IECore.HalfData(), zeroHalf )
		self.assertEqual( IECore.Object.create( IECore.HalfData.staticTypeId() ), zeroHalf )

if __name__ == "__main__":
	unittest.main()

