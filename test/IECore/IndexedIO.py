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


"""Unit test for IndexedIO binding"""
import os
import unittest
import math
import random

import IECore

class TestIndexedIO(unittest.TestCase):

	def testConstructors(self):
		"""Test IndexedIO constuctors"""
		self.assertRaises(RuntimeError, IECore.IndexedIO )

	def testCreate(self):
		"""Test IndexedIO create"""

		io2 = IECore.IndexedIO.create( "test/myFile.fio", [], IECore.IndexedIO.OpenMode.Write )
		io2 = IECore.IndexedIO.create( "test/myFile.fio", IECore.IndexedIO.OpenMode.Write )
		self.assertRaises(RuntimeError, IECore.IndexedIO.create, "myFileWith.invalidExtension", [], IECore.IndexedIO.OpenMode.Write )

	def testSupportedExtensions( self ) :

		e = IECore.IndexedIO.supportedExtensions()
		self.assertTrue( "fio" in e )

	def testOpenMode( self ) :

		for f in [ "test/myFile.fio" ] :

			io = IECore.IndexedIO.create( f, [], IECore.IndexedIO.OpenMode.Write | IECore.IndexedIO.OpenMode.Exclusive )
			self.assertEqual( io.openMode(), IECore.IndexedIO.OpenMode.Write | IECore.IndexedIO.OpenMode.Exclusive )
			del io
			io = IECore.IndexedIO.create( f, [], IECore.IndexedIO.OpenMode.Read | IECore.IndexedIO.OpenMode.Exclusive )
			self.assertEqual( io.openMode(), IECore.IndexedIO.OpenMode.Read | IECore.IndexedIO.OpenMode.Exclusive )
			del io

	def testEntryConstructor( self ) :

		e = IECore.IndexedIO.Entry( "n", IECore.IndexedIO.EntryType.Directory, IECore.IndexedIO.DataType.Invalid, 0 )
		self.assertEqual( e.id(), "n" )
		self.assertEqual( e.entryType(), IECore.IndexedIO.EntryType.Directory )

	def tearDown(self):

		if os.path.isfile("test/myFile.fio"):
			os.remove("test/myFile.fio")

