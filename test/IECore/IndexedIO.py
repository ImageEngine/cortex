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


"""Unit test for IndexedIOInterface binding"""
import os
import unittest
import math
import random

from IECore import *

class TestIndexedIOInterface(unittest.TestCase):

	def testConstructors(self):
		"""Test IndexedIOInterface constuctors"""
		self.assertRaises(RuntimeError, IndexedIOInterface )
		
	def testCreate(self):
		"""Test IndexedIOInterface create"""
		
		io2 = IndexedIOInterface.create( "test/myFile.fs", "/", IndexedIOOpenMode.Write )
		io2 = IndexedIOInterface.create( "test/myFile.fio", "/", IndexedIOOpenMode.Write )		
		self.assertRaises(RuntimeError, IndexedIOInterface.create, "myFileWith.invalidExtension", "/", IndexedIOOpenMode.Write )

	def testSupportedExtensions( self ) :
	
		e = IndexedIOInterface.supportedExtensions()
		self.assert_( "fio" in e )
		self.assert_( "fs" in e )
	
	def testOpenMode( self ) :
	
		for f in [ "test/myFile.fs", "test/myFile.fio" ] :
	
			io = IndexedIOInterface.create( f, "/", IndexedIOOpenMode.Write | IndexedIOOpenMode.Exclusive )
			self.assertEqual( io.openMode(), IndexedIOOpenMode.Write | IndexedIOOpenMode.Exclusive )
			del io
			io = IndexedIOInterface.create( f, "/", IndexedIOOpenMode.Read | IndexedIOOpenMode.Exclusive )
			self.assertEqual( io.openMode(), IndexedIOOpenMode.Read | IndexedIOOpenMode.Exclusive )
			del io
		
	def tearDown(self):
		
		for root, dirs, files in os.walk("test/myFile.fs", topdown=Read):
			
				for name in files:				
					os.remove(os.path.join(root, name))
				for name in dirs:				
					os.rmdir(os.path.join(root, name))
					
		if os.path.isdir("test/myFile.fs"):
			os.rmdir("test/myFile.fs")
						
		if os.path.isfile("test/myFile.fio"):
			os.remove("test/myFile.fio")	

