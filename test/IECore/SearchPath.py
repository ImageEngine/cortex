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

import os
import unittest

import IECore

class SearchPathTest( unittest.TestCase ) :

	def test( self ) :

		s = IECore.SearchPath()
		self.assertEqual( s.paths, [] )
		s.setPaths( os.pathsep.join( [ "a", "b", "c" ] ), os.pathsep )
		self.assertEqual( s.paths, [ "a", "b", "c" ] )
		s.paths = [ "one", "two", "three" ]
		self.assertEqual( s.paths, [ "one", "two", "three" ] )

		s = IECore.SearchPath( os.pathsep.join( [ "a", "b", "c" ] ), os.pathsep )
		self.assertEqual( s.paths, [ "a", "b", "c" ] )
		self.assertEqual( s.getPaths( os.pathsep ), os.pathsep.join( [ "a", "b", "c" ] ) )

	def testFind( self ) :

		s = IECore.SearchPath( os.path.join( "test", "IECore", "data", "pdcFiles" ), os.pathsep )

		self.assertEqual( s.find( "particleShape1.250.pdc" ), os.path.join( "test", "IECore", "data", "pdcFiles", "particleShape1.250.pdc" ) )

	def testCopyConstructor( self ) :

		s = IECore.SearchPath( os.pathsep.join( [ "a", "b", "c" ] ), os.pathsep )
		s2 = IECore.SearchPath( s )

		self.assertEqual( s, s2 )

	def testConstructFromSequence( self ) :

		s = IECore.SearchPath( [ "a", "b", "c" ] )
		self.assertEqual( s.paths, [ "a", "b", "c" ] )

		s = IECore.SearchPath( ( "a", "b", "c" ) )
		self.assertEqual( s.paths, [ "a", "b", "c" ] )

	def testConstructUsingPlatformSeparator( self ) :

		s = IECore.SearchPath( os.pathsep.join( [ "a", "b", "c" ] ) )
		self.assertEqual( s.paths, [ "a", "b", "c" ] )

if __name__ == "__main__":
    unittest.main()
