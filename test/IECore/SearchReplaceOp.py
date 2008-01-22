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

import stat
import os
import unittest
from IECore import *



class TestSearchReplaceOp( unittest.TestCase ) :

	inputLines = [ "how\n", "now\n", "brown\n", "cow\n" ]
	expectedLinesSimple = [ "hen\n", "nen\n", "brenn\n", "cen\n" ]
	expectedLinesRegExp = [ "haa\n", "naa\n", "braan\n", "caa\n" ]

	inputFileName = "test/IECore/searchReplace.ma"	
	inputFileNameBak = "test/IECore/searchReplace.ma.bak"			
	outputFileName = "test/IECore/searchReplaceOut.ma"
		

	def testSimple( self ) :
	
		inputFileName = TestSearchReplaceOp.inputFileName
		inputFileNameBak = TestSearchReplaceOp.inputFileNameBak
		outputFileName = TestSearchReplaceOp.outputFileName		
	
		inputFile = open( inputFileName, "w")
		
		for line in TestSearchReplaceOp.inputLines:
		
			inputFile.write( line ) 
		
		inputFile.close()
		
		inputFilePermissions = os.stat( inputFileName ).st_mode
		
		op = SearchReplaceOp()
		
		result = op(
			source = inputFileName,
			destination = outputFileName,
			searchFor = "ow",
			replaceWith = "en",
		)
		
		self.assertEqual( result.value, outputFileName )
		self.assert_( os.path.exists( inputFileName ) )
		self.assert_( os.path.exists( outputFileName ) )
		self.assertEqual( inputFilePermissions, os.stat( outputFileName ).st_mode )
		
		outputFile = open( outputFileName, "r" )
		
		for expectedLine in TestSearchReplaceOp.expectedLinesSimple:
		
		 	outputLine = outputFile.readline()
			
			self.assertEqual( outputLine, expectedLine )
			
		outputFile.close()
		
	def testInPlace( self ) :
	
		inputFileName = TestSearchReplaceOp.inputFileName
		inputFileNameBak = TestSearchReplaceOp.inputFileNameBak		
		outputFileName = TestSearchReplaceOp.inputFileName				
	
		inputFile = open( inputFileName, "w")
		
		for line in TestSearchReplaceOp.inputLines:
		
			inputFile.write( line )
		
		inputFile.close()
		
		inputFilePermissions = os.stat( inputFileName ).st_mode
		
		op = SearchReplaceOp()
		
		result = op(
			source = inputFileName,
			destination = inputFileName,
			searchFor = "ow",
			replaceWith = "en",
		)
		
		self.assertEqual( result.value, outputFileName )
		self.assertEqual( result.value, inputFileName )
		
		self.assert_( os.path.exists( inputFileName ) )		
		self.assert_( os.path.exists( outputFileName ) )
		self.assert_( os.path.exists( inputFileNameBak ) )
		self.assertEqual( inputFilePermissions, os.stat( outputFileName ).st_mode )
		
		outputFile = open( outputFileName, "r" )
		
		for expectedLine in TestSearchReplaceOp.expectedLinesSimple:
		
		 	outputLine = outputFile.readline()
			
			self.assertEqual( outputLine, expectedLine )
			
		outputFile.close()
		
	def testRegExp( self ) :
	
		inputFileName = TestSearchReplaceOp.inputFileName
		inputFileNameBak = TestSearchReplaceOp.inputFileNameBak		
		outputFileName = TestSearchReplaceOp.outputFileName		
	
		inputFile = open( inputFileName, "w")
		
		for line in TestSearchReplaceOp.inputLines:
		
			inputFile.write( line )
		
		inputFile.close()
		
		inputFilePermissions = os.stat( inputFileName ).st_mode
		
		op = SearchReplaceOp()
		
		result = op(
			source = inputFileName,
			destination = outputFileName,
			searchFor = "[ow]",
			replaceWith = "a",
			regexpSearch = True			
		)
		
		self.assertEqual( result.value, outputFileName )
		self.assert_( os.path.exists( inputFileName ) )
		self.assert_( os.path.exists( outputFileName ) )
		self.assertEqual( inputFilePermissions, os.stat( outputFileName ).st_mode )
		
		outputFile = open( outputFileName, "r" )
		
		for expectedLine in TestSearchReplaceOp.expectedLinesRegExp:
		
		 	outputLine = outputFile.readline()
			
			self.assertEqual( outputLine, expectedLine )
			
		outputFile.close()		
		
	def tearDown( self ):	
		
		if os.path.exists( TestSearchReplaceOp.inputFileName ):
			os.remove( TestSearchReplaceOp.inputFileName )
			
		if os.path.exists( TestSearchReplaceOp.inputFileNameBak ):
			os.remove( TestSearchReplaceOp.inputFileNameBak )
			
		if os.path.exists( TestSearchReplaceOp.outputFileName ):
			os.remove( TestSearchReplaceOp.outputFileName )			
			
	

if __name__ == "__main__":
    unittest.main()   
