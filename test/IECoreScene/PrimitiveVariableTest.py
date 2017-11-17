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
import IECoreScene

class PrimitiveVariableTest( unittest.TestCase ) :

	def testConstructors( self ) :

		p = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 10 ) )
		self.assertEqual( p.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.IntData( 10 ) )
		self.assertEqual( p.indices, None )

		p2 = IECoreScene.PrimitiveVariable( p )
		self.assertEqual( p2.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p2.data, IECore.IntData( 10 ) )
		self.assertEqual( p2.indices, None )
		self.failUnless( p2.data.isSame( p.data ) )

		p.data.value = 20
		self.assertEqual( p.data, IECore.IntData( 20 ) )
		self.assertEqual( p2.data, IECore.IntData( 20 ) )

		p.indices = IECore.IntVectorData( [ 0, 1 ] )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 1 ] ) )
		self.assertEqual( p2.indices, None )

		p3 = IECoreScene.PrimitiveVariable( p, True ) # deep copy
		self.assertEqual( p3.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p3.data, IECore.IntData( 20 ) )
		self.assertEqual( p3.indices, IECore.IntVectorData( [ 0, 1 ] ) )
		self.failIf( p3.data.isSame( p.data ) )
		self.failIf( p3.indices.isSame( p.indices ) )

		p3.data.value = 30
		p3.indices = IECore.IntVectorData( [ 2, 3 ] )
		self.assertEqual( p3.data, IECore.IntData( 30 ) )
		self.assertEqual( p.data, IECore.IntData( 20 ) )
		self.assertEqual( p2.data, IECore.IntData( 20 ) )
		self.assertEqual( p3.indices, IECore.IntVectorData( [ 2, 3 ] ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 0, 1 ] ) )
		self.assertEqual( p2.indices, None )

		p4 = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 40 ), IECore.IntVectorData( [ 4, 5 ] ) )
		self.assertEqual( p4.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p4.data, IECore.IntData( 40 ) )
		self.assertEqual( p4.indices, IECore.IntVectorData( [ 4, 5 ] ) )

		p5 = IECoreScene.PrimitiveVariable( p4 )
		self.assertEqual( p5.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p5.data, IECore.IntData( 40 ) )
		self.assertEqual( p5.indices, IECore.IntVectorData( [ 4, 5 ] ) )
		self.failUnless( p5.data.isSame( p4.data ) )
		self.failUnless( p5.indices.isSame( p4.indices ) )

		p5.indices[-1] = 6
		self.assertEqual( p5.indices, IECore.IntVectorData( [ 4, 6 ] ) )
		self.assertEqual( p4.indices, IECore.IntVectorData( [ 4, 6 ] ) )

		p5.indices.append( 7 )
		self.assertEqual( p5.indices, IECore.IntVectorData( [ 4, 6, 7 ] ) )
		self.assertEqual( p4.indices, IECore.IntVectorData( [ 4, 6, 7 ] ) )

	def testEquality( self ) :

		p = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 1 ) )
		p2 = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 1 ) )
		self.assertEqual( p, p2 )
		self.failIf( p != p2 )

		p.interpolation = IECoreScene.PrimitiveVariable.Interpolation.Varying
		self.assertNotEqual( p, p2 )
		self.failIf( p == p2 )

		p2.interpolation = IECoreScene.PrimitiveVariable.Interpolation.Varying
		self.assertEqual( p, p2 )
		self.failIf( p != p2 )

		p.data = IECore.IntData( 2 )
		self.assertNotEqual( p, p2 )
		self.failIf( p == p2 )

		p2.data = IECore.IntData( 2 )
		self.assertEqual( p, p2 )
		self.failIf( p != p2 )

		p.indices = IECore.IntVectorData( [ 0, 1 ] )
		self.assertNotEqual( p, p2 )
		self.failIf( p == p2 )

		p2.indices = IECore.IntVectorData( [ 0, 1 ] )
		self.assertEqual( p, p2 )
		self.failIf( p != p2 )

	def testEqualityWithNullData( self ) :

		p = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 1 ) )
		p2 = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, None )

		self.assertNotEqual( p, p2 )
		self.assertNotEqual( p2, p )

		self.assertEqual( p, p )
		self.assertEqual( p2, p2 )

		p.indices = IECore.IntVectorData( [ 0, 1 ] )
		self.assertNotEqual( p, p2 )
		self.assertNotEqual( p2, p )

		self.assertEqual( p, p )
		self.assertEqual( p2, p2 )

	def testExpandedData( self ) :

		p = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( range( 10, 20 ) ), IECore.IntVectorData( [ 4, 5 ] ) )
		self.assertEqual( p.data, IECore.IntVectorData( range( 10, 20 ) ) )
		self.assertEqual( p.indices, IECore.IntVectorData( [ 4, 5 ] ) )
		self.assertEqual( p.expandedData(), IECore.IntVectorData( [ 14, 15 ] ) )

		p.data = p.expandedData()
		p.indices = None
		self.assertEqual( p.expandedData(), p.data )

if __name__ == "__main__":
	unittest.main()
