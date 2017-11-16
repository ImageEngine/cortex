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

import IECore

class TestDataCastOp( unittest.TestCase ) :

	def testTypeConvertion( self ) :

		self.assertEqual( IECore.DataCastOp()( object = IECore.FloatData( 2 ), targetType = int(IECore.TypeId.DoubleData) ), IECore.DoubleData( 2 ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.DoubleData( 2 ), targetType = int(IECore.TypeId.FloatData) ), IECore.FloatData( 2 ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.IntData( 2 ), targetType = int(IECore.TypeId.UIntData) ), IECore.UIntData( 2 ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.V3fData( IECore.V3f( 2 ) ), targetType = int(IECore.TypeId.V3dData) ), IECore.V3dData( IECore.V3d( 2 ) ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.QuatfData( IECore.Quatf( 1,2,3,4 ) ), targetType = int(IECore.TypeId.QuatdData) ), IECore.QuatdData( IECore.Quatd( 1,2,3,4 ) ) )

	def testStructureConvertion( self ) :
		self.assertEqual( IECore.DataCastOp()( object = IECore.V3fData( IECore.V3f( 1, 2, 3 ) ), targetType = int(IECore.TypeId.FloatVectorData) ), IECore.FloatVectorData( [ 1, 2, 3 ] ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.V3fData( IECore.V3f( 1, 2, 3 ) ), targetType = int(IECore.TypeId.Color3fData) ), IECore.Color3fData( IECore.Color3f( 1, 2, 3 ) ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.V3fVectorData( [ IECore.V3f(1), IECore.V3f(2), IECore.V3f(3) ] ), targetType = int(IECore.TypeId.FloatVectorData) ), IECore.FloatVectorData( [ 1, 1, 1, 2, 2, 2, 3, 3, 3 ] ) )

		self.assertEqual( IECore.DataCastOp()( object = IECore.FloatVectorData( [ 1, 2, 3 ] ), targetType = int(IECore.TypeId.V3fData) ), IECore.V3fData( IECore.V3f( 1, 2, 3 ) ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.Color3fData( IECore.Color3f( 1, 2, 3 ) ), targetType = int(IECore.TypeId.V3fData) ), IECore.V3fData( IECore.V3f( 1, 2, 3 ) ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.FloatVectorData( [ 1, 1, 1, 2, 2, 2, 3, 3, 3 ] ), targetType = int(IECore.TypeId.V3fVectorData) ), IECore.V3fVectorData( [ IECore.V3f(1), IECore.V3f(2), IECore.V3f(3) ] ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.V3fVectorData( [ IECore.V3f(1), IECore.V3f(2), IECore.V3f(3) ] ), targetType = int(IECore.TypeId.Color3fVectorData) ), IECore.Color3fVectorData( [ IECore.Color3f(1), IECore.Color3f(2), IECore.Color3f(3) ] ) )
		self.assertEqual( IECore.DataCastOp()( object = IECore.V3dVectorData( [ IECore.V3d(1), IECore.V3d(2), IECore.V3d(3) ] ), targetType = int(IECore.TypeId.Color3fVectorData) ), IECore.Color3fVectorData( [ IECore.Color3f(1), IECore.Color3f(2), IECore.Color3f(3) ] ) )

	def testInvalidConversions( self ) :
		tests = [
			( IECore.FloatVectorData( [ 1, 2, 3 ] ), int(IECore.TypeId.V2fData) ),
			( IECore.M33f(), int( IECore.TypeId.M44fData ) ),
			( IECore.FloatVectorData( [ 1, 2, 3, 4 ] ), int(IECore.TypeId.V3fData) ),
			( IECore.FloatVectorData( [ 1, 2 ] ), int(IECore.TypeId.V3fData) ),
			( IECore.FloatVectorData( [ 1, 2, 3, 4 ] ), int(IECore.TypeId.V3fVectorData) ),
			( IECore.FloatVectorData( [ 1, 2, 3, 4, 5 ] ), int(IECore.TypeId.V3fVectorData) ),
		]
		i = 0
		for ( obj, tt ) in tests:
			try:
				IECore.DataCastOp()( object = obj, targetType = tt )
			except:
				i += 1
			else:
				raise Exception, "Should fail on this test " + i

if __name__ == "__main__":
        unittest.main()
