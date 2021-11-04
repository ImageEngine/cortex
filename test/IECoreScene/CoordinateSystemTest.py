##########################################################################
#
#  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

import os
import unittest
import imath
import IECore
import IECoreScene

class CoordinateSystemTest( unittest.TestCase ) :

	def test( self ) :

		a = IECoreScene.CoordinateSystem( "a" )
		self.assertEqual( a.getName(), "a" )

		a.setName( "b" )
		self.assertEqual( a.getName(), "b" )

		aa = a.copy()
		self.assertEqual( a, aa )

		IECore.ObjectWriter( a, os.path.join( "test", "IECore", "data", "coordSys.cob" ) ).write()
		aaa = IECore.ObjectReader( os.path.join( "test", "IECore", "data", "coordSys.cob" ) ).read()

		self.assertEqual( aaa, aa )
		self.assertEqual( aaa.getName(), "b" )

	def testHash( self ) :

		a = IECoreScene.CoordinateSystem( "a" )
		b = IECoreScene.CoordinateSystem( "a" )

		self.assertEqual( a.hash(), b.hash() )

		b.setName( "b" )
		self.assertNotEqual( a.hash(), b.hash() )

		b.setName( "a" )
		self.assertEqual( a.hash(), b.hash() )

		b.setTransform( IECoreScene.MatrixTransform( imath.M44f().translate( imath.V3f( 1 ) ) ) )
		self.assertNotEqual( a.hash(), b.hash() )

	def testTransform( self ) :

		c = IECoreScene.CoordinateSystem()
		self.assertEqual( c.getTransform(), None )

		c = IECoreScene.CoordinateSystem( "test" )
		self.assertEqual( c.getName(), "test" )
		self.assertEqual( c.getTransform(), None )
		self.assertEqual( c, c.copy() )

		c = IECoreScene.CoordinateSystem( "test", IECoreScene.MatrixTransform( imath.M44f() ) )
		self.assertEqual( c.getName(), "test" )
		self.assertEqual( c.getTransform(), IECoreScene.MatrixTransform( imath.M44f() ) )
		self.assertEqual( c, c.copy() )

		cc = c.copy()
		self.assertEqual( cc.getTransform(), IECoreScene.MatrixTransform( imath.M44f() ) )
		self.assertFalse( c.getTransform().isSame( cc.getTransform() ) )

		c.setTransform( IECoreScene.MatrixTransform( imath.M44f().translate( imath.V3f( 1 ) ) ) )
		self.assertEqual( c.getTransform(), IECoreScene.MatrixTransform( imath.M44f().translate( imath.V3f( 1 ) ) ) )

		c.setTransform( None )
		self.assertEqual( c.getTransform(), None )

		cc = c.copy()
		self.assertEqual( cc.getTransform(), None )

	def testLoadCobFromBeforeTransforms( self ) :

		c = IECore.ObjectReader( os.path.join( "test", "IECore", "data", "cobFiles", "coordinateSystemBeforeTransforms.cob" ) ).read()

		self.assertEqual( c.getName(), "test" )
		self.assertEqual( c.getTransform(), None )

	def testLoadCobWithTransform( self ) :

		c = IECoreScene.CoordinateSystem( "test", IECoreScene.MatrixTransform( imath.M44f() ) )
		IECore.ObjectWriter( c, os.path.join( "test", "IECore", "data", "coordSys.cob" ) ).write()
		c = IECore.ObjectReader( os.path.join( "test", "IECore", "data", "coordSys.cob" ) ).read()

		self.assertEqual( c.getTransform(), IECoreScene.MatrixTransform( imath.M44f() ) )

		c = IECoreScene.CoordinateSystem( "test", None )
		IECore.ObjectWriter( c, os.path.join( "test", "IECore", "data", "coordSys.cob" ) ).write()
		c = IECore.ObjectReader( os.path.join( "test", "IECore", "data", "coordSys.cob" ) ).read()

		self.assertEqual( c.getTransform(), None )

	def testEquality( self ) :

		c1 = IECoreScene.CoordinateSystem( "test" )
		c2 = IECoreScene.CoordinateSystem( "test" )
		self.assertEqual( c1, c2 )
		self.assertEqual( c2, c1 )

		c1.setName( "test2" )
		self.assertNotEqual( c1, c2 )
		self.assertNotEqual( c2, c1 )
		c1.setName( "test" )

		c1.setTransform( IECoreScene.MatrixTransform( imath.M44f() ) )
		self.assertNotEqual( c1, c2 )
		self.assertNotEqual( c2, c1 )

		c2.setTransform( IECoreScene.MatrixTransform( imath.M44f() ) )
		self.assertEqual( c1, c2 )
		self.assertEqual( c2, c1 )

	def testMemoryUsage( self ) :

		c = IECoreScene.CoordinateSystem( "test" )
		m = c.memoryUsage()
		c.setTransform( IECoreScene.MatrixTransform( imath.M44f() ) )
		self.assertTrue( c.memoryUsage() > m )

	def tearDown( self ) :

		if os.path.exists( os.path.join( "test", "IECore", "data", "coordSys.cob" ) ) :

			os.remove( os.path.join( "test", "IECore", "data", "coordSys.cob" ) )

if __name__ == "__main__":
	unittest.main()
