##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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


class TestIndexedIOAlgo( unittest.TestCase ) :

	def remove( self, paths ) :
		for path in paths :
			if os.path.isfile( path ) :
				os.remove( path )

	def setUp( self ) :
		self.remove( [os.path.join( ".", "test", "FileIndexedIO.fio" ), os.path.join( ".", "test", "FileIndexedIO2.fio" )] )

	def tearDown( self ) :
		self.remove( [os.path.join( ".", "test", "FileIndexedIO.fio" ), os.path.join( ".", "test", "FileIndexedIO2.fio" )] )

	def makeTestFile( self ) :
		f = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Write )
		f = f.subdirectory( "sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = IECore.FloatVectorData()

		for n in range( 0, 1024 ) :
			fv.append( n % 3 )

		name = "myFloatVector"
		f.write( name, fv )

		del f

		return fv

	def makeManyDirectoryTestFile( self ) :
		f = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Write )

		for d in range( 512 ) :
			subdir = f.subdirectory( "sub_{0}".format( d ), IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

			fv = IECore.FloatVectorData()

			for n in range( d ) :
				fv.append( n % 3 )

			name = "myFloatVector"
			subdir.write( name, fv )

		del f


	def testCanCopyFile( self ) :
		fv = self.makeTestFile()
		src = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Read )
		dst = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO2.fio" ), [], IECore.IndexedIO.OpenMode.Write )
		IECore.IndexedIOAlgo.copy( src, dst )

		del src
		del dst

		src = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO2.fio" ), [], IECore.IndexedIO.OpenMode.Read )

		sub1 = src.subdirectory( "sub1" )

		name = "myFloatVector"
		gv = sub1.read( name )

		self.assertEqual( len( fv ), len( gv ) )

		for n in range( 0, 1000 ) :
			self.assertEqual( fv[n], gv[n] )

	def testCanReadFileStats( self ) :
		self.makeTestFile()
		src = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Read )

		# todo
		# set thread count to 1

		copyStats = IECore.IndexedIOAlgo.parallelReadAll( src )

		self.assertEqual( copyStats[0], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1] )
		self.assertEqual( copyStats[1], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4096] )

		self.makeManyDirectoryTestFile()
		src2 = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Read )

		# set thread count to 1
		with IECore.tbb_task_scheduler_init( 1 ) as taskScheduler:
			copyStats1thread = IECore.IndexedIOAlgo.parallelReadAll( src2 )

		# set thread count to 8
		with IECore.tbb_task_scheduler_init( 8 ) as taskScheduler:
			copyStats8threads = IECore.IndexedIOAlgo.parallelReadAll( src2 )

		self.assertEqual( copyStats1thread, copyStats8threads )

		# 512 directories each containingn an increasing number of floats
		# sub_0 : 0 * sizeof(4) = 1 block  = 0 bytes  [1,0]
		#											  [0, 0] no blocks > 1 byte <= 2 bytes
		# sub_1 : 1 * sizeof(4) = 1 block  = 4 bytes  [1,4]
		# sub_2 : 2 * sizeof(4) = 2 blocks = 8 bytes  [1, 8]
		# sub_3 : 3 * sizeof(4) = 3 blocks = 12 bytes
		# sub_4 : 4 * sizeof(4) = 4 blocks = 16 bytes [2, 28]

		self.assertEqual( copyStats1thread[0], [1, 0, 1, 1, 2, 4, 8, 16, 32, 64, 128, 255] )
		self.assertEqual( copyStats1thread[1], [0, 0, 4, 8, 28, 104, 400, 1568, 6208, 24704, 98560, 391680] )

	def testCopyFileWithVariousTypes( self ) :

		f = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Write )
		s = f.subdirectory( "sub", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		s.write( "intS", 1 )
		s.write( "intA", IECore.IntVectorData( [0, 1, 2] ) )

		s.write( "floatS", 1.0 )
		s.write( "floatA", IECore.DoubleVectorData( [0.0, 1.0, 2.0] ) )

		s.write( "stringS", "foo" )
		s.write( "stringA", IECore.StringVectorData( ["foo_0", "foo_1", "foo_2"] ) )

		del s, f

		src = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Read )
		dst = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO2.fio" ), [], IECore.IndexedIO.OpenMode.Write )

		IECore.IndexedIOAlgo.copy( src, dst )

		del src, dst

		f = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO2.fio" ), [], IECore.IndexedIO.OpenMode.Read )
		s = f.subdirectory( "sub", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		self.assertEqual( s.read( "intS" ), IECore.IntData( 1 ) )
		self.assertEqual( s.read( "intA" ), IECore.IntVectorData( [0, 1, 2] ) )
		self.assertEqual( s.read( "floatS" ), IECore.DoubleData( 1.0 ) )
		self.assertEqual( s.read( "floatA" ), IECore.DoubleVectorData( [0.0, 1.0, 2.0] ) )
		self.assertEqual( s.read( "stringS" ), IECore.StringData( "foo" ) )
		self.assertEqual( s.read( "stringA" ), IECore.StringVectorData( ["foo_0", "foo_1", "foo_2"] ) )

	def testStringFileStats( self ) :
		f = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Write )
		s = f.subdirectory( "sub", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		s.write( "stringS", "123456789" )
		s.write( "stringA", IECore.StringVectorData( ["123", "123456", "123456789"] ) )

		del s, f

		src = IECore.FileIndexedIO( os.path.join( ".", "test", "FileIndexedIO.fio" ), [], IECore.IndexedIO.OpenMode.Read )
		stats = IECore.IndexedIOAlgo.parallelReadAll( src )

		self.assertEqual( stats[0], [0, 0, 0, 0, 1, 1] )
		self.assertEqual( stats[1], [0, 0, 0, 0, 9, 18] )

if __name__ == "__main__" :
	unittest.main()
