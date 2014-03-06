##########################################################################
#
#  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

class RefCountedTest( unittest.TestCase ) :

	def testEquality( self ) :

		# Equality should be based on the underlying
		# object identity.

		r1a = IECore.RefCounted()
		r1b = r1a
		
		r2a = IECore.RefCounted()
		r2b = r2a
		
		self.assertEqual( r1a, r1b )
		self.assertEqual( r2a, r2b )
		
		self.assertNotEqual( r2a, r1a )
		self.assertNotEqual( r2b, r1a )
	
	def testEqualityAgainstDifferentTypes( self ) :
	
		r = IECore.RefCounted()
		self.assertNotEqual( r, 10 )
		self.assertNotEqual( r, "10" )
		self.assertNotEqual( r, None )
		self.assertNotEqual( r, [] )
		
	def testEqualityAfterRetrievalFromCPP( self ) :
	
		# Equality should be preserved even when
		# two python objects refer to the same underlying
		# RefCounted object because we failed to preserve
		# identity when retrieving from C++. We need to
		# test this with a RefCounted class that isn't wrapped
		# and doesn't derive from Object, which defines its
		# own equality operator - ObjectParameters are one
		# of the few classes that fit this bill.
	
		p1 = IECore.ObjectParameter( "p1", "", IECore.IntData(), IECore.IntData.staticTypeId() )
		p2 = IECore.ObjectParameter( "p2", "", IECore.IntData(), IECore.IntData.staticTypeId() )
		
		self.assertEqual( p1, p1 )
		self.assertEqual( p2, p2 )
		self.assertNotEqual( p1, p2 )
		
		c = IECore.CompoundParameter( "c" )
		c.addParameter( p1 )
		c.addParameter( p2 )
		
		self.assertEqual( p1, c["p1"] )
		self.assertEqual( c["p1"], p1 )
		
		self.assertEqual( p2, c["p2"] )
		self.assertEqual( c["p2"], p2 )
		
		self.assertNotEqual( p1, c["p2"] )
		self.assertNotEqual( p1, c["p2"] )
	
	def testEqualityAfterRetrievalFromCPP( self ) :

		p1 = IECore.ObjectParameter( "p1", "", IECore.IntData(), IECore.IntData.staticTypeId() )
		p2 = IECore.ObjectParameter( "p2", "", IECore.IntData(), IECore.IntData.staticTypeId() )

		self.assertEqual( hash( p1 ), hash( p1 ) )
		self.assertEqual( hash( p2 ), hash( p2 ) )
		self.assertNotEqual( hash( p1 ), hash( p2 ) )
		
		c = IECore.CompoundParameter( "c" )
		c.addParameter( p1 )
		c.addParameter( p2 )

		self.assertEqual( hash( p1 ), hash( c["p1"] ) )
		self.assertEqual( hash( p2 ), hash( c["p2"] ) )
		self.assertNotEqual( hash( c["p1"] ), hash( c["p2"] ) )

if __name__ == "__main__":
    unittest.main()
