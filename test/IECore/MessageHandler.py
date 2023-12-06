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

from __future__ import with_statement
import gc
import inspect
import unittest
import pathlib
import subprocess
import sys
import threading
import time
import weakref

import IECore

class TestMessageHandler( unittest.TestCase ) :

	def testAbbreviation( self ) :

		self.assertEqual( IECore.Msg, IECore.MessageHandler )
		self.assertTrue( IECore.Msg is IECore.MessageHandler )

	def testStack( self ) :

		for i in range( 1, 10 ) :
			m = IECore.NullMessageHandler()
			with m :
				self.assertTrue( m.isSame( IECore.MessageHandler.currentHandler() ) )

		m1 = IECore.NullMessageHandler()
		m2 = IECore.NullMessageHandler()

		self.assertTrue( IECore.MessageHandler.currentHandler().isSame( IECore.MessageHandler.getDefaultHandler() ) )

		with m1 :
			self.assertTrue( IECore.MessageHandler.currentHandler().isSame( m1 ) )
			with m2 :
				self.assertTrue( IECore.MessageHandler.currentHandler().isSame( m2 ) )
			self.assertTrue( IECore.MessageHandler.currentHandler().isSame( m1 ) )

		self.assertTrue( IECore.MessageHandler.currentHandler().isSame( IECore.MessageHandler.getDefaultHandler() ) )

	def testLevelStringConversion( self ) :

		ll = [
			(IECore.MessageHandler.Level.Error, "ERROR"),
			(IECore.MessageHandler.Level.Warning, "WARNING"),
			(IECore.MessageHandler.Level.Info, "INFO"),
			(IECore.MessageHandler.Level.Debug, "DEBUG"),
			(IECore.MessageHandler.Level.Invalid, "INVALID"),
		]

		for l, s in ll :
			self.assertEqual( IECore.MessageHandler.levelAsString( l ), s )
			self.assertEqual( IECore.MessageHandler.stringAsLevel( s ), l )
			self.assertEqual( IECore.MessageHandler.stringAsLevel( s.lower() ), l )

	def testOutput( self ) :

		with IECore.NullMessageHandler() :

			IECore.MessageHandler.output( IECore.Msg.Level.Debug, "message handler test", "ignore me" )
			IECore.MessageHandler.output( IECore.Msg.Level.Info, "message handler test", "and me" )
			IECore.MessageHandler.output( IECore.Msg.Level.Warning, "message handler test", "and me" )
			IECore.MessageHandler.output( IECore.Msg.Level.Error, "message handler test", "and me" )

			IECore.msg( IECore.Msg.Level.Error, "message handler test", "and me" )

	def testOStreamHandler( self ) :

		IECore.OStreamMessageHandler.cErrHandler()
		IECore.OStreamMessageHandler.cOutHandler()

	def testCompoundHandler( self ) :

		h = IECore.CompoundMessageHandler()
		h.addHandler( IECore.OStreamMessageHandler.cErrHandler() )
		h.addHandler( IECore.OStreamMessageHandler.cOutHandler() )
		h.removeHandler( IECore.OStreamMessageHandler.cErrHandler() )
		h.removeHandler( IECore.OStreamMessageHandler.cOutHandler() )

	def testLevelFilteredMessageHandler( self ):

		with IECore.LevelFilteredMessageHandler( IECore.NullMessageHandler(), IECore.Msg.Level.Info ) :

			IECore.MessageHandler.output( IECore.Msg.Level.Debug, "message handler test", "ignore me" )
			IECore.MessageHandler.output( IECore.Msg.Level.Info, "message handler test", "and me" )
			IECore.MessageHandler.output( IECore.Msg.Level.Warning, "message handler test", "and me" )
			IECore.MessageHandler.output( IECore.Msg.Level.Error, "message handler test", "and me" )

	class Derived( IECore.MessageHandler ):

		def __init__( self ):
			IECore.MessageHandler.__init__( self )
			self.lastMessage = IECore.StringData("")
			self.lastContext = IECore.StringData("")
			self.lastLevel = IECore.IntData(0)

		def handle( self, level, context, msg ):
			self.lastLevel.value = level
			self.lastContext.value = context
			self.lastMessage.value = msg

	def testSubclassing( self ):

		myHandler = self.Derived()
		with myHandler :
			IECore.MessageHandler.output( IECore.Msg.Level.Info, "context", "message" )

		self.assertEqual( myHandler.lastLevel.value, IECore.Msg.Level.Info )
		self.assertEqual( myHandler.lastContext.value, "context" )
		self.assertEqual( myHandler.lastMessage.value, "message" )

	def testContextManager( self ) :

		currentHandler = IECore.MessageHandler.currentHandler()

		myHandler = self.Derived()
		with myHandler :

			IECore.MessageHandler.output( IECore.Msg.Level.Info, "context", "message" )

		self.assertTrue( currentHandler.isSame( IECore.MessageHandler.currentHandler() ) )

		self.assertEqual( myHandler.lastLevel.value, IECore.Msg.Level.Info )
		self.assertEqual( myHandler.lastContext.value, "context" )
		self.assertEqual( myHandler.lastMessage.value, "message" )

	def testIsRefCounted( self ) :

		self.assertTrue( issubclass( IECore.MessageHandler, IECore.RefCounted ) )

	def testDefaultHandler( self ) :

		self.assertTrue( isinstance( IECore.MessageHandler.currentHandler(), IECore.LevelFilteredMessageHandler ) )

	def testSetLogLevel( self ) :

		oldLevel = IECore.MessageHandler.currentHandler().getLevel()

		if oldLevel==IECore.MessageHandler.Level.Info :
			newLevel = IECore.MessageHandler.Level.Warning
		else :
			newLevel = IECore.MessageHandler.Level.Info

		IECore.setLogLevel( newLevel )

		self.assertEqual( IECore.MessageHandler.currentHandler().getLevel(), newLevel )

		IECore.setLogLevel( oldLevel )

		self.assertEqual( IECore.MessageHandler.currentHandler().getLevel(), oldLevel )

	def testContextManagerReturnValue( self ) :

		mh = self.Derived()
		with mh as mh2 :
			pass

		self.assertTrue( mh is mh2 )

	def testThreading( self ) :

		def f( handler ) :

			with handler :
				for i in range( 0, 100 ) :
					IECore.msg( IECore.Msg.Level.Info, "test", str( i ) )
					time.sleep( 0.0001 ) # encourage python to switch threads

		handlers = []
		threads = []
		for i in range( 0, 100 ) :
			handler = IECore.CapturingMessageHandler()
			thread = threading.Thread( target = f, args = [ handler ] )
			threads.append( thread )
			handlers.append( handler )
			thread.start()

		for thread in threads :
			thread.join()

		for handler in handlers :
			self.assertEqual( len( handler.messages ), 100 )
			for i, m in enumerate( handler.messages ) :
				self.assertEqual( str( i ), m.message )

	def testLifetime( self ) :

		m = IECore.NullMessageHandler()
		w = weakref.ref( m )

		with m :
			pass

		del m

		self.assertEqual( w(), None )

	def testExceptionHandling( self ) :

		# This is more a test of ExceptionAlgo than it is MessageHandler, but
		# this is the most convenient place to put the test right now.

		class ThrowingMessageHandler( IECore.MessageHandler ):

			def __init__( self ) :

				IECore.MessageHandler.__init__( self )

			def handle( self, level, context, msg ):

				if context == "python" :
					raise Exception( "Test" )
				else :
					assert( context == "c++" )
					# This will raise a C++ exception that gets translated to a
					# Python exception, and then gets translated back to a C++
					# exception by the MessageHandlerWrapper, before finally
					# being converted back to another Python exception by the
					# binding for `IECore.msg()`.
					IECore.StringAlgo.substitute( "##", { "frame" : IECore.BoolData( False ) } )

		for exceptionType in [ "python", "c++" ] :

			with self.subTest( exceptionType = exceptionType ) :

				while gc.collect() :
					pass

				expected = "Test" if exceptionType == "python" else "Unexpected data type"
				with self.assertRaisesRegex( Exception, expected ) :
					with ThrowingMessageHandler() :
						IECore.msg( IECore.Msg.Level.Error, exceptionType, "message" )

				# There should be no Exception objects in the garbage pool. If
				# there were, that would indicate a reference counting error in
				# `ExceptionAlgo::translatePythonException()`.
				self.assertEqual( [ o for o in gc.get_objects() if isinstance( o, Exception ) ], [] )

	def testOStreamLineSplitting( self ) :

		output = subprocess.check_output(
			[ sys.executable, str( pathlib.Path( __file__ ).parent / "scripts" / "messages.py" ) ],
			text = True, stderr = subprocess.STDOUT
		)

		self.assertEqual(
			output.split( "\n" ),
			[
				"ERROR : Context : Simple message",
				"ERROR : Context : Two line",
				"ERROR : message",
				"ERROR : Context : Terminating newline",
				"ERROR : Context : Blank",
				"ERROR : ",
				"ERROR : lines",
				"",
			]
		)

if __name__ == "__main__":
    unittest.main()
