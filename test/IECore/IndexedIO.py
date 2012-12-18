##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

from IECore import *

class TestIndexedIO(unittest.TestCase):

	def testConstructors(self):
		"""Test IndexedIO constuctors"""
		self.assertRaises(RuntimeError, IndexedIO )

	def testCreate(self):
		"""Test IndexedIO create"""

		io2 = IndexedIO.create( "test/myFile.fio", "/", IndexedIOOpenMode.Write )
		self.assertRaises(RuntimeError, IndexedIO.create, "myFileWith.invalidExtension", "/", IndexedIOOpenMode.Write )

	def testSupportedExtensions( self ) :

		e = IndexedIO.supportedExtensions()
		self.assert_( "fio" in e )

	def testOpenMode( self ) :

		for f in [ "test/myFile.fio" ] :

			io = IndexedIO.create( f, "/", IndexedIOOpenMode.Write | IndexedIOOpenMode.Exclusive )
			self.assertEqual( io.openMode(), IndexedIOOpenMode.Write | IndexedIOOpenMode.Exclusive )
			del io
			io = IndexedIO.create( f, "/", IndexedIOOpenMode.Read | IndexedIOOpenMode.Exclusive )
			self.assertEqual( io.openMode(), IndexedIOOpenMode.Read | IndexedIOOpenMode.Exclusive )
			del io

	def tearDown(self):

		if os.path.isfile("test/myFile.fio"):
			os.remove("test/myFile.fio")

