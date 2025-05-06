##########################################################################
#
#  Copyright (c) 2025, Cinesite VFX Ltd. All rights reserved.
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

import IECore

class ObjectMatrixTest( unittest.TestCase ) :

	def test( self ) :

		m = IECore.ObjectMatrix()
		self.assertEqual( m.numRows(), 0 )
		self.assertEqual( m.numColumns(), 0 )

		m = IECore.ObjectMatrix( 4, 3 )
		self.assertEqual( m.numRows(), 4 )
		self.assertEqual( m.numColumns(), 3 )

		self.assertIsNone( m[0, 0] )
		self.assertIsNone( m[0, 1] )
		m[0, 1] = IECore.IntData( 123 )

		self.assertIsNone( m[0, 0] )
		self.assertEqual( m[0, 1], IECore.IntData( 123 ) )

		m[0, 1] = None
		self.assertIsNone( m[0, 1] )

		m[0, 1] = "foo"
		self.assertEqual( m[0, 1], IECore.StringData( "foo" ) )

		m[3, 2] = IECore.StringData( "test" )
		self.assertEqual( m[3, 2], IECore.StringData( "test" ) )
		self.assertEqual( m[-1, -1], IECore.StringData( "test" ) )

		with self.assertRaises( IndexError ) :
			m[7, 0]
			m[0, 7]
			m[7, 7]
			m[-7, -7]

	def testConstructFromSequence( self ) :

		m = IECore.ObjectMatrix( [ [ IECore.IntData( 123 ) ] ] )
		self.assertEqual( m.numRows(), 1 )
		self.assertEqual( m.numColumns(), 1 )
		self.assertEqual( m[0, 0], IECore.IntData( 123 ) )

		m = IECore.ObjectMatrix( [ [ "foo" ], [ 1 ] ] )
		self.assertEqual( m.numRows(), 2 )
		self.assertEqual( m.numColumns(), 1 )
		self.assertEqual( m[0, 0], IECore.StringData( "foo" ) )
		self.assertEqual( m[1, 0], IECore.IntData( 1 ) )

		m = IECore.ObjectMatrix( [
			[ IECore.IntData( row * 4 + column ) for column in range( 4 ) ]
			for row in range( 3 )
		] )
		self.assertEqual( m.numRows(), 3 )
		self.assertEqual( m.numColumns(), 4 )

		for row in range( m.numRows() ) :
			for column in range( m.numColumns() ) :
				self.assertEqual( m[row, column], IECore.IntData( row * m.numColumns() + column ) )

		m = IECore.ObjectMatrix( [
			[ IECore.IntData( 1 ), IECore.StringData( "B" ), IECore.FloatData( 1.0 ) ],
			[ None, IECore.StringData( "C" ) ]
		] )
		self.assertEqual( m.numRows(), 2 )
		self.assertEqual( m.numColumns(), 3 )
		self.assertEqual( m[0, 0], IECore.IntData( 1 ) )
		self.assertEqual( m[0, 1], IECore.StringData( "B" ) )
		self.assertEqual( m[0, 2], IECore.FloatData( 1.0 ) )
		self.assertEqual( m[1, 0], None )
		self.assertEqual( m[1, 1], IECore.StringData( "C" ) )
		self.assertEqual( m[1, 2], None )

		with self.assertRaises( ValueError ) :
			IECore.ObjectMatrix( [ "notAList" ] )
			IECore.ObjectMatrix( [ [ IECore.IntData( 123 ), IECore.FloatData( 456.0 ) ], "alsoNotAList" ] )

	def testCompare( self ) :

		m1 = IECore.ObjectMatrix( 4, 4 )
		for row in range( 4 ) :
			for column in range( 4 ) :
				m1[row, column] = IECore.IntData( row * 4 + column )

		for value in [ IECore.FloatData( 1.0 ), None ] :

			m2 = m1.copy()
			self.assertEqual( m1, m2 )
			self.assertFalse( m2 != m1 )

			m2[0, 0] = value
			self.assertNotEqual( m1, m2 )
			self.assertNotEqual( m2, m1 )

			m3 = m2.copy()
			self.assertEqual( m2, m3 )
			self.assertEqual( m3[0, 0], value )

			m3[0, 0] = IECore.CompoundData()
			self.assertNotEqual( m2, m3 )
			self.assertEqual( m2[0, 0], value )
			self.assertEqual( m3[0, 0], IECore.CompoundData() )

			m3[0, 0] = m2[0, 0]
			self.assertEqual( m2, m3 )
			self.assertEqual( m3, m2 )

	def testHash( self ) :

		m = IECore.ObjectMatrix( 1, 1 )
		h = m.hash()

		m[0, 0] = IECore.IntData( 10 )
		self.assertNotEqual( m.hash(), h )

		m1 = IECore.ObjectMatrix( 2, 1 )
		m1[0, 0] = IECore.StringData( "A" )
		m1[1, 0] = IECore.FloatData( 2.0 )

		m2 = IECore.ObjectMatrix( 1, 2 )
		m2[0, 0] = IECore.StringData( "A" )
		m2[0, 1] = IECore.FloatData( 2.0 )
		self.assertNotEqual( m1.hash(), m2.hash() )

	def testRepr( self ) :

		m = IECore.ObjectMatrix( 0, 0 )
		self.assertEqual( repr( m ), "IECore.ObjectMatrix()" )

		m = IECore.ObjectMatrix( 1, 1 )
		self.assertEqual( repr( m ), "IECore.ObjectMatrix( [ [ None ] ] )" )

		m = IECore.ObjectMatrix( 1, 2 )
		self.assertEqual( repr( m ), "IECore.ObjectMatrix( [ [ None, None ] ] )" )

		m = IECore.ObjectMatrix( 2, 1 )
		self.assertEqual( repr( m ), "IECore.ObjectMatrix( [ [ None ], [ None ] ] )" )

		m = IECore.ObjectMatrix( 2, 2 )
		self.assertEqual( repr( m ), "IECore.ObjectMatrix( [ [ None, None ], [ None, None ] ] )" )

		m = IECore.ObjectMatrix( 3, 2 )
		m[0, 0] = IECore.IntData( 42 )
		m[0, 1] = IECore.StringData( "123" )
		m[1, 0] = IECore.FloatData( 0.1 )
		m[1, 1] = IECore.CompoundData()
		m[2, 1] = IECore.StringVectorData( [ "a" ] )
		self.assertEqual( repr( m ), "IECore.ObjectMatrix( [ [ IECore.IntData( 42 ), IECore.StringData( '123' ) ], [ IECore.FloatData( 0.1 ), IECore.CompoundData() ], [ None, IECore.StringVectorData( [ 'a' ] ) ] ] )" )

	def testResize( self ) :

		m1 = IECore.ObjectMatrix( 3, 3 )
		for row in range( 3 ) :
			for column in range( 3 ) :
				m1[row, column] = IECore.IntData( row * 3 + column )

		m2 = m1.copy()
		self.assertEqual( m1, m2 )

		# Enlarge m2. Its values should match their equivalent coordinate in m1.
		m2.resize( 5, 4 )
		self.assertNotEqual( m1, m2 )

		for row in range( m2.numRows() ) :
			for column in range( m2.numColumns() ) :
				if row < m1.numRows() and column < m1.numColumns() :
					self.assertEqual( m1[row, column], m2[row, column] )
				else :
					self.assertIsNone( m2[row, column] )

		# Shrink m2 back to the same size as m1. Both should match.
		m2.resize( 3, 3 )
		self.assertEqual( m1, m2 )

		# Shrink m2 smaller than m1, remaining values should still match their equivalent coordinate in m1.
		m2.resize( 2, 2 )
		for row in range( m2.numRows() ) :
			for column in range( m2.numRows() ) :
				self.assertEqual( m1[row, column], m2[row, column] )

if __name__ == "__main__":
	unittest.main()
