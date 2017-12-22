##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

class TestDataPromoteOp( unittest.TestCase ) :

	def __makeVectorSourceData( self, dataType ):

		d = dataType()

		for i in range( 0, 100 ) :

			d.append( i )

		return d

	def testVector( self ) :

		v = range( 0, 100 )

		op = IECore.DataPromoteOp()

		self.assertEqual( op( object = self.__makeVectorSourceData( IECore.IntVectorData ), targetType=int(IECore.TypeId.V3fVectorData) ), IECore.V3fVectorData( [ imath.V3f( x ) for x in v ] ) )
		self.assertEqual( op( object = self.__makeVectorSourceData( IECore.ShortVectorData ), targetType=int(IECore.TypeId.V3dVectorData) ), IECore.V3dVectorData( [ imath.V3d( x ) for x in v ] ) )
		self.assertEqual( op( object = self.__makeVectorSourceData( IECore.HalfVectorData ), targetType=int(IECore.TypeId.Color3fVectorData) ), IECore.Color3fVectorData( [ imath.Color3f( x ) for x in v ] ) )
		self.assertEqual( op( object = self.__makeVectorSourceData( IECore.HalfVectorData ), targetType=int(IECore.TypeId.Color3fVectorData) ), IECore.Color3fVectorData( [ imath.Color3f( x ) for x in v ] ) )
		self.assertEqual( op( object = self.__makeVectorSourceData( IECore.HalfVectorData ), targetType=int(IECore.TypeId.V2fVectorData) ), IECore.V2fVectorData( [ imath.V2f( x ) for x in v ] ) )

	def testSimple( self ) :

		op = IECore.DataPromoteOp()

		self.assertEqual( op( object = IECore.IntData(2), targetType=int(IECore.TypeId.V3fData) ), IECore.V3fData( imath.V3f(2.0) ) )
		self.assertEqual( op( object = IECore.IntData(2), targetType=int(IECore.TypeId.V3dData) ), IECore.V3dData( imath.V3d(2.0) ) )
		self.assertEqual( op( object = IECore.IntData(2), targetType=int(IECore.TypeId.Color3fData) ), IECore.Color3fData( imath.Color3f(2.0) ) )

if __name__ == "__main__":
        unittest.main()
