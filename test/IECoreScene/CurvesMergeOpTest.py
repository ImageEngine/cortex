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
import IECoreScene

class CurvesMergeOpTest( unittest.TestCase ) :

	def test( self ) :

		v = imath.V3f
		p1 = IECore.V3fVectorData( [ v( 0 ), v( 1 ), v( 2 ), v( 3 ) ], IECore.GeometricData.Interpretation.Point )
		p2 = IECore.V3fVectorData( [ v( 4 ), v( 5 ), v( 6 ), v( 7 ), v( 8 ), v( 9 ), v( 10 ), v( 11 ) ] )

		c1 = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.catmullRom(), False, p1 )
		c2 = IECoreScene.CurvesPrimitive( IECore.IntVectorData( [ 4, 4 ] ), IECore.CubicBasisf.catmullRom(), False, p2 )

		merged = IECoreScene.CurvesMergeOp()( input=c1, curves=c2 )

		for v in IECoreScene.PrimitiveVariable.Interpolation.values :
			i = IECoreScene.PrimitiveVariable.Interpolation( v )
			if i!=IECoreScene.PrimitiveVariable.Interpolation.Invalid and i!=IECoreScene.PrimitiveVariable.Interpolation.Constant :
				self.assertEqual( merged.variableSize( i ), c1.variableSize( i ) + c2.variableSize( i ) )

		pMerged = p1
		pMerged.extend( p2 )
		self.assertEqual( merged["P"].data, pMerged )

if __name__ == "__main__":
    unittest.main()
