##########################################################################
#
#  Copyright (c) 2010, John Haddon. All rights reserved.
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

class CamelCaseTest( unittest.TestCase ) :

	def testSplit( self ) :
	
		self.assertEqual( IECore.CamelCase.split( "A" ), [ "A" ] )
		self.assertEqual( IECore.CamelCase.split( "a" ), [ "a" ] )
		self.assertEqual( IECore.CamelCase.split( "AB" ), [ "AB" ] )
		self.assertEqual( IECore.CamelCase.split( "ab" ), [ "ab" ] )
		self.assertEqual( IECore.CamelCase.split( "aB" ), [ "a", "B" ] )
		self.assertEqual( IECore.CamelCase.split( "Ab" ), [ "Ab" ] )
		self.assertEqual( IECore.CamelCase.split( "TIFFImageReader" ), [ "TIFF", "Image", "Reader" ] )
		self.assertEqual( IECore.CamelCase.split( "camelCase" ), [ "camel", "Case" ] )
		self.assertEqual( IECore.CamelCase.split( "hsvToRGB" ), [ "hsv", "To", "RGB" ] )
		
	def testJoin( self ) :
	
		self.assertEqual( IECore.CamelCase.join( [ "camel", "case" ], IECore.CamelCase.Caps.Unchanged ), "camelcase" )
		self.assertEqual( IECore.CamelCase.join( [ "camel", "case" ], IECore.CamelCase.Caps.First ), "Camelcase" )
		self.assertEqual( IECore.CamelCase.join( [ "camel", "case" ], IECore.CamelCase.Caps.All ), "CamelCase" )
		self.assertEqual( IECore.CamelCase.join( [ "camel", "case" ], IECore.CamelCase.Caps.AllExceptFirst ), "camelCase" )

		self.assertEqual( IECore.CamelCase.join( [ "TIFF", "image", "reader" ], IECore.CamelCase.Caps.Unchanged ), "TIFFimagereader" )
		self.assertEqual( IECore.CamelCase.join( [ "TIFF", "image", "reader" ], IECore.CamelCase.Caps.First ), "TIFFimagereader" )
		self.assertEqual( IECore.CamelCase.join( [ "TIFF", "image", "reader" ], IECore.CamelCase.Caps.All ), "TIFFImageReader" )
		self.assertEqual( IECore.CamelCase.join( [ "TIFF", "image", "reader" ], IECore.CamelCase.Caps.AllExceptFirst ), "tiffImageReader" )
		
	def testToSpaced( self ) :
	
		self.assertEqual( IECore.CamelCase.toSpaced( "camelCase" ), "Camel Case" )
		self.assertEqual( IECore.CamelCase.toSpaced( "camelCase", IECore.CamelCase.Caps.All ), "Camel Case" )
		self.assertEqual( IECore.CamelCase.toSpaced( "camelCase", IECore.CamelCase.Caps.First ), "Camel case" )
		self.assertEqual( IECore.CamelCase.toSpaced( "camelCase", IECore.CamelCase.Caps.AllExceptFirst ), "camel Case" )

		self.assertEqual( IECore.CamelCase.toSpaced( "TIFFImageReader" ), "TIFF Image Reader" )
		self.assertEqual( IECore.CamelCase.toSpaced( "TIFFImageReader", IECore.CamelCase.Caps.All ), "TIFF Image Reader" )
		self.assertEqual( IECore.CamelCase.toSpaced( "TIFFImageReader", IECore.CamelCase.Caps.First ), "TIFF image reader" )
		self.assertEqual( IECore.CamelCase.toSpaced( "TIFFImageReader", IECore.CamelCase.Caps.AllExceptFirst ), "tiff Image Reader" )
		
	def testFromSpaced( self ) :
	
		self.assertEqual( IECore.CamelCase.fromSpaced( "camel case" ), "CamelCase" )
		self.assertEqual( IECore.CamelCase.fromSpaced( "camel case", IECore.CamelCase.Caps.All ), "CamelCase" )
		self.assertEqual( IECore.CamelCase.fromSpaced( "camel case", IECore.CamelCase.Caps.First ), "Camelcase" )
		self.assertEqual( IECore.CamelCase.fromSpaced( "camel case", IECore.CamelCase.Caps.AllExceptFirst ), "camelCase" )
	
	def testNumericCharacters( self ) :
	
		self.assertEqual( IECore.CamelCase.split( "wordsThenNumbers2346" ), [ "words", "Then", "Numbers2346" ] )
		self.assertEqual( IECore.CamelCase.split( "Numbers2346ThenWords" ), [ "Numbers2346", "Then", "Words" ] )
	
if __name__ == "__main__":
	unittest.main()
	
