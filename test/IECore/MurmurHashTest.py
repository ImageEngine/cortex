##########################################################################
#
#  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

class MurmurHashTest( unittest.TestCase ) :

	def testConstructor( self ) :

		h = IECore.MurmurHash()
		self.assertEqual( str( h ), "0" * 32 )
		self.assertEqual( h, IECore.MurmurHash() )

	def testCopyConstructor( self ) :

		h = IECore.MurmurHash()
		h.append( 1 )
		h.append( "hello" )

		self.assertEqual( h, IECore.MurmurHash( h ) )
		self.assertNotEqual( h, IECore.MurmurHash() )

	def testAppend( self ) :

		h = IECore.MurmurHash()
		for k in [ "hello", 'a', 1, 2.0, 2**62 ] :
			h2 = IECore.MurmurHash( h )
			h.append( k )
			self.assertNotEqual( h, h2 )

	def testKnownHashes( self ) :

		# test against hashes generated using the smhasher code directly

		h = IECore.MurmurHash()
		h.append( "the quick brown fox jumps over the lazy dog" )
		self.assertEqual(
			str( h ),
			"f476fee540bfc268dc36e7f3d95ddb72",
		)

		h = IECore.MurmurHash()
		h.append( 101 )
		self.assertEqual(
			str( h ),
			"1739807a7ecb8d70bbf1c02a2a649b8f",
		)

		h = IECore.MurmurHash()
		h.append( IECore.FloatVectorData( [ 1, 2, 3 ] ) )
		self.assertEqual(
			str( h ),
			"4f732afec7493057d3e7443b584cfd94",
		)

	def testStringRepeatability( self ) :

		h = IECore.MurmurHash()
		h.append( "i am a lovely string" )

		for i in range( 0, 1000 ) :
			h2 = IECore.MurmurHash()
			h2.append( "i am a lovely string" )
			self.assertEqual( h, h2 )

	def testStringDifferences( self ) :

		s = "i am a lovely string"
		h = IECore.MurmurHash()
		h.append( s )

		for i in range( 1, len( s ) ) :
			h2 = IECore.MurmurHash()
			h2.append( s[0:-i] )
			self.assertNotEqual( h, h2 )

	def testInternedString( self ) :

		# because the underlying data is identical,
		# InternedStrings should hash equal to their
		# string equivalents
		for s in [ "one", "apple", "two", "fish" ] :
			h1 = IECore.MurmurHash()
			h1.append( s )
			h2 = IECore.MurmurHash()
			h2.append( IECore.InternedString( s ) )
			self.assertEqual( h1, h2 )

	def testInternedStringVector( self ) :

		# because the underlying data is identical,
		# InternedStrings should hash equal to their
		# string equivalents
		s = [ "one", "apple", "two", "fish" ]

		h1 = IECore.MurmurHash()
		h1.append( IECore.StringVectorData( s ) )

		h2 = IECore.MurmurHash()
		h2.append( IECore.InternedStringVectorData( s ) )

		self.assertEqual( h1, h2 )

	def testTypeDoesntMatter( self ) :

		# although these are different types they should hash equal, because
		# the underlying data has exactly the same layout. see TypedDataTest.testHash()
		# for an equivalent test showing that the differing types /are/ taken
		# into account when hashing at the Object level.

		h1 = IECore.MurmurHash()
		h2 = IECore.MurmurHash()

		h1.append( imath.V3f( 1, 2, 3 ) )
		h2.append( imath.Color3f( 1, 2, 3 ) )

		self.assertEqual( h1, h2 )

		h1 = IECore.MurmurHash()
		h2 = IECore.MurmurHash()

		h1.append( IECore.IntVectorData( [ 0, 0, 0 ] ) )
		h2.append( IECore.UIntVectorData( [ 0, 0, 0 ] ) )

		self.assertEqual( h1, h2 )

	def testAllDimensionsOfImathVecs( self ) :

		vv = [ imath.V3f( 1, 2, 3 ), imath.Color4f( 1, 2, 3, 4 ) ]
		for v in vv :
			h = IECore.MurmurHash()
			h.append( v )
			for i in range( 0, v.dimensions() ) :
				v[i] = 0
				nextH = IECore.MurmurHash()
				nextH.append( v )
				self.assertNotEqual( h, nextH )
				h = nextH

	def testAllElementsOfImathBoxes( self ) :

		vv = [
			imath.Box3f( imath.V3f( 1, 2, 3 ), imath.V3f( 4, 5, 6 ) ),
			imath.Box2f( imath.V2f( 1, 2 ), imath.V2f( 3, 4 ) ),
		]

		for v in vv :
			h = IECore.MurmurHash()
			h.append( v )
			for m in range( 0, 1 ) :
				for i in range( 0, v.min().dimensions() ) :
					minMax = [ v.min(), v.max() ]
					minMax[m][i] = 0
					v2 = v.__class__( *minMax )
					h2 = IECore.MurmurHash()
					h2.append( v2 )
					self.assertNotEqual( h, h2 )

	def testCopyFrom( self ) :

		h1 = IECore.MurmurHash()
		h2 = IECore.MurmurHash()
		self.assertEqual( h1, h2 )

		h1.append( 1 )
		self.assertNotEqual( h1, h2 )

		h2.copyFrom( h1 )
		self.assertEqual( h1, h2 )

	def testHashOfEmptyStrings( self ) :

		h1 = IECore.MurmurHash()
		h2 = IECore.MurmurHash()

		h2.append( "" )
		self.assertNotEqual( h1, h2 )

		h1 = IECore.MurmurHash()
		h2 = IECore.MurmurHash()

		h1.append( IECore.StringVectorData( [ "" ] ) )
		h2.append( IECore.StringVectorData( [ "", "" ] ) )

		self.assertNotEqual( h1, h2 )

	def testDataAccess( self ) :

		# Test accessors for the internal data ( it's not usually necessary to access
		# these directly, but it can be useful in special cases )

		for t in [ "the quick", 101, IECore.FloatVectorData( [1, 2, 3] ) ]:

			h = IECore.MurmurHash()
			h.append( t )
			self.assertEqual( str( h ), "{:016x}{:016x}".format( h.h1(), h.h2() ) )

			h2 = IECore.MurmurHash( h.h1(), h.h2() )
			self.assertEqual( h, h2 )


if __name__ == "__main__":
	unittest.main()

