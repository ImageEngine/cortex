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

import unittest
import threading

import IECore
import os

class CachedReaderTest( unittest.TestCase ) :

	def testConstructors( self ) :

		# test default pool
		r = IECore.CachedReader( IECore.SearchPath( "./" ) )
		self.assertTrue( r.objectPool().isSame( IECore.ObjectPool.defaultObjectPool() ) )

		# test custom pool
		pool = IECore.ObjectPool( 100 * 1024 * 1024 )
		r = IECore.CachedReader( IECore.SearchPath( "./" ), pool )
		self.assertTrue( r.objectPool().isSame( pool ) )

	def test( self ) :

		pool = IECore.ObjectPool( 100 * 1024 * 1024 )
		r = IECore.CachedReader( IECore.SearchPath( "./" ), pool )

		o = r.read( "test/IECore/data/cobFiles/compoundData.cob" )
		self.assertEqual( o.typeName(), "CompoundData" )
		self.assertEqual( pool.memoryUsage(), o.memoryUsage() )
		self.failUnless( r.cached( "test/IECore/data/cobFiles/compoundData.cob" ) )

		oo = r.read( "test/IECore/data/cobFiles/intDataTen.cob" )
		self.assertEqual( pool.memoryUsage(), o.memoryUsage() + oo.memoryUsage() )
		self.failUnless( r.cached( "test/IECore/data/cobFiles/compoundData.cob" ) )
		self.failUnless( r.cached( "test/IECore/data/cobFiles/intDataTen.cob" ) )

		oo = r.read( "test/IECore/data/cobFiles/intDataTen.cob" )
		self.assertEqual( pool.memoryUsage(), o.memoryUsage() + oo.memoryUsage() )
		self.failUnless( r.cached( "test/IECore/data/cobFiles/compoundData.cob" ) )
		self.failUnless( r.cached( "test/IECore/data/cobFiles/intDataTen.cob" ) )

		self.assertRaises( RuntimeError, r.read, "doesNotExist" )
		self.assertRaises( RuntimeError, r.read, "doesNotExist" )

		# Here, the cache should throw away something, but we allow the
		# use of an approximate LRU strategy, so we can't guarantee which
		# object will be discarded.
		pool.setMaxMemoryUsage( o.memoryUsage() + oo.memoryUsage() - 1 )
		self.assertLess( pool.memoryUsage(), o.memoryUsage() + oo.memoryUsage() )
		self.failIf(
			r.cached( "test/IECore/data/cobFiles/compoundData.cob" ) and
			r.cached( "test/IECore/data/cobFiles/intDataTen.cob" )
		)

		# Here, the cache should throw away the remaining item because there
		# is no room for it.
		pool.setMaxMemoryUsage( pool.memoryUsage() / 2  )
		self.assertEqual( pool.memoryUsage(), 0 )
		self.failIf(
			r.cached( "test/IECore/data/cobFiles/compoundData.cob" ) or
			r.cached( "test/IECore/data/cobFiles/intDataTen.cob" )
		)

		pool.setMaxMemoryUsage( oo.memoryUsage() * 2 )
		oo = r.read( "test/IECore/data/cobFiles/intDataTen.cob" )
		pool.clear()
		r.clear()
		self.assertEqual( pool.memoryUsage(), 0 )
		self.failIf( r.cached( "test/IECore/data/cobFiles/intDataTen.cob" ) )

		oo = r.read( "test/IECore/data/cobFiles/intDataTen.cob" )
		self.assertEqual( pool.memoryUsage(), oo.memoryUsage() )
		self.failUnless( r.cached( "test/IECore/data/cobFiles/intDataTen.cob" ) )
		r.clear( "I don't exist" )
		self.failUnless( r.cached( "test/IECore/data/cobFiles/intDataTen.cob" ) )
		self.assertEqual( pool.memoryUsage(), oo.memoryUsage() )
		r.clear( "test/IECore/data/cobFiles/intDataTen.cob" )
		self.assertEqual( pool.memoryUsage(), oo.memoryUsage() )	# clearing CachedReader doesn't clear the object from the pool
		self.failIf( r.cached( "test/IECore/data/cobFiles/intDataTen.cob" ) )

		# testing insert.
		pool.clear()
		pool.setMaxMemoryUsage( o.memoryUsage() + oo.memoryUsage() )
		r.insert( "test/IECore/data/cobFiles/intDataTen.cob", oo )
		self.assertEqual( pool.memoryUsage(), oo.memoryUsage() )
		o2 = r.read( "test/IECore/data/cobFiles/intDataTen.cob" )
		self.assertEqual( oo, o2 )
		# testing overwritting existing item with insert
		r.insert( "test/IECore/data/cobFiles/intDataTen.cob", o )
		self.assertEqual( pool.memoryUsage(), o.memoryUsage()+oo.memoryUsage() )
		o2 = r.read( "test/IECore/data/cobFiles/intDataTen.cob" )
		self.assertEqual( o, o2 )

		# test clear after failed load
		self.assertRaises( RuntimeError, r.read, "/i/dont/exist" )
		r.clear( "/i/dont/exist" )

	def testRepeatedFailures( self ) :

		def check( fileName ) :

			r = IECore.CachedReader( IECore.SearchPath( "./" ), IECore.ObjectPool(100 * 1024 * 1024) )
			firstException = None
			try :
				r.read( fileName )
			except Exception as e :
				firstException = str( e )

			self.assert_( firstException )

			secondException = None
			try :
				r.read( fileName )
			except Exception as e :
				secondException = str( e )

			self.assert_( secondException )

			# we want the second exception to be different, as the CachedReader
			# shouldn't be wasting time attempting to load files again when
			# it failed the first time
			self.assertNotEqual( firstException, secondException )

		check( "iDontExist" )
		check( "include/IECore/IECore.h" )

	def testChangeSearchPaths( self ) :

		# read a file from one path
		r = IECore.CachedReader( IECore.SearchPath( "./test/IECore/data/cachedReaderPath1" ), IECore.ObjectPool(100 * 1024 * 1024) )

		o1 = r.read( "file.cob" )

		self.assertEqual( o1.value, 1 )
		self.failUnless( r.cached( "file.cob" ) )

		# read a file of the same name from a different path
		r.searchPath = IECore.SearchPath( "./test/IECore/data/cachedReaderPath2" )
		self.failIf( r.cached( "file.cob" ) )

		o2 = r.read( "file.cob" )

		self.assertEqual( o2.value, 2 )
		self.failUnless( r.cached( "file.cob" ) )

		# set the paths to the same as before and check we didn't obliterate the cache unecessarily
		r.searchPath = IECore.SearchPath( "./test/IECore/data/cachedReaderPath2" )
		self.failUnless( r.cached( "file.cob" ) )

	def testDefault( self ) :

		os.environ["IECORE_CACHEDREADER_PATHS"] = os.pathsep.join( [ "a", "test", "path" ] )

		r = IECore.CachedReader.defaultCachedReader()
		r2 = IECore.CachedReader.defaultCachedReader()
		self.assert_( r.isSame( r2 ) )
		self.assertTrue( r.objectPool().isSame( IECore.ObjectPool.defaultObjectPool() ) )
		self.assertEqual( r.searchPath, IECore.SearchPath( [ "a", "test", "path" ] ) )

	def testPostProcessing( self ) :

		r = IECore.CachedReader( IECore.SearchPath( "./test/IECore/data/cobFiles" ), IECore.ObjectPool(100 * 1024 * 1024) )
		i = r.read( "intDataTen.cob" )
		self.assertEqual( i.value, 10 )

		class PostProcessor( IECore.ModifyOp ) :

			def __init__( self ) :

				IECore.ModifyOp.__init__( self, "", IECore.IntParameter( "result", "" ), IECore.IntParameter( "input", "" ) )

			def modify( self, obj, args ) :

				obj.value *= 2

		r = IECore.CachedReader( IECore.SearchPath( "./test/IECore/data/cobFiles" ), PostProcessor(), IECore.ObjectPool(100 * 1024 * 1024) )
		i = r.read( "intDataTen.cob" )
		self.assertEqual( i.value, 20 )

	def testPostProcessingFailureMode( self ) :

		class PostProcessor( IECore.ModifyOp ) :

			def __init__( self ) :

				IECore.ModifyOp.__init__( self, "", IECore.Parameter( "result", "", IECore.NullObject() ), IECore.Parameter( "input", "", IECore.NullObject() ) )

			def modify( self, obj, args ) :

				raise Exception( "I am a very naughty op" )

		r = IECore.CachedReader( IECore.SearchPath( "./test/IECore/data/cobFiles" ), PostProcessor(), IECore.ObjectPool(100 * 1024 * 1024) )

		firstException = None
		try :
			m = r.read( "polySphereQuads.cob" )
		except Exception as e :
			firstException = str( e )

		self.failUnless( firstException is not None )

		secondException = None
		try :
			m = r.read( "polySphereQuads.cob" )
		except Exception as e :
			secondException = str( e )

		self.failUnless( secondException is not None )

		# we want the second exception to be different, as the CachedReader
		# shouldn't be wasting time attempting to load files again when
		# it failed the first time
		self.assertNotEqual( firstException, secondException )

	def testThreadingAndClear( self ) :

		# this tests a fix to the clear() method in the LRU cache,
		# which was causing a crash (or at least a spurious "Previous attempt
		# to get item failed" exception)

		def func1( r, files ):
			for i in range( 0,10000 ):
				r.read( files[ i % len( files ) ] )

		def func2( r ):
			for i in range( 0,10000 ):
				r.clear()

		files = [
			"test/IECore/data/cobFiles/compoundData.cob",
			"test/IECore/data/cobFiles/intDataTen.cob",
			"test/IECore/data/cachedReaderPath2/file.cob",
		]

		r = IECore.CachedReader( IECore.SearchPath( "./" ), IECore.ObjectPool(100 * 1024 * 1024) )

		t1 = threading.Thread( target=func1, args = [ r, files ] )
		t2 = threading.Thread( target=func1, args = [ r, files ] )
		t3 = threading.Thread( target=func2, args = [ r ] )
		t1.start()
		t2.start()
		t3.start()

		t1.join()
		t2.join()
		t3.join()

if __name__ == "__main__":
    unittest.main()
