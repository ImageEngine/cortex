##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
import shutil
from IECore import *

# \todo use setUp and tearDown to remove directories, also use shutil.rmtree instead of os.system("rm -rf")

class testFrameRange( unittest.TestCase ) :

	def testConstructor( self ) :
	
		self.assertRaises( TypeError, FrameRange, 1, 100.1 )
		self.assertRaises( TypeError, FrameRange, "1", "100" )
		self.assertRaises( ValueError, FrameRange, 10, 1 )
		self.assertRaises( ValueError, FrameRange, 1, 9, 3 )
		
		r = FrameRange( 1, 100 )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 100 )
		self.assertEqual( r.step, 1 )
		self.assertEqual( str(r), "1-100" )
		
		r = FrameRange( 2, 200, 2 )
		self.assertEqual( r.start, 2 )
		self.assertEqual( r.end, 200 )
		self.assertEqual( r.step, 2 )
		self.assertEqual( str(r), "2-200x2" )
		
		r = FrameRange( 3, 3 )
		self.assertEqual( r.start, r.end )
		self.assertEqual( r.start, 3 )
		self.assertEqual( str(r), "3" )
		
	def testParser( self ) :
	
		r = FrameList.parse( "1" )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 1 )
		self.assertEqual( r.step, 1 )
		
		r = FrameList.parse( "-1" )
		self.assertEqual( r.start, -1 )
		self.assertEqual( r.end, -1 )
		self.assertEqual( r.step, 1 )
		
		r = FrameList.parse( "1-100" )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 100 )
		self.assertEqual( r.step, 1 )
		
		r = FrameList.parse( "1-11x2" )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 11 )
		self.assertEqual( r.step, 2 )
		
		r = FrameList.parse( "-100--10x2" )
		self.assertEqual( r.start, -100 )
		self.assertEqual( r.end, -10 )
		self.assertEqual( r.step, 2 )
		
	def testClumping( self ) :
	
		r = FrameRange( 1, 32 )
		c = r.asClumpedList( 10 )
		self.assertEqual( c, [ [ 1, 2, 3, 4, 5, 6, 7 ,8, 9, 10 ], [ 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 ], [ 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 ], [ 31, 32 ] ] )

