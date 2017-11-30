##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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
import os.path

import IECore
import IECoreScene

class TestCamera( unittest.TestCase ) :

	def test( self ) :

		c = IECoreScene.Camera()
		self.assertEqual( c.getName(), "default" )
		self.assertEqual( c.getTransform(), None )
		self.assertEqual( c.parameters(), IECore.CompoundData() )

		cc = c.copy()
		self.assertEqual( cc.getName(), "default" )
		self.assertEqual( cc.getTransform(), None )
		self.assertEqual( cc.parameters(), IECore.CompoundData() )
		self.assertEqual( cc, c )

		IECore.Writer.create( cc, "test/IECore/data/camera.cob" ).write()
		ccc = IECore.Reader.create( "test/IECore/data/camera.cob" ).read()

		self.assertEqual( c, ccc )

		c.setName( "n" )
		self.assertEqual( c.getName(), "n" )

		c.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createScaled( IECore.V3f( 2 ) ) ) )
		self.assertEqual( c.getTransform(), IECoreScene.MatrixTransform( IECore.M44f.createScaled( IECore.V3f( 2 ) ) ) )

		c.parameters()["fov"] = IECore.FloatData( 45 )
		self.assertEqual( c.parameters()["fov"], IECore.FloatData( 45 ) )

		# test copying and saving with some parameters and a transform
		cc = c.copy()
		self.assertEqual( cc, c )

		IECore.Writer.create( cc, "test/IECore/data/camera.cob" ).write()
		ccc = IECore.Reader.create( "test/IECore/data/camera.cob" ).read()
		self.assertEqual( ccc, c )

	def testAddStandardParameters( self ) :

		c = IECoreScene.Camera()
		c.addStandardParameters()

		self.assertEqual( c.parameters()["resolution"].value, IECore.V2i( 640, 480 ) )
		self.assertEqual( c.parameters()["pixelAspectRatio"].value, 1.0 )
		aspectRatio = 640.0/480.0
		self.assertEqual( c.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -aspectRatio, -1 ), IECore.V2f( aspectRatio, 1 ) ) )

		self.assertEqual( c.parameters()["cropWindow"].value, IECore.Box2f( IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ) ) )
		self.assertEqual( c.parameters()["projection"].value, "orthographic" )
		self.assertEqual( c.parameters()["clippingPlanes"].value, IECore.V2f( 0.01, 100000 ) )
		self.assertEqual( c.parameters()["shutter"].value, IECore.V2f( 0 ) )

		c = IECoreScene.Camera()
		c.parameters()["projection"] = IECore.StringData( "perspective" )
		c.parameters()["resolution"] = IECore.V2iData( IECore.V2i( 500, 1000 ) )
		c.parameters()["cropWindow"] = IECore.Box2fData( IECore.Box2f( IECore.V2f( 0.1 ), IECore.V2f( 0.9 ) ) )
		c.parameters()["clippingPlanes"] = IECore.V2fData( IECore.V2f( 1, 1000 ) )
		c.parameters()["shutter"] = IECore.V2fData( IECore.V2f( 1, 2 ) )
		c.addStandardParameters()
		self.assertEqual( c.parameters()["resolution"].value, IECore.V2i( 500, 1000 ) )
		self.assertEqual( c.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -1, -2 ), IECore.V2f( 1, 2 ) ) )
		self.assertEqual( c.parameters()["cropWindow"].value, IECore.Box2f( IECore.V2f( 0.1 ), IECore.V2f( 0.9 ) ) )
		self.assertEqual( c.parameters()["projection"].value, "perspective" )
		self.assertEqual( c.parameters()["projection:fov"].value, 90 )
		self.assertEqual( c.parameters()["clippingPlanes"].value, IECore.V2f( 1, 1000 ) )
		self.assertEqual( c.parameters()["shutter"].value, IECore.V2f( 1, 2 ) )

		# Negative clip planes on ortho cameras should be supported
		c = IECoreScene.Camera()
		c.parameters()["clippingPlanes"] = IECore.V2fData( IECore.V2f( -1000, 1000 ) )
		c.addStandardParameters()
		self.assertEqual( c.parameters()["clippingPlanes"].value, IECore.V2f( -1000, 1000 ) )

	def testHash( self ) :

		c = IECoreScene.Camera()
		h = c.hash()

		c.setName( "summink" )
		self.assertNotEqual( c.hash(), h )
		h = c.hash()

		c.setTransform( IECoreScene.MatrixTransform( IECore.M44f() ) )
		self.assertNotEqual( c.hash(), h )
		h = c.hash()

		c.addStandardParameters()
		self.assertNotEqual( c.hash(), h )

	def testAddStandardParametersWithNonSquarePixels( self ) :

		c = IECoreScene.Camera()
		c.parameters()["resolution"] = IECore.V2i( 100, 200 )
		c.parameters()["pixelAspectRatio"] = 2.0

		c.addStandardParameters()
		self.assertEqual( c.parameters()["screenWindow"].value, IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )

	def tearDown( self ) :

		if os.path.isfile( "test/IECore/data/camera.cob" ) :
			os.remove( "test/IECore/data/camera.cob" )

if __name__ == "__main__":
        unittest.main()