class TestMemoryIndexedIO(unittest.TestCase):

	def testSaveWriteObjects(self):
		"""Test MemoryIndexedIO read/write operations."""
		f = IECore.MemoryIndexedIO( IECore.CharVectorData(), [], IECore.IndexedIO.OpenMode.Write)
		self.assertEqual( f.path() , [] )
		self.assertEqual( f.currentEntryId() , "/" )
		txt = IECore.StringData("test1")
		txt.save( f, "obj1" )
		size1 = len( f.buffer() )
		self.assertTrue( size1 > 0 )
		txt.save( f, "obj2" )
		size2 = len( f.buffer() )
		self.assertTrue( size2 > size1 )

		buf = f.buffer()

		f2 = IECore.MemoryIndexedIO( buf, [], IECore.IndexedIO.OpenMode.Read)
		self.assertEqual( txt, IECore.Object.load( f2, "obj1" ) )
		self.assertEqual( txt, IECore.Object.load( f2, "obj2" ) )

	@unittest.skipUnless( os.environ.get("CORTEX_PERFORMANCE_TEST", False), "'CORTEX_PERFORMANCE_TEST' env var not set" )
	def testRmStress(self) :
		"""Test MemoryIndexedIO rm (stress test)"""

		random.seed( 19 )

		dataPresent = set()

		f = IECore.MemoryIndexedIO( IECore.CharVectorData(), [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("data", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		buf = f.buffer() # Fails under gcc 3.3.4 when no data has been written, as a result of the way in which std::ostringstream::seekp(0) works when the stream is currently empty. Fixed in gcc 3.4.x and later.
		f = None
		f = IECore.MemoryIndexedIO(buf, [], IECore.IndexedIO.OpenMode.Append)
		f = f.subdirectory( "data" )

		numLoops = 500
		maxSize = 1000

		for i in range( 0, numLoops ) :

			for i in range( 0, maxSize ) :

				index = int( random.random() * maxSize )

				if not index in dataPresent :

					f.write( "data"+str(index), i )
					dataPresent.add( index )

				else :

					f.remove( "data"+str(index) )
					dataPresent.remove( index )


			# Reopen the file every now and then, to exercise the index reading/writing
			if random.random() > 0.8 :

				buf = f.buffer()
				f = None
				f = IECore.MemoryIndexedIO(buf, ["data"], IECore.IndexedIO.OpenMode.Append)

			entryNames = f.entryIds()

			for i in range( 0, maxSize ) :

				dataName = "data"+str(i)
				if dataName in entryNames :

					self.assertTrue( i in dataPresent )

				else :

					self.assertFalse( i in dataPresent )

			self.assertEqual( len(entryNames), len(dataPresent) )

class TestFileIndexedIO(unittest.TestCase):

	badNames = ['*', '!', '&', '^', '@', '#', '$', '(', ')', '<', '+',
		'>', '?', ',', '\', ''', ';', '{', '}', '[',
		']', '=', '`' ]

	def testConstructors(self):
		"""Test FileIndexedIO constuctors"""
		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		self.assertEqual( f.fileName() , "./test/FileIndexedIO.fio" )
		self.assertEqual( f.path() , [] )
		self.assertEqual( f.currentEntryId() , "/" )

		self.assertRaises( RuntimeError, IECore.FileIndexedIO, "./test/FileIndexedIO.fio", ["nonexistantentrypoint"], IECore.IndexedIO.OpenMode.Read)
		f = None
		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Read)
		self.assertEqual( f.path() , [] )
		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", IECore.IndexedIO.OpenMode.Read)
		self.assertEqual( f.path() , [] )

	def testEmptyWrite(self):
		"""Test FileIndexedIO empty file writing"""
		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		self.assertEqual( f.path() , [] )
		f = None
		self.assertTrue( os.path.exists( "./test/FileIndexedIO.fio" ) )

	def testSaveWriteObjects(self):
		"""Test FileIndexedIO read/write operations."""
		f = IECore.FileIndexedIO( "./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		self.assertEqual( f.path() , [] )
		self.assertEqual( f.currentEntryId() , "/" )
		txt = IECore.StringData("test1")
		txt.save( f, "obj1" )
		txt.save( f, "obj2" )
		del f

		f2 = IECore.FileIndexedIO( "./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Read)
		self.assertEqual( txt, IECore.Object.load( f2, "obj1" ) )
		self.assertEqual( txt, IECore.Object.load( f2, "obj2" ) )

	def testResetRoot(self):
		"""Test FileIndexedIO resetRoot"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		g.subdirectory("sub2", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		e = g.entryIds()
		self.assertEqual( len(e), 1 )

		self.assertEqual( e, ["sub2"])
		self.assertTrue( g.entry('sub2').entryType() == IECore.IndexedIO.EntryType.Directory )

	def testMkdir(self):
		"""Test FileIndexedIO mkdir"""
		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		self.assertEqual( f.path() , [] )
		self.assertEqual( f.currentEntryId() , "/" )
		self.assertEqual( g.path() , [ 'sub1' ] )
		self.assertEqual( g.currentEntryId() , "sub1" )

		g = f.createSubdirectory("sub2" )
		self.assertEqual( f.path() , [] )
		self.assertEqual( g.path() , [ 'sub2' ] )
		self.assertEqual( g.currentEntryId() , "sub2" )
		self.assertRaises( RuntimeError, f.createSubdirectory, "sub2" )

		# test directory
		h = f.directory(["sub2"], IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		self.assertEqual( h.path() , ["sub2"] )
		i = f.directory(["sub2","sub2.1"], IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		self.assertEqual( i.path() , ["sub2","sub2.1"] )
		j = h.directory(["sub2","sub2.1"], IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		self.assertEqual( j.path() , ["sub2","sub2.1"] )
		k = j.directory(["sub3"], IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		self.assertEqual( k.path() , ["sub3"] )
		l = f.directory(["sub4","sub4.1"], IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		self.assertEqual( l.path() , ["sub4","sub4.1"] )

	def testChdir(self):
		"""Test FileIndexedIO chdir"""
		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		f.subdirectory("sub2", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		g = f.subdirectory("sub1")
		self.assertEqual( g.path(), ["sub1"] )
		self.assertEqual( g.currentEntryId() , "sub1" )
		self.assertEqual( f.path(), [] )
		g = f.subdirectory("sub2")
		self.assertEqual( g.path(), ["sub2"] )
		self.assertEqual( g.currentEntryId() , "sub2" )

		e = g.subdirectory("sub2.1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		self.assertEqual( e.path(), ["sub2","sub2.1"] )
		self.assertEqual( e.currentEntryId() , "sub2.1" )
		g = e.parentDirectory()
		self.assertEqual( g.path(), ["sub2"] )
		self.assertEqual( g.currentEntryId() , "sub2" )
		f = g.parentDirectory()
		self.assertEqual( f.path(), [] )
		self.assertEqual( f.currentEntryId() , "/" )
		g = f.subdirectory("sub2")
		self.assertEqual( g.path(), ["sub2"] )
		self.assertEqual( g.currentEntryId() , "sub2" )
		e = g.subdirectory("sub2.1")
		self.assertEqual( e.path(), ["sub2","sub2.1"] )
		self.assertEqual( e.currentEntryId() , "sub2.1" )

		# test directory function
		h = f.directory( ["sub2","sub2.1"], IECore.IndexedIO.MissingBehaviour.ThrowIfMissing )
		self.assertEqual( h.path(), ["sub2","sub2.1"], IECore.IndexedIO.MissingBehaviour.ThrowIfMissing )
		h = g.directory( ["sub2","sub2.1"], IECore.IndexedIO.MissingBehaviour.ThrowIfMissing )
		self.assertEqual( h.path(), ["sub2","sub2.1"], IECore.IndexedIO.MissingBehaviour.ThrowIfMissing )
		h = e.directory( ["sub2","sub2.1"], IECore.IndexedIO.MissingBehaviour.ThrowIfMissing )
		self.assertEqual( h.path(), ["sub2","sub2.1"], IECore.IndexedIO.MissingBehaviour.ThrowIfMissing )

		# missingBehaviour should default to throw if missing
		h = e.directory( ["sub2","sub2.1"] )
		self.assertEqual( h.path(), ["sub2","sub2.1"] )
		self.assertRaises( RuntimeError, e.directory, [ "i", "dont", "exist" ] )
		self.assertRaises( RuntimeError, e.subdirectory, "idontexist" )
		self.assertEqual( None, e.directory( [ "i", "dont", "exist" ], IECore.IndexedIO.MissingBehaviour.NullIfMissing ) )
		self.assertEqual( None, e.subdirectory( "idontexist", IECore.IndexedIO.MissingBehaviour.NullIfMissing ) )

	def testLs(self):
		"""Test FileIndexedIO ls"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)

		f.subdirectory("sub2", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		f = f.subdirectory("sub2")
		f.subdirectory("sub2.1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		# Filter for files
		e = f.entryIds( IECore.IndexedIO.EntryType.File )
		self.assertEqual( len(e), 0 )

		# Filter for directories
		e = f.entryIds( IECore.IndexedIO.EntryType.Directory )
		self.assertEqual( len(e), 1 )

		self.assertEqual( e[0], "sub2.1")
		self.assertTrue( f.entry(e[0]).entryType() == IECore.IndexedIO.EntryType.Directory)

	def testRm(self):
		"""Test FileIndexedIO rm"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		g = f.subdirectory("sub2", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		g.subdirectory("sub2.1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		g.subdirectory("sub2.2", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		g.subdirectory("sub2.3", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		e = g.entryIds()
		self.assertEqual( len(e), 3 )
		g.remove("sub2.1")
		e = g.entryIds()
		self.assertEqual( len(e), 2 )
		f.subdirectory('sub2').remove("sub2.2")
		e = g.entryIds()
		self.assertEqual( len(e), 1 )
		g.remove("sub2.3")
		e = g.entryIds()
		self.assertEqual( len(e), 0 )
		f.remove("sub2")

	@unittest.skipUnless( os.environ.get("CORTEX_PERFORMANCE_TEST", False), "'CORTEX_PERFORMANCE_TEST' env var not set" )
	def testRmStress(self) :
		"""Test FileIndexedIO rm (stress test)"""

		random.seed( 19 )

		dataPresent = set()

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("data", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		f = None
		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Append)
		f = f.subdirectory( "data" )

		numLoops = 500
		maxSize = 1000

		for i in range( 0, numLoops ) :

			for i in range( 0, maxSize ) :

				index = int( random.random() * maxSize )

				if not index in dataPresent :

					f.write( "data"+str(index), i )
					dataPresent.add( index )

				else :

					f.remove( "data"+str(index) )
					dataPresent.remove( index )


			# Reopen the file every now and then, to exercise the index reading/writing
			if random.random() > 0.8 :

				f = None
				f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", ["data"], IECore.IndexedIO.OpenMode.Append)

			entryNames = f.entryIds()

			for i in range( 0, maxSize ) :

				dataName = "data"+str(i)
				if dataName in entryNames :

					self.assertTrue( i in dataPresent )

				else :

					self.assertFalse( i in dataPresent )

			self.assertEqual( len(entryNames), len(dataPresent) )

	def testReadWrite(self):
		"""Test FileIndexedIO read/write(generic)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		self.assertRaises( RuntimeError, f.read, "DOESNOTEXIST")

		# Name check
		for n in self.badNames:
			self.assertRaises(RuntimeError, f.read, n)

	def testReadWriteFloatVector(self):
		"""Test FileIndexedIO read/write(FloatVector)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = IECore.FloatVectorData()

		for n in range(0, 1000):
			fv.append(n* n * math.sin(n))

		name = "myFloatVector"
		f.write(name, fv)

		gv = f.read(name)

		self.assertFalse(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(fv[n], gv[n])

	def testReadWriteDoubleVector(self):
		"""Test FileIndexedIO read/write(DoubleVector)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = IECore.DoubleVectorData()

		for n in range(0, 1000):
			fv.append(n* n * math.sin(n))

		name = "myDoubleVector"
		f.write(name, fv)

		gv = f.read(name)

		self.assertFalse(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(fv[n], gv[n])

	def testReadWriteIntVector(self):
		"""Test FileIndexedIO read/write(IntVector)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = IECore.IntVectorData()

		for n in range(0, 1000):
			fv.append(n * n)

		name = "myIntVector"
		f.write(name, fv)

		gv = f.read(name)

		self.assertFalse(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(fv[n], gv[n])

	def testReadWriteStringVector(self):
		"""Test FileIndexedIO read/write(StringVector)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = IECore.StringVectorData()

		for n in range(0, 1000):
			fv.append(str(n))

		name = "myStringVector"
		f.write(name, fv)

		gv = f.read(name)

		self.assertFalse(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(str(fv[n]), str(gv[n]))

	def testReadWriteStringVector(self):
		"""Test FileIndexedIO read/write(InternedStringVector)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = IECore.InternedStringVectorData()

		for n in range(0, 1000):
			fv.append(str(n))

		name = "myInternedStringVector"
		f.write(name, fv)

		gv = f.read(name)

		self.assertFalse(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(str(fv[n]), str(gv[n]))

	def testReadWriteFloat(self):
		"""Test FileIndexedIO read/write(Float/Double)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = 2.0

		name = "myFloat"
		f.write(name, fv)

		gv = f.read(name).value

		self.assertFalse(fv is gv)
		self.assertEqual(fv, gv)

	def testReadWriteInt(self):
		"""Test FileIndexedIO read/write(Int/Long)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = 200

		name = "myInt"
		f.write(name, fv)

		gv = f.read(name).value

		self.assertEqual(fv, gv)

	def testReadWriteString(self):
		"""Test FileIndexedIO read/write(String)"""

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		f = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = "StringLiteral"

		name = "myString"
		f.write(name, fv)

		gv = f.read(name).value

		self.assertFalse(fv is gv)
		self.assertEqual(fv, gv)

	def testReadWriteSymbolicLink(self):
		"""Test FileIndexedIO read/write(SymbolicLink)"""

		# There isn't actually an explicit symbolic link capability in IndexedIO,
		# but it's pretty straightforward to emulate it by writing paths
		# into a file.

		f = IECore.FileIndexedIO("./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write)
		g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		h = g.subdirectory("sub2", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		fv = IECore.InternedStringVectorData( h.path() )

		name = "myLink"
		f.write( name, fv )

		gv = f.read(name)

		self.assertFalse(fv is gv)
		self.assertEqual(fv, gv)

	def testIncreasingCompressionLevelResultsInSmallerFile( self ):

		previousSize = None

		filePath = "./test/FileIndexedIO.fio"
		for level in range(9):
			options = IECore.CompoundData( { "compressor" : "lz4", "compressionLevel" : level } )
			f = IECore.IndexedIO.create( filePath, [], IECore.IndexedIO.OpenMode.Write, options = options )
			g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

			for b in range( 512 ):
				g.write( "foo_" + str( b ), IECore.FloatVectorData( [random.random() for i in range( 4096 )] ) )

			del g, f

			size = os.path.getsize( filePath )
			if previousSize:
				self.assertTrue( previousSize > size )

			previousSize = size

	def testCanWriteBlockGreaterThanCompressedBlockSize( self ):

		filePath = "./test/FileIndexedIO.fio"
		# set the compressedBlockSize to 1MB to ensure we're creating multiple blocks
		options = IECore.CompoundData( { "compressor" : "lz4", "compressionLevel" : 9, "maxCompressedBlockSize" : IECore.UIntData( 1024 * 1024 ) } )

		f = IECore.IndexedIO.create( filePath, [], IECore.IndexedIO.OpenMode.Write, options = options )
		g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		# write 80 MB of integers
		d = IECore.IntVectorData( 20 * 1024 * 1024 )

		for i in range( 20 * 1024 * 1024 ):
			d[i] = i

		g.write( "foo", d )

		del g, f

		f = IECore.IndexedIO.create( filePath, [], IECore.IndexedIO.OpenMode.Read )
		g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.ThrowIfMissing)
		d2 = g.read ( "foo" )

		size = os.path.getsize( filePath )
		# harsh test as this is 1/16 of the data size but ensures we've compressed the data
		self.assertTrue( size < 5 * 1024 * 1024 )

		self.assertEqual( d, d2 )

	def testCompressionParametersAndVersionStoredInMetaData( self ):

		options = IECore.CompoundData( { "compressor" : "zlib", "compressionLevel" : 3 } )
		f = IECore.IndexedIO.create( "./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write, options = options )
		g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		g.write( "foo", IECore.IntVectorData( range( 4096 ) ) )
		del g, f

		f = IECore.IndexedIO.create( "./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Read )

		m = f.metadata()

		self.assertEqual(m["compressor"], IECore.StringData("zlib") )
		self.assertEqual(m["compressionLevel"], IECore.IntData(3) )
		self.assertEqual(m["version"], IECore.IntData(7) )

	def testInvalidCompressionParametersRevertsToSensibleDefaults( self ):

		options = IECore.CompoundData( { "compressor" : "foobar", "compressionLevel" : 12,  "compressionThreadCount" : 100, "decompressionThreadCount" : -10 } )

		f = IECore.IndexedIO.create( "./test/FileIndexedIO.fio", [], IECore.IndexedIO.OpenMode.Write, options = options )
		g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )

		self.assertEqual( f.metadata(),
			IECore.CompoundData( { "compressor" : "lz4", "compressionLevel" : 9, 'version': IECore.IntData( 7 ), "compressionThreadCount" : 32, "decompressionThreadCount" : 1 } ) )

	def testDefaultCompressionIsOff( self ):

		filePath = "./test/FileIndexedIO.fio"

		f = IECore.IndexedIO.create( filePath, [], IECore.IndexedIO.OpenMode.Write )
		g = f.subdirectory("sub1", IECore.IndexedIO.MissingBehaviour.CreateIfMissing )
		g.write( "foo", IECore.IntVectorData( range( 1024 ) ) )

		del g, f

		size = os.path.getsize( filePath )

		self.assertTrue( ( size > (1024 * 4 ) ) )
		self.assertTrue( ( size < (1024 * 4 + 512 ) ) )

		f = IECore.IndexedIO.create( filePath, [], IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( f.metadata(),
			IECore.CompoundData( { "compressor" : "lz4", "compressionLevel" : 0, 'version': IECore.IntData( 7 ), "compressionThreadCount" : 1, "decompressionThreadCount" : 1 } ) )

	def setUp( self ):

		if os.path.isfile("./test/FileIndexedIO.fio") :
			os.remove("./test/FileIndexedIO.fio")

	def tearDown(self):

		# cleanup
		if os.path.isfile("./test/FileIndexedIO.fio") :
			os.remove("./test/FileIndexedIO.fio")


if __name__ == "__main__":
	unittest.main()

