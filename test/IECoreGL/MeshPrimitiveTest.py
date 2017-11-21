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
import os
import shutil

import IECore
import IECoreScene
import IECoreImage
import IECoreGL

IECoreGL.init( False )

class MeshPrimitiveTest( unittest.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/testMesh.tif"

	def testVertexAttributes( self ) :

		vertexSource = """
		#include "IECoreGL/VertexShader.h"

		IECOREGL_VERTEXSHADER_IN vec3 vertexP;
		IECOREGL_VERTEXSHADER_IN vec2 vertexuv;
		IECOREGL_VERTEXSHADER_OUT vec4 color;

		void main()
		{
			vec4 pCam = gl_ModelViewMatrix * vec4( vertexP, 1 );
			gl_Position = gl_ProjectionMatrix * pCam;
			// Note that we're only flipping V here because the expected
			// output image was generated with the wrong texture coordinates.
			// It is _not_ expected that you would need to modify texture
			// coordinates in the general case.
			color = vec4(vertexuv.x, 1.0 - vertexuv.y, 0.0, 1.0);
		}
		"""

		fragmentSource = """
		varying vec4 color;

		void main()
		{
			gl_FragColor = color;
		}
		"""

		m = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob").read()

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -15 ) ) )
			r.shader( "surface", "showUV",
				{ "gl:fragmentSource" : IECore.StringData( fragmentSource ),
				  "gl:vertexSource" : IECore.StringData( vertexSource   )
				}
			)

			m.render( r )

		reader = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/meshST.tif" )
		reader["rawChannels"].setTypedValue( True )
		expectedImage = reader.read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()

		self.assertEqual( IECoreImage.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.05 ).value, False )

	def testUniformCs( self ) :

		fragmentSource = """
		#include "IECoreGL/FragmentShader.h"

		IECOREGL_FRAGMENTSHADER_IN vec3 fragmentCs;

		void main()
		{
			gl_FragColor = vec4( fragmentCs, 1.0 );
		}
		"""

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -15 ) ) )

			r.shader( "surface", "test", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )

			m = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ), IECore.V2i( 2 ) )
			m["Cs"] = IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Uniform,
				IECore.Color3fVectorData( [
					IECore.Color3f( 1, 0, 0 ),
					IECore.Color3f( 0, 1, 0 ),
					IECore.Color3f( 0, 0, 1 ),
					IECore.Color3f( 1, 1, 1, ),
				] )
			)

			m.render( r )

		image = IECore.Reader.create( self.outputFileName ).read()
		dimensions = image.dataWindow.size() + IECore.V2i( 1 )
		index = dimensions.x * int(dimensions.y * 0.75) + int(dimensions.x * 0.25)
		self.assertEqual( image["R"][index], 1 )
		self.assertEqual( image["G"][index], 0 )
		self.assertEqual( image["B"][index], 0 )

		index = dimensions.x * int(dimensions.y * 0.75) + int(dimensions.x * 0.75)
		self.assertEqual( image["R"][index], 0 )
		self.assertEqual( image["G"][index], 1 )
		self.assertEqual( image["B"][index], 0 )

		index = dimensions.x * int(dimensions.y * 0.25) + int(dimensions.x * 0.75)
		self.assertEqual( image["R"][index], 1 )
		self.assertEqual( image["G"][index], 1 )
		self.assertEqual( image["B"][index], 1 )

		index = dimensions.x * int(dimensions.y * 0.25) + int(dimensions.x * 0.25)
		self.assertEqual( image["R"][index], 0 )
		self.assertEqual( image["G"][index], 0 )
		self.assertEqual( image["B"][index], 1 )

	def testBound( self ) :

		m = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.5 ), IECore.V2f( 0.5 ) ) )
		m2 = IECoreGL.ToGLMeshConverter( m ).convert()

		self.assertEqual( m.bound(), m2.bound() )

	def testFaceNormals( self ) :

		# when a polygon mesh has no normals, we must calculate face normals so we can
		# shade it in a faceted manner.

		fragmentSource = """
		#include "IECoreGL/FragmentShader.h"
		IECOREGL_FRAGMENTSHADER_IN vec3 fragmentN;

 		void main()
 		{
 			gl_FragColor = vec4( fragmentN, 1.0 );
 		}
		"""

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -15 ) ) )

			r.shader( "surface", "test", { "gl:fragmentSource" : IECore.StringData( fragmentSource ) } )

			m = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.5 ), IECore.V2f( 0.5 ) ) )
			self.assertTrue( "N" not in m )
			m.render( r )

		image = IECore.Reader.create( self.outputFileName ).read()
		dimensions = image.dataWindow.size() + IECore.V2i( 1 )
		index = dimensions.x * dimensions.y/2 + dimensions.x/2
		self.assertEqual( image["R"][index], 0 )
		self.assertEqual( image["G"][index], 0 )
		self.assertEqual( image["B"][index], 1 )

	def setUp( self ) :

		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )

	def tearDown( self ) :

		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )

if __name__ == "__main__":
    unittest.main()
