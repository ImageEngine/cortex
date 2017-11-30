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

import unittest
import IECore
import IECoreScene

class OptionsTest( unittest.TestCase ) :

	def testCopy( self ) :

		o = IECoreScene.Options()
		o.options["test"] = IECore.FloatData( 10 )

		oo = o.copy()
		self.assertEqual( o, oo )

	def testConstructFromDict( self ) :

		o = IECoreScene.Options( {
			"a" : IECore.StringData( "a" ),
			"b" : IECore.IntData( 10 ),
		} )

		self.assertEqual( len( o.options ), 2 )
		self.assertEqual( o.options["a"], IECore.StringData( "a" ) )
		self.assertEqual( o.options["b"], IECore.IntData( 10 ) )

	def testHash( self ) :

		o1 = IECoreScene.Options()
		o2 = IECoreScene.Options()

		self.assertEqual( o1.hash(), o2.hash() )

		o1.options["a"] = IECore.StringData( "a" )
		self.assertNotEqual( o1.hash(), o2.hash() )

		o2.options["a"] = IECore.StringData( "a" )
		self.assertEqual( o1.hash(), o2.hash() )

if __name__ == "__main__":
	unittest.main()
