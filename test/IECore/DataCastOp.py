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
import os.path

from IECore import *

class TestDataCastOp( unittest.TestCase ) :

	def testTypeConvertion( self ) :

		self.assertEqual( DataCastOp()( object = FloatData( 2 ), targetType = int(TypeId.DoubleData) ), DoubleData( 2 ) ) 
		self.assertEqual( DataCastOp()( object = DoubleData( 2 ), targetType = int(TypeId.FloatData) ), FloatData( 2 ) ) 
		self.assertEqual( DataCastOp()( object = IntData( 2 ), targetType = int(TypeId.UIntData) ), UIntData( 2 ) ) 
		self.assertEqual( DataCastOp()( object = V3fData( V3f( 2 ) ), targetType = int(TypeId.V3dData) ), V3dData( V3d( 2 ) ) ) 
		self.assertEqual( DataCastOp()( object = QuatfData( Quatf( 1,2,3,4 ) ), targetType = int(TypeId.QuatdData) ), QuatdData( Quatd( 1,2,3,4 ) ) ) 
	
	def testStructureConvertion( self ) :
		self.assertEqual( DataCastOp()( object = V3fData( V3f( 1, 2, 3 ) ), targetType = int(TypeId.FloatVectorData) ), FloatVectorData( [ 1, 2, 3 ] ) ) 
		self.assertEqual( DataCastOp()( object = V3fData( V3f( 1, 2, 3 ) ), targetType = int(TypeId.Color3fData) ), Color3fData( Color3f( 1, 2, 3 ) ) ) 
		self.assertEqual( DataCastOp()( object = V3fVectorData( [ V3f(1), V3f(2), V3f(3) ] ), targetType = int(TypeId.FloatVectorData) ), FloatVectorData( [ 1, 1, 1, 2, 2, 2, 3, 3, 3 ] ) ) 

		self.assertEqual( DataCastOp()( object = FloatVectorData( [ 1, 2, 3 ] ), targetType = int(TypeId.V3fData) ), V3fData( V3f( 1, 2, 3 ) ) ) 
		self.assertEqual( DataCastOp()( object = Color3fData( Color3f( 1, 2, 3 ) ), targetType = int(TypeId.V3fData) ), V3fData( V3f( 1, 2, 3 ) ) ) 
		self.assertEqual( DataCastOp()( object = FloatVectorData( [ 1, 1, 1, 2, 2, 2, 3, 3, 3 ] ), targetType = int(TypeId.V3fVectorData) ), V3fVectorData( [ V3f(1), V3f(2), V3f(3) ] ) ) 
		self.assertEqual( DataCastOp()( object = V3fVectorData( [ V3f(1), V3f(2), V3f(3) ] ), targetType = int(TypeId.Color3fVectorData) ), Color3fVectorData( [ Color3f(1), Color3f(2), Color3f(3) ] ) ) 
		self.assertEqual( DataCastOp()( object = V3dVectorData( [ V3d(1), V3d(2), V3d(3) ] ), targetType = int(TypeId.Color3fVectorData) ), Color3fVectorData( [ Color3f(1), Color3f(2), Color3f(3) ] ) ) 
	
	def testInvalidConversions( self ) :
		tests = [
			( FloatVectorData( [ 1, 2, 3 ] ), int(TypeId.V2fData) ),
			( M33f(), int( TypeId.M44fData ) ),
			( FloatVectorData( [ 1, 2, 3, 4 ] ), int(TypeId.V3fData) ),
			( FloatVectorData( [ 1, 2 ] ), int(TypeId.V3fData) ),
			( FloatVectorData( [ 1, 2, 3, 4 ] ), int(TypeId.V3fVectorData) ),
			( FloatVectorData( [ 1, 2, 3, 4, 5 ] ), int(TypeId.V3fVectorData) ),
		]
		i = 0
		for ( obj, tt ) in tests:
			try:
				DataCastOp()( object = obj, targetType = tt )
			except:
				i += 1
			else:
				raise Exception, "Should fail on this test " + i

if __name__ == "__main__":
        unittest.main()