class testFileSequence( unittest.TestCase ) :


	def testConstructor( self ) :
		
		self.assertRaises( ValueError, FileSequence, "", FrameRange( 0, 1 ) )
		
		s = FileSequence( "seq.#.tif", FrameRange( 0, 2 ) )
		self.assertEqual( s.fileNames(), ["seq.0.tif", "seq.1.tif", "seq.2.tif"] )
		
	def testPadding( self ) :
		
		s = FileSequence( "seq.#.tif", FrameRange( 0, 2 ) )
		self.assertEqual( s.getPadding(), 1 )
		s.setPadding( 4 )
		self.assertEqual( s.getPadding(), 4 )
		self.assertEqual( s.fileName, "seq.####.tif" )
		
		self.assertEqual( s.fileNames(), ["seq.0000.tif", "seq.0001.tif", "seq.0002.tif"] )
	
	def testPrefix( self ) :
	
		s = FileSequence( "myPrefix.##.mySuffix", FrameRange( 0, 1 ) )
		self.assertEqual( s.getPrefix(), "myPrefix." )
		self.assertEqual( s.getSuffix(), ".mySuffix" )
		
		s = FileSequence( "#", FrameRange( 0, 1 ) )
		self.assertEqual( s.getPrefix(), "" )
		self.assertEqual( s.getSuffix(), "" )
		
		s = FileSequence( "abc.#.tif", FrameRange( 0, 1 ) )
		self.assertEqual( s.getPrefix(), "abc." )
		self.assertEqual( s.getSuffix(), ".tif" )
		self.assertEqual( s.getPadding(), 1 )
		s.setPrefix( "def." )
		self.assertEqual( s.getPrefix(), "def." )
		self.assertEqual( s.getSuffix(), ".tif" )
		self.assertEqual( s.getPadding(), 1 )
		s.setSuffix( ".gif" )
		self.assertEqual( s.getPrefix(), "def." )
		self.assertEqual( s.getSuffix(), ".gif" )
		self.assertEqual( s.getPadding(), 1 )
		
	def testCopy( self ) :
	
		s = FileSequence( "seq.#.tif", FrameRange( 0, 2 ) )
		self.assertEqual( s, s.copy() )
		
	def testMapping( self ) :
	
		s = FileSequence( "seq.####.tif", FrameRange( 3, 7, 2 ) )
		s2 = s.copy()
		s2.setPadding( 1 )
		m = s.mapTo( s2 )
		
		self.assertEqual( m, { "seq.0003.tif" : "seq.3.tif", "seq.0005.tif" : "seq.5.tif", "seq.0007.tif" : "seq.7.tif" } )
		
		m = s.mapTo( s2, True )
		self.assertEqual( m, [ ("seq.0003.tif", "seq.3.tif" ), ("seq.0005.tif", "seq.5.tif"), ("seq.0007.tif", "seq.7.tif") ] ) 

	def testClumping( self ) :
	
		s = FileSequence( "a.#.tif", FrameRange( 1, 5 ) )
		self.assertEqual( s.clumpedFileNames( 2 ), [ ["a.1.tif", "a.2.tif"], ["a.3.tif", "a.4.tif"], ["a.5.tif"] ] )

	def testFileNameForFrame( self ) :
	
		s = FileSequence( "a.#.tif", FrameRange( 1, 5 ) )
		self.assertEqual( s.fileNameForFrame( 10 ), "a.10.tif" )
		self.assertEqual( s.fileNameForFrame( 2 ), "a.2.tif" )
		self.assertEqual( s.fileNameForFrame( 201 ), "a.201.tif" )
		
	def testFrameForFileName( self ) :
	
		s = FileSequence( "a.#.tif", FrameRange( 1, 100 ) )
		
		self.assertEqual( s.frameForFileName( "a.10.tif" ), 10 )
		self.assertEqual( s.frameForFileName( "a.100.tif" ), 100 )
		
		self.assertRaises( RuntimeError, s.frameForFileName, "aaa.10.tif" )
		self.assertRaises( RuntimeError, s.frameForFileName, "a.10.exr" )
		self.assertRaises( RuntimeError, s.frameForFileName, "a..exr" )
		
	def testFormatStringInFilename( self ) :
	
		s = FileSequence( "a%20ctest.#.tif", FrameRange( 10000, 10001 ) )
		
		self.assertEqual( s.fileNames(), [ "a%20ctest.10000.tif", "a%20ctest.10001.tif" ] )
		
	def testSpacesInFilename( self ) :
	
		s = FileSequence( "space test.#.tif", FrameRange( 10000, 10001 ) )
		
		self.assertEqual( s.fileNames(), [ "space test.10000.tif", "space test.10001.tif" ] )	
		
		s = FileSequence( "spaces  test .#.tif", FrameRange( 10000, 10001 ) )
		
		self.assertEqual( s.fileNames(), [ "spaces  test .10000.tif", "spaces  test .10001.tif" ] )	
		
class testCompoundFrameList( unittest.TestCase ) :

	def testConstructor( self ) :
	
		self.assertRaises( TypeError, CompoundFrameList, "" )
		self.assertRaises( TypeError, CompoundFrameList, [ "" ] )
		
		c = CompoundFrameList()
		c = CompoundFrameList([])
		
		c = CompoundFrameList( [ FrameRange( 0, 2 ), FrameRange( 4, 6 ) ] )
		self.assertEqual( c.asList(), [ 0, 1, 2, 4, 5, 6 ] )
		
		c = CompoundFrameList( [ FrameRange( 2, 4 ), FrameRange( 3, 6 ) ] )
		self.assertEqual( c.asList(), [ 2, 3, 4, 5, 6 ] )
		
	def testParser( self ) :
	
		c = FrameList.parse( "1-2, 5, 6, 9-11, 100" )
		self.assert_( isinstance( c, CompoundFrameList ) )
		self.assertEqual( c.asList(), [1,2,5,6,9,10,11,100] )
			
		
