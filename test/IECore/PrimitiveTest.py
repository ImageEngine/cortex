##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

from IECore import *

class PrimitiveTest( unittest.TestCase ) :

	def test( self ) :
	
		m = MeshPrimitive( IntVectorData( [ 3, 3 ] ), IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )
		
		self.assertEqual( m.inferInterpolation( 1 ), PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( 2 ), PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( m.inferInterpolation( 4 ), PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m.inferInterpolation( 6 ), PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m.inferInterpolation( 0 ), PrimitiveVariable.Interpolation.Invalid )
		self.assertEqual( m.inferInterpolation( 10 ), PrimitiveVariable.Interpolation.Invalid )

		self.assertEqual( m.inferInterpolation( FloatData( 1 ) ), PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( V3fVectorData( [ V3f( 1 ) ] ) ), PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( FloatVectorData( [ 2, 3 ] ) ), PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( m.inferInterpolation( IntVectorData( [ 1, 2, 3, 4 ] ) ), PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m.inferInterpolation( IntVectorData( [ 1, 2, 3, 4, 5, 6 ] ) ), PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m.inferInterpolation( IntVectorData( [ 1, 2, 3, 4, 5, 6, 7 ] ) ), PrimitiveVariable.Interpolation.Invalid )
		
if __name__ == "__main__":
    unittest.main()   
