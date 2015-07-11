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
import math
from IECore import *

class DataAlgoTest( unittest.TestCase ) :

	def testGetGeometricInterpretation( self ) :

		self.assertEqual( GeometricData.Interpretation.Vector, getGeometricInterpretation( V3fVectorData( [], GeometricData.Interpretation.Vector ) ) )
		self.assertEqual( GeometricData.Interpretation.Normal, getGeometricInterpretation( V3fVectorData( [], GeometricData.Interpretation.Normal ) ) )
		self.assertEqual( GeometricData.Interpretation.Point, getGeometricInterpretation( V3fData( V3f( 1 ), GeometricData.Interpretation.Point ) ) )
		self.assertEqual( GeometricData.Interpretation.None, getGeometricInterpretation( V3fData( V3f( 1 ), GeometricData.Interpretation.None ) ) )
		self.assertEqual( GeometricData.Interpretation.None, getGeometricInterpretation( FloatData( 5 ) ) )
		self.assertEqual( GeometricData.Interpretation.None, getGeometricInterpretation( StringData( "foo" ) ) )

	def testSetGeometricInterpretation( self ) :

		v = V3fVectorData( [] )
		self.assertEqual( GeometricData.Interpretation.None, getGeometricInterpretation( v ) )
		setGeometricInterpretation( v, GeometricData.Interpretation.Vector )
		self.assertEqual( GeometricData.Interpretation.Vector, getGeometricInterpretation( v ) )
		setGeometricInterpretation( v, GeometricData.Interpretation.Normal )
		self.assertEqual( GeometricData.Interpretation.Normal, getGeometricInterpretation( v ) )

		v = V3fData( V3f( 0 ) )
		self.assertEqual( GeometricData.Interpretation.None, getGeometricInterpretation( v ) )
		setGeometricInterpretation( v, GeometricData.Interpretation.Point )
		self.assertEqual( GeometricData.Interpretation.Point, getGeometricInterpretation( v ) )
		setGeometricInterpretation( v, GeometricData.Interpretation.None )
		self.assertEqual( GeometricData.Interpretation.None, getGeometricInterpretation( v ) )


		#Setting the geometric interpretation of data that is not geometric is OK if you set it to None, but is otherwise an exception
		setGeometricInterpretation( FloatData( 5 ), GeometricData.Interpretation.None )
		setGeometricInterpretation( StringData( "foo" ), GeometricData.Interpretation.None )

		self.assertRaises( RuntimeError, setGeometricInterpretation, FloatData( 5 ), GeometricData.Interpretation.Normal )
		self.assertRaises( RuntimeError, setGeometricInterpretation, StringData( "foo" ), GeometricData.Interpretation.Point )

				

if __name__ == "__main__":
	unittest.main()
