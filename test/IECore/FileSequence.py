##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
import IECore

# \todo use setUp to create directories

class testFrameRange( unittest.TestCase ) :

	def testConstructor( self ) :

		self.assertRaises( TypeError, IECore.FrameRange, 1, 100.1 )
		self.assertRaises( TypeError, IECore.FrameRange, "1", "100" )
		self.assertRaises( RuntimeError, IECore.FrameRange, 10, 1 )

		r = IECore.FrameRange( 1, 100 )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 100 )
		self.assertEqual( r.step, 1 )
		self.assertEqual( str(r), "1-100" )

		r = IECore.FrameRange( 2, 200, 2 )
		self.assertEqual( r.start, 2 )
		self.assertEqual( r.end, 200 )
		self.assertEqual( r.step, 2 )
		self.assertEqual( str(r), "2-200x2" )

		r = IECore.FrameRange( 3, 3 )
		self.assertEqual( r.start, r.end )
		self.assertEqual( r.start, 3 )
		self.assertEqual( str(r), "3" )

	def testRepr( self ) :
		import IECore
		r = IECore.FrameRange( 2, 200, 2 )
		self.assertEqual( r, eval( repr( r ) ) )

	def testParser( self ) :

		r = IECore.FrameList.parse( "1" )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 1 )
		self.assertEqual( r.step, 1 )

		r = IECore.FrameList.parse( "-1" )
		self.assertEqual( r.start, -1 )
		self.assertEqual( r.end, -1 )
		self.assertEqual( r.step, 1 )

		r = IECore.FrameList.parse( "1-100" )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 100 )
		self.assertEqual( r.step, 1 )

		r = IECore.FrameList.parse( "1-11x2" )
		self.assertEqual( r.start, 1 )
		self.assertEqual( r.end, 11 )
		self.assertEqual( r.step, 2 )

		r = IECore.FrameList.parse( "-100--10x2" )
		self.assertEqual( r.start, -100 )
		self.assertEqual( r.end, -10 )
		self.assertEqual( r.step, 2 )

	def testImperfectMultiples( self ) :

		"""It was getting a bit annoying being forced to make the ranges a perfect multiple of the step."""

		r = IECore.FrameRange( 1, 10, 2 )
		self.assertEqual( r.asList(), [ 1, 3, 5, 7, 9 ] )
		self.assertEqual( str( r ), "1-10x2" )

		r = IECore.FrameList.parse( "1-100x10" )
		self.assertEqual( r, IECore.FrameRange( 1, 100, 10 ) )
		self.assertEqual( r.asList(), [ 1, 11, 21, 31, 41, 51, 61, 71, 81, 91 ] )
		self.assertEqual( str( r ), "1-100x10" )

	def testClumping( self ) :

		r = IECore.FrameRange( 1, 32 )
		c = r.asClumpedList( 10 )
		self.assertEqual( c, [ [ 1, 2, 3, 4, 5, 6, 7 ,8, 9, 10 ], [ 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 ], [ 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 ], [ 31, 32 ] ] )

