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

class TestMenus( unittest.TestCase ) :

	def paths( self, m ) :

		return [ x[0] for x in m.items() ]

	def test( self ) :

		m = MenuDefinition()
		m.append( "/a/b", {} )
		self.assertEqual( self.paths( m ), [ "/a/b" ] )
		m.append( "/a/c", {} )
		self.assertEqual( self.paths( m ), [ "/a/b", "/a/c" ] )

		# adding a duplicate should remove the original (just reorder)
		m.append( "/a/b", {} )
		self.assertEqual( self.paths( m ), [ "/a/c", "/a/b" ] )

		# test removal
		m.remove( "/a/b" )
		self.assertEqual( self.paths( m ), [ "/a/c" ] )
		self.assertRaises( KeyError, m.remove, "/a/b" )
		self.assertRaises( KeyError, m.remove, "/a/b", True )
		m.remove( "/a/b", False )
		self.assertEqual( self.paths( m ), [ "/a/c" ] )

		# test insertion
		m.insertBefore( "/a/b", {}, "/a/c" )
		self.assertEqual( self.paths( m ), [ "/a/b", "/a/c" ] )
		m.insertBefore( "/a/d", {}, "/a/c" )
		self.assertEqual( self.paths( m ), [ "/a/b", "/a/d", "/a/c" ] )
		m.insertAfter( "/a/b", {}, "/a/c" )
		self.assertEqual( self.paths( m ), [ "/a/d", "/a/c", "/a/b" ] )

		# test matching removal
		m.append( "/b/e", {} )
		self.assertEqual( self.paths( m ), [ "/a/d", "/a/c", "/a/b", "/b/e" ] )
		m.removeMatching( "/a/.*" )
		self.assertEqual( self.paths( m ), [ "/b/e" ] )

		# test rerooting
		m.append( "/a/d", {} )
		self.assertEqual( self.paths( m ), [ "/b/e", "/a/d" ] )
		mm = m.reRooted( "/a/" )
		self.assertEqual( self.paths( mm ), [ "/d" ] )

		# test clearing
		m.clear()
		self.assertEqual( m.items(), [] )


if __name__ == "__main__":
    unittest.main()
