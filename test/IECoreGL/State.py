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

import unittest

import IECore
import IECoreGL

IECoreGL.init( False )

class TestState( unittest.TestCase ) :

	def testConstructor( self ) :

		s = IECoreGL.State( False )
		self.assert_( not s.isComplete() )

		s = IECoreGL.State( True )
		self.assert_( s.isComplete() )

	def testUserAttributes( self ) :

		s = IECoreGL.State( False)

		self.assertEqual( s.userAttributes(), IECore.CompoundData() )
		self.failUnless( s.userAttributes().isSame( s.userAttributes() ) )

		s.userAttributes()["test"] = IECore.IntData( 1 )
		self.assertEqual( s.userAttributes(), IECore.CompoundData( { "test" : IECore.IntData( 1 ) } ) )

		s2 = IECoreGL.State( s )
		self.assertEqual( s.userAttributes(), s2.userAttributes() )

		s2.userAttributes()["test2"] = IECore.IntData( 20 )
		self.assertEqual( s.userAttributes(), IECore.CompoundData( { "test" : IECore.IntData( 1 ) } ) )
		self.assertEqual( s2.userAttributes(), IECore.CompoundData( { "test" : IECore.IntData( 1 ), "test2" : IECore.IntData( 20 ) } ) )

	def testScopedBinding( self ) :

		state1 = IECoreGL.State( True )
		state1.add( IECoreGL.NameStateComponent( "billy" ) )
		state2 = IECoreGL.State( False )
		state2.add( IECoreGL.NameStateComponent( "bob" ) )

		self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "billy" )
		self.assertEqual( state2.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "bob" )

		with IECoreGL.State.ScopedBinding( state2, state1 ) :

			self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "bob" )
			self.assertEqual( state2.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "bob" )

		self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "billy" )
		self.assertEqual( state2.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "bob" )

	def testOverrides( self ) :

		state1 = IECoreGL.State( True )
		state1.add( IECoreGL.NameStateComponent( "billy" ) )

		state2 = IECoreGL.State( False )
		state2.add( IECoreGL.NameStateComponent( "bob" ), override = True )

		state3 = IECoreGL.State( False )
		state3.add( IECoreGL.NameStateComponent( "jane" ), override = False )

		with IECoreGL.State.ScopedBinding( state2, state1 ) :

			self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "bob" )

			with IECoreGL.State.ScopedBinding( state3, state1 ) :

				self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "bob" )

			self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "bob" )

		self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "billy" )

		with IECoreGL.State.ScopedBinding( state3, state1 ) :

			self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "jane" )

			with IECoreGL.State.ScopedBinding( state2, state1 ) :

				self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "bob" )

			self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "jane" )

		self.assertEqual( state1.get( IECoreGL.NameStateComponent.staticTypeId() ).name(), "billy" )

if __name__ == "__main__":
    unittest.main()
