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


"""Unit test for IndexedIOInterface binding"""
import os
import unittest
import math

from IECore import *

class TestIndexedIOInterface(unittest.TestCase):

	def testConstructors(self):
		"""Test IndexedIOInterface constuctors"""
		self.assertRaises(RuntimeError, IndexedIOInterface )
		
	def testCreate(self):
		"""Test IndexedIOInterface create"""
		
		io = IndexedIOInterface.create( "test/myFile.sql", "/", IndexedIOOpenMode.Write )
		io2 = IndexedIOInterface.create( "test/myFile.fs", "/", IndexedIOOpenMode.Write )
		io2 = IndexedIOInterface.create( "test/myFile.fio", "/", IndexedIOOpenMode.Write )		
		self.assertRaises(RuntimeError, IndexedIOInterface.create, "myFileWith.invalidExtension", "/", IndexedIOOpenMode.Write )

class TestFileSystemIndexedIO(unittest.TestCase):
	
	badNames = ['*', '!', '&', '^', '@', '#', '$', '(', ')', '<', '+',
		'>', '?', ',', '\', ''', ':', ';', '{', '}', '[',
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
				

class TestSQLiteIndexedIO(unittest.TestCase):
	
	badNames = ['*', '!', '&', '^', '@', '#', '$', '(', ')', '<', '+',
		'>', '?', ',', '\', ''', ':', ';', '{', '}', '[',
		']', '=', '`' ]
		
	def testConstructors(self):
		"""Test SQLiteIndexedIO constuctors"""
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		self.assertEqual( f.pwd() , "/" )
		
		self.assertRaises( RuntimeError, SQLiteIndexedIO, "./test", "/nonexistantentrypoint", IndexedIOOpenMode.Write)
		
		# Check use of "Read" flag on non-existent database
		self.assertRaises( RuntimeError, SQLiteIndexedIO, "./nonexistentfile", "/", IndexedIOOpenMode.Read)
		
	def testPermissions(self):
		"""Test SQLiteIndexedIO default permissions"""
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)				
		sqlInfo = os.stat("./test/SQLiteIndexedIO.sql")
		
		fl = open( "./test/testFile.txt", 'w')
		fl.write( "test string" )
		fl.close()

		fileInfo = os.stat("./test/testFile.txt")
		self.assertEqual( sqlInfo.st_mode, fileInfo.st_mode )
		
	def testOpenMode(self):
		"""Test SQLiteIndexedIO open-mode flags"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)		
		self.assertEqual( f.pwd() , "/" )
		f.mkdir("sub1")
		f.mkdir("sub2")
		f.mkdir("sub3")
		e = f.ls()

		self.assertEqual( len(e), 3 )
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Read)
		e = f.ls()
		self.assertEqual( len(e), 3 )
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Append)
		e = f.ls()
		self.assertEqual( len(e), 3 )
		f.mkdir("sub4")
		e = f.ls()
		self.assertEqual( len(e), 4 )
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		e = f.ls()
		self.assertEqual( len(e), 0 )
		
	def testAppendMode(self):
		"""Test SQLiteIndexedIO append mode"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		f.write( "obj", 1 )

		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Append)
		f.write( "obj", 2 )
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Read)
		obj = f.read( "obj" )
		self.assertEqual( obj, LongData( 2 ) )
		
	def testResetRoot(self):
		"""Test SQLiteIndexedIO resetRoot"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
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
		"""Test SQLiteIndexedIO mkdir"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		
		# Name check
		for n in self.badNames:
			self.assertRaises(RuntimeError, f.mkdir, n)
		
	def testChdir(self):
		"""Test SQLiteIndexedIO chdir"""
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
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
		
		# Test '.', to remain in current directory
		f.chdir(".")
		self.assertEqual( f.pwd(), "/" )
		
		# Try to cd to non-existant directory
		self.assertRaises(RuntimeError, f.chdir, "DOESNOTEXIST")
		
		# Name check
		for n in self.badNames:
			self.assertRaises(RuntimeError, f.chdir, n)
		

	def testLs(self):
		"""Test SQLiteIndexedIO ls"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
				
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
		
	def testRm(self):
		"""Test SQLiteIndexedIO rm"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		f.mkdir("sub2")
		f.chdir("sub2")
		
		f.mkdir("sub2.1");
		f.mkdir("sub2.2");
		f.mkdir("sub2.3");

		e = f.ls()
		self.assertEqual( len(e), 3 )
		f.rm("sub2.1")		
		e = f.ls()
		
		self.assertEqual( len(e), 2 )
		f.rm("sub2.2")		
		e = f.ls()
		self.assertEqual( len(e), 1 )
		f.rm("sub2.3")		
		e = f.ls()
		self.assertEqual( len(e), 0 )
		
		f.chdir("..")
		f.rm("sub2")
		
		self.assertRaises( RuntimeError, f.rm, "DOESNOTEXIST")
		
		# Name check
		for n in self.badNames:
			self.assertRaises(RuntimeError, f.rm, n)
		
	def testReadWrite(self):
		"""Test SQLiteIndexedIO read/write(generic)"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		self.assertRaises( RuntimeError, f.read, "DOESNOTEXIST")
		
		# Name check
		for n in self.badNames:
			self.assertRaises(RuntimeError, f.read, n)
		
	def testReadWriteFloatVector(self):
		"""Test SQLiteIndexedIO read/write(FloatVector)"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		
		fv = FloatVectorData()
		
		for n in range(0, 1000):
			fv.append(n* n * math.sin(n))

		self.assertEqual(len(fv), 1000)
		
		name = "myFloatVector"
		f.write(name, fv)		
		
		gv = f.read(name)
		
		self.failIf(fv is gv)
		self.assertEqual(len(fv), len(gv))
		
		for n in range(0, 1000):
			self.assertEqual(fv[n], gv[n])
			
			
		fv = FloatVectorData()
		name = "myEmptyFloatVector"
		f.write(name, fv)
		gv = f.read(name)
		
	def testReadWriteDoubleVector(self):
		"""Test SQLiteIndexedIO read/write(DoubleVector)"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		
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
			
			
		fv = DoubleVectorData()
		name = "myEmptyDoubleVector"
		f.write(name, fv)
		gv = f.read(name)
		
	def testReadWriteIntVector(self):
		"""Test SQLiteIndexedIO read/write(IntVector)"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		
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
			
			
		fv = IntVectorData()
		name = "myEmptyIntVector"
		f.write(name, fv)
		gv = f.read(name)
			
	def testReadWriteStringVector(self):
		"""Test SQLiteIndexedIO read/write(StringVector)"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		
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
			
		fv = StringVectorData()
		name = "myEmptyStringVector"
		f.write(name, fv)
		gv = f.read(name)	
			
	def testReadWriteFloat(self):
		"""Test SQLiteIndexedIO read/write(Float/Double)"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		
		fv = 2.0
						
		name = "myFloat"
		f.write(name, fv)
				
		gv = f.read(name).value
		
		self.failIf(fv is gv)
		self.assertEqual(fv, gv)
		
	def testReadWriteInt(self):
		"""Test SQLiteIndexedIO read/write(Int/Long)"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		
		fv = 200
						
		name = "myInt"
		f.write(name, fv)
				
		gv = f.read(name).value
		
		self.assertEqual(fv, gv)
		
	def testReadWriteString(self):
		"""Test SQLiteIndexedIO read/write(String)"""
		
		f = SQLiteIndexedIO("./test/SQLiteIndexedIO.sql", "/", IndexedIOOpenMode.Write)
		
		fv = "StringLiteral"
						
		name = "myString"
		f.write(name, fv)
				
		gv = f.read(name).value
		
		self.failIf(fv is gv)
		self.assertEqual(fv, gv)
		
	def tearDown(self):
		
		# cleanup
	
		if os.path.isfile("./test/SQLiteIndexedIO.sql") :	
			os.remove("./test/SQLiteIndexedIO.sql")
			
		if os.path.isfile("./test/testFile.txt") :	
			os.remove("./test/testFile.txt")
			
class TestFileIndexedIO(unittest.TestCase):
	
	badNames = ['*', '!', '&', '^', '@', '#', '$', '(', ')', '<', '+',
		'>', '?', ',', '\', ''', ':', ';', '{', '}', '[',
		']', '=', '`' ]
		
	def testConstructors(self):
		"""Test FileIndexedIO constuctors"""
		f = FileIndexedIO("./test/FileIndexedIO.fio", "/", IndexedIOOpenMode.Write)
		self.assertEqual( f.pwd() , "/" )
		
		#self.assertRaises( RuntimeError, FileIndexedIO, "./test/FileIndexedIO.fio", "/nonexistantentrypoint", IndexedIOOpenMode.Read)
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
	
	def tearDown(self):
		
		# cleanup
		if os.path.isfile("./test/FileIndexedIO.fio") :	
			os.remove("./test/FileIndexedIO.fio")
								
		
if __name__ == "__main__":
	unittest.main()   
	
