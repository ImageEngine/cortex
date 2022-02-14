##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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
import imath
import IECore
import IECoreScene
import IECoreImage
import IECoreGL
IECoreGL.init( False )
import os.path
import os
import shutil

class CameraTest( unittest.TestCase ) :

	def testPositioning( self ) :

		# render a plane at z = 0 with the default camera
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.join( os.path.dirname( __file__ ), "shaders" ) ) )
		r.display( os.path.join( os.path.dirname( __file__ ), "output", "testCamera.tif" ), "tiff", "rgba", {} )

		r.camera( "main", { "resolution" : IECore.V2iData( imath.V2i( 512 ) ), "projection" : IECore.StringData( "perspective" ) } )

		r.worldBegin()
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )
		IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -0.1 ), imath.V2f( 0.1 ) ) ).render( r )
		r.worldEnd()

		# check that nothing appears in the output image
		i = IECore.Reader.create( os.path.join( os.path.dirname( __file__ ), "output", "testCamera.tif" ) ).read()
		dimensions = i.dataWindow.size() + imath.V2i( 1 )
		midpoint = dimensions.x * dimensions.y//2 + dimensions.x//2
		self.assertEqual( i["G"][midpoint], 0 )

		# render a plane at z = 0 with the camera moved back a touch to see it
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.join( os.path.dirname( __file__ ), "shaders" ) ) )
		r.display( os.path.join( os.path.dirname( __file__ ), "output", "testCamera.tif" ), "tiff", "rgba", {} )

		r.transformBegin()
		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, 1 ) ) )
		r.camera( "main", { "resolution" : IECore.V2iData( imath.V2i( 512 ) ), "projection" : IECore.StringData( "perspective" ) } )
		r.transformEnd()

		r.worldBegin()

		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )
		IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -0.1 ), imath.V2f( 0.1 ) ) ).render( r )
		r.worldEnd()

		# check that something appears in the output image
		i = IECore.Reader.create( os.path.join( os.path.dirname( __file__ ), "output", "testCamera.tif" ) ).read()
		dimensions = i.dataWindow.size() + imath.V2i( 1 )
		midpoint = dimensions.x * dimensions.y//2 + dimensions.x//2
		self.assertEqual( i["A"][midpoint], 1 )

	def testXYOrientation( self ) :

		# render a red square at x==1, and a green one at y==1

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.join( os.path.dirname( __file__ ), "shaders" ) ) )
		r.display( os.path.join( os.path.dirname( __file__ ), "output", "testCamera.tif" ), "tiff", "rgba", {} )
		r.transformBegin()
		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, 1 ) ) )
		r.camera( "main", { "resolution" : IECore.V2iData( imath.V2i( 512 ) ) } )
		r.transformEnd()

		r.worldBegin()
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )
		IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0.75, -0.25 ), imath.V2f( 1.25, 0.25 ) ) ).render( r )
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 0, 1, 0 ) ) } )
		IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -0.25, 0.75 ), imath.V2f( 0.25, 1.25 ) ) ).render( r )
		r.worldEnd()

		# check we get the colors we'd expect where we expect them
		i = IECore.Reader.create( os.path.join( os.path.dirname( __file__ ), "output", "testCamera.tif" ) ).read()
		dimensions = i.dataWindow.size() + imath.V2i( 1 )
		index = dimensions.x * dimensions.y//2 + dimensions.x - 1
		self.assertEqual( i["A"][index], 1 )
		self.assertAlmostEqual( i["R"][index], 1, 6 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 0 )
		index = dimensions.x//2
		self.assertEqual( i["A"][index], 1 )
		self.assertEqual( i["R"][index], 0 )
		self.assertAlmostEqual( i["G"][index], 1, 6 )
		self.assertEqual( i["B"][index], 0 )

	def setUp( self ) :

		if not os.path.isdir( os.path.join( "test", "IECoreGL", "output" ) ) :
			os.makedirs( os.path.join( "test", "IECoreGL", "output" ) )

	def tearDown( self ) :

		if os.path.isdir( os.path.join( "test", "IECoreGL", "output" ) ) :
			shutil.rmtree( os.path.join( "test", "IECoreGL", "output" ) )

if __name__ == "__main__":
    unittest.main()