class TestMemoryIndexedIO(unittest.TestCase):

	def test(self):
		"""Test MemoryIndexedIO read/write operations."""
		f = MemoryIndexedIO( CharVectorData(), "/", IndexedIOOpenMode.Write)
		self.assertEqual( f.path() , [] )
		txt = StringData("test1")
		txt.save( f, "obj1" )
		size1 = len( f.buffer() )
		self.assert_( size1 > 0 )
		txt.save( f, "obj2" )
		size2 = len( f.buffer() )
		self.assert_( size2 > size1 )

		buf = f.buffer()

		f2 = MemoryIndexedIO( buf, "/", IndexedIOOpenMode.Read)
		self.assertEqual( txt, Object.load( f2, "obj1" ) )
		self.assertEqual( txt, Object.load( f2, "obj2" ) )

	def testRmStress(self) :
		"""Test MemoryIndexedIO rm (stress test)"""

		random.seed( 19 )

		dataPresent = set()

		f = MemoryIndexedIO( CharVectorData(), "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("data", IndexedIO.MissingBehavior.CreateIfMissing )
		buf = f.buffer() # Fails under gcc 3.3.4 when no data has been written, as a result of the way in which std::ostringstream::seekp(0) works when the stream is currently empty. Fixed in gcc 3.4.x and later.
		f = None
		f = MemoryIndexedIO(buf, "/", IndexedIOOpenMode.Append)
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
				f = MemoryIndexedIO(buf, "/data", IndexedIOOpenMode.Append)

			entryNames = f.entryIds()

			for i in range( 0, maxSize ) :

				dataName = "data"+str(i)
				if dataName in entryNames :

					self.assert_( i in dataPresent )

				else :

					self.failIf( i in dataPresent )

			self.assertEqual( len(entryNames), len(dataPresent) )

class TestFileIndexedIO(unittest.TestCase):

	badNames = ['*', '!', '&', '^', '@', '#', '$', '(', ')', '<', '+',
		'>', '?', ',', '\', ''', ';', '{', '}', '[',
		']', '=', '`' ]

	def testConstructors(self):
		"""Test FileIndexedIO constuctors"""
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		self.assertEqual( f.path() , [] )

		self.assertRaises( RuntimeError, FileIndexedIO, "./test/FileIndexedIO.fio", "/nonexistantentrypoint", IndexedIOOpenMode.Read)
		f = None
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Read)

	def testEmptyWrite(self):
		"""Test FileIndexedIO empty file writing"""
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		self.assertEqual( f.path() , [] )
		f = None
		self.assert_( os.path.exists( "./test/FileIndexedIO.fio" ) )

	def testResetRoot(self):
		"""Test FileIndexedIO resetRoot"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		g = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )
		g.subdirectory("sub2", IndexedIO.MissingBehavior.CreateIfMissing )

		e = g.entryIds()
		self.assertEqual( len(e), 1 )

		self.assertEqual( e, ["sub2"])
		self.assert_( g.entry('sub2').entryType() == Directory )

	def testMkdir(self):
		"""Test FileIndexedIO mkdir"""
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		g = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )
		self.assertEqual( f.path() , [] )
		self.assertEqual( g.path() , [ 'sub1' ] )

		g = f.subdirectory("sub2", IndexedIO.MissingBehavior.CreateIfMissing )
		self.assertEqual( f.path() , [] )
		self.assertEqual( g.path() , [ 'sub2' ] )

	def testChdir(self):
		"""Test FileIndexedIO chdir"""
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )
		f.subdirectory("sub2", IndexedIO.MissingBehavior.CreateIfMissing )

		g = f.subdirectory("sub1")
		self.assertEqual( g.path(), ["sub1"] )
		self.assertEqual( f.path(), [] )
		g = f.subdirectory("sub2")
		self.assertEqual( g.path(), ["sub2"] )

		e = g.subdirectory("sub2.1", IndexedIO.MissingBehavior.CreateIfMissing )
		self.assertEqual( e.path(), ["sub2","sub2.1"] )
		g = e.parentDirectory()
		self.assertEqual( g.path(), ["sub2"] )
		f = g.parentDirectory()
		self.assertEqual( f.path(), [] )
		g = f.subdirectory("sub2")
		self.assertEqual( g.path(), ["sub2"] )
		e = g.subdirectory("sub2.1")
		self.assertEqual( e.path(), ["sub2","sub2.1"] )

	def testLs(self):
		"""Test FileIndexedIO ls"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)

		f.subdirectory("sub2", IndexedIO.MissingBehavior.CreateIfMissing )
		f = f.subdirectory("sub2")
		f.subdirectory("sub2.1", IndexedIO.MissingBehavior.CreateIfMissing )

		# Filer for files
		e = f.entryIds( File )
		self.assertEqual( len(e), 0 )

		# Filter for directories
		e = f.entryIds( Directory )
		self.assertEqual( len(e), 1 )

		self.assertEqual( e[0], "sub2.1")
		self.assert_( f.entry(e[0]).entryType() == Directory)

	def testRm(self):
		"""Test FileIndexedIO rm"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		g = f.subdirectory("sub2", IndexedIO.MissingBehavior.CreateIfMissing )

		g.subdirectory("sub2.1", IndexedIO.MissingBehavior.CreateIfMissing )
		g.subdirectory("sub2.2", IndexedIO.MissingBehavior.CreateIfMissing )
		g.subdirectory("sub2.3", IndexedIO.MissingBehavior.CreateIfMissing )

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

	def testRmStress(self) :
		"""Test FileIndexedIO rm (stress test)"""

		random.seed( 19 )

		dataPresent = set()

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("data", IndexedIO.MissingBehavior.CreateIfMissing )

		f = None
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Append)
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
				f = FileIndexedIO("./test/FileIndexedIO.fio", "/data", IndexedIOOpenMode.Append)

			entryNames = f.entryIds()

			for i in range( 0, maxSize ) :

				dataName = "data"+str(i)
				if dataName in entryNames :

					self.assert_( i in dataPresent )

				else :

					self.failIf( i in dataPresent )

			self.assertEqual( len(entryNames), len(dataPresent) )



	def testReadWrite(self):
		"""Test FileIndexedIO read/write(generic)"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		self.assertRaises( RuntimeError, f.read, "DOESNOTEXIST")

		# Name check
		for n in self.badNames:
			self.assertRaises(RuntimeError, f.read, n)

	def testReadWriteFloatVector(self):
		"""Test FileIndexedIO read/write(FloatVector)"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )

		fv = FloatVectorData()

		for n in range(0, 1000):
			fv.append(n* n * math.sin(n))

		name = "myFloatVector"
		f.write(name, fv)

		gv = f.read(name)

		self.failIf(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(fv[n], gv[n])

	def testReadWriteDoubleVector(self):
		"""Test FileIndexedIO read/write(DoubleVector)"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )

		fv = DoubleVectorData()

		for n in range(0, 1000):
			fv.append(n* n * math.sin(n))

		name = "myDoubleVector"
		f.write(name, fv)

		gv = f.read(name)

		self.failIf(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(fv[n], gv[n])

	def testReadWriteIntVector(self):
		"""Test FileIndexedIO read/write(IntVector)"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )

		fv = IntVectorData()

		for n in range(0, 1000):
			fv.append(n * n)

		name = "myIntVector"
		f.write(name, fv)

		gv = f.read(name)

		self.failIf(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(fv[n], gv[n])

	def testReadWriteStringVector(self):
		"""Test FileIndexedIO read/write(StringVector)"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )

		fv = StringVectorData()

		for n in range(0, 1000):
			fv.append(str(n))

		name = "myStringVector"
		f.write(name, fv)

		gv = f.read(name)

		self.failIf(fv is gv)
		self.assertEqual(len(fv), len(gv))

		for n in range(0, 1000):
			self.assertEqual(str(fv[n]), str(gv[n]))

	def testReadWriteFloat(self):
		"""Test FileIndexedIO read/write(Float/Double)"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )

		fv = 2.0

		name = "myFloat"
		f.write(name, fv)

		gv = f.read(name).value

		self.failIf(fv is gv)
		self.assertEqual(fv, gv)

	def testReadWriteInt(self):
		"""Test FileIndexedIO read/write(Int/Long)"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )

		fv = 200

		name = "myInt"
		f.write(name, fv)

		gv = f.read(name).value

		self.assertEqual(fv, gv)

	def testReadWriteString(self):
		"""Test FileIndexedIO read/write(String)"""

		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f = f.subdirectory("sub1", IndexedIO.MissingBehavior.CreateIfMissing )

		fv = "StringLiteral"

		name = "myString"
		f.write(name, fv)

		gv = f.read(name).value

		self.failIf(fv is gv)
		self.assertEqual(fv, gv)

	def setUp( self ):

		if os.path.isfile("./test/FileIndexedIO.fio") :
			os.remove("./test/FileIndexedIO.fio")

	def tearDown(self):

		# cleanup
		if os.path.isfile("./test/FileIndexedIO.fio") :
			os.remove("./test/FileIndexedIO.fio")


if __name__ == "__main__":
	unittest.main()