class testFileSequence( unittest.TestCase ) :


	def testConstructor( self ) :

		self.assertRaises( RuntimeError, IECore.FileSequence, "", IECore.FrameRange( 0, 1 ) )

		s = IECore.FileSequence( "seq.#.tif", IECore.FrameRange( 0, 2 ) )
		self.assertEqual( s.fileNames(), ["seq.0.tif", "seq.1.tif", "seq.2.tif"] )

	def testFrameListConstructor( self ):

		s = IECore.FileSequence( "seq.#.tif 0-2" )
		self.assertEqual( s.fileNames(), ["seq.0.tif", "seq.1.tif", "seq.2.tif"] )

		s = IECore.FileSequence( "with space.#.tif 5-6" )
		self.assertEqual( s.fileNames(), ["with space.5.tif", "with space.6.tif"] )

		s = IECore.FileSequence( "seq.#.tif" )
		self.assertEqual( s.fileNames(), [] )

	def testPadding( self ) :

		s = IECore.FileSequence( "seq.#.tif", IECore.FrameRange( 0, 2 ) )
		self.assertEqual( s.getPadding(), 1 )
		s.setPadding( 4 )
		self.assertEqual( s.getPadding(), 4 )
		self.assertEqual( s.fileName, "seq.####.tif" )

		self.assertEqual( s.fileNames(), ["seq.0000.tif", "seq.0001.tif", "seq.0002.tif"] )

	def testRepr( self ) :
		import IECore
		s = IECore.FileSequence( "seq.#.tif", IECore.FrameRange( 0, 2 ) )
		self.assertEqual( s, eval( repr( s ) ) )

	def testPrefix( self ) :

		s = IECore.FileSequence( "myPrefix.##.mySuffix", IECore.FrameRange( 0, 1 ) )
		self.assertEqual( s.getPrefix(), "myPrefix." )
		self.assertEqual( s.getSuffix(), ".mySuffix" )

		s = IECore.FileSequence( "#", IECore.FrameRange( 0, 1 ) )
		self.assertEqual( s.getPrefix(), "" )
		self.assertEqual( s.getSuffix(), "" )

		s = IECore.FileSequence( "abc.#.tif", IECore.FrameRange( 0, 1 ) )
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

		s = IECore.FileSequence( "seq.#.tif", IECore.FrameRange( 0, 2 ) )
		self.assertEqual( s, s.copy() )

	def testMapping( self ) :

		s = IECore.FileSequence( "seq.####.tif", IECore.FrameRange( 3, 7, 2 ) )
		s2 = s.copy()
		s2.setPadding( 1 )
		m = s.mapTo( s2 )

		self.assertEqual( m, { "seq.0003.tif" : "seq.3.tif", "seq.0005.tif" : "seq.5.tif", "seq.0007.tif" : "seq.7.tif" } )

		m = s.mapTo( s2, True )
		self.assertEqual( m, [ ("seq.0003.tif", "seq.3.tif" ), ("seq.0005.tif", "seq.5.tif"), ("seq.0007.tif", "seq.7.tif") ] )

	def testClumping( self ) :

		s = IECore.FileSequence( "a.#.tif", IECore.FrameRange( 1, 5 ) )
		self.assertEqual( s.clumpedFileNames( 2 ), [ ["a.1.tif", "a.2.tif"], ["a.3.tif", "a.4.tif"], ["a.5.tif"] ] )

	def testFileNameForFrame( self ) :

		s = IECore.FileSequence( "a.#.tif", IECore.FrameRange( 1, 5 ) )
		self.assertEqual( s.fileNameForFrame( 10 ), "a.10.tif" )
		self.assertEqual( s.fileNameForFrame( 2 ), "a.2.tif" )
		self.assertEqual( s.fileNameForFrame( 201 ), "a.201.tif" )

	def testFrameForFileName( self ) :

		s = IECore.FileSequence( "a.#.tif", IECore.FrameRange( 1, 100 ) )

		self.assertEqual( s.frameForFileName( "a.10.tif" ), 10 )
		self.assertEqual( s.frameForFileName( "a.100.tif" ), 100 )

		self.assertRaises( RuntimeError, s.frameForFileName, "aaa.10.tif" )
		self.assertRaises( RuntimeError, s.frameForFileName, "a.10.exr" )
		self.assertRaises( RuntimeError, s.frameForFileName, "a..exr" )

	def testFormatStringInFilename( self ) :

		s = IECore.FileSequence( "a%20ctest.#.tif", IECore.FrameRange( 10000, 10001 ) )

		self.assertEqual( s.fileNames(), [ "a%20ctest.10000.tif", "a%20ctest.10001.tif" ] )

	def testSpacesInFilename( self ) :

		s = IECore.FileSequence( "space test.#.tif", IECore.FrameRange( 10000, 10001 ) )

		self.assertEqual( s.fileNames(), [ "space test.10000.tif", "space test.10001.tif" ] )

		s = IECore.FileSequence( "spaces  test .#.tif", IECore.FrameRange( 10000, 10001 ) )

		self.assertEqual( s.fileNames(), [ "spaces  test .10000.tif", "spaces  test .10001.tif" ] )

class testCompoundFrameList( unittest.TestCase ) :

	def testConstructor( self ) :

		self.assertRaises( TypeError, IECore.CompoundFrameList, "" )
		self.assertRaises( RuntimeError, IECore.CompoundFrameList, [ "" ] )

		c = IECore.CompoundFrameList()
		c = IECore.CompoundFrameList([])

		c = IECore.CompoundFrameList( [ IECore.FrameRange( 0, 2 ), IECore.FrameRange( 4, 6 ) ] )
		self.assertEqual( c.asList(), [ 0, 1, 2, 4, 5, 6 ] )

		c = IECore.CompoundFrameList( [ IECore.FrameRange( 2, 4 ), IECore.FrameRange( 3, 6 ) ] )
		self.assertEqual( c.asList(), [ 2, 3, 4, 5, 6 ] )

	def testParser( self ) :

		c = IECore.FrameList.parse( "1-2, 5, 6, 9-11, 100" )
		self.assertTrue( isinstance( c, IECore.CompoundFrameList ) )
		self.assertEqual( c.asList(), [1,2,5,6,9,10,11,100] )

	def testRepr( self ) :

		import IECore
		c = IECore.CompoundFrameList( [ IECore.FrameRange( 2, 4 ), IECore.FrameRange( 3, 6 ) ] )
		self.assertEqual( c, eval( repr( c ) ) )