class testLs( unittest.TestCase ) :

	def doSequences( self, sequences ) :
	
		os.system( "rm -rf test/sequences/lsTest" )
		os.system( "mkdir -p test/sequences/lsTest" )
		
		for sequence in sequences :	
			for f in sequence.fileNames() :
				os.system( "touch test/sequences/lsTest/" + f )
		
		l = ls( "test/sequences/lsTest" )
				
		self.assertEqual( len( sequences ), len( l ) )	
		for sequence in sequences :
			lsFoundSequence = False
			for ll in l :
				if ll==sequence :
					lsFoundSequence = True
					break
					
			self.assert_( lsFoundSequence )
				
		os.system( "rm -rf test/sequences/lsTest" )
		
	def testSimple( self ) :
		
		self.doSequences( [FileSequence( "seq2.####.tif", FrameRange( 0, 100 ) )] )
		
	def testNegativeFrames( self ) :
	
		self.doSequences( [FileSequence( "seq2.####.tif", FrameRange( -100, 100 ) )] )
		self.doSequences( [FileSequence( "seq2.####.tif", FrameRange( -20, -10 ) )] )
		
	def testNoSuffix( self ) :
	
		self.doSequences( [FileSequence( "s.#", FrameRange( 0, 100 ) )] )
		
	def testNoPrefix( self ) :
	
		self.doSequences( [FileSequence( "#.blah", FrameRange( 50, 100 ) )] )
		
	def testStupidMixOfPadding( self ) :
	
		s1 = FileSequence( "s.###.tif", FrameRange( 0, 100 ) )
		s2 = FileSequence( "s.####.tif", FrameRange( 0, 1000 ) )

		self.doSequences( [s1, s2] )
		
	def testMultipleSequences( self ) :
	
		s = []
		s.append( FileSequence( "a.#.b", FrameRange( 0, 100 ) ) )
		s.append( FileSequence( "b.####.b", FrameRange( 0, 10 ) ) )
		s.append( FileSequence( "c####.b", FrameRange( -100, 100 ) ) )
		s.append( FileSequence( "d####", FrameRange( -100, -90 ) ) )
		s.append( FileSequence( "x#.tif", FrameRange( 10, 20 ) ) )
		s.append( FileSequence( "y.######.tif", FrameRange( 100, 200 ) ) )
		
		self.doSequences( s )
		
	def testNumberedSequences( self ) :
	
		s = []
		s.append( FileSequence( "a2.#.b", FrameRange( 0, 100 ) ) )
		s.append( FileSequence( "b2.####.b", FrameRange( 0, 10 ) ) )
		s.append( FileSequence( "c-2.####.b", FrameRange( 0, 10 ) ) )
		
	def testSpecificSequences( self ) :
	
		os.system( "rm -rf test/sequences/lsTest" )
		os.system( "mkdir -p test/sequences/lsTest" )
		
		s1 = FileSequence( "test/sequences/lsTest/a.####.tif", FrameRange( 0, 100 ) )
		s2 = FileSequence( "test/sequences/lsTest/a.###.tif", FrameRange( 0, 100 ) )
		s3 = FileSequence( "test/sequences/lsTest/b.####.tif", FrameRange( 0, 100 ) )
		
		for sequence in [s1, s2, s3] :	
			for f in sequence.fileNames() :
				os.system( "touch " + f )
		
		l = ls( "test/sequences/lsTest/a.####.tif" )
		self.assertEqual( s1, l )
		
		l = ls( "test/sequences/lsTest/a.###.tif" )
		self.assertEqual( s2, l )
		
		l = ls( "test/sequences/lsTest/b.####.tif" )
		self.assertEqual( s3, l )
		
		self.assertEqual( ls( "test/sequences/lsTest/c.####.tif" ), None )
						
		os.system( "rm -rf test/sequences/lsTest" )
		
	def testMinLength( self ) :
	
		"""Verify that single files on their own aren't treated as sequences even
		if the name matches the something.#.ext pattern."""
	
		l = findSequences( [ "a.0001.tif", "b.0010.gif", "b.0011.gif" ] )
		self.assertEqual( len( l ), 1 )
		self.assertEqual( l[0], FileSequence( "b.####.gif", FrameRange( 10, 11 ) ) )				
		
	def testErrors( self ):	
	
		os.system( "rm -rf test/sequences/lsTest" )
		os.system( "mkdir -p test/sequences/lsTest" )		
		
		os.system( "touch test/sequences/lsTest/test100.#.tif.tmp" )
		l = ls( "test/sequences/lsTest" )		
		self.assertEqual( len( l ), 0 )
			
		
