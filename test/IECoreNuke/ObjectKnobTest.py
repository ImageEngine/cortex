##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

import unittest
import os
import weakref

import nuke

import IECore
import IECoreNuke

class ObjectKnobTest( IECoreNuke.TestCase ) :

	def testNameAndLabel( self ) :

		n = nuke.createNode( "ieObject" )

		k = n.knob( "object" )

		self.assertEqual( k.name(), "object" )
		self.assertEqual( k.label(), "Object" )

	def testAccessors( self ) :

		n = nuke.createNode( "ieObject" )

		k = n.knob( "object" )

		self.assertTrue( isinstance( k, IECoreNuke.ObjectKnob ) )

		self.assertEqual( k.getValue(), None )

		i = IECore.IntData( 10 )
		k.setValue( i )
		self.assertFalse( k.getValue().isSame( i ) )

		self.assertEqual( k.getValue(), IECore.IntData( 10 ) )
		i.value = 20
		self.assertEqual( k.getValue(), IECore.IntData( 10 ) )

	def testLifetime( self ) :

		n = nuke.createNode( "ieObject" )

		k = n.knob( "object" )

		nuke.scriptClear()

		self.assertRaises( RuntimeError, k.name )
		self.assertRaises( RuntimeError, k.label )
		self.assertRaises( RuntimeError, k.setValue, None )
		self.assertRaises( RuntimeError, k.getValue )

		w = weakref.ref( k )
		self.assertFalse( w() is None )

		del k

		self.assertFalse( w() is not None )

	def testSetValueReturn( self ) :

		n = nuke.createNode( "ieObject" )

		k = n.knob( "object" )

		self.assertEqual( k.setValue( None ), False )
		self.assertEqual( k.setValue( IECore.IntData( 1 ) ), True )
		self.assertEqual( k.setValue( IECore.IntData( 1 ) ), False )
		self.assertEqual( k.setValue( IECore.IntData( 10 ) ), True )
		self.assertEqual( k.setValue( None ), True )

	def testCopyPaste( self ) :

		n = nuke.createNode( "ieObject" )
		self.assertEqual( nuke.selectedNodes(), [ n ] )

		n.knob( "object" ).setValue( IECore.IntData( 10 ) )

		nuke.nodeCopy( "test/IECoreNuke/objectKnob.nk" )

		nuke.scriptClear()

		n2 = nuke.nodePaste( "test/IECoreNuke/objectKnob.nk" )
		self.assertEqual( n2.knob( "object" ).getValue(), IECore.IntData( 10 ) )

	def testCopyPasteNoValue( self ) :

		n = nuke.createNode( "ieObject" )
		self.assertEqual( nuke.selectedNodes(), [ n ] )

		nuke.nodeCopy( "test/IECoreNuke/objectKnob.nk" )

		nuke.scriptClear()

		n2 = nuke.nodePaste( "test/IECoreNuke/objectKnob.nk" )
		self.assertEqual( n2.knob( "object" ).getValue(), None )

	def testUndo( self ) :

		# check our custom knob undoes in the same way as
		# standard knobs

		n = nuke.createNode( "ieObject" )
		n2 = nuke.createNode( "Blur" )

		self.assertEqual( n.knob( "object" ).getValue(), None )
		self.assertEqual( n2.knob( "size" ).getValue(), 0 )

		self.assertEqual( nuke.Undo.disabled(), True )

		with IECoreNuke.UndoEnabled() :

			self.assertEqual( nuke.Undo.disabled(), False )

			with IECoreNuke.UndoBlock() :

				n.knob( "object" ).setValue( IECore.IntData( 10 ) )
				self.assertEqual( n.knob( "object" ).getValue(), IECore.IntData( 10 ) )

				n2.knob( "size" ).setValue( 10 )
				self.assertEqual( n2.knob( "size" ).getValue(), 10 )

		self.assertEqual( nuke.Undo.disabled(), True )

		self.assertEqual( n.knob( "object" ).getValue(), IECore.IntData( 10 ) )
		self.assertEqual( n2.knob( "size" ).getValue(), 10 )

		nuke.undo()

		self.assertEqual( n2.knob( "size" ).getValue(), 0 )
		self.assertEqual( n.knob( "object" ).getValue(), None )

	def tearDown( self ) :

		for f in [
				"test/IECoreNuke/objectKnob.nk",
			] :

			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
	unittest.main()