class testLs( unittest.TestCase ) :

	def doSequences( self, sequences ) :

		self.tearDown()
		os.system( "mkdir -p test/sequences/lsTest" )

		for sequence in sequences :
			for f in sequence.fileNames() :
				os.system( "touch 'test/sequences/lsTest/" + f + "'" )

		l = IECore.ls( "test/sequences/lsTest" )

		self.assertEqual( len( sequences ), len( l ) )
		for sequence in sequences :
			lsFoundSequence = False
			for ll in l :
				if ll==sequence :
					lsFoundSequence = True
					break

			self.assertTrue( lsFoundSequence )

	def testSimple( self ) :

		self.doSequences( [IECore.FileSequence( "seq2.####.tif", IECore.FrameRange( 0, 100 ) )] )

	def testNegativeFrames( self ) :

		self.doSequences( [IECore.FileSequence( "seq2.####.tif", IECore.FrameRange( -100, 100 ) )] )
		self.doSequences( [IECore.FileSequence( "seq2.####.tif", IECore.FrameRange( -20, -10 ) )] )

	def testNoSuffix( self ) :

		self.doSequences( [IECore.FileSequence( "s.#", IECore.FrameRange( 0, 100 ) )] )

	def testNoPrefix( self ) :

		self.doSequences( [IECore.FileSequence( "#.blah", IECore.FrameRange( 50, 100 ) )] )

	def testStupidMixOfPadding( self ) :

		s1 = IECore.FileSequence( "s.###.tif", IECore.FrameRange( 0, 100 ) )
		s2 = IECore.FileSequence( "s.####.tif", IECore.FrameRange( 0, 1000 ) )

		self.doSequences( [s1, s2] )

	def testMultipleSequences( self ) :

		s = []
		s.append( IECore.FileSequence( "a.#.b", IECore.FrameRange( 0, 100 ) ) )
		s.append( IECore.FileSequence( "b.####.b", IECore.FrameRange( 0, 10 ) ) )
		s.append( IECore.FileSequence( "c####.b", IECore.FrameRange( -100, 100 ) ) )
		s.append( IECore.FileSequence( "d####", IECore.FrameRange( -100, -90 ) ) )
		s.append( IECore.FileSequence( "x#.tif", IECore.FrameRange( 10, 20 ) ) )
		s.append( IECore.FileSequence( "y.######.tif", IECore.FrameRange( 100, 200 ) ) )

		self.doSequences( s )

	def testUnorderedSequences( self ) :

		self.tearDown()
		os.system( "mkdir -p test/sequences/lsTest" )

		s = IECore.FileSequence( "a.###.b", IECore.FrameRange.parse( '100-110, 1-10' ) )
		fileNames = s.fileNames()

		for f in fileNames :
			os.system( "touch 'test/sequences/lsTest/" + f + "'" )

		l = IECore.ls( "test/sequences/lsTest" )

		self.assertEqual( len( l ), 1 )

		fileNames.sort()
		lFileNames = l[0].fileNames()
		lFileNames.sort()

		for f in fileNames :
			i = fileNames.index( f )
			self.assertEqual( f, lFileNames[i] )

	def testNumberedSequences( self ) :

		s = []
		s.append( IECore.FileSequence( "a2.#.b", IECore.FrameRange( 0, 100 ) ) )
		s.append( IECore.FileSequence( "b2.####.b", IECore.FrameRange( 0, 10 ) ) )
		s.append( IECore.FileSequence( "c-2.####.b", IECore.FrameRange( 0, 10 ) ) )

	def testSpecificSequences( self ) :

		self.tearDown()
		os.system( "mkdir -p test/sequences/lsTest" )

		s1 = IECore.FileSequence( "test/sequences/lsTest/a.####.tif", IECore.FrameRange( 0, 100 ) )
		s2 = IECore.FileSequence( "test/sequences/lsTest/a.###.tif", IECore.FrameRange( 0, 100 ) )
		s3 = IECore.FileSequence( "test/sequences/lsTest/b.####.tif", IECore.FrameRange( 0, 100 ) )

		for sequence in [s1, s2, s3] :
			for f in sequence.fileNames() :
				os.system( "touch '" + f + "'" )

		l = IECore.ls( "test/sequences/lsTest/a.####.tif" )
		self.assertEqual( s1, l )

		l = IECore.ls( "test/sequences/lsTest/a.###.tif" )
		self.assertEqual( s2, l )

		l = IECore.ls( "test/sequences/lsTest/b.####.tif" )
		self.assertEqual( s3, l )

		self.assertEqual( IECore.ls( "test/sequences/lsTest/c.####.tif" ), None )

	def testAmbiguousPadding( self ):

		self.tearDown()
		os.system( "mkdir -p test/sequences/lsTest" )

		s1 = IECore.FileSequence( "test/sequences/lsTest/a.#.tif", IECore.FrameRange( 99, 110 ) )
		s2 = IECore.FileSequence( "test/sequences/lsTest/a.##.tif", IECore.FrameRange( 99, 110 ) )

		for f in s1.fileNames() :
			os.system( "touch '" + f + "'" )

		l = IECore.ls( "test/sequences/lsTest/a.#.tif" )
		self.assertEqual( s1, l )

		l = IECore.ls( "test/sequences/lsTest/a.##.tif" )
		self.assertEqual( s2, l )

	def testMinLength( self ) :

		"""Verify that single files on their own aren't treated as sequences even
		if the name matches the something.#.ext pattern."""

		self.tearDown()
		os.system( "mkdir -p test/sequences/lsTest" )

		l = IECore.findSequences( [ "a.0001.tif", "b.0010.gif", "b.0011.gif" ] )
		self.assertEqual( len( l ), 1 )
		self.assertEqual( l[0], IECore.FileSequence( "b.####.gif", IECore.FrameRange( 10, 11 ) ) )

		# test minSequenceSize for findSequences
		l = IECore.findSequences( [ "a.0001.tif", "b.0010.gif", "b.0011.gif", "c.tif" ], 1 )
		self.assertEqual( len( l ), 2 )
		self.assertTrue( IECore.FileSequence( "a.####.tif", IECore.FrameRange( 1, 1 ) ) in l )
		self.assertTrue( IECore.FileSequence( "b.####.gif", IECore.FrameRange( 10, 11 ) ) in l )

		l = IECore.findSequences( [ "a.0001.tif", "b.0010.gif", "b.0011.gif", "b.0012.gif", "c.tif" ], 3 )
		self.assertEqual( len( l ), 1 )
		self.assertTrue( IECore.FileSequence( "b.####.gif", IECore.FrameRange( 10, 12 ) ) in l )

		s1 = IECore.FileSequence( "test/sequences/lsTest/a.#.tif", IECore.FrameRange( 1, 1 ) )
		for f in s1.fileNames() :
			os.system( "touch '" + f + "'" )

		l = IECore.ls( "test/sequences/lsTest/a.#.tif" )
		self.assertEqual( None, l )

		l = IECore.ls( "test/sequences/lsTest/a.#.tif", 1 )
		self.assertEqual( s1, l )

	def testSpecialExtensions( self ):
		l = IECore.findSequences( [ "a.001.cr2", "b.002.cr2", "b.003.cr2" ] )
		self.assertEqual( len( l ), 1 )

	def testErrors( self ):

		self.tearDown()
		os.system( "mkdir -p test/sequences/lsTest" )

		os.system( "touch test/sequences/lsTest/test100.#.tif.tmp" )
		l = IECore.ls( "test/sequences/lsTest" )
		self.assertEqual( len( l ), 0 )

	def testAmbiguousPaddingNonContiguous( self ):

		self.tearDown()
		os.system( "mkdir -p test/sequences/lsTest" )

		s1 = IECore.FileSequence( "test/sequences/lsTest/a.#.tif 98,100,103" )
		s2 = IECore.FileSequence( "test/sequences/lsTest/a.##.tif 98,100,103" )

		for f in s1.fileNames() :
			os.system( "touch '" + f + "'" )

		l = IECore.ls( "test/sequences/lsTest/a.#.tif" )
		self.assertTrue( l )
		self.assertEqual( s1.fileName, l.fileName )
		self.assertEqual( s1.frameList.asList(), l.frameList.asList() )

		l = IECore.ls( "test/sequences/lsTest/a.##.tif" )
		self.assertTrue( l )
		self.assertEqual( s2.fileName, l.fileName )
		self.assertEqual( s2.frameList.asList(), l.frameList.asList() )

		l = IECore.ls( "test/sequences/lsTest/a.###.tif" )
		self.assertFalse( l )

	def tearDown( self ) :

		if os.path.exists( "test/sequences" ) :
			shutil.rmtree( "test/sequences" )

