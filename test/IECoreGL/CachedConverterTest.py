##########################################################################
#
#  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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
import threading
import imath

import IECore
import IECoreScene
import IECoreImage
import IECoreGL

IECoreGL.init( False )

class CachedConverterTest( unittest.TestCase ) :

	def test( self ) :

		c = IECoreGL.CachedConverter( 500 * 1024 * 1024 ) # 500 megs

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( -1 ) ) )
		gm = c.convert( m )

		m2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( -1 ) ) )
		gm2 = c.convert( m2 )

		self.assertTrue( gm.isSame( gm2 ) )

		m3 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( -1 ) ) )
		m3["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.IntData( 1 ) )

		gm3 = c.convert( m3 )

		self.assertFalse( gm.isSame( gm3 ) )

	def testDefaultInstance( self ) :

		c1 = IECoreGL.CachedConverter.defaultCachedConverter()
		c2 = IECoreGL.CachedConverter.defaultCachedConverter()

		self.assertTrue( c1.isSame( c2 ) )

	def testThreading( self ) :

		r = imath.Rand32()
		pv = IECore.V3fVectorData()
		for i in range( 0, 10000 ) :
			pv.append( imath.V3f( r.nextf(), r.nextf(), r.nextf() ) )

		p = IECoreScene.PointsPrimitive( pv.copy() )

		pv.append( imath.V3f( r.nextf(), r.nextf(), r.nextf() ) )
		p2 = IECoreScene.PointsPrimitive( pv.copy() )

		self.assertNotEqual( p, p2 )

		for i in range( 0, 100 ) :

			pg = []
			pg2 = []

			c = IECoreGL.CachedConverter( 5000 * 1024 * 1024 ) # 500 megs

			def t() :

				pg.append( c.convert( p ) )
				pg2.append( c.convert( p2 ) )

			threads = []
			for j in range( 0, 100 ) :
				thread = threading.Thread( target = t )
				threads.append( thread )
				thread.start()

			for thread in threads :
				thread.join()

			for o in pg :
				self.assertTrue( o.isSame( c.convert( p ) ) )
			for o in pg2 :
				self.assertTrue( o.isSame( c.convert( p2 ) ) )

	def testThrashing( self ) :

		r = imath.Rand32()
		pv = IECore.V3fVectorData()
		for i in range( 0, 10000 ) :
			pv.append( imath.V3f( r.nextf(), r.nextf(), r.nextf() ) )

		p = IECoreScene.PointsPrimitive( pv.copy() )

		pv.append( imath.V3f( r.nextf(), r.nextf(), r.nextf() ) )
		p2 = IECoreScene.PointsPrimitive( pv.copy() )

		self.assertNotEqual( p, p2 )

		for i in range( 0, 100 ) :

			c = IECoreGL.CachedConverter( int( p.memoryUsage() * 1.5 ) ) # not enough for both objects

			def t() :

				c.convert( p )
				c.convert( p2 )

			threads = []
			for j in range( 0, 100 ) :
				thread = threading.Thread( target = t )
				threads.append( thread )
				thread.start()

			for thread in threads :
				thread.join()

	def testThrashingAndThreadingWithTextures( self ) :

		dataWindow = imath.Box2i( imath.V2i( 0 ), imath.V2i( 15 ) )

		i1 = IECoreImage.ImagePrimitive.createRGBFloat( imath.Color3f( 1, 0.5, 0.25 ), dataWindow, dataWindow )
		i2 = IECoreImage.ImagePrimitive.createRGBFloat( imath.Color3f( 0.75, 0.65, 0.55 ), dataWindow, dataWindow )
		p = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 0 ) ] * 10 ) )

		for i in range( 0, 1 ) :

			c = IECoreGL.CachedConverter( i1.memoryUsage() + i2.memoryUsage() ) # not enough for both the textures and the points

			# convert the textures. this is ok because we're doing it on the thread with the gl context.
			c.convert( i1 )
			c.convert( i2 )

			# convert the points on a different thread. if this removed the textures from the cache and destroyed them
			# immediately, we'd be destroying the gl resources on a different thread than the one used to create them,
			# which would explode.
			def t() :

				c.convert( p )

			thread = threading.Thread( target = t )
			thread.start()
			thread.join()

			# do the deferred removals now we're back on the main thread
			c.clearUnused()

	def testThrashingWithConcurrentClearUnused( self ) :

		r = imath.Rand32()
		pv = IECore.V3fVectorData()
		for i in range( 0, 10000 ) :
			pv.append( imath.V3f( r.nextf(), r.nextf(), r.nextf() ) )

		p = IECoreScene.PointsPrimitive( pv.copy() )

		pv.append( imath.V3f( r.nextf(), r.nextf(), r.nextf() ) )
		p2 = IECoreScene.PointsPrimitive( pv.copy() )

		self.assertNotEqual( p, p2 )

		for i in range( 0, 100 ) :

			c = IECoreGL.CachedConverter( int( p.memoryUsage() * 1.5 ) ) # not enough for both objects

			def t() :

				c.convert( p )
				c.convert( p2 )

			threads = []
			for j in range( 0, 100 ) :
				thread = threading.Thread( target = t )
				threads.append( thread )
				thread.start()

			for k in range( 0, 100 ) :
				c.clearUnused()

			for thread in threads :
				thread.join()

if __name__ == "__main__":
    unittest.main()
