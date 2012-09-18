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

from IECore import *

class TestDataPromoteOp( unittest.TestCase ) :

	def __makeVectorSourceData( self, dataType ):

		d = dataType()

		for i in range( 0, 100 ) :

			d.append( i )

		return d

	def testVector( self ) :

		v = range( 0, 100 )

		op = DataPromoteOp()

		self.assertEqual( op( object = self.__makeVectorSourceData( IntVectorData ), targetType=int(TypeId.V3fVectorData) ), V3fVectorData( [ V3f( x ) for x in v ] ) )
		self.assertEqual( op( object = self.__makeVectorSourceData( ShortVectorData ), targetType=int(TypeId.V3dVectorData) ), V3dVectorData( [ V3d( x ) for x in v ] ) )
		self.assertEqual( op( object = self.__makeVectorSourceData( HalfVectorData ), targetType=int(TypeId.Color3fVectorData) ), Color3fVectorData( [ Color3f( x ) for x in v ] ) )
		self.assertEqual( op( object = self.__makeVectorSourceData( HalfVectorData ), targetType=int(TypeId.Color3fVectorData) ), Color3fVectorData( [ Color3f( x ) for x in v ] ) )
		self.assertEqual( op( object = self.__makeVectorSourceData( HalfVectorData ), targetType=int(TypeId.V2fVectorData) ), V2fVectorData( [ V2f( x ) for x in v ] ) )

	def testSimple( self ) :

		op = DataPromoteOp()

		self.assertEqual( op( object = IntData(2), targetType=int(TypeId.V3fData) ), V3fData( V3f(2.0) ) )
		self.assertEqual( op( object = IntData(2), targetType=int(TypeId.V3dData) ), V3dData( V3d(2.0) ) )
		self.assertEqual( op( object = IntData(2), targetType=int(TypeId.Color3fData) ), Color3fData( Color3f(2.0) ) )

if __name__ == "__main__":
        unittest.main()
