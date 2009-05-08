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

import unittest

from IECore import *

class TestMessageHandler( unittest.TestCase ) :

	def testAbbreviation( self ) :

		self.assertEqual( Msg, MessageHandler )
		self.assert_( Msg is MessageHandler )

	def testStack( self ) :

		for i in range( 1, 10 ) :
			MessageHandler.pushHandler( NullMessageHandler() )
		for i in range( 1, 10 ) :
			MessageHandler.popHandler()

	def testLevelStringConversion( self ) :

		ll = [
			(MessageHandler.Level.Error, "ERROR"),
			(MessageHandler.Level.Warning, "WARNING"),
			(MessageHandler.Level.Info, "INFO"),
			(MessageHandler.Level.Debug, "DEBUG"),
			(MessageHandler.Level.Invalid, "INVALID"),
		]

		for l, s in ll :
			self.assertEqual( MessageHandler.levelAsString( l ), s )
			self.assertEqual( MessageHandler.stringAsLevel( s ), l )
			self.assertEqual( MessageHandler.stringAsLevel( s.lower() ), l )

	def testOutput( self ) :

		MessageHandler.pushHandler( NullMessageHandler() )

		MessageHandler.output( Msg.Level.Debug, "message handler test", "ignore me" )
		MessageHandler.output( Msg.Level.Info, "message handler test", "and me" )
		MessageHandler.output( Msg.Level.Warning, "message handler test", "and me" )
		MessageHandler.output( Msg.Level.Error, "message handler test", "and me" )

		msg( Msg.Level.Error, "message handler test", "and me" )

		MessageHandler.popHandler()

	def testOStreamHandler( self ) :

		OStreamMessageHandler.cErrHandler()
		OStreamMessageHandler.cOutHandler()

	def testCompoundHandler( self ) :

		h = CompoundMessageHandler()
		h.addHandler( OStreamMessageHandler.cErrHandler() )
		h.addHandler( OStreamMessageHandler.cOutHandler() )
		h.removeHandler( OStreamMessageHandler.cErrHandler() )
		h.removeHandler( OStreamMessageHandler.cOutHandler() )

	def testLevelFilteredMessageHandler( self ):

		MessageHandler.pushHandler( LevelFilteredMessageHandler(NullMessageHandler(), Msg.Level.Info ) )

		MessageHandler.output( Msg.Level.Debug, "message handler test", "ignore me" )
		MessageHandler.output( Msg.Level.Info, "message handler test", "and me" )
		MessageHandler.output( Msg.Level.Warning, "message handler test", "and me" )
		MessageHandler.output( Msg.Level.Error, "message handler test", "and me" )

		MessageHandler.popHandler()

	def testSubclassing( self ):

		class derived( MessageHandler ):

			def __init__( self ):
				MessageHandler.__init__( self )
				self.lastMessage = StringData("")
				self.lastContext = StringData("")
				self.lastLevel = IntData(0)

			def handle( self, level, context, msg ):
				self.lastLevel.value = level
				self.lastContext.value = context
				self.lastMessage.value = msg

		myHandler = derived()
		MessageHandler.pushHandler( myHandler )
		MessageHandler.output( Msg.Level.Info, "context", "message" )
		MessageHandler.popHandler()

		self.assertEqual( myHandler.lastLevel.value, Msg.Level.Info )
		self.assertEqual( myHandler.lastContext.value, "context" )
		self.assertEqual( myHandler.lastMessage.value, "message" )

	def testIsRefCounted( self ) :

		self.assert_( issubclass( MessageHandler, RefCounted ) )

if __name__ == "__main__":
    unittest.main()
