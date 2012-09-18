##########################################################################
#
#  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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
import os, sys
import IECore

class TestObjectReader( unittest.TestCase ) :

	def testConstruction( self ) :

		r = IECore.Reader.create( "test/IECore/data/cobFiles/compoundData.cob" )
		self.assertEqual( type( r ), IECore.ObjectReader )
		self.assertEqual( r["fileName"].getValue().value, "test/IECore/data/cobFiles/compoundData.cob" )

	def testRead( self ) :

		r = IECore.Reader.create( "test/IECore/data/cobFiles/compoundData.cob" )
		self.assertEqual( type( r ), IECore.ObjectReader )

		c = r.read()

		self.assertEqual( len(c), 4 )
		self.assertEqual( c["banana"].value, 2)
		self.assertEqual( c["apple"].value, 12)
		self.assertEqual( c["lemon"].value, -3)
		self.assertEqual( c["melon"].value, 2.5)

	def testSlashInKey( self ) :
		
		c = IECore.CompoundData( { "a/b" : IECore.StringData( "test" ) } )
		IECore.ObjectWriter( c, "test/compoundData.cob" ).write()
		c2 = IECore.ObjectReader( "test/compoundData.cob" ).read()
		
		self.assertEqual( c, c2 )
	
	def tearDown( self ) :
		
		for f in ( "test/compoundData.cob" ) :
			if os.path.isfile( f ) :
				os.remove( f )

if __name__ == "__main__":
	unittest.main()

