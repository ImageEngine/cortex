# -*- coding: utf-8 -*-
##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

class InternedStringTest( unittest.TestCase ) :

	def test( self ) :

		originalSize = IECore.InternedString.numUniqueStrings()

		s1 = IECore.InternedString( "nothingElseIsUsingThisStringYet" )
		s2 = IECore.InternedString( "nothingElseIsUsingThisStringYet" )
		self.assertEqual( s1, s2 )
		self.assertEqual( str( s1 ), "nothingElseIsUsingThisStringYet" )
		self.assertEqual( str( s1 ), str( s2 ) )
		self.assertEqual( IECore.InternedString.numUniqueStrings(), originalSize + 1 )

		s3 = IECore.InternedString( "nothingElseIsUsingThisStringYetEither" )
		self.assertNotEqual( s1, s3 )
		self.assertEqual( str( s3 ), "nothingElseIsUsingThisStringYetEither" )
		self.assertEqual( str( s1 ), "nothingElseIsUsingThisStringYet" )
		self.assertEqual( str( s2 ), "nothingElseIsUsingThisStringYet" )
		self.assertEqual( IECore.InternedString.numUniqueStrings(), originalSize + 2 )

		s4 = IECore.InternedString( s1 )
		self.assertEqual( s1, s4 )
		self.assertEqual( s2, s4 )
		self.assertNotEqual( s3, s4 )
		self.assertEqual( IECore.InternedString.numUniqueStrings(), originalSize + 2 )

	def testDefaultConstructor( self ) :

		self.assertEqual( IECore.InternedString(), IECore.InternedString( "" ) )

	def testNumberConstructor( self ) :

		self.assertEqual( IECore.InternedString(0), IECore.InternedString("0") )
		self.assertEqual( IECore.InternedString(-1), IECore.InternedString("-1") )
		self.assertEqual( IECore.InternedString(1), IECore.InternedString("1") )

	def testHashForSetsAndDicts( self ) :

		def makeStrings( r ) :
			return [ IECore.InternedString( "unique%d"%i ) for i in r ]

		# create a list of unique interned strings
		strings = makeStrings( range(0,30000) )

		# make sure the hash creates unique ids
		uniqueStringsSet = set(strings)
		self.assertEqual( len(strings), len(uniqueStringsSet) )

		# make sure set comparison works
		stringRange = makeStrings( range(10,200) )
		stringRange.reverse()
		self.assertEqual( set(stringRange), set(strings[10:200]) )

		self.assertEqual( set(stringRange).difference(uniqueStringsSet), set() )

	def testLength( self ):
		emptyString = IECore.InternedString()
		helloString = IECore.InternedString( 'Hello' )

		self.assertEqual( len( emptyString ), 0 )
		self.assertFalse( emptyString )

		self.assertEqual( len( helloString ), 5 )
		self.assertTrue( helloString )

	def testUTF8( self ) :

		s = u"abcdé"
		i = IECore.InternedString( s )
		self.assertEqual( str( i ), s )

if __name__ == "__main__":
	unittest.main()