class testMv( unittest.TestCase ) :

	def test( self ) :

		self.tearDown()
		os.system( "mkdir -p test/sequences/mvTest" )
		s = IECore.FileSequence( "test/sequences/mvTest/s.####.tif", IECore.FrameRange( 0, 100 ) )
		for f in s.fileNames() :
			os.system( "touch '" + f + "'" )

		s2 = IECore.FileSequence( "test/sequences/mvTest/s2.####.tif", IECore.FrameRange( 100, 200 ) )
		IECore.mv( s, s2 )
		l = IECore.ls( "test/sequences/mvTest" )
		self.assertEqual( len( l ), 1 )
		self.assertEqual( l[0], IECore.FileSequence( "s2.####.tif", IECore.FrameRange( 100, 200 ) ) )

	def testOverlapping( self ) :

		self.tearDown()
		os.system( "mkdir -p test/sequences/mvTest" )
		s = IECore.FileSequence( "test/sequences/mvTest/s.####.tif", IECore.FrameRange( 0, 100 ) )
		for f in s.fileNames() :
			os.system( "touch '" + f + "'" )

		s2 = IECore.FileSequence( "test/sequences/mvTest/s.####.tif", IECore.FrameRange( 50, 150 ) )
		IECore.mv( s, s2 )
		l = IECore.ls( "test/sequences/mvTest" )
		self.assertEqual( len( l ), 1 )
		self.assertEqual( l[0], IECore.FileSequence( "s.####.tif", IECore.FrameRange( 50, 150 ) ) )

	def tearDown( self ) :

		if os.path.exists( "test/sequences" ) :
			shutil.rmtree( "test/sequences" )

