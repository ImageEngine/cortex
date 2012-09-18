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

import random
import unittest
from IECore import *

class RadixSortTest( unittest.TestCase ) :

	def testFloat( self ) :

		random.seed( 12 )

		s = RadixSort()

		d = FloatVectorData()

		for i in range( 0, 10000 ):
			d.append( random.uniform( FloatData().minValue, FloatData().maxValue ) )

		idx = s.sort( d )

		self.assertEqual( len(idx), 10000 )

		for i in range( 1, 10000 ):

			self.assert_( d[ idx[ i ] ] >= d[ idx[ i - 1 ] ] )

	def testInt( self ) :

		random.seed( 13 )

		s = RadixSort()

		d = IntVectorData()

		for i in range( 0, 10000 ):
			d.append( int( random.uniform( IntData().minValue, IntData().maxValue ) ) )

		idx = s.sort( d )

		self.assertEqual( len(idx), 10000 )

		for i in range( 1, 10000 ):

			self.assert_( d[ idx[ i ] ] >= d[ idx[ i - 1 ] ] )

	def testUInt( self ) :

		random.seed( 14 )

		s = RadixSort()

		d = UIntVectorData()

		for i in range( 0, 10000 ):
			d.append( int( random.uniform( UIntData().minValue, UIntData().maxValue ) ) )

		idx = s.sort( d )

		self.assertEqual( len(idx), 10000 )

		for i in range( 1, 10000 ):

			self.assert_( d[ idx[ i ] ] >= d[ idx[ i - 1 ] ] )


if __name__ == "__main__":
	unittest.main()

