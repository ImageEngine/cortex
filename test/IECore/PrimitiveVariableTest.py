##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

class PrimitiveVariableTest( unittest.TestCase ) :

	def testConstructors( self ) :

		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 10 ) )
		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.IntData( 10 ) )

		p2 = IECore.PrimitiveVariable( p )
		self.assertEqual( p2.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p2.data, IECore.IntData( 10 ) )
		self.failUnless( p2.data.isSame( p.data ) )
		
		p.data.value = 20
		self.assertEqual( p.data, IECore.IntData( 20 ) )
		self.assertEqual( p2.data, IECore.IntData( 20 ) )
		
		p3 = IECore.PrimitiveVariable( p, True ) # deep copy
		self.assertEqual( p3.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p3.data, IECore.IntData( 20 ) )
		self.failIf( p3.data.isSame( p.data ) )
		
		p3.data.value = 30
		self.assertEqual( p3.data, IECore.IntData( 30 ) )
		self.assertEqual( p.data, IECore.IntData( 20 ) )
		self.assertEqual( p2.data, IECore.IntData( 20 ) )
		
	def testEquality( self ) :
	
		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 1 ) )
		p2 = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 1 ) )
		self.assertEqual( p, p2 )
		self.failIf( p != p2 )
		
		p.interpolation = IECore.PrimitiveVariable.Interpolation.Varying
		self.assertNotEqual( p, p2 )
		self.failIf( p == p2 )
		
		p2.interpolation = IECore.PrimitiveVariable.Interpolation.Varying
		self.assertEqual( p, p2 )
		self.failIf( p != p2 )
		
		p.data = IECore.IntData( 2 )
		self.assertNotEqual( p, p2 )
		self.failIf( p == p2 )
		
		p2.data = IECore.IntData( 2 )
		self.assertEqual( p, p2 )
		self.failIf( p != p2 )
		
	def testEqualityWithNullData( self ) :
	
		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 1 ) )
		p2 = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, None )	
		
		self.assertNotEqual( p, p2 )
		self.assertNotEqual( p2, p )		
		
		self.assertEqual( p, p )
		self.assertEqual( p2, p2 )
				
if __name__ == "__main__":
    unittest.main()
