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
import IECore

class testParameterParser( unittest.TestCase ) :

	def testMultiply( self ) :
	
		l = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) )
		a = l.load( "maths/multiply" )()

		p = a.parameters()
		IECore.ParameterParser().parse( ["-a", "10", "-b", "20" ], p )

		self.assertEqual( a(), IECore.IntData( 200 ) )

	def testParameterTypes( self ) :
	
		a = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) ).load( "parameterTypes" )()
		
		IECore.ParameterParser().parse( [
			"-a", "10",
			"-b", "20.2",
			"-c", "40.5",
			"-d", "hello",
			"-e", "2", "4", "5",
			"-f", "one", "two", "three",
			"-g", "2", "4",
			"-h", "1", "4", "8",
			"-i", "2", "4",
			"-compound.j", "1", "4", "8",
			"-compound.k", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
			"-l", "1", "0", "0",
			"-m", "1", "1", "0", "1",
			"-o", "myFile.tif",
			"-p", "test",
			"-q", "true",
			"-r", "mySequence.####.tif",
			"-s", "-1", "-2", "10", "20",
			"-t", "-1", "-2", "-3", "10", "20", "30",
			"-u", "64", "128",
			"-v", "25", "26", "27",
			], a.parameters() )
		
		a()
		
	def testPresetParsing( self ) :
	
		a = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) ).load( "presetParsing" )()
		
		IECore.ParameterParser().parse( [
			"-h", "x",
			"-compound", "one",
			], a.parameters() )
		
		a()
		
	def testReadParsing( self ) :
	
		l = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) )
		a = l.load( "maths/multiply" )()

		p = a.parameters()
		IECore.ParameterParser().parse( ["-a", "read:test/data/cobFiles/intDataTen.cob", "-b", "30" ], p )
		
		self.assertEqual( a(), IECore.IntData( 300 ) )
		
	def testSerialising( self ) :
	
		a = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) ).load( "parameterTypes" )()
		
		IECore.ParameterParser().parse( [
			"-a", "10",
			"-b", "20.2",
			"-c", "40.5",
			"-d", "hello",
			"-e", "2", "4", "5",
			"-f", "one", "two", "three",
			"-g", "2", "4",
			"-h", "1", "4", "8",
			"-i", "2", "4",
			"-compound.j", "1", "4", "8",
			"-compound.k", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
			"-l", "1", "0", "0",
			"-m", "1", "1", "0", "1",
			"-o", "myFile.tif",
			"-p", "test",
			"-q", "true",
			"-r", "mySequence.####.tif",
			"-s", "-1", "-2", "10", "20",
			"-t", "-1", "-2", "-3", "10", "20", "30",
			"-u", "64", "128",
			"-v", "25", "26", "27",
			], a.parameters() )
			
		a()
		
		s = IECore.ParameterParser().serialise( a.parameters() )
		a = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) ).load( "parameterTypes" )()
		IECore.ParameterParser().parse( s, a.parameters() )
		
		a()
		
	def testStringParsing( self ) :
	
		a = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) ).load( "stringParsing" )()
		
		IECore.ParameterParser().parse( [
			"-emptyString", "",
			"-normalString", "hello",
			"-stringWithSpace", "hello there",
			"-stringWithManySpaces", "hello there old chap",
			], a.parameters() )
		
		a()
		
		a = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) ).load( "stringParsing" )()
		
		IECore.ParameterParser().parse( "-emptyString '' -normalString 'hello' -stringWithSpace 'hello there' -stringWithManySpaces 'hello there old chap'", a.parameters() )
		
		a()

if __name__ == "__main__":
        unittest.main()

