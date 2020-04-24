##########################################################################
#
#  Copyright (c) 2015, Image Engine Design Inc. All rights reserved.
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
import ctypes
import math
import imath
import IECore

class DataAlgoTest( unittest.TestCase ) :

	def testGetGeometricInterpretation( self ) :

		self.assertEqual( IECore.GeometricData.Interpretation.Vector, IECore.getGeometricInterpretation( IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Vector ) ) )
		self.assertEqual( IECore.GeometricData.Interpretation.Normal, IECore.getGeometricInterpretation( IECore.V3fVectorData( [], IECore.GeometricData.Interpretation.Normal ) ) )
		self.assertEqual( IECore.GeometricData.Interpretation.Point, IECore.getGeometricInterpretation( IECore.V3fData( imath.V3f( 1 ), IECore.GeometricData.Interpretation.Point ) ) )
		self.assertEqual( IECore.GeometricData.Interpretation.None_, IECore.getGeometricInterpretation( IECore.V3fData( imath.V3f( 1 ), IECore.GeometricData.Interpretation.None_ ) ) )
		self.assertEqual( IECore.GeometricData.Interpretation.None_, IECore.getGeometricInterpretation( IECore.FloatData( 5 ) ) )
		self.assertEqual( IECore.GeometricData.Interpretation.None_, IECore.getGeometricInterpretation( IECore.StringData( "foo" ) ) )

	def testSetGeometricInterpretation( self ) :

		v = IECore.V3fVectorData( [] )
		self.assertEqual( IECore.GeometricData.Interpretation.None_, IECore.getGeometricInterpretation( v ) )
		IECore.setGeometricInterpretation( v, IECore.GeometricData.Interpretation.Vector )
		self.assertEqual( IECore.GeometricData.Interpretation.Vector, IECore.getGeometricInterpretation( v ) )
		IECore.setGeometricInterpretation( v, IECore.GeometricData.Interpretation.Normal )
		self.assertEqual( IECore.GeometricData.Interpretation.Normal, IECore.getGeometricInterpretation( v ) )

		v = IECore.V3fData( imath.V3f( 0 ) )
		self.assertEqual( IECore.GeometricData.Interpretation.None_, IECore.getGeometricInterpretation( v ) )
		IECore.setGeometricInterpretation( v, IECore.GeometricData.Interpretation.Point )
		self.assertEqual( IECore.GeometricData.Interpretation.Point, IECore.getGeometricInterpretation( v ) )
		IECore.setGeometricInterpretation( v, IECore.GeometricData.Interpretation.None_ )
		self.assertEqual( IECore.GeometricData.Interpretation.None_, IECore.getGeometricInterpretation( v ) )


		#Setting the geometric interpretation of data that is not geometric is OK if you set it to None, but is otherwise an exception
		IECore.setGeometricInterpretation( IECore.FloatData( 5 ), IECore.GeometricData.Interpretation.None_ )
		IECore.setGeometricInterpretation( IECore.StringData( "foo" ), IECore.GeometricData.Interpretation.None_ )

		self.assertRaises( RuntimeError, IECore.setGeometricInterpretation, IECore.FloatData( 5 ), IECore.GeometricData.Interpretation.Normal )
		self.assertRaises( RuntimeError, IECore.setGeometricInterpretation, IECore.StringData( "foo" ), IECore.GeometricData.Interpretation.Point )

	def testCanGenerateUniqueValuesForStringVectorData( self ) :
		data = IECore.StringVectorData( ['a', 'b', 'a', 'c', 'c', 'a', 'b'] )

		output = IECore.uniqueValues( data )

		self.assertEqual( set( output ), set( IECore.StringVectorData( ['a', 'b', 'c'] ) ) )

	def testCanGenerateUniqueValuesForIntVectorData( self ) :
		data = IECore.IntVectorData( [1, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1] )

		output = IECore.uniqueValues( data )

		self.assertEqual( set( output ), set( IECore.IntVectorData( [1, 2] ) ) )

	def testSize( self ) :

		self.assertEqual(
			IECore.size( IECore.StringData( "hi" ) ),
			1
		)

		self.assertEqual(
			IECore.size(
				IECore.V3fVectorData( [ imath.V3f( 1 ), imath.V3f( 2 ) ] )
			),
			2
		)

	def testAddress( self ) :

		v = IECore.IntVectorData( [ 1, 2, 3 ] )

		p = ctypes.cast(
			IECore.address( v ),
			ctypes.POINTER( ctypes.c_int )
		)

		for i in range( 0, len( v ) ) :
			self.assertEqual( p[i], v[i] )

if __name__ == "__main__":
	unittest.main()
