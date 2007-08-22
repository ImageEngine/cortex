##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
from IECore import *

class TestObjectInterpolation( unittest.TestCase ) :

	def testSimpleLinearInterpolation( self ) :
	
		self.assertEqual( linearObjectInterpolation( IntData(1), IntData(2), 0.5 ), None )
		self.assertEqual( linearObjectInterpolation( FloatData(1), FloatData(2), 0.5 ), FloatData(1.5) )
		self.assertEqual( linearObjectInterpolation( DoubleData(1), DoubleData(2), 0.5 ), DoubleData(1.5) )
		self.assertEqual( linearObjectInterpolation( V2fData( V2f(1) ), V2fData( V2f(2) ), 0.5 ), V2fData( V2f(1.5) ) )
		self.assertEqual( linearObjectInterpolation( V3fData( V3f(1) ), V3fData( V3f(2) ), 0.5 ), V3fData( V3f(1.5) ) )
		self.assertEqual( linearObjectInterpolation( V2dData( V2d(1) ), V2dData( V2d(2) ), 0.5 ), V2dData( V2d(1.5) ) )
		self.assertEqual( linearObjectInterpolation( V3dData( V3d(1) ), V3dData( V3d(2) ), 0.5 ), V3dData( V3d(1.5) ) )

	def testVectorLinearInterpolation( self ):

		self.assertEqual( linearObjectInterpolation( IntVectorData( [1] ), IntVectorData( [2] ), 0.5 ), None )
		self.assertEqual( linearObjectInterpolation( FloatVectorData( [1]), FloatVectorData( [2] ), 0.5 ), FloatVectorData([1.5]) )
		self.assertEqual( linearObjectInterpolation( DoubleVectorData( [1]), DoubleVectorData( [2] ), 0.5 ), DoubleVectorData([1.5]) )
		self.assertEqual( linearObjectInterpolation( V2fVectorData( [V2f(1)] ), V2fVectorData( [V2f(2)] ), 0.5 ), V2fVectorData( [V2f(1.5)] ) )
		self.assertEqual( linearObjectInterpolation( V3fVectorData( [V3f(1)] ), V3fVectorData( [V3f(2)] ), 0.5 ), V3fVectorData( [V3f(1.5)] ) )
		self.assertEqual( linearObjectInterpolation( V2dVectorData( [V2d(1)] ), V2dVectorData( [V2d(2)] ), 0.5 ), V2dVectorData( [V2d(1.5)] ) )
		self.assertEqual( linearObjectInterpolation( V3dVectorData( [V3d(1)] ), V3dVectorData( [V3d(2)] ), 0.5 ), V3dVectorData( [V3d(1.5)] ) )
		
	def testSimpleCosineInterpolation( self ) :
	
		self.assertEqual( cosineObjectInterpolation( IntData(1), IntData(2), 0.5 ), None )
		self.assertEqual( cosineObjectInterpolation( FloatData(1), FloatData(2), 0.5 ), FloatData(1.5) )
		self.assertEqual( cosineObjectInterpolation( DoubleData(1), DoubleData(2), 0.5 ), DoubleData(1.5) )
		self.assertEqual( cosineObjectInterpolation( V2fData( V2f(1) ), V2fData( V2f(2) ), 0.5 ), V2fData( V2f(1.5) ) )
		self.assertEqual( cosineObjectInterpolation( V3fData( V3f(1) ), V3fData( V3f(2) ), 0.5 ), V3fData( V3f(1.5) ) )
		self.assertEqual( cosineObjectInterpolation( V2dData( V2d(1) ), V2dData( V2d(2) ), 0.5 ), V2dData( V2d(1.5) ) )
		self.assertEqual( cosineObjectInterpolation( V3dData( V3d(1) ), V3dData( V3d(2) ), 0.5 ), V3dData( V3d(1.5) ) )

	def testVectorCosineInterpolation( self ):

		self.assertEqual( cosineObjectInterpolation( IntVectorData( [1] ), IntVectorData( [2] ), 0.5 ), None )
		self.assertEqual( cosineObjectInterpolation( FloatVectorData( [1]), FloatVectorData( [2] ), 0.5 ), FloatVectorData([1.5]) )
		self.assertEqual( cosineObjectInterpolation( DoubleVectorData( [1]), DoubleVectorData( [2] ), 0.5 ), DoubleVectorData([1.5]) )
		self.assertEqual( cosineObjectInterpolation( V2fVectorData( [V2f(1)] ), V2fVectorData( [V2f(2)] ), 0.5 ), V2fVectorData( [V2f(1.5)] ) )
		self.assertEqual( cosineObjectInterpolation( V3fVectorData( [V3f(1)] ), V3fVectorData( [V3f(2)] ), 0.5 ), V3fVectorData( [V3f(1.5)] ) )
		self.assertEqual( cosineObjectInterpolation( V2dVectorData( [V2d(1)] ), V2dVectorData( [V2d(2)] ), 0.5 ), V2dVectorData( [V2d(1.5)] ) )
		self.assertEqual( cosineObjectInterpolation( V3dVectorData( [V3d(1)] ), V3dVectorData( [V3d(2)] ), 0.5 ), V3dVectorData( [V3d(1.5)] ) )

	def testSimpleCubicInterpolation( self ) :
	
		self.assertEqual( cubicObjectInterpolation( IntData(1), IntData(2), IntData(3), IntData(4), 0.5 ), None )
		self.assertEqual( cubicObjectInterpolation( FloatData(1), FloatData(2), FloatData(3), FloatData(4), 0.5 ), FloatData(2.5) )
		self.assertEqual( cubicObjectInterpolation( DoubleData(1), DoubleData(2), DoubleData(3), DoubleData(4), 0.5 ), DoubleData(2.5) )
		self.assertEqual( cubicObjectInterpolation( V2fData( V2f(1) ), V2fData( V2f(2) ), V2fData( V2f(3) ), V2fData( V2f(4) ), 0.5 ), V2fData( V2f(2.5) ) )
		self.assertEqual( cubicObjectInterpolation( V3fData( V3f(1) ), V3fData( V3f(2) ), V3fData( V3f(3) ), V3fData( V3f(4) ), 0.5 ), V3fData( V3f(2.5) ) )
		self.assertEqual( cubicObjectInterpolation( V2dData( V2d(1) ), V2dData( V2d(2) ), V2dData( V2d(3) ), V2dData( V2d(4) ), 0.5 ), V2dData( V2d(2.5) ) )
		self.assertEqual( cubicObjectInterpolation( V3dData( V3d(1) ), V3dData( V3d(2) ), V3dData( V3d(3) ), V3dData( V3d(4) ), 0.5 ), V3dData( V3d(2.5) ) )

	def testVectorCubicInterpolation( self ):

		self.assertEqual( cubicObjectInterpolation( IntVectorData( [1] ), IntVectorData( [2] ), IntVectorData( [1] ), IntVectorData( [2] ), 0.5 ), None )
		self.assertEqual( cubicObjectInterpolation( FloatVectorData( [1]), FloatVectorData( [2] ), FloatVectorData( [3]), FloatVectorData( [4] ), 0.5 ), FloatVectorData([2.5]) )
		self.assertEqual( cubicObjectInterpolation( DoubleVectorData( [1]), DoubleVectorData( [2] ), DoubleVectorData( [3]), DoubleVectorData( [4] ), 0.5 ), DoubleVectorData([2.5]) )
		self.assertEqual( cubicObjectInterpolation( V2fVectorData( [V2f(1)] ), V2fVectorData( [V2f(2)] ), V2fVectorData( [V2f(3)] ), V2fVectorData( [V2f(4)] ), 0.5 ), V2fVectorData( [V2f(2.5)] ) )
		self.assertEqual( cubicObjectInterpolation( V3fVectorData( [V3f(1)] ), V3fVectorData( [V3f(2)] ), V3fVectorData( [V3f(3)] ), V3fVectorData( [V3f(4)] ), 0.5 ), V3fVectorData( [V3f(2.5)] ) )
		self.assertEqual( cubicObjectInterpolation( V2dVectorData( [V2d(1)] ), V2dVectorData( [V2d(2)] ), V2dVectorData( [V2d(3)] ), V2dVectorData( [V2d(4)] ), 0.5 ), V2dVectorData( [V2d(2.5)] ) )
		self.assertEqual( cubicObjectInterpolation( V3dVectorData( [V3d(1)] ), V3dVectorData( [V3d(2)] ), V3dVectorData( [V3d(3)] ), V3dVectorData( [V3d(4)] ), 0.5 ), V3dVectorData( [V3d(2.5)] ) )
		

if __name__ == "__main__":
    unittest.main()   
