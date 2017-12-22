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

import unittest
import os.path
import shutil
import imath

import IECore
import IECoreImage
import IECoreGL

IECoreGL.init( False )

class TestImmediateRenderer( unittest.TestCase ) :

	def test( self ) :

		outputFileName = os.path.dirname( __file__ ) + "/output/testImmediate.tif"

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "perspective" ),
				"projection:fov" : IECore.FloatData( 45 ),
				"resolution" : IECore.V2iData( imath.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( imath.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( imath.Box2f( imath.V2f( -0.5 ), imath.V2f( 0.5 ) ) )
			}
		)
		r.display( outputFileName, "tif", "rgba", {} )

		r.worldBegin()

		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -5 ) ) )
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 0, 0, 1 ) ) } )
		r.sphere( 1, -1, 1, 360, {} )

		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 1, 0 ) ) )
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 1, 0 ) ) } )
		r.sphere( 1, -1, 1, 360, {} )
		r.worldEnd()

		i = IECore.Reader.create( outputFileName ).read()
		dimensions = i.dataWindow.size() + imath.V2i( 1 )
		index = int(dimensions.x * 0.5)
		self.assertEqual( i["A"][index], 1 )
		self.assertEqual( i["R"][index], 1 )
		self.assertEqual( i["G"][index], 1 )
		self.assertEqual( i["B"][index], 0 )
		index = dimensions.x * int(dimensions.y * 0.5) + int(dimensions.x * 0.5)
		self.assertEqual( i["A"][index], 1 )
		self.assertEqual( i["R"][index], 0 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 1 )
		index = 0
		self.assertEqual( i["A"][index], 0 )
		self.assertEqual( i["R"][index], 0 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 0 )

	def setUp( self ) :

		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )

	def tearDown( self ) :

		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )

if __name__ == "__main__":
    unittest.main()