class TestFileSystemIndexedIO(unittest.TestCase):
	
	badNames = ['*', '!', '&', '^', '@', '#', '$', '(', ')', '<', '+',
		'>', '?', ',', '\', ''', ';', '{', '}', '[',
		']', '=', '`' ]
		
	def testConstructors(self):
		"""Test FileSystemIndexedIO constuctors"""
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		self.assertEqual( f.pwd() , "/" )
		
		self.assertRaises( RuntimeError, FileSystemIndexedIO, "./test/FileSystemIndexedIO.fs", "/nonexistantentrypoint", IndexedIOOpenMode.Read)
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Read)

	def testResetRoot(self):
		"""Test FileSystemIndexedIO resetRoot"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub1")
		f.chdir("sub1")
		f.mkdir("sub2")
		              
		g = f.resetRoot()
		g.chdir("/")   
		              
		filter = IndexedIOEntryTypeFilter( IndexedIOEntryType.Directory )
		e = g.ls( filter )
		self.assertEqual( len(e), 1 )
		
		self.assertEqual( e[0].id(), "sub2")
		self.assert_( e[0].entryType() == Directory)				
		
	def testMkdir(self):
		"""Test FileSystemIndexedIO mkdir"""
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub1")
		self.assertEqual( f.pwd() , "/" )
		self.assert_( os.path.isdir("./test/FileSystemIndexedIO.fs/sub1") )
		
		f.mkdir("sub2")
		self.assertEqual( f.pwd() , "/" )
		self.assert_( os.path.isdir("./test/FileSystemIndexedIO.fs/sub2") )
		
	def testPwd(self):
		"""Test FileSystemIndexedIO pwd"""
		pass
		
	def testChdir(self):
		"""Test FileSystemIndexedIO chdir"""
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub1")
		f.mkdir("sub2")
		
		f.chdir("sub1")
		self.assertEqual( f.pwd(), "/sub1" )
		f.chdir("..")
		self.assertEqual( f.pwd(), "/" )
		f.chdir("sub2")
		self.assertEqual( f.pwd(), "/sub2" )
		
		f.mkdir("sub2.1")
		f.chdir("sub2.1")
		self.assertEqual( f.pwd(), "/sub2/sub2.1" )
		f.chdir("..")
		self.assertEqual( f.pwd(), "/sub2" )
		f.chdir(".")
		self.assertEqual( f.pwd(), "/sub2" )
		f.chdir("..")
		self.assertEqual( f.pwd(), "/" )
		f.chdir("sub2")
		self.assertEqual( f.pwd(), "/sub2" )
		f.chdir("/sub2/sub2.1")
		self.assertEqual( f.pwd(), "/sub2/sub2.1" )
		f.chdir("/")
		
		# Try to chdir above root directory
		f.chdir("..")
		self.assertEqual( f.pwd(), "/" )
		
		
		# Try to cd to non-existant directory
		self.assertRaises(RuntimeError, f.chdir, "DOESNOTEXIST")
		
			
		
	def testLs(self):
		"""Test FileSystemIndexedIO ls"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		
		
		f.mkdir("sub2")
		f.chdir("sub2")
		f.mkdir("sub2.1")
		
		# Filer for files
		filter = IndexedIOEntryTypeFilter( IndexedIOEntryType.File )
		e = f.ls( filter )
		self.assertEqual( len(e), 0 )
		
		# Filter for directories
		filter = IndexedIOEntryTypeFilter( IndexedIOEntryType.Directory )
		e = f.ls( filter )
		self.assertEqual( len(e), 1 )
		
		self.assertEqual( e[0].id(), "sub2.1")
		self.assert_( e[0].entryType() == Directory)
		
		f.mkdir("sub2.2")
		
		#No filter
		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 2 )
		
		# No filter via default argument
		g = f.ls( )
		self.assertEqual( len(e), len(g) )

		# regex filter
		filter = IndexedIORegexFilter( ".*2" )
		e = f.ls( filter )
		self.assertEqual( len(e), 1 )

		
	def testRm(self):
		"""Test FileSystemIndexedIO rm"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub2")
		f.chdir("sub2")
		
		f.mkdir("sub2.1");
		f.mkdir("sub2.2");
		f.mkdir("sub2.3");

		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 3 )
		f.rm("sub2.1")		
		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 2 )
		f.rm("sub2.2")		
		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 1 )
		f.rm("sub2.3")		
		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 0 )
		
		f.chdir("..")
		f.rm("sub2")
		
		self.failIf( os.path.isdir("./test/FileSystemIndexedIO.fs/sub2") )
		self.failIf( os.path.isdir("./test/FileSystemIndexedIO.fs/sub2/sub2.1") )
		self.failIf( os.path.isdir("./test/FileSystemIndexedIO.fs/sub2/sub2.2") )
		self.failIf( os.path.isdir("./test/FileSystemIndexedIO.fs/sub2/sub2.3") )
		
	def testReadWrite(self):
		"""Test FileSystemIndexedIO read/write(generic)"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		self.assertRaises( RuntimeError, f.read, "DOESNOTEXIST")
		
		# Name check
		for n in self.badNames:
			self.assertRaises(RuntimeError, f.read, n)

	def testReadWriteFloatVector(self):
		"""Test FileSystemIndexedIO read/write(FloatVector)"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
		"""Test FileSystemIndexedIO read/write(DoubleVector)"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
		"""Test FileSystemIndexedIO read/write(IntVector)"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
		"""Test FileSystemIndexedIO read/write(StringVector)"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
		"""Test FileSystemIndexedIO read/write(Float/Double)"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
		fv = 2.0
						
		name = "myFloat"
		f.write(name, fv)
				
		gv = f.read(name).value
		
		self.failIf(fv is gv)
		self.assertEqual(fv, gv)
		
	def testReadWriteInt(self):
		"""Test FileSystemIndexedIO read/write(Int/Long)"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
		fv = 200
						
		name = "myInt"
		f.write(name, fv)
				
		gv = f.read(name).value
		
		self.assertEqual(fv, gv)
		
	def testReadWriteString(self):
		"""Test FileSystemIndexedIO read/write(String)"""
		
		f = FileSystemIndexedIO("./test/FileSystemIndexedIO.fs", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
		fv = "StringLiteral"
						
		name = "myString"
		f.write(name, fv)
				
		gv = f.read(name).value
		
		self.failIf(fv is gv)
		self.assertEqual(fv, gv)
	
	def tearDown(self):
		
		# cleanup
		if os.path.isdir("./test/FileSystemIndexedIO.fs/sub1") :
			for root, dirs, files in os.walk("./test/FileSystemIndexedIO.fs/sub1", topdown=Read):
			
				for name in files:				
					os.remove(os.path.join(root, name))
				for name in dirs:				
					os.rmdir(os.path.join(root, name))
				
			
			os.rmdir("./test/FileSystemIndexedIO.fs/sub1")
		
		
		if os.path.isdir("./test/FileSystemIndexedIO.fs/sub2") :
			for root, dirs, files in os.walk("./test/FileSystemIndexedIO.fs/sub2", topdown=Read):
			
				for name in files:				
					os.remove(os.path.join(root, name))
				for name in dirs:				
					os.rmdir(os.path.join(root, name))
			
			os.rmdir("./test/FileSystemIndexedIO.fs/sub2")
			
		if os.path.isdir("./test/FileSystemIndexedIO.fs") :	
			os.rmdir("./test/FileSystemIndexedIO.fs")

class TestMemoryIndexedIO(unittest.TestCase):

	def test(self):
		"""Test MemoryIndexedIO read/write operations."""
		f = MemoryIndexedIO( CharVectorData(), "/", IndexedIOOpenMode.Write)
		self.assertEqual( f.pwd() , "/" )
		self.assertRaises( RuntimeError, FileSystemIndexedIO, "./test/FileSystemIndexedIO.fs", "/nonexistantentrypoint", IndexedIOOpenMode.Read)
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
				
class TestFileIndexedIO(unittest.TestCase):
	
	badNames = ['*', '!', '&', '^', '@', '#', '$', '(', ')', '<', '+',
		'>', '?', ',', '\', ''', ';', '{', '}', '[',
		']', '=', '`' ]
		
	def testConstructors(self):
		"""Test FileIndexedIO constuctors"""
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		self.assertEqual( f.pwd() , "/" )
		
		self.assertRaises( RuntimeError, FileIndexedIO, "./test/FileIndexedIO.fio", "/nonexistantentrypoint", IndexedIOOpenMode.Read)
		f = None
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Read)

	def testResetRoot(self):
		"""Test FileIndexedIO resetRoot"""
		
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub1")
		f.chdir("sub1")
		f.mkdir("sub2")
		              
		g = f.resetRoot()
		g.chdir("/")   
		              
		filter = IndexedIOEntryTypeFilter( IndexedIOEntryType.Directory )
		e = g.ls( filter )
		self.assertEqual( len(e), 1 )
		
		self.assertEqual( e[0].id(), "sub2")
		self.assert_( e[0].entryType() == Directory)				
		
	def testMkdir(self):
		"""Test FileIndexedIO mkdir"""
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub1")
		self.assertEqual( f.pwd() , "/" )
		
		f.mkdir("sub2")
		self.assertEqual( f.pwd() , "/" )
		
	def testPwd(self):
		"""Test FileIndexedIO pwd"""
		pass
		
	def testChdir(self):
		"""Test FileIndexedIO chdir"""
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub1")
		f.mkdir("sub2")
		
		f.chdir("sub1")
		self.assertEqual( f.pwd(), "/sub1" )
		f.chdir("..")
		self.assertEqual( f.pwd(), "/" )
		f.chdir("sub2")
		self.assertEqual( f.pwd(), "/sub2" )
		
		f.mkdir("sub2.1")
		f.chdir("sub2.1")
		self.assertEqual( f.pwd(), "/sub2/sub2.1" )
		f.chdir("..")
		self.assertEqual( f.pwd(), "/sub2" )
		f.chdir(".")
		self.assertEqual( f.pwd(), "/sub2" )
		f.chdir("..")
		self.assertEqual( f.pwd(), "/" )
		f.chdir("sub2")
		self.assertEqual( f.pwd(), "/sub2" )
		f.chdir("/sub2/sub2.1")
		self.assertEqual( f.pwd(), "/sub2/sub2.1" )
		f.chdir("/")
		
		# Try to chdir above root directory
		f.chdir("..")
		self.assertEqual( f.pwd(), "/" )
		
		
		# Try to cd to non-existant directory
		self.assertRaises(RuntimeError, f.chdir, "DOESNOTEXIST")
		
			
		
	def testLs(self):
		"""Test FileIndexedIO ls"""
		
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		
		
		f.mkdir("sub2")
		f.chdir("sub2")
		f.mkdir("sub2.1")
		
		# Filer for files
		filter = IndexedIOEntryTypeFilter( IndexedIOEntryType.File )
		e = f.ls( filter )
		self.assertEqual( len(e), 0 )
		
		# Filter for directories
		filter = IndexedIOEntryTypeFilter( IndexedIOEntryType.Directory )
		e = f.ls( filter )
		self.assertEqual( len(e), 1 )
		
		self.assertEqual( e[0].id(), "sub2.1")
		self.assert_( e[0].entryType() == Directory)
		
		f.mkdir("sub2.2")
		
		#No filter
		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 2 )
		
		# No filter via default argument
		g = f.ls( )
		self.assertEqual( len(e), len(g) )

		# regex filter
		filter = IndexedIORegexFilter( ".*2" )
		e = f.ls( filter )
		self.assertEqual( len(e), 1 )

		
	def testRm(self):
		"""Test FileIndexedIO rm"""
		
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub2")
		f.chdir("sub2")
		
		f.mkdir("sub2.1");
		f.mkdir("sub2.2");
		f.mkdir("sub2.3");

		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 3 )
		f.rm("sub2.1")		
		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 2 )
		f.rm("sub2.2")		
		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 1 )
		f.rm("sub2.3")		
		e = f.ls( IndexedIONullFilter() )
		self.assertEqual( len(e), 0 )
		
		f.chdir("..")
		f.rm("sub2")
		
		
	def testRmStress(self) :		
		"""Test FileIndexedIO rm (stress test)"""	
		
		random.seed( 19 )
		
		dataPresent = set()
		
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		f.mkdir("data")
		f.chdir("data")
		
		f = None
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Append)
		f.chdir( "data" )	
		
		numLoops = 500
		maxSize = 1000
		
		for i in range( 0, numLoops ) :
			
			for i in range( 0, maxSize ) :

				index = int( random.random() * maxSize )

				if not index in dataPresent :

					f.write( "data"+str(index), i )
					dataPresent.add( index )
					
				else :
				
					f.rm( "data"+str(index) )
					dataPresent.remove( index )
				
				
			# Reopen the file every now and then, to exercise the index reading/writing
			if random.random() > 0.8 :
			
				f = None
				f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Append)
				f.chdir( "data" )		
			
			entries = f.ls( IndexedIONullFilter() )

			entryNames = [ x.id() for x in entries ]

			for i in range( 0, maxSize ) :

				dataName = "data"+str(i)
				if dataName in entryNames :

					self.assert_( i in dataPresent )

				else :

					self.failIf( i in dataPresent )

			self.assertEqual( len(entries), len(dataPresent) )
		
			
		
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
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
		fv = 2.0
						
		name = "myFloat"
		f.write(name, fv)
				
		gv = f.read(name).value
		
		self.failIf(fv is gv)
		self.assertEqual(fv, gv)
		
	def testReadWriteInt(self):
		"""Test FileIndexedIO read/write(Int/Long)"""
		
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
		fv = 200
						
		name = "myInt"
		f.write(name, fv)
				
		gv = f.read(name).value
		
		self.assertEqual(fv, gv)
		
	def testReadWriteString(self):
		"""Test FileIndexedIO read/write(String)"""
		
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		
		f.mkdir("sub1");
		f.chdir("sub1");
		
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
	