class testCp( unittest.TestCase ) :

	def testNoOverlapping( self ) :

		s = IECore.FileSequence( "s.####.tif", IECore.FrameRange( 0, 100 ) )
		s2 = IECore.FileSequence( "s.####.tif", IECore.FrameRange( 50, 150 ) )
		self.assertRaises( RuntimeError, IECore.cp, s, s2 )

	def test( self ) :

		self.tearDown()
		os.system( "mkdir -p test/sequences/cpTest" )

		s = IECore.FileSequence( "test/sequences/cpTest/s.####.tif", IECore.FrameRange( 0, 100 ) )
		for f in s.fileNames() :
			os.system( "touch '" + f + "'" )

		s2 = IECore.FileSequence( "test/sequences/cpTest/t.####.tif", IECore.FrameRange( 50, 150 ) )

		IECore.cp( s, s2 )
		IECore.rm( s )

		l = IECore.ls( "test/sequences/cpTest" )
		self.assertEqual( len( l ), 1 )
		self.assertEqual( l[0], IECore.FileSequence( "t.####.tif", IECore.FrameRange( 50, 150 ) ) )

	def tearDown( self ) :

		if os.path.exists( "test/sequences" ) :
			shutil.rmtree( "test/sequences" )

class testBigNumbers( unittest.TestCase ) :

	def test( self ) :

		s = IECore.FileSequence( "s.####.tif", IECore.FrameRange( 10000000000, 10000000001 ) )

	def testRenumber( self ) :

		startFrame = 300010321
		s = IECore.FileSequence( "test/IECore/sequences/renumberTest/s.#.tif", IECore.FrameRange( startFrame, startFrame + 4 ) )
		os.system( "mkdir -p test/IECore/sequences/renumberTest" )

		for f in s.fileNames() :
			os.system( "touch '" + f + "'" )

		offset = -300010000
		IECore.SequenceRenumberOp()( src="test/IECore/sequences/renumberTest/s.#.tif", offset=offset )

		s2 = IECore.ls( "test/IECore/sequences/renumberTest" )
		self.assertEqual( len( s2 ), 1 )
		s2 = s2[0]

		self.assertEqual( s2.frameList.asList(), range( startFrame + offset, startFrame + offset + 5 ) )

	def tearDown( self ) :

		if os.path.exists( "test/IECore/sequences" ) :
			shutil.rmtree( "test/IECore/sequences" )

class testMissingFrames( unittest.TestCase ) :

	def test( self ) :

		s = IECore.frameListFromList( [ 1, 3, 4, 5, 6, 7 ] )
		self.assertEqual( len( s.frameLists ), 2 )

if __name__ == "__main__":
	unittest.main()
