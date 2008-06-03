##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

"""Unit test for Imath binding"""

import math
import unittest
import random
import IECore

class ImathRootsTest( unittest.TestCase ) :

	def testLinear( self ) :
	
		x = IECore.solveLinear( 1, 1 )
		self.assert_( isinstance( x, tuple ) )
		self.assertEqual( len( x ), 1 )
		self.assertEqual( x[0], -1 )
		
		x = IECore.solveLinear( 0, 1 )
		self.assert_( isinstance( x, tuple ) )
		self.assertEqual( len( x ), 0 )
		
		self.assertRaises( ArithmeticError, IECore.solveLinear, 0, 0 )
	
	def testQuadratic( self ) :
	
		x = IECore.solveQuadratic( 1, 0, -1 )
		self.assert_( isinstance( x, tuple ) )
		self.assertEqual( len( x ), 2 )
		self.assert_( -1 in x )
		self.assert_( 1 in x )
		
		self.assertRaises( ArithmeticError, IECore.solveQuadratic, 0, 0, 0 )
		
	def testCubic( self ) :
	
		x = IECore.solveCubic( 1, 0, 0, -1 )
		self.assert_( isinstance( x, tuple ) )
		self.assertEqual( len( x ), 1 )
		self.assertEqual( x[0], 1 )
		
		self.assertRaises( ArithmeticError, IECore.solveCubic, 0, 0, 0, 0 )	
			
if __name__ == "__main__":
    unittest.main()   

