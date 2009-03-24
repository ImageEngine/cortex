##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
import warnings

from IECore import *

class derived( Parameterised ):

	def __init__( self ):
		self.objectAttribute1 = 1
		Parameterised.__init__( self, "name", "description" )
		a = self.parameters()
		self.objectAttribute2 = 2
		a.addParameter( IntParameter( "number", "number parameter", 0 ) )

class TestParameterised( unittest.TestCase ) :

	def testSubclass( self ) :
		""" Test Parameterised subclassing """
		b = derived()
		
	def testUserData( self ) :
	
		a = Parameterised( "name", "description" )
		self.assertEqual( a.userData(), CompoundObject() )

	def testSmartAssignment( self ):
		
		b = derived()
		b.anotherAttribute = [ 1, "lala", 1.2 ]
		b.objectAttribute1 = 2
		self.assert_( isinstance( b.anotherAttribute, list ) )
		self.assertEqual( b.objectAttribute1, 2 )
		b["number"] = 30
		self.assertEqual( b["number"].getTypedValue(), 30 )

	def testAttributeDeprecation( self ) :
			
		b = derived()
		self.assertRaises( DeprecationWarning, getattr, b, "number" )
		self.assertRaises( DeprecationWarning, setattr, b, "number", 10 )

if __name__ == "__main__":
        unittest.main()
