##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
import imath

import IECore

class TestMultiplyMatrixOp( unittest.TestCase ) :

	def testMultiplication( self ) :
		vectorTypes = [
			IECore.V3fVectorData( [ imath.V3f(1), imath.V3f(2), imath.V3f(3) ], IECore.GeometricData.Interpretation.Vector ),
			IECore.V3dVectorData( [ imath.V3d(1), imath.V3d(2), imath.V3d(3) ], IECore.GeometricData.Interpretation.Vector ),
		]
		matrixTypes = [
			IECore.M33fData( imath.M33f() * 3 ),
			IECore.M33dData( imath.M33d() * 3 ),
			IECore.M44fData( imath.M44f().scale( imath.V3f(3) ) ),
			IECore.M44dData( imath.M44d().scale( imath.V3d(3) ) ),
			IECore.TransformationMatrixfData( IECore.TransformationMatrixf( imath.V3f( 3 ), imath.Eulerf(), imath.V3f( 0 ) ) ),
			IECore.TransformationMatrixdData( IECore.TransformationMatrixd( imath.V3d( 3 ), imath.Eulerd(), imath.V3d( 0 ) ) ),
		]
		for vector in vectorTypes:

			targetVector = vector.copy()
			for i in xrange( len( targetVector) ):
				targetVector[ i ] = targetVector[ i ] * 3

			for matrix in matrixTypes:
				res = IECore.MatrixMultiplyOp()( object = vector.copy(), matrix = matrix )
				if res == targetVector:
					continue
				raise Exception, "Error testing vector " + str(type(vector)) + " against matrix " + str(type(matrix)) + ". Resulted " + str( res )

	def testInterpretations( self ) :

		v = IECore.V3fVectorData( [ imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ) ], IECore.GeometricData.Interpretation.Point )
		o = IECore.MatrixMultiplyOp()

		# as points
		vt = o( object = v.copy(), matrix = IECore.M44fData( imath.M44f().translate( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertEqual( vt[i], v[i] + imath.V3f( 1, 2, 3 ) )

		# as vectors
		v2 = v.copy()
		v2.setInterpretation( IECore.GeometricData.Interpretation.Vector )
		vt = o( object = v2, matrix = IECore.M44fData( imath.M44f().translate( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertEqual( vt[i], v[i] )

		vt = o( object = v2, matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertEqual( vt[i], v[i] * imath.V3f( 1, 2, 3 ) )

		# as normals
		v3 = v.copy()
		v3.setInterpretation( IECore.GeometricData.Interpretation.Normal )
		vt = o( object = v3, matrix = IECore.M44fData( imath.M44f().translate( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertEqual( vt[i], v[i] )

		vt = o( object = v3, matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertNotEqual( vt[i], v[i] * imath.V3f( 1, 2, 3 ) )

		# nothing happens for numeric
		v4 = v.copy()
		v4.setInterpretation( IECore.GeometricData.Interpretation.None )
		vt = o( object = v4, matrix = IECore.M44fData( imath.M44f().translate( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertEqual( vt[i], v[i] )

		vt = o( object = v4, matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertEqual( vt[i], v[i] )

		# nothing happens for color
		v5 = v.copy()
		v5.setInterpretation( IECore.GeometricData.Interpretation.Color )
		vt = o( object = v5, matrix = IECore.M44fData( imath.M44f().translate( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertEqual( vt[i], v[i] )

		vt = o( object = v5, matrix = IECore.M44fData( imath.M44f().scale( imath.V3f( 1, 2, 3 ) ) ) )
		for i in range( v.size() ) :
			self.assertEqual( vt[i], v[i] )

if __name__ == "__main__":
        unittest.main()
