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
import random

import IECore

class TestVectorDataFilterOp( unittest.TestCase ) :

	def test( self ) :

		i = IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] )
		f = IECore.BoolVectorData( [ 0, 1, 0, 1 ] )

		ii = IECore.VectorDataFilterOp()( input = i, filter = f, invert = False, clip = True )
		self.assertEqual( ii, IECore.IntVectorData( [ 2, 4 ] ) )

		ii = IECore.VectorDataFilterOp()( input = i, filter = f, invert = True, clip = True )
		self.assertEqual( ii, IECore.IntVectorData( [ 1, 3 ] ) )

		ii = IECore.VectorDataFilterOp()( input = i, filter = f, invert = False, clip = False )
		self.assertEqual( ii, IECore.IntVectorData( [ 2, 4, 5, 6 ] ) )

		ii = IECore.VectorDataFilterOp()( input = i, filter = f, invert = True, clip = False )
		self.assertEqual( ii, IECore.IntVectorData( [ 1, 3, 5, 6 ] ) )

	def testOperateInPlace( self ) :

		f = IECore.BoolVectorData( [ 0, 1, 0, 1 ] )

		i = IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] )
		IECore.VectorDataFilterOp()( input = i, copyInput = False, filter = f, invert = False, clip = True )
		self.assertEqual( i, IECore.IntVectorData( [ 2, 4 ] ) )

		i = IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] )
		IECore.VectorDataFilterOp()( input = i, copyInput = False, filter = f, invert = True, clip = True )
		self.assertEqual( i, IECore.IntVectorData( [ 1, 3 ] ) )

		i = IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] )
		IECore.VectorDataFilterOp()( input = i, copyInput = False, filter = f, invert = False, clip = False )
		self.assertEqual( i, IECore.IntVectorData( [ 2, 4, 5, 6 ] ) )

		i = IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] )
		IECore.VectorDataFilterOp()( input = i, copyInput = False, filter = f, invert = True, clip = False )
		self.assertEqual( i, IECore.IntVectorData( [ 1, 3, 5, 6 ] ) )

		for i in range( 0, 1000 ) :
			m = IECore.BoolVectorData()
			v = IECore.V3fVectorData()
			n = 0
			for j in range( 0, random.randint( 0, 1000 ) ) :
				m.append( random.randint( 0,1 ) )
				n += m[-1]
				v.append( IECore.V3f( 0 ) )
			IECore.VectorDataFilterOp()( input = v, copyInput = False, filter = m )
			self.assertEqual( len( v ), n )

if __name__ == "__main__":
	unittest.main()