class testMv( unittest.TestCase ) :

	def test( self ) :
	
		os.system( "rm -rf test/sequences/mvTest" )
		os.system( "mkdir -p test/sequences/mvTest" )
		s = FileSequence( "test/sequences/mvTest/s.####.tif", FrameRange( 0, 100 ) )
		for f in s.fileNames() :
			os.system( "touch " + f )
			
		s2 = FileSequence( "test/sequences/mvTest/s2.####.tif", FrameRange( 100, 200 ) )
		mv( s, s2 )
		l = ls( "test/sequences/mvTest" )
		self.assertEqual( len( l ), 1 )
		self.assertEqual( l[0], FileSequence( "s2.####.tif", FrameRange( 100, 200 ) ) )

		os.system( "rm -rf test/sequences/mvTest" )
		
	def testOverlapping( self ) :
	
		os.system( "rm -rf test/sequences/mvTest" )
		os.system( "mkdir -p test/sequences/mvTest" )
		s = FileSequence( "test/sequences/mvTest/s.####.tif", FrameRange( 0, 100 ) )
		for f in s.fileNames() :
			os.system( "touch " + f )
			
		s2 = FileSequence( "test/sequences/mvTest/s.####.tif", FrameRange( 50, 150 ) )
		mv( s, s2 )
		l = ls( "test/sequences/mvTest" )
		self.assertEqual( len( l ), 1 )
		self.assertEqual( l[0], FileSequence( "s.####.tif", FrameRange( 50, 150 ) ) )

		os.system( "rm -rf test/sequences/mvTest" )
		
class testCp( unittest.TestCase ) :

	def testNoOverlapping( self ) :
	
		s = FileSequence( "s.####.tif", FrameRange( 0, 100 ) )
		s2 = FileSequence( "s.####.tif", FrameRange( 50, 150 ) )
		self.assertRaises( RuntimeError, cp, s, s2 )
		
	def test( self ) :
	
		os.system( "rm -rf test/sequences/cpTest" )
		os.system( "mkdir -p test/sequences/cpTest" )
		
		s = FileSequence( "test/sequences/cpTest/s.####.tif", FrameRange( 0, 100 ) )
		for f in s.fileNames() :
			os.system( "touch " + f )
		
		s2 = FileSequence( "test/sequences/cpTest/t.####.tif", FrameRange( 50, 150 ) )
		
		cp( s, s2 )
		rm( s )
		
		l = ls( "test/sequences/cpTest" )
		self.assertEqual( len( l ), 1 )
		self.assertEqual( l[0], FileSequence( "t.####.tif", FrameRange( 50, 150 ) ) )
		
		os.system( "rm -rf test/sequences/cpTest" )

class testBigNumbers( unittest.TestCase ) :

	def test( self ) :
	
		s = FileSequence( "s.####.tif", FrameRange( 10000000000, 10000000001 ) )
		
	def testRenumber( self ) :
	
		startFrame = 300010321
		s = FileSequence( "test/IECore/sequences/renumberTest/s.#.tif", FrameRange( startFrame, startFrame + 4 ) )
		os.system( "mkdir -p test/IECore/sequences/renumberTest" )
		
		for f in s.fileNames() :
			os.system( "touch " + f )
		
		offset = -300010000
		SequenceRenumberOp()( src="test/IECore/sequences/renumberTest/s.#.tif", offset=offset )
		
		s2 = ls( "test/IECore/sequences/renumberTest" )
		self.assertEqual( len( s2 ), 1 )
		s2 = s2[0]
		
		self.assertEqual( s2.frameList.asList(), range( startFrame + offset, startFrame + offset + 5 ) )
				
	def tearDown( self ) :
	
		if os.path.exists( "test/IECore/sequences/renumberTest" ) :
			shutil.rmtree( "test/IECore/sequences/renumberTest" )

class testMissingFrames( unittest.TestCase ) :

	def test( self ) :
	
		s = frameListFromList( [ 1, 3, 4, 5, 6, 7 ] )
		self.assertEqual( len( s.frameLists ), 2 )
				
if __name__ == "__main__":
	unittest.main()
