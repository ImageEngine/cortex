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

import os
import unittest

import IECore
import IECoreScene

class TestMotionPrimitive( unittest.TestCase ) :

	def test( self ) :

		m = IECoreScene.MotionPrimitive()
		self.assertTrue( m.isInstanceOf( "MotionPrimitive" ) )
		self.assertTrue( m.isInstanceOf( "VisibleRenderable" ) )

		self.assertEqual( m.keys(), [] )
		self.assertEqual( m.values(), [] )
		self.assertEqual( len( m ), 0 )

		self.assertRaises( Exception, m.__setitem__, "notAFloat", IECoreScene.PointsPrimitive( 1 ) )

		m[0] = IECoreScene.PointsPrimitive( 1 )
		self.assertEqual( len( m ), 1 )
		self.assertEqual( m.keys(), [ 0 ] )
		self.assertEqual( m.values(), [ IECoreScene.PointsPrimitive( 1 ) ] )

		m[1] = IECoreScene.PointsPrimitive( 1 )
		self.assertEqual( len( m ), 2 )
		self.assertEqual( m.keys(), [ 0, 1 ] )
		self.assertEqual( m.values(), [ IECoreScene.PointsPrimitive( 1 ), IECoreScene.PointsPrimitive( 1 ) ] )

		iface = IECore.IndexedIO.create( os.path.join( "test", "motionPrimitive.fio" ), IECore.IndexedIO.OpenMode.Write )
		m.save( iface, "test" )

		mm = IECore.Object.load( iface, "test" )
		self.assertEqual( m, mm )

		mmm = m.copy()
		self.assertEqual( m, mmm )

		del m[0]
		self.assertEqual( len( m ), 1 )
		self.assertEqual( m.keys(), [ 1 ] )
		self.assertEqual( m.values(), [ IECoreScene.PointsPrimitive( 1 ) ] )

		del m[1]
		self.assertEqual( m.keys(), [] )
		self.assertEqual( m.values(), [] )
		self.assertEqual( len( m ), 0 )

	def testItems( self ) :

		m = IECoreScene.MotionPrimitive()
		m[0] = IECoreScene.PointsPrimitive( 1 )
		m[1] = IECoreScene.PointsPrimitive( 2 )
		self.assertEqual( m.items(), [ ( 0, IECoreScene.PointsPrimitive( 1 ) ), ( 1, IECoreScene.PointsPrimitive( 2 ) ) ] )

	def testHash( self ) :

		m = IECoreScene.MotionPrimitive()
		m2 = IECoreScene.MotionPrimitive()
		self.assertEqual( m.hash(), m2.hash() )

		m[0] = IECoreScene.SpherePrimitive()
		self.assertNotEqual( m.hash(), m2.hash() )

		m2[0] = IECoreScene.SpherePrimitive()
		self.assertEqual( m.hash(), m2.hash() )

		m[1] = IECoreScene.SpherePrimitive()
		self.assertNotEqual( m.hash(), m2.hash() )

		m2[2] = IECoreScene.SpherePrimitive()
		self.assertNotEqual( m.hash(), m2.hash() )

	def tearDown( self ) :

		if os.path.isfile( os.path.join( "test", "motionPrimitive.fio" ) ):
			os.remove( os.path.join( "test", "motionPrimitive.fio" ) )

if __name__ == "__main__":
    unittest.main()
