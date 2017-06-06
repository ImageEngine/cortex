##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
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

import os
import unittest
import ctypes

import arnold

import IECore
import IECoreArnold

class UniverseBlockTest( unittest.TestCase ) :

	def test( self ) :

		self.failIf( arnold.AiUniverseIsActive() )

		with IECoreArnold.UniverseBlock( writable = False ) :

			self.failUnless( arnold.AiUniverseIsActive() )

			with IECoreArnold.UniverseBlock( writable = False ) :

				self.failUnless( arnold.AiUniverseIsActive() )

			self.failUnless( arnold.AiUniverseIsActive() )

	def testWritable( self ) :

		def createBlock( writable ) :

			with IECoreArnold.UniverseBlock( writable ) :

				self.failUnless( arnold.AiUniverseIsActive() )

		with IECoreArnold.UniverseBlock( writable = True ) :

			self.failUnless( arnold.AiUniverseIsActive() )

			createBlock( False )
			self.assertRaisesRegexp( RuntimeError, "Arnold is already in use", createBlock, True )

		with IECoreArnold.UniverseBlock( writable = False ) :

			self.failUnless( arnold.AiUniverseIsActive() )

			createBlock( True )
			createBlock( False )

	def testMetadataLoading( self ) :

		os.environ["ARNOLD_PLUGIN_PATH"] = os.path.join( os.path.dirname( __file__ ), "metadata" )
		with IECoreArnold.UniverseBlock( writable = True ) :
			pass

		with IECoreArnold.UniverseBlock( writable = False ) :

			e = arnold.AiNodeEntryLookUp( "options" )

			s = ctypes.c_char_p()
			i = ctypes.c_int()

			arnold.AiMetaDataGetStr( e, "", "cortex.testString", s )
			self.assertEqual( s.value, "test" )

			arnold.AiMetaDataGetInt( e, "", "cortex.testInt", i )
			self.assertEqual( i.value, 25 )

			arnold.AiMetaDataGetStr( e, "AA_samples", "cortex.testString", s )
			self.assertEqual( s.value, "test2" )

			arnold.AiMetaDataGetInt( e, "AA_samples", "cortex.testInt", i )
			self.assertEqual( i.value, 12 )

	def testReadOnlyUniverseDoesntPreventWritableUniverseCleanup( self ) :

		with IECoreArnold.UniverseBlock( writable = False ) :

			with IECoreArnold.UniverseBlock( writable = True ) :

				node = arnold.AiNode( "polymesh" )
				arnold.AiNodeSetStr( node, "name", "test" )

			self.assertEqual( arnold.AiNodeLookUpByName( "test" ), None )

if __name__ == "__main__":
    unittest.main()
